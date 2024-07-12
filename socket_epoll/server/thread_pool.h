#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include "blocking_queue.h"

struct thread_pool {
    pthread_t * thread_ids;
    uint thread_count;
    struct blocking_queue * bq;
};

void tp_init(struct thread_pool* thread_pool, int thread_count);
void tp_start(struct thread_pool* thread_pool);
void tp_submit(struct thread_pool* thread_pool, struct task* task);

#endif //THREAD_POOL_H
