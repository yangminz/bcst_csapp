/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include "headers/allocator.h"

static int explicit_list_heap_init();
static uint64_t explicit_list_mem_alloc(uint32_t size);
static void explicit_list_mem_free(uint64_t payload_vaddr);

/* ------------------------------------- */
/*  Implementation of the Interfaces     */
/* ------------------------------------- */

#ifdef EXPLICIT_FREE_LIST

int heap_init()
{
    return explicit_list_heap_init();
}

uint64_t mem_alloc(uint32_t size)
{
    return explicit_list_mem_alloc(size);
}

void mem_free(uint64_t payload_vaddr)
{
    explicit_list_mem_free(payload_vaddr);
}

#endif

/* ------------------------------------- */
/*  Explicit Free List                   */
/* ------------------------------------- */

/*  Allocated block:

    ff ff ff f9/f1  [8n + 24] - footer
    ?? ?? ?? ??     [8n + 20] - padding
    xx xx ?? ??     [8n + 16] - payload & padding
    xx xx xx xx     [8n + 12] - payload
    xx xx xx xx     [8n + 8] - payload
    hh hh hh h9/h1  [8n + 4] - header

    Free block:

    ff ff ff f8/f0  [8n + 24] - footer
    ?? ?? ?? ??     [8n + 20]
    ?? ?? ?? ??     [8n + 16]
    nn nn nn nn     [8n + 12] - next free block address
    pp pp pp pp     [8n + 8] - previous free block address
    hh hh hh h8/h0  [8n + 4] - header
*/
#define MIN_EXPLICIT_FREE_LIST_BLOCKSIZE (16)

static uint64_t explicit_list_head = NIL;
static uint32_t explicit_list_counter = 0;

/* ------------------------------------- */
/*  Operations for List Block Structure  */
/* ------------------------------------- */

static uint64_t get_prevfree(uint64_t header_vaddr)
{
    return get_field32_block_ptr(header_vaddr, MIN_EXPLICIT_FREE_LIST_BLOCKSIZE, 4);
}

static uint64_t get_nextfree(uint64_t header_vaddr)
{
    return get_field32_block_ptr(header_vaddr, MIN_EXPLICIT_FREE_LIST_BLOCKSIZE, 8);
}

static void set_prevfree(uint64_t header_vaddr, uint64_t prev_vaddr)
{
    set_field32_block_ptr(header_vaddr, prev_vaddr, MIN_EXPLICIT_FREE_LIST_BLOCKSIZE, 4);
}

static void set_nextfree(uint64_t header_vaddr, uint64_t next_vaddr)
{
    set_field32_block_ptr(header_vaddr, next_vaddr, MIN_EXPLICIT_FREE_LIST_BLOCKSIZE, 8);
}

/* ------------------------------------- */
/*  Operations for Linked List           */
/* ------------------------------------- */

