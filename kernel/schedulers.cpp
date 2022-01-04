//
// Created by os on 1/3/22.
//
#include "schedulers.h"
#include "proc.h"

// Default scheduling algo for the kernel is set to be SJF
unsigned current_algorithm = 0;

// Value that determines if SJF scheduling algorithm will
// behave preemptively or not
unsigned preemptive_sjf;

// Constant used for exponential weighted averaging in SJF.
int alfa;

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
struct proc* get_sjf() {
    return 0;
}

// Return process to RUNNABLE state, taking care
// of the SJF logic and arithmetic calculations.
void put_sjf(struct proc* proc) {

}

// CFS
struct proc* get_cfs() {
    return 0;
}

// CFS
void put_cfs(struct proc* proc) {

}
