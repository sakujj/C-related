#include  "blocking_queue.h"

#include <stdio.h>
#include <stdlib.h>

int bq_init(struct blocking_queue * bq) {
    bq->head = NULL;
    bq->tail = NULL;
    pthread_mutex_init(&bq->mutex, 0);
    sem_init(&bq->semaphore, 0, 0);
    bq->size = 0;

    return 0;
}

struct task* bq_take(struct blocking_queue* bq) {
    sem_wait(&bq->semaphore);
    pthread_mutex_lock(&bq->mutex);

    struct task* task = bq->head->task;
    struct node* old_head = bq->head;

    if (bq->head == bq->tail) {
        bq->head = bq->tail = NULL;
    } else {
        bq->head = bq->head->next;
    }
    free(old_head);

    bq->size--;
    printf("TAKEN, QUEUE SIZE IS %ld\n", bq->size);
    fflush(stdout);
    pthread_mutex_unlock(&bq->mutex);
    return task;
};

void bq_put(struct blocking_queue* bq, struct task *task) {
    pthread_mutex_lock(&bq->mutex);

    struct node * new_node = malloc(sizeof (struct node));
    new_node->task = task;
    new_node->next = NULL;

    if (bq->tail == NULL) {
        bq->head = bq->tail = new_node;
    } else {
        bq->tail->next = new_node;
        bq->tail = bq->tail->next;
    }

    bq->size++;
    pthread_mutex_unlock(&bq->mutex);
    sem_post(&bq->semaphore);
};
