add_executable(server main.c
        thread_pool.c
        blocking_queue.c
        blocking_queue.h
        thread_pool.h
        task.h
        task_arg.h)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O1 -fsanitize=address -fno-omit-frame-pointer")