#ifndef PTABLE_H_
#define PTABLE_H_

struct first_ptable {
    struct second_ptable **entries; // array of pointers to secondlvl tables
}

struct second_ptable {
    struct third_ptable **entries; // array of pointers to third level tables
}

struct third_ptable {
    paddr_t *entries; // array of physical addresses
}

struct first_ptable *init_first_ptable();
struct ptable *init_second_ptable(void);
struct second_ptable *copy_second_ptable(struct second_ptable *);
struct third_ptable *init_third_ptable();
struct third_ptable *copy_third_ptable(struct third_ptable *);
paddr_t* ptable_lookup(vaddr_t address, struct pagetable** tableref);

#define FIRST_PAGE_LIMIT 256
#define SECOND_PAGE_LIMIT 64
#define THIRD_PAGE_LIMIT 64