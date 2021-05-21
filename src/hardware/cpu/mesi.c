#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// states of MESI
typedef enum
{
    // exclusive modified: the only dirty copy
    MODIFIED,
    // exclusive clean: the only copy
    EXCLUSIVE,
    // shared clean: multiple processors may hold this exact same copy
    SHARED,
    // invalid copy of the physical address
    // the cache line may hold data of other physical address
    INVALID
} cachestate_t;

// struct of a MESI cache line
typedef struct
{
    cachestate_t    state;
    int             value;
} cacheline_t;

#ifndef NUM_PROCESSOR
#define NUM_PROCESSOR (128)
#endif

cacheline_t cache[NUM_PROCESSOR];

// the value on L3 shared cache
int l3_value = 15213;

// count the number of each state: MESI
int state_count[4];

int check_states()
{
    // initialize
    state_count[0] = 0;  // M
    state_count[1] = 0;  // E
    state_count[2] = 0;  // S
    state_count[3] = 0;  // I

    // counting
    for (int i = 0; i < NUM_PROCESSOR; ++ i)
    {
        state_count[(int)cache[i].state] += 1;
    }

    /*  The legal states:
        +----+----+----+----+----+
        |    | M  | E  | S  | I  |
        +----+----+----+----+----+
        | M  | X  | X  | X  | O  |
        | E  | X  | X  | X  | O  |
        | S  | X  | X  | O  | O  |
        | I  | O  | O  | O  | O  |
        +----+----+----+----+----+
    */
    if (    (state_count[MODIFIED] == 1  && state_count[INVALID] == NUM_PROCESSOR - 1) ||
            (state_count[EXCLUSIVE] == 1 && state_count[INVALID] == NUM_PROCESSOR - 1) ||
            (state_count[SHARED] + state_count[INVALID] == NUM_PROCESSOR)
    )
    {
        return 1;
    }
    
#ifdef DEBUG
    printf("statistics: M <%d> E <%d> S <%d> I <%d> \n", 
        state_count[0], state_count[1], state_count[2], state_count[3]);
    exit(0);
#endif

    // illegal
    return 0;
}

// id - the index of processor
int read_cacheline(int i)
{
    if (cache[i].state == MODIFIED)
    {
#ifdef DEBUG
        printf("[%d] read hit; value: %d\n", i, cache[i].value);
#endif
        return 1;
    }
    else if (cache[i].state == EXCLUSIVE)
    {
#ifdef DEBUG
        printf("[%d] read hit; value: %d\n", i, cache[i].value);
#endif
        return 1;
    }
    else if (cache[i].state == SHARED)
    {
#ifdef DEBUG
        printf("[%d] read hit; value: %d\n", i, cache[i].value);
#endif
        return 1;
    }
    else if (cache[i].state == INVALID)
    {
        // need to sniff other processors' caches first
#ifdef DEBUG
        printf("\tbus boardcast: [%d] read", i);
#endif
        for (int j = 0; j < NUM_PROCESSOR; ++ j)
        {
            if (j != i)
            {
                // another cache
                if (cache[j].state == SHARED)
                {
                    // there exists multiple copies in the chip
                    // copy from here to avoid I/O to l3
                    cache[i].state = SHARED;
                    cache[i].value = cache[j].value;
#ifdef DEBUG
                    printf("[%d] read miss; [%d] supplies data %d\n", i, j, cache[j].value);
#endif
                    return 1;
                }
                else if (cache[j].state == EXCLUSIVE)
                {
                    // there exists only one copy in the chip here
                    // copy from here to avoid I/O to l3
                    cache[i].state = SHARED;
                    cache[i].value = cache[j].value;

                    cache[j].state = SHARED;
#ifdef DEBUG
                    printf("[%d] read miss; [%d] supplies data %d\n", i, j, cache[j].value);
#endif
                    return 1;
                }
                else if (cache[j].state == MODIFIED)
                {
                    // there exists only one copy in the chip here
                    // but it's DIRTY! write it back to l3
                    // do I/O transaction over the bus to l3
                    // copy from here to avoid I/O to l3

                    // write back the dirty data
                    l3_value = cache[j].value;
                    cache[j].state = SHARED;

                    // copy from the NOW shared cache line
                    cache[i].state = SHARED;
                    cache[i].value = cache[j].value;
#ifdef DEBUG
                    printf("[%d] read miss; [%d] supplies data %d; write back ** BUS WRITE **\n", i, j, cache[j].value);
#endif
                    return 1;
                }
            }
        }

        // no return before means:
        // all other cache lines are invalid
        cache[i].state = EXCLUSIVE;     // so I am the only copy among all processors
        cache[i].value = l3_value;      // do one I/O transaction over bus to l3
#ifdef DEBUG
        printf("[%d] read miss; L3 shared cache supplies data %d ** BUS READ **\n", i, l3_value);
#endif
        return 1;
    }

    return 0;
}

