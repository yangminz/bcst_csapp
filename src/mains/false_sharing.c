#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>

#define PAGE_BYTES (4096)

int64_t result_page0[PAGE_BYTES / sizeof(int64_t)];
int64_t result_page1[PAGE_BYTES / sizeof(int64_t)];
int64_t result_page2[PAGE_BYTES / sizeof(int64_t)];
int64_t result_page3[PAGE_BYTES / sizeof(int64_t)];

typedef struct
{
    int64_t *cache_write_ptr;
    int cpu_id;
    int length;
} param_t;

void *work_thread(void *param)
{
    param_t *p = (param_t *)param;
    int64_t *ptr = p->cache_write_ptr;
    int cpu_id = p->cpu_id;
    int length = p->length;

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);
    pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);

    printf("    * thread[%lu] running on cpu[%d] writes to %p\n",
        pthread_self(), sched_getcpu(), ptr);

    for (int i = 0; i < length; ++ i)
    {
        // write - not thread safe
        // just write to make cache line dirty
        *ptr += 1;
    }

    return NULL;
}

int LENGTH = 200000000;

void true_sharing_run()
{
    pthread_t t1, t2;

    param_t p1 = {
        .cache_write_ptr = &result_page0[0],
        .cpu_id = 0,
        .length = LENGTH
    };

    param_t p2 = {
        .cache_write_ptr = &result_page0[0],
        .cpu_id = 1,
        .length = LENGTH
    };

    long t0 = clock();

    pthread_create(&t1, NULL, work_thread, (void *)&p1);
    pthread_create(&t2, NULL, work_thread, (void *)&p2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("[True Sharing]\n\tresult: %ld; elapsed tick tock: %ld\n", 
        result_page0[0],
        clock() - t0);
}

void false_sharing_run()
{
    pthread_t t1, t2;

    param_t p1 = {
        .cache_write_ptr = &result_page1[0],
        .cpu_id = 0,
        .length = LENGTH
    };

    param_t p2 = {
        .cache_write_ptr = &result_page1[1],
        .cpu_id = 1,
        .length = LENGTH
    };

    long t0 = clock();

    pthread_create(&t1, NULL, work_thread, (void *)&p1);
    pthread_create(&t2, NULL, work_thread, (void *)&p2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("[False Sharing]\n\tresult: %ld; elapsed tick tock: %ld\n", 
        result_page1[0] + result_page1[1], clock() - t0);
}

void no_sharing_run()
{
    pthread_t t1, t2;

    param_t p1 = {
        .cache_write_ptr = &result_page2[0],
        .cpu_id = 0,
        .length = LENGTH
    };

    param_t p2 = {
        .cache_write_ptr = &result_page3[0],
        .cpu_id = 1,
        .length = LENGTH
    };

    long t0 = clock();

    pthread_create(&t1, NULL, work_thread, (void *)&p1);
    pthread_create(&t2, NULL, work_thread, (void *)&p2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("[No Sharing]\n\tresult: %ld; elapsed tick tock: %ld\n", 
        result_page2[0] + result_page3[0], clock() - t0);
}

int main()
{
    true_sharing_run();
    false_sharing_run();
    no_sharing_run();
}