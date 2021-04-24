#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/tlb.h>

#include <proc.h>
#include <spl.h>

/* Place your page table functions here */


void vm_bootstrap(void)
{
    /* Initialise any global components of your VM sub-system here.
     *
     * You may or may not need to add anything here depending what's
     * provided or required by the assignment spec.
     */
}

int
vm_fault(int faulttype, vaddr_t faultaddress)
{
    // check NULL address
    if( faultaddress == (vaddr_t) NULL) {
        return EFAULT;
    }
    // write to readonly region
    if (faulttype == VM_FAULT_READONLY) {
        return EFAULT;
    }

    // check faulttype
    // Check if read attempt
    else if (faulttype == VM_FAULT_READ || faulttype == VM_FAULT_WRITE) {
        //get current process address space, check if valid
        struct addrspace *addr = proc_getas();
        if (addr == NULL) {
            return EFAULT;
        }
        // check each region in list if fault address exists
        struct region *curr = addr->region_list;
        while(curr != NULL) {
            if( curr->vaddr <= faultaddress && faultaddress < (curr->vaddr + curr->memsize) ) {

                // get physical frame from pagetable entry
                paddr_t *frame = get_pt_frame(faultaddress);
                // If not in page table, allocate frame
                if( *frame == (paddr_t) NULL) {
                    *frame = KVADDR_TO_PADDR(alloc_kpages(1));
                }
                int s = splhigh();
                uint32_t entryhi = faultaddress & PAGE_FRAME;
                uint32_t entrylo = (*frame & PAGE_FRAME) | ((curr->writeable & 1) ? TLBLO_DIRTY : 0) | ((curr->readable || (curr->writeable & 1) || curr->executable) ? TLBLO_VALID : 0);
                // random pick a tlb slot and allocate entryhi and entrylo into tlb
                tlb_random(entryhi, entrylo);
                splx(s);
                return 0;
            }
            curr = curr->next;
        }
    // if address not found valid regions and not in pagetable
    }
    return EFAULT;
}

/*
 * SMP-specific functions.  Unused in our UNSW configuration.
 */

void
vm_tlbshootdown(const struct tlbshootdown *ts)
{
	(void)ts;
	panic("vm tried to do tlb shootdown?!\n");
}
