#include <ptable.h>
#include <vm.h>
#include <lib.h>
#include <addrspace.h>
#include <proc.h>
#include <current.h>

 // returns a first level page table initialised to 0 
struct first_ptable *init_first_ptable()
{
    struct first_ptable *first_ptable;
    first_ptable = kmalloc(sizeof(struct first_ptable));

    // initialise pointers to second_ptable
    first_ptable->entries = kmalloc(FIRST_PAGE_LIMIT * sizeof(struct second_ptable *);
    bzero(first_ptable->entries, FIRST_PAGE_LIMIT * sizeof(struct second_ptable *));


    return first_ptable;
}

// returns a new second level page table initialised to 0
struct second_ptable *init_second_ptable() {
    struct second_ptable *second_ptable;
    
    // allocating and initialising pointers to third level page table as 0
    second_ptable->entries = kmalloc(SECOND_PAGE_LIMIT * sizeof(struct third_ptable *));
    bzero(second_ptable->entries, SECOND_PAGE_LIMIT * sizeof(struct third_ptable *));

    return second_ptable;
}

// returns a copy of a second level page table
struct second_ptable *copy_second_ptable(struct second_ptable *original_ptable) {
    if (original_ptable == NULL) return NULL;

    // Create new pagetable
    struct second_ptable *ptable = kmalloc(sizeof(struct second_ptable));
    if (ptable == NULL) return NULL;

    // initialise new pagetable
    ptable->entries = kmalloc(SECOND_PAGE_LIMIT * sizeof(paddr_t));
    bzero(ptable->entries, SECOND_PAGE_LIMIT * sizeof(paddr_t));

    // copy over the page table pointers to third level tables
    for (int i = 0; i < SECOND_PAGE_LIMIT; i++) {
        if (original_ptable->entries[i] != (struct third_ptable) NULL) {
            ptable->entries[i] = original_ptable->entries[i];
        }
    }
    return ptable;
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