// i - the index of this processor
// data - the data to be written to physical address on cache
int write_cacheline(int i, int value)
{
    if (cache[i].state == MODIFIED)
    {
        // write hit, directly update the dirty value
        cache[i].value = value;
#ifdef DEBUG
        printf("[%d] write hit; update data to %d\n", i, value);
#endif 
        return 1;
    }
    else if (cache[i].state == EXCLUSIVE)
    {
        // in MESI, we do not need to boardcast the write when Exclusive clean
        cache[i].state = MODIFIED;
        cache[i].value = value;
#ifdef DEBUG
        printf("[%d] write hit; update data to %d; dirty this cache\n", i, value);
#endif
        return 1;
    }
    else if (cache[i].state == SHARED)
    {
        // multiple copies among the processors
        // invalid them through the boardcast on the shared bus
        for (int j = 0; j < NUM_PROCESSOR; ++ j)
        {
            if (j != i)
            {
                cache[j].state = INVALID;
                cache[j].value = 0;
            }
        }
        // update this cache line
        cache[i].state = MODIFIED;
        cache[i].value = value;
#ifdef DEBUG
        printf("[%d] write hit; boardcast writing %d\n", i, value);
#endif 
        return 1;
    }
    else if (cache[i].state == INVALID)
    {
        // check the cache line in other processors
#ifdef DEBUG
        printf("\tbus boardcast: [%d] write", i);
#endif
        for (int j = 0; j < NUM_PROCESSOR; ++ j)
        {
            if (j != i)
            {
                if (cache[j].state == MODIFIED)
                {
                    // existing only one modified copy, just invalid it
                    // no bus transaction here, no I/O to l3
                    cache[j].state = INVALID;
                    cache[j].value = 0;
                    // update the current cache
                    cache[i].state = MODIFIED;
                    cache[i].value = value;
#ifdef DEBUG
                    printf("[%d] write miss; invalid the Modified [%d]\n", i, j);
#endif
                    return 1;
                }
                else if (cache[j].state == EXCLUSIVE)
                {
                    // existing only one Exclusive copy, just invalid it
                    // no bus transaction here, no I/O to l3
                    cache[j].state = INVALID;
                    cache[j].value = 0;
                    // update the current cache
                    cache[i].state = MODIFIED;
                    cache[i].value = value;
#ifdef DEBUG
                    printf("[%d] write miss; invalid the Exclusive [%d]\n", i, j);
#endif
                    return 1;
                }
                else if (cache[j].state == SHARED)
                {
                    // existing multiple clean copies, must invalid all of them
                    for (int k = 0; k < NUM_PROCESSOR; ++ k)
                    {
                        if (k != i)
                        {
                            cache[k].state = INVALID;
                            cache[k].value = 0;
                        }
                    }
                    // update the current cache
                    cache[i].state = MODIFIED;
                    cache[i].value = value;
#ifdef DEBUG
                    printf("[%d] write miss; boardcast writing %d\n", i, value);
#endif 
                    return 1;
                }
            }
        }

        // all other processors are not holding any copy
        // need to load from l3
        cache[i].state = MODIFIED;
        cache[i].value = value;
#ifdef DEBUG
        printf("[%d] write miss; no copies in chip; update value in place %d\n", i, value);
#endif 
        return 1;
    }

    return 0;
}

// i - the index of current processor
int evict_cacheline(int i)
{
    if (cache[i].state == MODIFIED)
    {
        // write back to l3 and transit to invalid
        l3_value = cache[i].value;

        // invalid this cache line since the physical address is no longer in the cache
        cache[i].state = INVALID;
        cache[i].value = 0;
#ifdef DEBUG
        printf("[%d] evict; write back value %d ** BUS WRITE **\n", i, l3_value);
#endif 
        return 1;
    }
    else if (cache[i].state != INVALID)
    {
        // we do not consider invalid evict since the paddr is already detached
        // for other states: invalid, exclusive, shared
        // they are all clean, so no bus I/O transaction
        cache[i].state = INVALID;
        cache[i].value = 0;
#ifdef DEBUG
        printf("[%d] evict in place\n", i);
#endif 
        return 1;
    }
    
    return 0;
}

#ifdef DEBUG
void print_cache()
{
    for (int i = 0; i < NUM_PROCESSOR; ++ i)
    {
        char c;
        switch (cache[i].state)
        {
            case MODIFIED:
                c = 'M';
                break;
            case EXCLUSIVE:
                c = 'E';
                break;
            case SHARED:
                c = 'S';
                break;
            case INVALID:
                c = 'I';
                break;
            default:
                break;
        }

        printf("\t[%4d]   state %c   value %d\n", i, c, cache[i].value);
    }
        printf("                            L3 Shared cache copy: %d\n", l3_value);
}
#endif

int main()
{
    srand(12345);

    for (int i = 0; i < NUM_PROCESSOR; ++ i)
    {
        cache[i].state = INVALID;
        cache[i].value = 0;
    }

#ifdef DEBUG
        print_cache();
#endif

    int pc = 0;
    for (int i = 0; i < 20000; ++ i)
    {
        int op_case = rand() % 3;
        int index_proc = rand() % NUM_PROCESSOR;

        if (op_case == 0)
        {
            pc = read_cacheline(index_proc);
        }
        else if (op_case == 1)
        {
            pc = write_cacheline(index_proc, rand());
        }
        else if (op_case == 2)
        {
            pc = evict_cacheline(index_proc);
        }

#ifdef DEBUG
        if (pc == 1)
        {
            print_cache();
        }
#endif

        if (check_states() == 0)
        {
            printf("Failed\n");
            return 0;
        }
    }

    printf("Pass\n");
    return 0;
}