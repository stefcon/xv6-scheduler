// Min-heap struct and functions
struct heapnd {
    int pr;             // Value that we are prioritising by
    struct proc* p;     // Pointer to PCB
};

// Structure used for different kinds of scheduling
struct scheduling_queues {
    struct spinlock heap_lock;  // Mutual exclusion for accessing local heap
    int procnums[NCPU];         // Num of processes inside of queues
    struct heapnd heaps[NCPU][NPROC];  // Local queue for cpu processes
    int max_size, min_size, max_ind, min_ind;   // Used for load balancing more efficiently
};

extern struct scheduling_queues sched_queues;
