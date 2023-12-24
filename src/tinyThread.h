#ifndef __TINY_THREAD_H__
#define __TINY_THREAD_H__
#include <stdint.h>

typedef void* (*TINY_THREAD_FUNC)(void*);

int tinythread_init(void);

int tinythread_destroy(void);

int tinythread_run(void);

int tinythread_create(TINY_THREAD_FUNC func, void* args);

int tinythread_join(int tid, void* ret);

int tinythread_detach(int tid);

#endif // !__TINY_THREAD_H__
