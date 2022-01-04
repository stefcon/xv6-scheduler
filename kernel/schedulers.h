//
// Created by os on 1/3/22.
//
#ifndef XV6_OS2_SCHEDULERS_H
#define XV6_OS2_SCHEDULERS_H
struct proc;
struct spinlock;

// Min-heap struct and functions
struct heapnd {
    int pr;             // Value that we are prioritising by
    struct proc* p;     // Pointer to PCB
};

void swap(struct heapnd* a, struct heapnd* b);
void heapify(struct heapnd array[], int size, int i);
void insert(struct heapnd array[], int* size, struct proc* p, int pr);
void delete_root(struct heapnd array[], int* size);

// Structure used for different kinds of scheduling
struct scheduling_queues {
    struct spinlock heap_lock;  // Mutual exclusion for accessing local heap
    int procnums[NCPU];         // Num of processes inside of queues
    struct heapnd heaps[NCPU][NPROC];  // Local queue for cpu processes
};

extern struct scheduling_queues sched_queues;

extern unsigned current_algorithm;

extern unsigned preemptive_sjf;
extern int alfa;

void put(struct proc* proc);
struct proc* get();

struct proc* get_sjf();
void put_sjf(struct proc* proc);

struct proc* get_cfs();
void put_cfs(struct proc* proc);
#endif //XV6_OS2_SCHEDULERS_H
