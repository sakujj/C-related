#ifndef LAMDA_H
#define LAMDA_H

#include "task_arg.h"

struct task {
    struct task_arg arg;
    void (*func) (struct task_arg);
};

#endif //LAMDA_H
