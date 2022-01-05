//
// Created by os on 1/3/22.
//
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "schedulers.h"
#include "proc.h"

struct scheduling_queues sched_queues;

void swap(struct heapnd* a, struct heapnd* b) {
    struct heapnd tmp = *b;
    *b = *a;
    *a = tmp;
}

void heapify_min(struct heapnd *array, int size, int i) {
    if (size == 1) {
        return;
    }
    else {
        int minimum = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        if (left < size && array[left].pr < array[minimum].pr)
            minimum = left;
        if (right < size && array[right].pr < array[minimum].pr)
            minimum = right;
        if (minimum != i) {
            swap(&array[i], &array[minimum]);
            heapify_min(array, size, minimum);
        }
    }
}

void heapify_max(struct heapnd *array, int size, int i) {
    if (size == 1) {
        return;
    }
    else {
        int maximum = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        if (left < size && array[left].pr > array[maximum].pr)
            maximum = left;
        if (right < size && array[right].pr > array[maximum].pr)
            maximum = right;
        if (maximum != i) {
            swap(&array[i], &array[maximum]);
            heapify_max(array, size, maximum);
        }
    }
}

void heapsort(struct heapnd *array, int n) {
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify_max(array, n, i);

    for (int i = n - 1; i > 0; i--) {
        swap(&array[0], &array[i]);

        heapify_max(array, i, 0);
    }
}

void insert(struct heapnd *array, int* size, struct proc* p, int pr) {
    if (*size == 0) {
        array[0].pr = pr;
        array[0].p = p;
        (*size)++;
    }
    else {
        array[*size].pr = pr;
        array[*size].p = p;
        (*size)++;
        for (int i = *size / 2 - 1; i >= 0; i--) {
            heapify_min(array, *size, i);
        }
    }
}

void delete_root(struct heapnd *array, int* size) {
    array[0].p = 0;

    swap(&array[0], &array[*size-1]);
    (*size)--;
    for (int i = *size / 2 - 1; i >= 0; i--) {
        heapify_min(array, *size, i);
    }
}

// --------------------------------------------------------------------

// Instance that is responsible for holding RUNNABLE processes
struct scheduling_queues sched_queues;

// Default scheduling algo for the kernel is set to be SJF
unsigned current_algorithm = 0;

// Value that determines if SJF scheduling algorithm will
// behave preemptively or not
unsigned preemptive_sjf = 0;

// Constant used for exponential weighted averaging in SJF.
int alfa = 50;

//struct scheduling_queues {
//    struct spinlock heap_lock;  // Mutual exclusion for accessing local heap
//    int procnums[NCPU];         // Num of processes inside of queues
//    struct heapnd heaps[NCPU][NPROC];  // Local queue for cpu processes
//};

void initsched(void) {
    // Initialize heap_lock for the scheduling struct
    initlock(&sched_queues.heap_lock, "heap_lock");
    // Initializing number of processes inside of each queue
    for (int i = 0; i < NCPU; i++) {
        sched_queues.procnums[i] = 0;
    }
}

void put(struct proc* proc){
    int curr_ticks = ticks;
    if (proc->affinity != -1) {
        //acquire(&tickslock);
        uint curr_burst = calculate_length(proc->start_tick, curr_ticks);
        //release(&tickslock);
        proc->time += curr_burst;
    }
    if (proc->state == SLEEPING) {
        // Process came out of suspension, new approximation for tau has to be made
        proc->tau = (alfa * proc->time) / 100 + ((100 - alfa) * proc->tau) / 100;
        proc->time = 0;
    }
    //acquire(&tickslock);
    proc->sched_tick = curr_ticks;   // Save the moment process arrived to scheduler
    //release(&tickslock);
    acquire(&sched_queues.heap_lock);
    if (proc->affinity == -1) {
        // Search for the proccesor with the smallest number of ready processes
        min_index(sched_queues.procnums, NCPU, &proc->affinity);
    }
    int priority;
    if (current_algorithm == 0) {
        // In SJF, we set tau to be the criteria of choosing next process
        priority = proc->tau;
    }
    else {
        // In CFS, we choose the process that spent the least time running on the CPU, since getting out
        // of suspended state
        priority = proc->time;
    }
    proc->state = RUNNABLE;
    insert(sched_queues.heaps[proc->affinity], &sched_queues.procnums[proc->affinity], proc, priority);
    release(&sched_queues.heap_lock);
}

