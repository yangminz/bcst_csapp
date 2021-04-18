/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "headers/common.h"
#include "headers/algorithm.h"

array_t *array_construct(int size)
{
    array_t *arr = malloc(sizeof(array_t));
    arr->count = 0;
    arr->size = size;
    arr->table = malloc(sizeof(uint64_t) * arr->size);
    return arr;
}

void array_free(array_t *arr)
{
    free(arr->table);
    free(arr);
}

int array_insert(array_t **addr, uint64_t value)
{
    array_t *arr = *addr;
    if (arr == NULL)
    {
        return 0;
    }

    if (arr->count == arr->size)
    {
        // malloc to twice size
        uint64_t *old_table = arr->table;

        arr->size = 2 * arr->size;
        arr->table = malloc(arr->size * sizeof(uint64_t));
        // count unchanged

        // copy from the old to the new
        for (int i = 0; i < arr->count; ++ i)
        {
            arr->table[i] = old_table[i];
        }
        // free the old table memory
        free(old_table);
    }

    // insert the new value
    arr->table[arr->count] = value;
    arr->count += 1;

    return 1;
}

int array_delete(array_t *arr, int index)
{
    if (arr == NULL || (index < 0 || arr->count <= index))
    {
        return 0;
    }

    if (arr->count - 1 <= arr->size / 4)
    {
        // shrink the table to half when payload is quater
        uint64_t *old_table = arr->table;

        arr->size = arr->size / 2;
        arr->table = malloc(arr->size * sizeof(uint64_t));
        // count unchanged

        // copy from the old to the new
        int j = 0;
        for (int i = 0; i < arr->count; ++ i)
        {
            if (i != index)
            {
                arr->table[j] = old_table[i];
                j += 1;
            }
        }
        arr->count -= 1;

        free(old_table);
        return 1;
    }
    else
    {
        // copy move from index forward
        for (int i = index; i < arr->count; ++ i)
        {
            if (0 <= i - 1)
            {
                arr->table[i - 1] = arr->table[i];
            }
        }
        arr->count -= 1;
        return 1;
    }

    return 0;
}

int array_get(array_t *arr, int index, uint64_t *valptr)
{
    if (index > arr->count)
    {
        // not found
        return 0;
    }
    else
    {
        // found at [index]
        *valptr = arr->table[index];
        return 1;
    }
}

void print_array(array_t *arr)
{
    if ((DEBUG_VERBOSE_SET & DEBUG_DATASTRUCTURE) == 0)
    {
        return;
    }

    printf("array size: %u count: %u\n", arr->size, arr->count);
    for (int i = 0; i < arr->count; ++ i)
    {
        printf("\t[%d] %16lx\n", i, arr->table[i]);
    }
}