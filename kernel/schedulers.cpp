//
// Created by os on 1/3/22.
//
#include "param.h"
#include "proc.h"
#include "schedulers.h"

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

// Instance that is responsible for holding RUNNABLE processes
struct scheduling_queues sched_queues;

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
struct proc* get_sjf(){
    return 0;
}

// Return process to RUNNABLE state, taking care
// of the SJF logic and arithmetic calculations.
void put_sjf(struct proc* proc){

}

// CFS
struct proc* get_cfs(){
    return 0;
}

// CFS
void put_cfs(struct proc* proc){

}
