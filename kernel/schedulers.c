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

void swap(struct heapnd* a, struct heapnd* b) {
    struct heapnd tmp = *b;
    *b = *a;
    *a = tmp;
}

void heapify(struct heapnd array[], int size, int i) {
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
            heapify(array, size, minimum);
        }
    }
}

void insert(struct heapnd array[], int* size, struct proc* p, int pr) {
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
            heapify(array, *size, i);
        }
    }
}

void delete_root(struct heapnd array[], int* size) {
    swap(&array[0], &array[*size-1]);
    (*size)--;
    for (int i = *size / 2 - 1; i >= 0; i--) {
        heapify(array, *size, i);
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

void put(struct proc* proc) {
    if (current_algorithm == 0) {
        put_sjf(proc);
    }
    else {
        put_cfs(proc);
    }
}

struct proc* get() {
    if (current_algorithm == 0) {
        return get_sjf();
    }
    else {
        return get_cfs();
    }
}


// Picks next process for execution based on SJF algorithm
// and its approximated CPU burst.
struct proc* get_sjf(){
    struct proc *p, *next_p = 0;
    uint min_tau = ~0U;
    for(p = proc; p < &proc[NPROC]; p++) {
        acquire(&p->lock);
        if (p->state == RUNNABLE &&p->tau < min_tau) {
            min_tau = p->tau;
            next_p = p;
        }
        release(&p->lock);
    }
    if (next_p != 0)
        acquire(&next_p->lock);
    return next_p;
}

// Return process to RUNNABLE state, taking care
// of the SJF logic and arithmetic calculations.
void put_sjf(struct proc* proc){
    uint curr_burst = calculate_length(proc->start_tick, ticks);
    proc->time += curr_burst;
    if (proc->state == SLEEPING) {
        // Process came out of suspension, new approximation has to be made
        proc->tau = (alfa * proc->time) / 100 + ((100 - alfa) * proc->tau) / 100;
        proc->time = 0;
    }
    proc->state = RUNNABLE;
}

// CFS
struct proc* get_cfs(){
    return 0;
}

// CFS
void put_cfs(struct proc* proc){

}

// --------------------------------------------------------------------

// Helper function used to calculate lengths of time between current tick
// and some saved starting tick
uint calculate_length(uint start, uint end) {
    if (start <= end) {
        return end - start + 1;
    }
    else {
        // Overflow occurred, so the result needs to be corrected
        return (~0U - start) + end + 1;
    }
}