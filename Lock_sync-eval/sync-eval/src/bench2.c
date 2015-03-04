v#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include "sync.h"

#define DEFAULT_NCLIENTS 4
#define DEFAULT_NSERVERS 2
#define DEFAULT_TARGET 1000000

typedef struct {
	int clientnum;
	unsigned *counter;
	unsigned target;
	unsigned sample_counter;
	unsigned sample_target;
} client_t;

typedef struct {
	int servernum;
	unsigned *counter;
	unsigned target;
} server_t;

struct lock2 lock;

void *client(void *data)
{
	client_t *c = (client_t*) data;
	unsigned current;
	
	printf("Client %d starting...\n", c->clientnum);
	while (1) {
//		printf("Client %d working...counter at %u\n", c->clientnum, current);
		lock2_lock(&lock, 0);
		current = *c->counter;
		lock2_unlock(&lock, 0);

		if (/*current == c->target && */(++c->sample_counter == c->sample_target)) {
			printf("Client %d hit sample target %u, exiting...\n",
						c->clientnum, c->sample_target);
			break;
		}

	}
	return 0;
}

void *server(void *data)
{
	server_t *s = (server_t*) data;
	unsigned current;

	printf("Server %d starting...\n", s->servernum);
	while (1) {
		lock2_lock(&lock, 1);
		current = (*s->counter);
		if (current < s->target)
			current = ++(*s->counter);
		lock2_unlock(&lock, 1);

//		printf("Server %d working...counter at %u\n", s->servernum, current);
		if (current == s->target)
			break;
	}
	printf("Server %d hit counter target %u, exiting...\n", s->servernum, s->target);
	return 0;
}

int main(int argc, char *argv[])
{
	int c;
	int i;
	int n_clients = DEFAULT_NCLIENTS;
	int n_servers = DEFAULT_NSERVERS;
	int target = DEFAULT_TARGET;
	unsigned counter = 0;

	client_t *client_data;
	server_t *server_data;
	pthread_t *ctid;
	pthread_t *stid;
	pthread_attr_t attr;

	while ((c = getopt(argc, argv, "c:s:t:")) >= 0) {
		switch(c) {
			case 'c': n_clients = atoi(optarg); break;
			case 's': n_servers = atoi(optarg); break;
			case 't': target = atoi(optarg); break;
		}
	}

	printf("CS 519: bench2:\n");
	printf("Running with n_clients = %d, n_servers = %d, target = %d\n",
						n_clients, n_servers, target);

	lock2_init(&lock);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	ctid = malloc(sizeof(pthread_t) * n_clients);
	stid = malloc(sizeof(pthread_t) * n_servers);
	client_data = malloc(sizeof(client_t) * n_clients);
	server_data = malloc(sizeof(server_t) * n_servers);

	for (i = 0; i < n_clients; i++) {
		client_data[i].clientnum = i;
		client_data[i].counter = &counter;
		client_data[i].target = target;
		client_data[i].sample_counter = 0;
		client_data[i].sample_target = target;
	}

	for (i = 0; i < n_servers; i++) {
		server_data[i].servernum = i;
		server_data[i].counter = &counter;
		server_data[i].target = target;
	}

	for (i = 0; i < n_clients; i++)
		pthread_create(&ctid[i], &attr, client, &client_data[i]);

	for (i = 0; i < n_servers; i++)
		pthread_create(&stid[i], &attr, server, &server_data[i]);

	for (i = 0; i < n_clients; i++)
		pthread_join(ctid[i], 0);

	for (i = 0; i < n_servers; i++)
		pthread_join(stid[i], 0);

	free(ctid);
	free(stid);
	free(client_data);
	free(server_data);
	lock2_destroy(&lock);

	return 0;
}