struct proc* get(int cpu_id) {
    if (sched_queues.procnums[cpu_id] == 0) {
        // Maybe I will do some load balancing here as well, but first I'll have to see if everything else works
        return 0;
    }
    else {
        struct proc *next_p = sched_queues.heaps[cpu_id][0].p;
        acquire(&next_p->lock);

        delete_root(sched_queues.heaps[cpu_id], &sched_queues.procnums[cpu_id]);

        if (current_algorithm == 0) {
            // Unless we request preemptive SJF by using a system call, every process has unlimited CPU time
            next_p->timeslice = 0;
        }
        else {
            //acquire(&tickslock);
            int curr_ticks = ticks;
            //release(&tickslock);

            acquire(&active_lock);
            int curr_active = active_proc_num;
            release(&active_lock);

            next_p->timeslice = (int)(calculate_length(proc->sched_tick, curr_ticks) / curr_active);
        }
        return next_p;
    }
}

//// Picks next process for execution based on SJF algorithm
//// and its approximated CPU burst.
//struct proc* get_sjf(){
//    struct proc *p, *next_p = 0;
//    uint min_tau = ~0U;
//    for(p = proc; p < &proc[NPROC]; p++) {
//        acquire(&p->lock);
//        if (p->state == RUNNABLE &&p->tau < min_tau) {
//            min_tau = p->tau;
//            next_p = p;
//        }
//        release(&p->lock);
//    }
//    if (next_p != 0)
//        acquire(&next_p->lock);
//    return next_p;
//}
//
//// Return process to RUNNABLE state, taking care
//// of the SJF logic and arithmetic calculations.
//void put_sjf(struct proc* proc){
//    uint curr_burst = calculate_length(proc->start_tick, ticks);
//    proc->time += curr_burst;
//    if (proc->state == SLEEPING) {
//        // Process came out of suspension, new approximation has to be made
//        proc->tau = (alfa * proc->time) / 100 + ((100 - alfa) * proc->tau) / 100;
//        proc->time = 0;
//    }
//    proc->state = RUNNABLE;
//}
//
//// CFS
//struct proc* get_cfs(){
//    return 0;
//}
//
//// CFS
//void put_cfs(struct proc* proc){
//    proc->sched_tick = ticks;      // Save the moment process came into the scheduler
//    uint curr_burst = calculate_length(proc->start_tick, ticks);
//    proc->time += curr_burst;
//    if (proc->state == SLEEPING) {
//        proc->time = 0;
//    }
//}

// --------------------------------------------------------------------

// Helper function used to calculate lengths of time between current tick
// and some saved starting tick
uint calculate_length(uint start, uint end){
    if (start <= end) {
        return end - start + 1;
    }
    else {
        // Overflow occurred, so the result needs to be corrected
        return (~0U - start) + end + 1;
    }
}

int min_index(int *array, int n, int *min_ind){
    long min_value = ((long)1 << 31) - 1;
    *min_ind = 0;
    for (int i = 0; i < n; i++) {
        if (min_value > array[i]) {
            min_value = array[i];
            *min_ind = i;
        }
    }
    return (int)min_value;
}

// Potentially needed for load balancing if one processor has nothing to run
//int max_index(int *array, int n, int *max_ind){
//    long max_value = -(1 << 31);
//    *max_ind = 0;
//    for (int i = 0; i < n; i++) {
//        if (max_value < array[i]) {
//            max_value = array[i];
//            *max_ind = i;
//        }
//    }
//    return (int)max_value;
//}