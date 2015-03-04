#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include "sync.h"

typedef struct {
	int n, m, type, start, step;
	uint64_t *bits;
} worker_t;

#define slow_comp(i, n) ((uint64_t)(i) * (i) * (i) % (n))

struct lock1 lock;

void *worker(void *data)
{
	worker_t *w = (worker_t*)data;
	int64_t i, nm;
	if (w->type == 0) { // only works for a single thread
		for (i = w->start, nm = (int64_t)w->n * w->m; i < nm; i += w->step) {
			uint64_t x = slow_comp(i, w->n);
			w->bits[x>>6] ^= 1LLU << (x&0x3f);
		}
	} else if (w->type == 1) {
		for (i = w->start, nm = (int64_t)w->n * w->m; i < nm; i += w->step) {
			uint64_t x = slow_comp(i, w->n);
			uint64_t *p = &w->bits[x>>6];
			uint64_t y = 1LLU << (x & 0x3f);
			lock1_lock(&lock, 0);
			*p ^= y;
			lock1_unlock(&lock, 0);
		}
	} 
	return 0;
}

int main(int argc, char *argv[])
{
	int c, n_threads = 1, i, tmp,thread_count=0;
	uint64_t z;
	worker_t *w, w0;
	pthread_t *tid;
	pthread_attr_t attr;

	w0.n = 1000000; w0.m = 100; w0.type = 1; w0.start = 0; w0.step = 1;
	while ((c = getopt(argc, argv, "t:n:s:m:l:")) >= 0) {
		
		switch (c) {
			case 't': 
			
			n_threads = atoi(optarg);
			break;
			
			case 'n': 
			w0.n = atoi(optarg); 
			break;
			
			case 'm': 
			w0.m = atoi(optarg);
		    break;
		    
			case 'l': 
			w0.type = atoi(optarg);
			break;
		}
	}
	fprintf(stderr, "Usage: %s [-t nThreads=%d] [-n size=%d] [-m repeat=%d] [-l lockType=%d]\n",
			argv[0], n_threads, w0.n, w0.m, w0.type);
	fprintf(stderr, "Lock type: 0 for single-thread (no locking); 1 for lock1;\n");

	w0.bits = (uint64_t*)calloc((w0.n + 63) / 64, 8);

	lock1_init(&lock);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	tid = alloca(sizeof(pthread_t) * n_threads);
	w = alloca(sizeof(worker_t) * n_threads);
	for (i = 0; i < n_threads; ++i) {
		w[i] = w0;
		w[i].start = i; w[i].step = n_threads;
	}
	for (i = 0; i < n_threads; ++i)
	{
		if( pthread_create(&tid[i], &attr, worker, &w[i])==0)
		thread_count++;
	}
	for (i = 0; i < n_threads; ++i) pthread_join(tid[i], 0);

	for (i = 0, z = 0, tmp = (w0.n + 63)/64; i < tmp; ++i) z ^= w0.bits[i];
	fprintf(stderr, "Hash: %llx (should be 0 if -m is even or aaaaaaaaaaaaaaa if -m is odd)\n", (unsigned long long)z);
	free(w0.bits);
	lock1_destroy(&lock);
	
	printf("\n No of threads = %d",thread_count);
	return 0;
}
