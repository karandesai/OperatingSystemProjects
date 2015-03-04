#include<stdio.h>
#include<sys/types.h>
#include "sync.h"

void lock1_init(struct lock1 *lock)
{
lock->lock_state=0;  //0 for lock is free  ... 1 for lock is held
lock->ticket=0;
lock->users=0;
  
   
 
}
     
void lock1_destroy(struct lock1 *lock)
{
		lock->lock_state=0;lock->ticket=0;
lock->users=0;

void lock1_lock(struct lock1 *lock, unsigned flags)
{
        
	int my_ticket = __sync_fetch_and_add(&lock->users, 1);
 // printf("\n Thread id %ld \n", pthread_self());


	while(lock->ticket!=my_ticket);
	while(__sync_bool_compare_and_swap (&lock->lock_state,1,1));
	
	
		
}

void lock1_unlock(struct lock1 *lock, unsigned flags)
{
	
	__sync_bool_compare_and_swap (&lock->lock_state,1,0);
        lock->ticket++;
//printf("Thread UNLOCKED %ld \n", pthread_self());		
}

void lock2_init(struct lock2 *lock)
{
   lock->rcount=0;              //number of readers
   lock->sect_lock=0;          //lock to gain entry to that particular section of code
   lock->action_lock=0;        //lock to gain to read or write access to resource
   lock->rcount_lock=0;        //lock to gain access to rcount
}

void lock2_destroy(struct lock2 *lock)
{
   lock->rcount=0;
   lock->sect_lock=0;
   lock->action_lock=0;
   lock->rcount_lock=0;
}

void lock2_lock(struct lock2 *lock, unsigned flags)
{
	
  if(flags==1) //if write lock required
	{
		//printf("\nwrite lock acquired by thread id = %ld\n",pthread_self());

	            //gain access to this section of code
	while(__sync_bool_compare_and_swap (&lock->sect_lock,1,1));
   	 while(__sync_bool_compare_and_swap (&lock->action_lock,1,1));          // Request exclusive access to the resource 
   	__sync_bool_compare_and_swap (&lock->sect_lock,1,0);

	}
   else    //if read lock required
	{
	//printf("\nread lock acquired by thread id = %ld\n",pthread_self());

		   // gain access to this section of code
 	while(__sync_bool_compare_and_swap (&lock->sect_lock,1,1));
	while(__sync_bool_compare_and_swap (&lock->rcount_lock,1,1));	// gain access to the readers counter
   	         
   		if (lock->rcount == 0)        // If there are currently no readers (we came first)...
     		while(__sync_bool_compare_and_swap (&lock->action_lock,1,1));  // ...requests exclusive access to the resource for readers
   		__sync_fetch_and_add (&lock->rcount,1);                  // one more reader
            __sync_bool_compare_and_swap (&lock->sect_lock,1,0);        // Release order of arrival semaphore (we have been served)
            __sync_bool_compare_and_swap (&lock->rcount_lock,1,0);     // done accessing the rcount for now
	}


}

void lock2_unlock(struct lock2 *lock, unsigned flags)
{
	if(flags==1)                               //
	{
		//printf("\nwrite lock released by thread id = %ld\n",pthread_self());
		__sync_bool_compare_and_swap (&lock->action_lock,1,0);
	}
	else
	{
		//printf("\nread lock released by thread id = %ld\n",pthread_self());
		while(__sync_bool_compare_and_swap (&lock->rcount_lock,1,1));	// gain access to the readers counter
		__sync_fetch_and_sub (&lock->rcount,1);
		if(lock->rcount==0)
		__sync_bool_compare_and_swap (&lock->action_lock,1,0);
		__sync_bool_compare_and_swap (&lock->rcount_lock,1,0);
	}

}

void barrier_init(struct barrier *barrier, int c)
{
	barrier->thread_counter=0;     
	barrier->tot_threads=c;   
	barrier->state=0;            //0 = dont allow further progress        
}

void barrier_destroy(struct barrier *barrier)
{
	barrier->thread_counter=0;
	barrier->tot_threads=0;
	barrier->state=0;
}

void barrier_wait(struct barrier *barrier, unsigned flags)
{
	//while(__sync_bool_compare_and_swap (&barrier->sect_lock,1,1));

	struct lock1 mylock;
	lock1_init(&mylock);

	__sync_fetch_and_add (&barrier->thread_counter,1);
	
		//printf("\n thread id = %ld enter thread counter = %d \n",pthread_self(),barrier->thread_counter);
	
		lock1_lock(&mylock,0);		
		//printf("\n entering thread is %ld \n",pthread_self());
	
		if(__sync_bool_compare_and_swap (&barrier->thread_counter,barrier->tot_threads,0))   //if last thread reached release barrier
			{
				__sync_lock_test_and_set (&barrier->state,1);
				//break;
                              /*This function atomically assigns &barrier-state to 1
                               An acquire memory barrier is created when this function is invoked.*/
			}
		
		
		//printf("\nexiting testing thread is %ld \n",pthread_self());
		lock1_unlock(&mylock,0);
		while(__sync_bool_compare_and_swap (&barrier->state,0,0));     //incoming threads spin until barrier is released 
		

	lock1_destroy(&mylock);
	
 	
	

}