static void explicit_list_insert(uint64_t *head_vaddr, uint32_t *counter_ptr, uint64_t block)
{
    assert(get_firstblock() <= block && block <= get_lastblock());
    assert(block % 8 == 4);
    assert(get_blocksize(block) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

    if ((*head_vaddr) == NIL || (*counter_ptr) == 0)
    {
        assert((*head_vaddr) == NIL);
        assert((*counter_ptr) == 0);

        set_prevfree(block, block);
        set_nextfree(block, block);

        (*head_vaddr) = block;
        (*counter_ptr) = 1;

        return;
    }

    // list is not empty
    uint64_t head = (*head_vaddr);
    uint64_t tail = get_prevfree(head);

    set_nextfree(block, head);
    set_prevfree(head, block);
    set_nextfree(tail, block);
    set_prevfree(block, tail);

    (*head_vaddr) = block;
    (*counter_ptr) += 1;
}

static void explicit_list_delete(uint64_t *head_vaddr, uint32_t *counter_ptr, uint64_t block)
{
    assert(get_firstblock() <= block && block <= get_lastblock());
    assert(block % 8 == 4);
    assert(get_blocksize(block) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

    if ((*head_vaddr) == NIL || (*counter_ptr) == 0)
    {
        assert(explicit_list_head == NIL);
        assert((*counter_ptr) == 0);
        return;
    }

    if ((*counter_ptr) == 1)
    {
        assert(get_prevfree((*head_vaddr)) == (*head_vaddr));
        assert(get_nextfree((*head_vaddr)) == (*head_vaddr));

        (*head_vaddr) = NIL;
        (*counter_ptr) = 0;

        return;
    }

    // counter >= 2
    uint64_t prev = get_prevfree(block);
    uint64_t next = get_nextfree(block);

    if (block == (*head_vaddr))
    {
        (*head_vaddr) = next;
    }
    
    set_nextfree(prev, next);
    set_prevfree(next, prev);

    (*counter_ptr) -= 1;
}

/* ------------------------------------- */
/*  For Debugging                        */
/* ------------------------------------- */

static void explicit_list_print()
{
    uint64_t p = explicit_list_head;
    printf("explicit free list <{%lu},{%u}>:\n", explicit_list_head, explicit_list_counter);
    for (int i = 0; i < explicit_list_counter; ++ i)
    {
        printf("<%lu:%u/%u> ", p, get_blocksize(p), get_allocated(p));
        p = get_nextfree(p);
    }
    printf("\n");
}

#if defined(DEBUG_MALLOC) && defined(EXPLICIT_FREE_LIST)
void on_sigabrt(int signum)
{
    // like a try-catch for the asserts
    printf("%s\n", debug_message);
    print_heap();
    explicit_list_print();
    exit(0);
}
#endif

static void check_explicit_list_correctness()
{
    uint32_t free_counter = 0;
    uint64_t p = get_firstblock();
    while (p != NIL && p <= get_lastblock())
    {
        if (get_allocated(p) == FREE)
        {
            free_counter += 1;

            assert(get_blocksize(p) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);
            assert(get_allocated(get_nextfree(p)) == FREE);
            assert(get_allocated(get_prevfree(p)) == FREE);
        }

        p = get_nextheader(p);
    }
    assert(free_counter == explicit_list_counter);

    p = explicit_list_head;
    uint64_t n = explicit_list_head;
    for (int i = 0; i < explicit_list_counter; ++ i)
    {
        assert(get_allocated(p) == FREE);
        assert(get_blocksize(p) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

        assert(get_allocated(n) == FREE);
        assert(get_blocksize(n) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

        p = get_prevfree(p);
        n = get_nextfree(n);
    }
    assert(p == explicit_list_head);
    assert(n == explicit_list_head);
}

/* ------------------------------------- */
/*  Implementation                       */
/* ------------------------------------- */

static int explicit_list_heap_init()
{
    // reset all to 0
    for (int i = 0; i < HEAP_MAX_SIZE / 8; i += 8)
    {
        *(uint64_t *)&heap[i] = 0;
    }

    // heap_start_vaddr is the starting address of the first block
    // the payload of the first block is 8B aligned ([8])
    // so the header address of the first block is [8] - 4 = [4]
    heap_start_vaddr = 0;
    heap_end_vaddr = 4096;

    // set the prologue block
    uint64_t prologue_header = get_prologue();
    set_blocksize(prologue_header, 8);
    set_allocated(prologue_header, ALLOCATED);

    uint64_t prologue_footer = prologue_header + 4;
    set_blocksize(prologue_footer, 8);
    set_allocated(prologue_footer, ALLOCATED);

    // set the epilogue block
    // it's a footer only
    uint64_t epilogue = get_epilogue();
    set_blocksize(epilogue, 0);
    set_allocated(epilogue, ALLOCATED);

    // set the block size & allocated of the only regular block
    uint64_t first_header = get_firstblock();
    set_blocksize(first_header, 4096 - 4 - 8 - 4);
    set_allocated(first_header, FREE);

    uint64_t first_footer = get_footer(first_header);
    set_blocksize(first_footer, 4096 - 4 - 8 - 4);
    set_allocated(first_footer, FREE);

#ifdef DEBUG_MALLOC
    // like a try-catch
    signal(SIGABRT, &on_sigabrt);
#endif

    uint64_t first_block = get_firstblock();
    set_prevfree(first_block, first_block);
    set_nextfree(first_block, first_block);

    explicit_list_head = 0;
    explicit_list_counter = 0;
    explicit_list_insert(&explicit_list_head, &explicit_list_counter, first_block);

    return 1;
}

static uint64_t explicit_list_mem_alloc(uint32_t size)
{
    assert(0 < size && size < HEAP_MAX_SIZE - 4 - 8 - 4);

    uint64_t payload_vaddr = NIL;
    
    uint32_t request_blocksize = round_up(size, 8) + 4 + 4;
    request_blocksize = request_blocksize < MIN_EXPLICIT_FREE_LIST_BLOCKSIZE ?
        MIN_EXPLICIT_FREE_LIST_BLOCKSIZE : request_blocksize;

    uint64_t b = explicit_list_head;

    // not thread safe
    uint32_t counter_copy = explicit_list_counter;
    // T(explicit) <= 1/2 * T(implicit)
    // much more fast when the heap is nearly full
    for (int i = 0; i < counter_copy; ++ i)
    {
        uint32_t b_old_blocksize = get_blocksize(b);
        payload_vaddr = try_alloc_with_splitting(b, request_blocksize, MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

        if (payload_vaddr != NIL)
        {
            uint32_t b_new_blocksize = get_blocksize(b);
            assert(b_new_blocksize <= b_old_blocksize);
            explicit_list_delete(&explicit_list_head, &explicit_list_counter, b);

            if (b_old_blocksize > b_new_blocksize)
            {
                // b has been splitted
                uint64_t a = get_nextheader(b);
                assert(get_allocated(a) == FREE);
                assert(get_blocksize(a) == b_old_blocksize - b_new_blocksize);
                explicit_list_insert(&explicit_list_head, &explicit_list_counter, a);
            }

#ifdef DEBUG_MALLOC
            check_heap_correctness();
            check_explicit_list_correctness();
#endif
            return payload_vaddr;
        }
        else
        {
            // go to next block
            b = get_nextfree(b);
        }
    }

    // when no enough free block for current heap
    // request a new free physical & virtual page from OS
    uint64_t old_last = get_lastblock();
    if (get_allocated(old_last) == FREE)
    {
        explicit_list_delete(&explicit_list_head, &explicit_list_counter, old_last);
    }

    payload_vaddr = try_extend_heap_to_alloc(request_blocksize, MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

    uint64_t new_last = get_lastblock();
    if (get_allocated(new_last) == FREE)
    {
        explicit_list_insert(&explicit_list_head, &explicit_list_counter, new_last);
    }

#ifdef DEBUG_MALLOC
    check_heap_correctness();
    check_explicit_list_correctness();
#endif

    return payload_vaddr;
}

static void explicit_list_mem_free(uint64_t payload_vaddr)
{
    if (payload_vaddr == NIL)
    {
        return;
    }

    assert(get_firstblock() < payload_vaddr && payload_vaddr < get_epilogue());
    assert((payload_vaddr & 0x7) == 0x0);

    // request can be first or last block
    uint64_t req = get_header(payload_vaddr);
    uint64_t req_footer = get_footer(req); // for last block, it's 0

    uint32_t req_allocated = get_allocated(req);
    uint32_t req_blocksize = get_blocksize(req);
    assert(req_allocated == ALLOCATED); // otherwise it's free twice

    // block starting address of next & prev blocks
    uint64_t next = get_nextheader(req);    // for req last block, it's 0
    uint64_t prev = get_prevheader(req);    // for req first block, it's 0

    uint32_t next_allocated = get_allocated(next);  // for req last, 1
    uint32_t prev_allocated = get_allocated(prev);  // for req first, 1

    if (next_allocated == ALLOCATED && prev_allocated == ALLOCATED)
    {
        // case 1: *A(A->F)A*
        // ==> *AFA*
        set_allocated(req, FREE);
        set_allocated(req_footer, FREE);

        explicit_list_insert(&explicit_list_head, &explicit_list_counter, req);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
        check_explicit_list_correctness();
#endif
    }
    else if (next_allocated == FREE && prev_allocated == ALLOCATED)
    {
        // case 2: *A(A->F)FA
        // ==> *AFFA ==> *A[FF]A merge current and next
        explicit_list_delete(&explicit_list_head, &explicit_list_counter, next);

        uint64_t one_free  = merge_blocks_as_free(req, next);
        
        explicit_list_insert(&explicit_list_head, &explicit_list_counter, one_free);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
        check_explicit_list_correctness();
#endif
    }
    else if (next_allocated == ALLOCATED && prev_allocated == FREE)
    {
        // case 3: AF(A->F)A*
        // ==> AFFA* ==> A[FF]A* merge current and prev
        explicit_list_delete(&explicit_list_head, &explicit_list_counter, prev);

        uint64_t one_free  = merge_blocks_as_free(prev, req);
        
        explicit_list_insert(&explicit_list_head, &explicit_list_counter, one_free);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
        check_explicit_list_correctness();
#endif
    }
    else if (next_allocated == FREE && prev_allocated == FREE)
    {
        // case 4: AF(A->F)FA
        // ==> AFFFA ==> A[FFF]A merge current and prev and next
        explicit_list_delete(&explicit_list_head, &explicit_list_counter, prev);
        explicit_list_delete(&explicit_list_head, &explicit_list_counter, next);

        uint64_t one_free = merge_blocks_as_free(merge_blocks_as_free(prev, req), next);
        
        explicit_list_insert(&explicit_list_head, &explicit_list_counter, one_free);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
        check_explicit_list_correctness();
#endif
    }
    else
    {
#ifdef DEBUG_MALLOC
        printf("exception for free\n");
        exit(0);
#endif
    }
}