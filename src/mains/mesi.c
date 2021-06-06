#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

typedef enum
{
    MODIFIED,   // exclusive dirty, global 1
    EXCLUSIVE,  // exclusive clean, global 1
    SHARED,     // shared clean, global >= 2
    INVALID,    // invalid
} state_t;

typedef struct 
{
    state_t state;
    int value;
} line_t;

#define NUM_PROCESSOR (2048)

line_t cache[NUM_PROCESSOR];

int mem_value = 15213;

int check_state()
{
    int m_count = 0;
    int e_count = 0; 
    int s_count = 0; 
    int i_count = 0; 

    for (int i = 0; i < NUM_PROCESSOR; ++ i)
    {
        if (cache[i].state == MODIFIED)
        {
            m_count += 1;
        }
        else if (cache[i].state == EXCLUSIVE)
        {
            e_count += 1;
        }
        else if (cache[i].state == SHARED)
        {
            s_count += 1;
        }
        else if (cache[i].state == INVALID)
        {
            i_count += 1;
        }
    }

    /*
        M   E   S   I
    M   X   X   X   O
    E   X   X   X   O
    S   X   X   O   O
    I   O   O   O   O
    */

    #ifdef DEBUG
    printf("M %d\t E %d\t S %d\t I %d\n", m_count, e_count, s_count, i_count);
    #endif

    if ((m_count == 1 && i_count == (NUM_PROCESSOR - 1)) || 
        (e_count == 1 && i_count == (NUM_PROCESSOR - 1)) || 
        (s_count >= 2 && i_count == (NUM_PROCESSOR - s_count)) ||
        (i_count == NUM_PROCESSOR))
    {
        return 1;
    }

    return 0;
}

// i - the index of processor
// read_value - the address of read value
// int return - if this event is related with target physical address
int read_cacheline(int i, int *read_value)
{
    if (cache[i].state == MODIFIED)
    {
        // read hit
        #ifdef DEBUG
        printf("[%d] read hit; dirty value %d\n", i, cache[i].value);
        #endif
        *read_value = cache[i].value;
        return 1;
    }
    else if (cache[i].state == EXCLUSIVE)
    {
        // read hit
        #ifdef DEBUG
        printf("[%d] read hit; exclusive clean value %d\n", i, cache[i].value);
        #endif
        *read_value = cache[i].value;
        return 1;
    }
    else if (cache[i].state == SHARED)
    {
        // read hit
        #ifdef DEBUG
        printf("[%d] read hit; shared clean value %d\n", i, cache[i].value);
        #endif
        *read_value = cache[i].value;
        return 1;
    }
    else
    {
        // read miss
        // bus boardcast read miss
        for (int j = 0; j < NUM_PROCESSOR; ++ j)
        {
            if (i != j)
            {
                if (cache[j].state == MODIFIED)
                {
                    // write back
                    // there are eaxctly 2 copies in processors
                    mem_value = cache[j].value;
                    cache[j].state = SHARED;

                    // update read miss cache
                    cache[i].state = SHARED;
                    cache[i].value = cache[j].value;

                    *read_value = cache[i].value;

                    #ifdef DEBUG
                    printf("[%d] read miss; [%d] supplies dirty value %d; write back; s_count == 2\n", i, j, cache[i].value);
                    #endif

                    return 1;
                }
                else if (cache[j].state == EXCLUSIVE)
                {
                    // no memory transaction
                    cache[i].state = SHARED;
                    cache[i].value = cache[j].value;

                    // there are eaxctly 2 copies in processors
                    cache[j].state = SHARED;

                    *read_value = cache[i].value;

                    #ifdef DEBUG
                    printf("[%d] read miss; [%d] supplies clean value %d; s_count == 2\n", i, j, cache[i].value);
                    #endif

                    return 1;
                }
                else if (cache[j].state == SHARED)
                {
                    // >= 3
                    cache[i].state = SHARED;
                    cache[i].value = cache[j].value;

                    *read_value = cache[i].value;

                    #ifdef DEBUG
                    printf("[%d] read miss; [%d] supplies clean value %d; s_count >= 3\n", i, j, cache[i].value);
                    #endif

                    return 1;
                }
            }
        }

        // all others are invalid
        cache[i].state = EXCLUSIVE;
        cache[i].value = mem_value;

        *read_value = cache[i].value;

        #ifdef DEBUG
        printf("[%d] read miss; mem supplies clean value %d; e_count == 1\n", i, cache[i].value);
        #endif

        return 1;
    }

    return 0;
}

