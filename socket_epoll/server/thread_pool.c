#include "thread_pool.h"

#include <stdio.h>
#include <stdlib.h>

void tp_init(struct thread_pool* thread_pool, int thread_count) {

    thread_pool->thread_count = thread_count;

    pthread_t* threads = malloc(thread_count * sizeof(pthread_t));
    thread_pool->thread_ids = threads;

    struct blocking_queue* bq = malloc(sizeof (struct blocking_queue));
    bq_init(bq);

    thread_pool->bq = bq;
};

void tp_submit(struct thread_pool* thread_pool, struct task* task) {
    bq_put(thread_pool->bq, task);
}

struct thread_data {
    ulong thread_id;
    struct blocking_queue* bq;
};


static void* task_executor(void * argp) {
    struct thread_data * ti = argp;
    void* task_vp;
    while ((task_vp = bq_take(ti->bq)) != NULL) {
        struct task* task = task_vp;

        task->func(task->arg);

        free(task);
    }

    return NULL;
}

void tp_start(struct thread_pool* thread_pool) {
    int n = thread_pool-> thread_count;
    pthread_t* threads = thread_pool->thread_ids;

    for (int i = 0; i < n; ++i, ++threads) {
        struct thread_data* thread_data = malloc(sizeof (struct thread_data));
        thread_data->bq = thread_pool->bq;
        pthread_create(threads, NULL, task_executor, thread_data);
        thread_data->thread_id = *threads;
    }
};

