#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "sync.h"

#define DEFAULT_NTHREADS 1
#define DEFAULT_XSIZE 1000
#define DEFAULT_YSIZE 100

typedef struct {
	int threadnum;
	int **matrix;
	int x_size;
	int y_size;
} worker_t;

struct barrier barrier;

void *worker(void *data)
{
	worker_t *w = (worker_t*)data;
	int i, j, tmp;
	struct timespec sleeptime;
	sleeptime.tv_sec = 0;
	sleeptime.tv_nsec = 0;

	for (i = w->threadnum; i < (w->y_size + w->threadnum); i++) {
		if (i < w->y_size) {
			sleeptime.tv_nsec = random() % 10;
			sleeptime.tv_nsec = !sleeptime.tv_nsec;
			for (j=0; j < w->x_size; j++) {
				tmp = w->matrix[i][j];
				if(sleeptime.tv_nsec > 0) {
					nanosleep(&sleeptime, NULL);
				}
				w->matrix[i][j] = tmp + 1;
			}
		}
		barrier_wait(&barrier, 0);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int c;
	int i, j;
	int n_threads = DEFAULT_NTHREADS;
	int x_size = DEFAULT_XSIZE;
	int y_size = DEFAULT_YSIZE;
	int **matrix;
	int bad_rows = 0;

	worker_t *w;
	pthread_t *tid;
	pthread_attr_t attr;
	
	while ((c = getopt(argc, argv, "t:x:y:")) >= 0) {
		switch(c) {
			case 't': n_threads = atoi(optarg); break;
			case 'x': x_size = atoi(optarg); break;
			case 'y': y_size = atoi(optarg); break;
		}
	}

	printf("CS 519: bench3:\n");
	printf("Running with n_threads = %d, matrix_size = %d x %d\n",
						n_threads, x_size, y_size);

	barrier_init(&barrier, n_threads);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	tid = malloc(sizeof(pthread_t) * n_threads);
	w = malloc(sizeof(worker_t) * n_threads);
	matrix = malloc(sizeof(int*) * y_size);
	for (i = 0; i < y_size; i++) {
		matrix[i] = malloc(sizeof(int) * x_size);
		if (!matrix[i]) {
			printf("ENOMEM\n");
			exit(0);
		}
	}

	for (i = 0; i < n_threads; i++) {
		w[i].threadnum = i;
		w[i].matrix = matrix;
		w[i].x_size = x_size;
		w[i].y_size = y_size;
	}

	for (i = 0; i < n_threads; i++)
		pthread_create(&tid[i], &attr, worker, &w[i]);
	
	for (i = 0; i < n_threads; i++)
		pthread_join(tid[i], 0);

	printf("Done!\n\n");
	for (i=0; i < y_size; i++) {
		for (j = 1; j < x_size; j++) {
			if (matrix[i][j-1] != matrix[i][j]) {
				bad_rows++;
				break;
			}
		}
	}
	if (bad_rows)
		printf("Found %d bad rows...\n", bad_rows);
	else
		printf("All rows are consistent...\n");

	free(tid);
	free(w);
	for (i = 0; i < y_size; i++)
		free(matrix[i]);
	free(matrix);
	barrier_destroy(&barrier);

	return 0;
}
