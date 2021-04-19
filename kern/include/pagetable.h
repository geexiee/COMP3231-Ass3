#ifndef PTABLE_H_
#define PTABLE_H_

struct first_ptable {
    struct spinlock lock;
    struct second_ptable **entries; // array of pointers to secondlvl tables
}

struct second_ptable {
    paddr_t *entries; // array of third level tables
}

struct first_ptable *init_first_ptable();
struct ptable *create_second_ptable(void);
struct second_ptable *copy_second_ptable(struct second_ptable *);