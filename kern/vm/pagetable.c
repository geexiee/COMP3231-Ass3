#include <pagetable.h>
#include <vm.h>
#include <lib.h>
#include <addrspace.h>
#include <proc.h>
#include <current.h>

// create first level page table initialised to 0
struct first_ptable *init_first_ptable()
{
   struct first_ptable *first_ptable = kmalloc(sizeof(struct first_ptable));
   if(first_ptable == NULL) {
       return NULL; //deal with ENOMEM in call function
   }
   // initialise pointers to second_ptable
   first_ptable->entries = kmalloc(FIRST_PTABLE_LIMIT * sizeof(struct second_ptable *);
   if(first_ptable->entries == NULL) {
       kfree(first_ptable);
       return NULL; //deal with ENOMEM in call function
   }
   // initialise all bits to zero
   bzero(first_ptable->entries, FIRST_PTABLE_LIMIT * sizeof(struct second_ptable *));
   spinlock_init(&first_ptable->lock);

   return first_ptable;
}

// Creates a new second level page table initialised to 0
struct second_ptable *create_second_ptable() {
   struct second_ptable *second_ptable;
   second_ptable = kmalloc(sizeof(struct second_ptable));
   if(second_ptable == NULL) {
       return NULL; // deal with ENOMEM in call function
   }
   second_ptable->entries = kmalloc(SECOND_PTABLE_LIMIT * sizeof(struct third_ptable *));
   if(second_ptable->entries == NULL) {
       kfree(second_ptable);
       return NULL;
   }
   bzero(second_ptable->entries, SECOND_PTABLE_LIMIT * sizeof(third_ptable *));
   return second_ptable;
}

struct third_ptable *create_third_ptable() {
    struct third_ptable *third_ptable;
    third_ptable = kmalloc(sizeof(struct third_ptable));
    if(third_ptable == NULL) {
        return NULL;
    }

    third_ptable->entries = kmalloc(THIRD_PTABLE_LIMIT * sizeof(paddr_t));
    if(third_ptable->entries == NULL) {
        kfree(third_ptable);
        return NULL;
    }
    bzero(third_ptable->entries, THIRD_PTABLE_LIMIT * sizeof(paddr_t));

    return third_ptable;
}

struct third_ptable *copy_third_ptable(struct third_ptable *old) {

    if (old == NULL) {
        return NULL;
    }
    struct third_ptable *new = kmalloc(sizeof(struct third_ptable))
    if(new == NULL) {
        return NULL; //deal with ENOMEM in call fucntion
    }
    new->entries = (paddr_t *) kmalloc(THIRD_PTABLE_LIMIT * sizeof(paddr_t));
    if(new->entries == NULL) {
        kfree(new);
        return NULL;
    }
    // intialise to 0
    bzero(new->entries, THIRD_PTABLE_LIMIT * sizeof(paddr_t));

    // duplicate old into new
    int i = 0;
    while(i < THIRD_PTABLE_LIMIT) {
        if(old->entries[i] != (paddr_t) NULL) {
            vaddr_t frame_alloc = alloc_kpages(1);
            memcpy((void*) frame_alloc, (void*) PADDR_TO_KVADDR(old->entries[i]), PAGE_SIZE);
            new->entries[i] = KVADDR_TO_PADDR(frame_alloc); // return new PADDR_T
        }
        i++;
    }
    return new;
}

// get physical address from virtual address
paddr_t get_pt_frame(vaddr_t addr) {
    struct addrspace *as = proc_getas();
    struct first_ptable *first_ptable;
    struct second_ptable **second_ptable;
    struct third_ptable **third_ptable;
    first_ptable = as->first_ptable;

    // remove 12bit offset, we only want the 8-6-6 bits. & respective bits
    int page = address >> 12;
    int first_level = (page >> 12) & 0xFF; // first 8 bits
    int second_level = (page >> 6) & 0x3F; // second 6 bits
    int third_level = (page) & 0x3F; // third 6 bits

    //lock
    spinlock_acquire(&(first_ptable->lock));

    second_ptable = &first_ptable->entries[first_level];
    // if second level page table is NULL, create lazily
    if(*second_ptable == NULL) {
        *second_ptable = create_second_ptable();
        KASSERT(*second_ptable != NULL);
    }

    third_ptable = &second_ptable->entries[second_level];
    // if third level page table is NULL, create lazily
    if(*third_ptable == NULL) {
        *third_ptable = create_third_ptable();
        KASSERT(*third_ptable != NULL);
    }
    //release lock
    spinlock_acquire(&(first_ptable->lock));

    // return physical frame.
    // Return null if frame not allocated
    return &((*third_ptable->entries[third_ptable]));

}

// returns a new third level page table initialised to 0
struct third_ptable *init_third_ptable() {
    struct third_ptable *third_ptable;
    
    // allocating and initialising pointers to physical addresses as 0
    third_ptable->entries = kmalloc(THIRD_PAGE_LIMIT * sizeof(paddr_t));
    bzero(third_ptable->entries, THIRD_PAGE_LIMIT * sizeof(paddr_t));

    return third_ptable;
}

struct third_ptable *copy_third_ptable(struct original_third_ptable *) {
    struct third_ptable *third_ptable_copy;

    // initialise new third_pagetable
    third_ptable_copy->entries = kmalloc(THIRD_PAGE_LIMIT * sizeof(paddr_t));
    bzero(third_ptable_copy->entries, THIRD_PAGE_LIMIT * sizeof(paddr_t));

    // copy over the paddr ptrs to the third level table copy
    for (int i = 0; i < THIRD_PAGE_LIMIT; i++) {
        if (original_third_ptable->entries[i] != (struct third_ptable) NULL) {
            third_ptable_copy->entries[i] = original_third_ptable->entries[i];
            vaddr_t new_frame = alloc_kpages(1);
            memcpy((void*) new_frame, (void*) PADDR_TO_KVADDR(original_third_ptable->entries[i]), THIRD_PAGE_LIMIT);
            third_ptable_copy->entries[i] = KVADDR_TO_PADDR(new_frame);
        }
    }
}

paddr_t* ptable_lookup(vaddr_t address, struct pagetable** first_ptable) {
    struct first_ptable *first_ptable = proc_getas()->pagetable; // 
}
