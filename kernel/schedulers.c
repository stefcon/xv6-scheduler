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

void create_min_heap(struct heapnd *array, int n) {
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify_min(array, n, i);
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

void initsched(void) {
    // Initializing number of processes inside each queue
    for (int i = 0; i < NCPU; i++) {
        sched_queues.procnums[i] = 0;
        sched_queues.cpu_misses[i] = 0;
    }
    // Initialize scheduler's spinlock
    initlock(&sched_queues.heap_lock, "heap_lock");
}

void aging(void) {
    // Decrease the priority of all processes that are currently in the scheduler.
    // The more they spend in the scheduler, the more their priority will decrease,
    // and eventually they will be picked for execution.
    acquire(&sched_queues.heap_lock);
    for (int i = 0; i < NCPU; i++) {
        for (int j = 0; j < sched_queues.procnums[i]; j++) {
            sched_queues.heaps[i][j].pr--;
        }
    }
    release(&sched_queues.heap_lock);
}

void put(struct proc* proc){
    acquire(&sched_queues.heap_lock);
    if (proc->affinity != -1) {
        proc->time += proc->curr_time;
    }
    if (proc->state == SLEEPING) {
        // Process came out of suspension, new approximation for tau has to be made
        proc->tau = (alfa * proc->time) / 100 + ((100 - alfa) * proc->tau) / 100;
        proc->time = 0;
    }

    int curr_ticks = ticks;
    proc->sched_tick = curr_ticks;   // Save the moment process arrived to scheduler

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
    if (proc->affinity == -1) {
        // Search for the processor with the smallest number of ready processes
        min_index(sched_queues.procnums, NCPU, &proc->affinity);
    }
    insert(sched_queues.heaps[proc->affinity], &sched_queues.procnums[proc->affinity], proc, priority);
    release(&sched_queues.heap_lock);
}

struct proc* get() {
    acquire(&sched_queues.heap_lock);
    struct proc *next_p;
    int cpu_id = cpuid();
    if (sched_queues.procnums[cpu_id] == 0) {
        if (++sched_queues.cpu_misses[cpu_id] >= 5) {
            sched_queues.cpu_misses[cpu_id] = 0;
            int max_id;
            max_index(sched_queues.procnums, NCPU, &max_id);
            if (sched_queues.procnums[max_id] <= 1){
                release(&sched_queues.heap_lock);
                return 0;
            }
            else {
                //acquire(&sched_queues.heaps[max_id][0].p->lock);
                next_p = sched_queues.heaps[max_id][0].p;
                next_p->affinity = cpu_id;
                delete_root(sched_queues.heaps[max_id], &sched_queues.procnums[max_id]);
            }
        }
        else {
            release(&sched_queues.heap_lock);
            return 0;
        }
    }
    else {
        //acquire(&sched_queues.heaps[cpu_id][0].p->lock);
        next_p = sched_queues.heaps[cpu_id][0].p;
        delete_root(sched_queues.heaps[cpu_id], &sched_queues.procnums[cpu_id]);
    }
    release(&sched_queues.heap_lock);
    acquire(&next_p->lock);

    if (current_algorithm == 0) {
        // Unless we request preemptive SJF by using a system call, every process has unlimited CPU time
        next_p->timeslice = 0;
    }
    else {
        acquire(&active_lock);
        int curr_active = active_proc_num;
        release(&active_lock);

        int curr_ticks = ticks;
        next_p->timeslice = (int)(calculate_length(proc->sched_tick, curr_ticks) / curr_active);
    }
    next_p->curr_time = 0;              // Will be used for measuring running time
    return next_p;
}

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
    long min_value = 2147483647;
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
int max_index(int *array, int n, int *max_ind){
    long max_value = -2147483648;
    *max_ind = 0;
    for (int i = 0; i < n; i++) {
        if (max_value < array[i]) {
            max_value = array[i];
            *max_ind = i;
        }
    }
    return (int)max_value;
}