#ifndef __SYNC_H__
#define __SYNC_H__
#include<glib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include<malloc.h>


struct lock1
{
	int lock_state;
    int ticket;
	int users;
	
	};

struct lock2
{
    int rcount;               //number of readers              
    int sect_lock;           //lock to perform a group of operations in a mutually exclusive manner
    int action_lock;        //exclusive action to read or write action
    int rcount_lock;        //lock to manipulate the rcount variable in a mutually exclusive manner
    
};

struct barrier
{
	int thread_counter;
	int tot_threads;        //total number of threads
	int state;         
};

 

void lock1_init(struct lock1 *lock);
void lock1_destroy(struct lock1 *lock);
void lock1_lock(struct lock1 *lock, unsigned flags);
void lock1_unlock(struct lock1 *lock, unsigned flags);

void lock2_init(struct lock2 *lock);
void lock2_destroy(struct lock2 *lock);
void lock2_lock(struct lock2 *lock, unsigned flags);
void lock2_unlock(struct lock2 *lock, unsigned flags);

void barrier_init(struct barrier *barrier, int c);
void barrier_destroy(struct barrier *barrier);
void barrier_wait(struct barrier *barrier, unsigned flags);

#endif

