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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "headers/cpu.h"
#include "headers/process.h"
#include "headers/interrupt.h"
#include "headers/algorithm.h"
#include "headers/address.h"

int allocate_physicalframe(pte4_t *pte);

// the implementation of VMA list interface
static uint64_t construct_vma_node()
{
    vm_area_t *vma = KERNEL_malloc(sizeof(vm_area_t));
    vma->filepath[0] = '\0';
    vma->next = vma;
    vma->prev = vma;
    vma->vma_start = 0;
    vma->vma_end = 0;
    vma->mode_value = 0;

    return (uint64_t)vma;
}

static int destruct_vma_node(uint64_t vma_addr)
{
    vm_area_t *vma = (vm_area_t *)vma_addr;
    KERNEL_free(vma);
    return 0;
}

static int is_null_vma_node(uint64_t vma_addr)
{
    return vma_addr == (uint64_t)NULL;
}

static int compare_vma_nodes(uint64_t a, uint64_t b)
{
    vm_area_t *vma = (vm_area_t *)a;
    vm_area_t *vmb = (vm_area_t *)b;

    int same = (strcmp(vma->filepath, vmb->filepath) == 0);
    same = same && (vma->vma_start == vmb->vma_start);
    same = same && (vma->vma_end == vmb->vma_end);
    same = same && (vma->mode_value == vmb->mode_value);
    return same;
}

static uint64_t get_prev_vma(uint64_t vma_addr)
{
    vm_area_t *vma = (vm_area_t *)vma_addr;
    if (vma != NULL)
    {
        return (uint64_t)vma->prev;
    }

    return (uint64_t)NULL;
}

static int set_prev_vma(uint64_t vma_addr, uint64_t vma_prev_addr)
{
    vm_area_t *vma = (vm_area_t *)vma_addr;
    if (vma != NULL)
    {
        vma->prev = (vm_area_t *)vma_prev_addr;
        return 1;
    }

    return 0;
}

static uint64_t get_next_vma(uint64_t vma_addr)
{
    vm_area_t *vma = (vm_area_t *)vma_addr;
    if (vma != NULL)
    {
        return (uint64_t)vma->next;
    }

    return (uint64_t)NULL;
}

static int set_next_vma(uint64_t vma_addr, uint64_t vma_next_addr)
{
    vm_area_t *vma = (vm_area_t *)vma_addr;
    if (vma != NULL)
    {
        vma->next = (vm_area_t *)vma_next_addr;
        return 1;
    }

    return 0;
}

static uint64_t get_vma_value(uint64_t vma_addr)
{
    // bypass this function
    return 0;
}

static int set_vma_value(uint64_t vma_addr, uint64_t vma_value)
{
    // bypass this function
    return 1;
}

// fill in the interface
static linkedlist_node_interface vma_interface = {
    .construct_node = construct_vma_node,
    .destruct_node = destruct_vma_node,
    .is_null_node = is_null_vma_node,
    .compare_nodes = compare_vma_nodes,
    .get_node_prev = get_prev_vma,
    .set_node_prev = set_prev_vma,
    .get_node_next = get_prev_vma,
    .set_node_next = set_next_vma,
    .get_node_value = get_vma_value,
    .set_node_value = set_vma_value
};

static int vmalist_update_head(linkedlist_internal_t *this, uint64_t new_head)
{
    assert(this != NULL);

    if (this->count == 0)
    {
        this->count = 1;
    }
    this->head = new_head;
    return 1;
}

static int can_vmas_merge(vm_area_t *prev, vm_area_t *next)
{
    assert(prev != NULL && next != NULL);
    assert(prev->vma_start < next->vma_start);
    assert(prev->vma_end < next->vma_start);
    return strcmp(prev->filepath, next->filepath) == 0 &&
        prev->mode_value == next->mode_value &&
        prev->vma_end == next->vma_start;
}

static void check_vm_areas(pcb_t *proc)
{
    assert(proc != NULL);
    // usually the first area is .text, code area
    vm_area_t *head = (vm_area_t *)proc->mm.vma.head;
    vm_area_t *prev = head;
    vm_area_t *next = head->next;
    
    assert((prev->vma_start % PAGE_SIZE) == 0);
    assert((prev->vma_end % PAGE_SIZE) == 0);
    assert(prev->vma_start < prev->vma_end);
    assert((prev->vma_end - prev->vma_start) % PAGE_SIZE == 0);

    for (int i = 1; i < proc->mm.vma.count; ++ i)
    {
        if (strcmp(prev->filepath, next->filepath) == 0 &&
            prev->mode_value == next->mode_value)
        {
            assert(prev->vma_end < next->vma_start);
        }
        else
        {
            assert(prev->vma_end <= next->vma_start);
        }

        assert((next->vma_start % PAGE_SIZE) == 0);
        assert((next->vma_end % PAGE_SIZE) == 0);
        assert(next->vma_start < next->vma_end);
        assert((next->vma_end - next->vma_start) % PAGE_SIZE == 0);
    }
}

