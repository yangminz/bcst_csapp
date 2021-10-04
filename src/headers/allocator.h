/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

// include guards to prevent double declaration of any identifiers 
// such as types, enums and variables
#ifndef MEM_ALLOCATOR_GUARD
#define MEM_ALLOCATOR_GUARD

#include <stdint.h>

// heap's bytes range:
// [heap_start_vaddr, heap_end_vaddr) or [heap_start_vaddr, heap_end_vaddr - 1]
// [0,1,2,3] - unused
// [4,5,6,7,8,9,10,11] - prologue block
// [12, ..., 4096 * n - 5] - regular blocks
// 4096 * n + [- 4, -3, -2, -1] - epilogue block (header only)
uint64_t heap_start_vaddr;
uint64_t heap_end_vaddr;

#define HEAP_MAX_SIZE (4096 * 8)
uint8_t heap[HEAP_MAX_SIZE];

#define FREE (0)
#define ALLOCATED (1)
#define NIL (0)

// to allocate one physical page for heap
uint32_t extend_heap(uint32_t size);
void os_syscall_brk();

// round up
uint64_t round_up(uint64_t x, uint64_t n);

// operations for all blocks
uint32_t get_blocksize(uint64_t header_vaddr);
void set_blocksize(uint64_t header_vaddr, uint32_t blocksize);

uint32_t get_allocated(uint64_t header_vaddr);
void set_allocated(uint64_t header_vaddr, uint32_t allocated);

uint64_t get_payload(uint64_t vaddr);
uint64_t get_header(uint64_t vaddr);
uint64_t get_footer(uint64_t vaddr);

// operations for heap linked list

uint64_t get_nextheader(uint64_t vaddr);
uint64_t get_prevheader(uint64_t vaddr);

uint64_t get_prologue();
uint64_t get_epilogue();

uint64_t get_firstblock();
uint64_t get_lastblock();

int is_lastblock(uint64_t vaddr);
int is_firstblock(uint64_t vaddr);

// for free block as data structure
uint64_t get_field32_block_ptr(uint64_t header_vaddr, uint32_t min_blocksize, uint32_t offset);
void set_field32_block_ptr(uint64_t header_vaddr, uint64_t block_ptr, uint32_t min_blocksize, uint32_t offset);

// for debugging
void check_heap_correctness();
void check_freeblock_correctness();

// implemented by different strategy
int initialize_free_block();
uint64_t search_free_block(uint32_t payload_size, uint32_t *alloc_blocksize);
int insert_free_block(uint64_t free_header);
int delete_free_block(uint64_t free_header);

// interface
int heap_init();
uint64_t mem_alloc(uint32_t size);
void mem_free(uint64_t payload_vaddr);

#endif