// i - the index of processor
// write_value - the value to be written to the physical address
// int return - if this event is related with target physical address
int write_cacheline(int i, int write_value)
{
    if (cache[i].state == MODIFIED)
    {
        // write hit
        cache[i].value = write_value;
        
        #ifdef DEBUG
        printf("[%d] write hit; update to value %d\n", i, cache[i].value);
        #endif

        return 1;
    }
    else if (cache[i].state == EXCLUSIVE)
    {
        cache[i].state = MODIFIED;
        cache[i].value = write_value;
        
        #ifdef DEBUG
        printf("[%d] write hit; update to value %d\n", i, cache[i].value);
        #endif

        return 1;
    }
    else if (cache[i].state == SHARED)
    {
        // boardcast write invalid
        for (int j = 0; j < NUM_PROCESSOR; j ++)
        {
            if (j != i)
            {
                cache[j].state = INVALID;
                cache[j].value = 0;
            }
        }

        cache[i].state = MODIFIED;
        cache[i].value = write_value;
        
        #ifdef DEBUG
        printf("[%d] write hit; boardcast invalid; update to value %d\n", i, cache[i].value);
        #endif

        return 1;
    }
    else if (cache[i].state == INVALID)
    {
        for (int j = 0; j < NUM_PROCESSOR; ++ j)
        {
            if (i != j)
            {
                if (cache[j].state == MODIFIED)
                {
                    // write back
                    mem_value = cache[j].value;

                    // invalid old cache line
                    cache[j].state = INVALID;
                    cache[j].value = 0;

                    // write allocate
                    cache[i].value = mem_value;

                    // update to modified
                    cache[i].state = MODIFIED;
                    cache[i].value = write_value;
        
                    #ifdef DEBUG
                    printf("[%d] write miss; boardcast invalid to M; update to value %d\n", i, cache[i].value);
                    #endif
                    return 1;
                }
                else if (cache[j].state == EXCLUSIVE)
                {
                    cache[j].state = INVALID;
                    cache[j].value = 0;

                    cache[i].state = MODIFIED;
                    cache[i].value = write_value;
        
                    #ifdef DEBUG
                    printf("[%d] write miss; boardcast invalid to E; update to value %d\n", i, cache[i].value);
                    #endif
                    return 1;
                }
                else if (cache[j].state == SHARED)
                {
                    for (int k = 0; k < NUM_PROCESSOR; ++ k)
                    {
                        if (i != k)
                        {
                            cache[k].state = INVALID;
                            cache[k].value = 0;
                        }
                    } 

                    cache[i].state = MODIFIED;
                    cache[i].value = write_value;
                    
                    #ifdef DEBUG
                    printf("[%d] write miss; boardcast invalid to S; update to value %d\n", i, cache[i].value);
                    #endif
                    return 1;
                }
            }
        }

        // all other are invalid
        // write allcoate
        cache[i].value = mem_value;
        cache[i].state = MODIFIED;
        cache[i].value = write_value;

        #ifdef DEBUG
        printf("[%d] write miss; all invalid; update to value %d\n", i, cache[i].value);
        #endif

        return 1;
    }

    return 0;
}

// i - the index of processor
// int return - if this event is related with target physical address
int evict_cacheline(int i)
{
    if (cache[i].state == MODIFIED)
    {
        // write back
        mem_value = cache[i].value;
        cache[i].state = INVALID;
        cache[i].value = 0;
        
        #ifdef DEBUG
        printf("[%d] evict; write back value %d\n", i, cache[i].value);
        #endif

        return 1;
    }
    else if (cache[i].state == EXCLUSIVE)
    {
        cache[i].state = INVALID;
        cache[i].value = 0;
        
        #ifdef DEBUG
        printf("[%d] evict\n", i);
        #endif

        return 1;
    }
    else if (cache[i].state == SHARED)
    {
        cache[i].state = INVALID;
        cache[i].value = 0;

        // may left only one shared to be exclusive
        int s_count = 0;
        int last_s = -1;

        for (int j = 0; j < NUM_PROCESSOR; ++ j)
        {
            if (cache[j].state == SHARED)
            {
                last_s = j;
                s_count ++;
            }
        }

        if (s_count == 1)
        {
            cache[last_s].state = EXCLUSIVE;
        }
        
        #ifdef DEBUG
        printf("[%d] evict\n", i);
        #endif

        return 1;
    }

    // evict when cache line is Invalid
    // not related with target physical address
    return 0;
}

void print_cacheline()
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
            c = '?';
        }
        
        printf("\t[%d]      state %c        value %d\n", i, c, cache[i].value);
    }
        printf("\t                          mem value %d\n", mem_value);
}

int main()
{
    srand(123456);

    int read_value;

    for (int i = 0; i < NUM_PROCESSOR; ++ i)
    {
        cache[i].state = INVALID;
        cache[i].value = 0;
    }

    #ifdef DEBUG
    print_cacheline();
    #endif

    for (int i = 0; i < 100000; ++ i)
    {
        int core_index = rand() % NUM_PROCESSOR;
        int op = rand() % 3;
    
        int do_print = 0;

        if (op == 0)
        {
            // printf("read [%d]\n", core_index);
            do_print = read_cacheline(core_index, &read_value);
        }
        else if (op == 1)
        {
            // printf("write [%d]\n", core_index);
            do_print = write_cacheline(core_index, rand() % 1000);
        }
        else if (op == 2)
        {
            // printf("evict [%d]\n", core_index);
            do_print = evict_cacheline(core_index);
        }

        #ifdef DEBUG
        if (do_print)
        {
            print_cacheline();
        }
        #endif

        if (check_state() == 0)
        {
            printf("failed\n");

            return 0;
        }
    }

    printf("pass\n");

    return 0;

}