// exposed interface
// TODO: bad implementation. Maybe refactor it later.
int vma_add_area(pcb_t *proc, vm_area_t *area)
{
    assert(proc != NULL);

#ifdef VMA_DEBUG
    if (proc->mm.vma.count > 0)
    {
        check_vm_areas(proc);
    }
#endif
    
    // need to insert the node in sequence
    proc->mm.vma.update_head = vmalist_update_head;

    if (proc->mm.vma.count == 0)
    {
        linkedlist_internal_insert(&(proc->mm.vma),
            &vma_interface, (uint64_t)area);
        return 1;
    }
    else
    {
        // search the node and insert behind
        vm_area_t *head = (vm_area_t *)proc->mm.vma.head;
        assert(area->vma_start != head->vma_start);
            
        // search for the correct vma
        vm_area_t *p = head;
        int count = proc->mm.vma.count;

        // note that vma should be SORTED
        for (int i = 0; i < count; ++ i)
        {
            if (area->vma_start < p->vma_start)
            {
                // check if vma and a can merge
                if (can_vmas_merge(area, p) == 1)
                {
                    // we just enlarge a to vma
                    // [vma], [a]
                    // [a-------]
                    p->vma_start = area->vma_start;
                    return 1;
                }
                else
                {
                    // insert a new vma node
                    linkedlist_internal_insert_before(&(proc->mm.vma),
                        &vma_interface, (uint64_t)p, (uint64_t)area);
                    return 1;
                }
            }

            p = p->next;
        }

        // list.end.start < vma.start
        if (can_vmas_merge(p, area) == 1)
        {
            p->vma_end = area->vma_end;
            return 1;
        }
        else
        {
            linkedlist_internal_insert_after(&(proc->mm.vma),
                &vma_interface, (uint64_t)p, (uint64_t)area);
            return 1;
        }
    }

    return 0;
}

static pte4_t *create_pagetable(pte123_t *pt, uint64_t *vpns, int level)
{
    assert(pt != NULL);

    if (level == 4)
    {
        pte4_t *pte4 = (pte4_t *)(&pt[vpns[3]]);
        return pte4;
    }

    pte123_t *pte = &(pt[vpns[level - 1]]);
    if (pte->present == 1)
    {
        // hit, no need to malloc for next level page
        uint64_t pt_next = pte->paddr;
        pte->paddr = (uint64_t)pt_next;
        return create_pagetable((pte123_t *)pt_next, vpns, level + 1);
    }
    else
    {
        pte123_t *newpt_next = KERNEL_malloc(PAGE_TABLE_ENTRY_NUM * sizeof(pte123_t));
        pte->present = 1;
        pte->paddr = (uint64_t)newpt_next;
        return create_pagetable(newpt_next, vpns, level + 1);
    }

    return NULL;
}

// This function should be called during memory loading from
// executable file into memory. The loading should load the 
// sections from executable, and build process control block
// and build virtual memory areas based on the sections.
// Then build the page table from the virtual memory areas.
void setup_pagetable_from_vma(pcb_t *proc)
{
    // need to insert the node in sequence
    assert(proc != NULL);

#ifdef VMA_DEBUG
    check_vm_areas(proc);
#endif

    if (proc->mm.pgd == NULL)
    {
        proc->mm.pgd = KERNEL_malloc(PAGE_TABLE_ENTRY_NUM * sizeof(pte123_t));
    }

    vm_area_t *a = (vm_area_t *)(proc->mm.vma.head);
    for (int i = 0; i < proc->mm.vma.count; ++ i)
    {
        uint64_t readonly = (a->vma_mode.read == 1) && (a->vma_mode.write == 0);

        uint64_t page_num = (a->vma_end - a->vma_start) / PAGE_SIZE;

        for (int j = 0; j < page_num; ++ j)
        {
            uint64_t vpn1234 = a->vma_start + j * PAGE_SIZE;
            assert((vpn1234 % PAGE_SIZE) == 0);

            address_t vaddr = {.address_value = vpn1234};
            uint64_t vpns[4] = {vaddr.vpn1, vaddr.vpn2, vaddr.vpn3, vaddr.vpn4};

            pte4_t *pte4 = create_pagetable(proc->mm.pgd, vpns, 1);
            pte4->present = 1;
            pte4->readonly = readonly;
            allocate_physicalframe(pte4);
        }

        // move to next area
        a = a->next;
    }
}

// Actually, this function should be implemented by Red-Black Tree
vm_area_t *search_vma_vaddr(pcb_t *p, uint64_t vaddr)
{
    assert(p != NULL);

    vm_area_t *a = (vm_area_t *)(p->mm.vma.head);
    for (size_t i = 0; i < p->mm.vma.count; i++)
    {
        if (a->vma_start <= vaddr && vaddr < a->vma_end)
        {
            return a;
        }
        a = a->next;
    }
    return NULL;
}