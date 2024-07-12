#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H

#include "task.h"
#include <pthread.h>
#include <semaphore.h>

struct node {
    struct node * next;
    struct task * task;
};

struct blocking_queue {
    pthread_mutex_t mutex;
    sem_t semaphore;
    size_t size;

    struct node* head;
    struct node* tail;
};

int bq_init(struct blocking_queue * bq);

struct task* bq_take(struct blocking_queue* bq);

void bq_put(struct blocking_queue* bq, struct task* task);

#endif //BLOCKING_QUEUE_H