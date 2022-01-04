//
// Created by os on 1/3/22.
//
#ifndef XV6_OS2_SCHEDULERS_H
#define XV6_OS2_SCHEDULERS_H
struct proc;

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
