#ifndef _PAGETABLE_H_
#define _PAGETABLE_H_

// Pagetable entry limits:
#define FIRST_PTABLE_LIMIT 256
#define SECOND_PTABLE_LIMIT 64
#define THIRD_PTABLE_LIMIT 64

#include <spinlock.h>

struct first_ptable {
    struct second_ptable **entries; // array of pointers to secondlvl tables
    struct spinlock lock;
};

struct second_ptable {
    struct third_ptable **entries; // array of pointers to third level tables
};

struct third_ptable {
    paddr_t *entries; // array of paddr_t entries
};

struct first_ptable *init_first_ptable(void);
struct second_ptable *create_second_ptable(void);
struct third_ptable *create_third_ptable(void);

struct third_ptable *copy_third_ptable(struct third_ptable *old);
paddr_t get_pt_frame(vaddr_t addr);


#endif /* _PAGETABLE_H_ */
