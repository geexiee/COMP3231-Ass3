/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <spl.h>
#include <spinlock.h>
#include <current.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <vm.h>
#include <proc.h>

#include <pagetable.h>

/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 *
 * UNSW: If you use ASST3 config as required, then this file forms
 * part of the VM subsystem.
 *
 */

struct addrspace *
as_create(void)
{
 	struct addrspace *as;
 	as = kmalloc(sizeof(struct addrspace));
 	if (as == NULL) {
 		return NULL;
 	}

 	// Create first level page table "lazy" when needed > first process
 	as->first_ptable = init_first_ptable();
 	if( as->first_ptable == NULL ) {
 		kfree(as);
 		return NULL;
 	}

    // initialise head of list to NULL
    as->region_list = NULL;
 	return as;
}


int
as_copy(struct addrspace *old, struct addrspace **ret)
{
    // Create a new address space - calls as_create
    struct addrspace *new_as;
    if ((new_as = as_create()) == NULL) {
        return ENOMEM;
    }

    // pointer to the array of top level pagetable
    struct first_ptable *old_first_pt = old->first_ptable;
    struct first_ptable *new_first_pt = new_as->first_ptable;

    // array of pointers to second level pagetable
    struct second_ptable **old_second_pt = old_first_pt->entries;
    struct second_ptable **new_second_pt = new_first_pt->entries;

    // array of pointers to third level pagetable
    struct third_ptable **old_entries = (*old_second_pt)->entries;
    struct third_ptable **new_entries = (*new_second_pt)->entries;

    // set lock when replicating
    spinlock_acquire(&(new_first_pt->lock));

    int i = 0;
    int j = 0;
    while (i < FIRST_PTABLE_LIMIT) {
        //  second level replicate 
        if (old_second_pt[i] != NULL) {
			// create new array of second_level_ptable arrays
            new_second_pt[i] = create_second_ptable();

            if (new_second_pt[i] == NULL) {
                // release lock due to ENOMEM error
                spinlock_release(&(new_first_pt->lock));
                as_destroy(new_as);
                return ENOMEM;
            }

            j = 0;
            // loop through second level table
            old_entries = old_second_pt[i]->entries;
            new_entries = new_second_pt[i]->entries;

            // populate third level entries
            while(j < SECOND_PTABLE_LIMIT) {
                // copy non-NULL entries into second level page table
                if(old_entries[j] != NULL) {
                    new_entries[j] = copy_third_ptable(old_entries[j]);
                    if(new_entries[j] == NULL) {
                        // release lock as ENOMEM error
                        spinlock_release(&(new_first_pt->lock));
                        as_destroy(new_as);
                        return ENOMEM;
                    }
                }
                j++;
            }
			(*new_second_pt)->entries[i] = (*new_entries);
        }
        i++;
    }
    // release lock
    spinlock_release(&(new_first_pt->lock));

    // loop through and copy regions
    struct region *curr = old->region_list;
    while(curr != NULL) {
        as_define_region(new_as, curr->vaddr, curr->memsize, curr->readable, curr->writeable,curr->executable);
        curr = curr->next;
    }
    *ret = new_as;
    return 0;
}



void
as_destroy(struct addrspace *as)
{
    // loop through and free regions
    struct region *curr = as->region_list;
    struct region *next = curr;
	while (curr != NULL) {
		next = curr->next;
		kfree(curr);
		curr = next;
	}
	ptable_cleanup(as->first_ptable);
	kfree(as);
}


// taken from dumbvm.c
void
as_activate(void)
{
	struct addrspace *as;

	as = proc_getas();
	if (as == NULL) {
		return;
	}

    int s = splhigh();
  	for (int i = 0; i < NUM_TLB; i++) {
  		tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
  	}
	splx(s);
}

// taken from dumbvm.c
void
as_deactivate(void)
{
    // NUM_TLB given as 64 in /MIPS/include/tlb.h
    int s = splhigh();
 	for (int i = 0; i < NUM_TLB; i++) {
 		tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
 	}
 	splx(s);
}

// define region at vaddr of size memsize
// include permission flags for the segment
int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t memsize,
		 int readable, int writeable, int executable)
{
    struct region *region = kmalloc(sizeof(struct region));
    if (region == NULL) {
        return ENOMEM;
    }

    region->vaddr = vaddr;
    region->memsize = memsize;
    region->readable = readable != 0; 
    region->writeable = writeable != 0;
    region->executable = executable != 0;
    region->next = NULL;

    struct region *curr = as->region_list;
    struct region *end = curr;

    // check if region list is empty
    if (as->region_list != NULL) {
        while (curr != NULL) {
            if(curr->next == NULL) {
                // get end of list
                end = curr;
            }
            curr = curr->next;
        }
        // add region to end of list
        end->next = region;
    } else {
        as->region_list = region;
    }
    return 0;
}

// before load - mark all region as writeable for loading of content into addrspace.
int
as_prepare_load(struct addrspace *as)
{
	struct region *curr = as->region_list;
    while(curr != NULL) {
        curr->writeable = (curr->writeable << 1) | 1; // shift
        curr = curr->next;
    }

	return 0;
}

// before load - mark all region as read only after all content loaded onto addrspace.
int
as_complete_load(struct addrspace *as)
{
    struct region *curr = as->region_list;
    while(curr != NULL) {
		curr->writeable >>= 1;
		curr = curr->next;
	}
    return 0;
}


int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
    int errno = as_define_region(as, USERSTACK - FIXED_STACK_SIZE, FIXED_STACK_SIZE, 1, 1, 0);
    if (errno) {
        return errno;
    }
    // userlvl stack pointer
	*stackptr = USERSTACK;
	return 0;
}
