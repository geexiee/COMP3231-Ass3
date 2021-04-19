#include <ptable.h>
#include <vm.h>
#include <lib.h>
#include <addrspace.h>
#include <proc.h>
#include <current.h>
#include <spinlock.h>

 // returns a first level page table initialised to 0 
struct first_ptable *init_first_ptable()
{
    struct first_ptable *first_ptable;
    first_ptable = kmalloc(sizeof(struct first_ptable));

    // initialise pointers to second_ptable
    first_ptable->entries = kmalloc(PAGE_ENTRY_LIMIT * sizeof(struct second_ptable *);
    bzero(first_ptable->entries, PAGE_ENTRY_LIMIT * sizeof(struct second_ptable *));

    spinlock_init(&first_ptable->lock);

    return first_ptable;
}

// returns a new second level page table initialised to 0
struct second_ptable *create_second_ptable() {
    struct second_ptable *second_ptable;
    
    // allocating and initialising page entries as 0
    second_ptable->entries = kmalloc(PAGE_ENTRY_LIMIT * sizeof(paddr_t));
    bzero(second_ptable->entries, PAGE_ENTRY_LIMIT * sizeof(paddr_t));

    return second_ptable;
}

// returns a copy of a second level page table
struct second_ptable *copy_second_ptable(struct second_ptable *original_ptable) {
    if (original_ptable == NULL) return NULL;

    // Create new pagetable
    struct second_ptable *ptable = kmalloc(sizeof(struct second_ptable));
    if (ptable == NULL) return NULL;

    // initialise new pagetable
    ptable->entries = kmalloc(PAGE_ENTRY_LIMIT * sizeof(paddr_t));
    bzero(ptable->entries, PAGE_ENTRY_LIMIT * sizeof(paddr_t));

    // copy over the page table entries
    for (int i = 0; i < PAGE_ENTRY_LIMIT; i++) {
        if (original_ptable->entries[i] != (paddr_t) NULL) {
            vaddr_t new_frame = alloc_kpages(1);
            memcpy((void*) new_frame, (void*) PADDR_TO_KVADDR(original_ptable->entries[i]), PAGE_SIZE);
            ptable->entries[i] = KVADDR_TO_PADDR(new_frame);
        }
    }
    return ptable;
}
