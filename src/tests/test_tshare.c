#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <sched.h>

// different page size to make sure the spatial relation
#define PAGE_BYTES (4096)

int64_t result_page0[PAGE_BYTES / sizeof(uint64_t)];
int64_t result_page1[PAGE_BYTES / sizeof(uint64_t)];
int64_t result_page2[PAGE_BYTES / sizeof(uint64_t)];
int64_t result_page3[PAGE_BYTES / sizeof(uint64_t)];

int LENGTH = 200000000;

typedef struct
{
    int64_t *cache_write_ptr;
    int cpu_id;
} param_t;

void *work_thread(void *param)
{
    param_t *p = (param_t *)param;
    int64_t *w = p->cache_write_ptr;
    int cpu_id = p->cpu_id;

    // try to run on cpu
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);
    pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);

    // sync or async
    printf("    * thread[%lu] running on cpu[%d] writes to %p\n",
        pthread_self(), cpu_id, w);

    for (int i = 0; i < LENGTH; ++ i)
    {
        *w += 1;
    }

    return NULL;
}

void sequential_run()
{
    // true sharing counting
    pthread_t ts_tid_1, ts_tid_2;

    int64_t seq_result;

    param_t p = {
        .cache_write_ptr = &seq_result,
        .cpu_id = 0
    };
    
    printf("[Sequential]\n");

    long t0 = clock();

    pthread_create(&ts_tid_1, NULL, work_thread, (void *)&p);
    pthread_join(ts_tid_1, NULL);

    pthread_create(&ts_tid_2, NULL, work_thread, (void *)&p);
    pthread_join(ts_tid_2, NULL);

    printf("  Result %ld; elapsed tick tock: %ld\n", seq_result, clock() - t0);
}

void true_sharing_run()
{
    // true sharing counting
    pthread_t ts_tid_1, ts_tid_2;

    param_t param_t1 = {
        .cache_write_ptr = &result_page0[0],
        .cpu_id = 0
    };
    param_t param_t2 = {
        .cache_write_ptr = &result_page0[0],
        .cpu_id = 1
    };

    printf("[True sharing]\n");

    long t0 = clock();
    
    pthread_create(&ts_tid_1, NULL, work_thread, (void *)&param_t1);
    pthread_create(&ts_tid_2, NULL, work_thread, (void *)&param_t2);

    pthread_join(ts_tid_1, NULL);
    pthread_join(ts_tid_2, NULL);

    printf("  Result %ld; elapsed tick tock: %ld\n", result_page0[0], clock() - t0);
}

void false_sharing_run()
{
    // true sharing counting
    pthread_t ts_tid_1, ts_tid_2;

    param_t param_t1 = {
        .cache_write_ptr = &result_page1[0],
        .cpu_id = 0
    };
    param_t param_t2 = {
        .cache_write_ptr = &result_page1[1],
        .cpu_id = 1
    };

    printf("[False sharing]\n");

    long t0 = clock();
    
    pthread_create(&ts_tid_1, NULL, work_thread, (void *)&param_t1);
    pthread_create(&ts_tid_2, NULL, work_thread, (void *)&param_t2);

    pthread_join(ts_tid_1, NULL);
    pthread_join(ts_tid_2, NULL);

    printf("  Result %ld; elapsed tick tock: %ld\n", result_page1[0] + result_page1[1], clock() - t0);
}

void exclusive_run()
{
    // true sharing counting
    pthread_t ts_tid_1, ts_tid_2;

    param_t param_t1 = {
        .cache_write_ptr = &result_page2[0],
        .cpu_id = 0
    };
    param_t param_t2 = {
        .cache_write_ptr = &result_page3[0],
        .cpu_id = 1
    };

    printf("[Exclusive]\n");

    long t0 = clock();
    
    pthread_create(&ts_tid_1, NULL, work_thread, (void *)&param_t1);
    pthread_create(&ts_tid_2, NULL, work_thread, (void *)&param_t2);

    pthread_join(ts_tid_1, NULL);
    pthread_join(ts_tid_2, NULL);

    printf("  Result %ld; elapsed tick tock: %ld\n\n", 
        result_page2[0] + result_page3[0], clock() - t0);
}

int main()
{
    assert((LENGTH % 0x1) == 0);
    srand(12306);
    
    sequential_run();
    true_sharing_run();
    false_sharing_run();
    exclusive_run();

    return 0;
}