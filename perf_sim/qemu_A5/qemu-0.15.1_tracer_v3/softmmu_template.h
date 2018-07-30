/*
 *  Software MMU support
 *
 *  Copyright (c) 2003 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#include "qemu-timer.h"

#ifdef CONFIG_TRACE //wswu
#include "trace_timestamp.h"
extern FILE* trace_log_file;
extern int   start_tracing;
extern uint64_t sim_time;
extern uint64_t inst_count;
#endif

#define DATA_SIZE (1 << SHIFT)

#if DATA_SIZE == 8
#define SUFFIX q
#define USUFFIX q
#define DATA_TYPE uint64_t
#elif DATA_SIZE == 4
#define SUFFIX l
#define USUFFIX l
#define DATA_TYPE uint32_t
#elif DATA_SIZE == 2
#define SUFFIX w
#define USUFFIX uw
#define DATA_TYPE uint16_t
#elif DATA_SIZE == 1
#define SUFFIX b
#define USUFFIX ub
#define DATA_TYPE uint8_t
#else
#error unsupported data size
#endif

#ifdef SOFTMMU_CODE_ACCESS
#define READ_ACCESS_TYPE 2
#define ADDR_READ addr_code
#else
#define READ_ACCESS_TYPE 0
#define ADDR_READ addr_read
#endif

static DATA_TYPE glue(glue(slow_ld, SUFFIX), MMUSUFFIX)(target_ulong addr,
                                                        int mmu_idx,
                                                        void *retaddr);
static inline DATA_TYPE glue(io_read, SUFFIX)(target_phys_addr_t physaddr,
                                              target_ulong addr,
                                              void *retaddr)
{
    DATA_TYPE res;
    int index;
    index = (physaddr >> IO_MEM_SHIFT) & (IO_MEM_NB_ENTRIES - 1);
    physaddr = (physaddr & TARGET_PAGE_MASK) + addr;
    env->mem_io_pc = (unsigned long)retaddr;
    if (index > (IO_MEM_NOTDIRTY >> IO_MEM_SHIFT)
            && !can_do_io(env)) {
        cpu_io_recompile(env, retaddr);
    }

    env->mem_io_vaddr = addr;
#if SHIFT <= 2
    res = io_mem_read[index][SHIFT](io_mem_opaque[index], physaddr);
#else
#ifdef TARGET_WORDS_BIGENDIAN
    res = (uint64_t)io_mem_read[index][2](io_mem_opaque[index], physaddr) << 32;
    res |= io_mem_read[index][2](io_mem_opaque[index], physaddr + 4);
#else
    res = io_mem_read[index][2](io_mem_opaque[index], physaddr);
    res |= (uint64_t)io_mem_read[index][2](io_mem_opaque[index], physaddr + 4) << 32;
#endif
#endif /* SHIFT > 2 */
    return res;
}

/* handle all cases except unaligned access which span two pages */
DATA_TYPE REGPARM glue(glue(__ld, SUFFIX), MMUSUFFIX)(target_ulong addr,
                                                      int mmu_idx)
{
    DATA_TYPE res;
    int index;
    target_ulong tlb_addr;
    target_phys_addr_t ioaddr;
    unsigned long addend;
    void *retaddr;

    /* test if there is match for unaligned or IO access */
    /* XXX: could done more in memory macro in a non portable way */
    index = (addr >> TARGET_PAGE_BITS) & (CPU_TLB_SIZE - 1);
 redo:
    tlb_addr = env->tlb_table[mmu_idx][index].ADDR_READ;
    if ((addr & TARGET_PAGE_MASK) == (tlb_addr & (TARGET_PAGE_MASK | TLB_INVALID_MASK))) {
        if (tlb_addr & ~TARGET_PAGE_MASK) {
            /* IO access */
            if ((addr & (DATA_SIZE - 1)) != 0)
                goto do_unaligned_access;
            retaddr = GETPC();
            ioaddr = env->iotlb[mmu_idx][index];
            res = glue(io_read, SUFFIX)(ioaddr, addr, retaddr);
#ifdef CONFIG_TRACE_SOFT_IO//wswu
		if(READ_ACCESS_TYPE==0&&start_tracing){//data access read   2->code access
			uint32_t gaddr;
			int prot,ret;
			target_ulong page_size;
			ret=get_phys_addr(env,addr,READ_ACCESS_TYPE,mmu_idx,&gaddr,&prot,&page_size);
			if(!ret){
				//load data (cpu id) (tb pc:0) (phys addr) (mem size:1 2 4 8)
#ifdef CONFIG_TRACEFILE
				fprintf(trace_log_file,"2 %d %u %u %d\n",env->cpu_index,0,gaddr,DATA_SIZE);
#endif
#ifdef CONFIG_SHMEM
				my_trace_log(2);
				my_trace_log(env->cpu_index);
				my_trace_log(0);
				my_trace_log(gaddr);
				my_trace_log(DATA_SIZE);
#endif
#ifdef CONFIG_TRACE_TIME
				sim_time += env->sim_time; 
				env->sim_time = 0;
				//timestamp (cpu id) (timstamp)
#ifdef CONFIG_TRACEFILE
				fprintf(trace_log_file,"4 %d %u\n",env->cpu_index,sim_time);
#endif
#ifdef CONFIG_SHMEM
				my_trace_log(4);
				my_trace_log(env->cpu_index);
				my_trace_log(sim_time);
#endif
#endif
			}
		}
#endif//wswu end
        } else if (((addr & ~TARGET_PAGE_MASK) + DATA_SIZE - 1) >= TARGET_PAGE_SIZE) {
            /* slow unaligned access (it spans two pages or IO) */
        do_unaligned_access:
            retaddr = GETPC();
#ifdef ALIGNED_ONLY
            do_unaligned_access(addr, READ_ACCESS_TYPE, mmu_idx, retaddr);
#endif
            res = glue(glue(slow_ld, SUFFIX), MMUSUFFIX)(addr,
                                                         mmu_idx, retaddr);
        } else {
            /* unaligned/aligned access in the same page */
#ifdef ALIGNED_ONLY
            if ((addr & (DATA_SIZE - 1)) != 0) {
                retaddr = GETPC();
                do_unaligned_access(addr, READ_ACCESS_TYPE, mmu_idx, retaddr);
            }
#endif
            addend = env->tlb_table[mmu_idx][index].addend;

			//printf("ld phy addr:0x%016lx \n", addr+addend); //sue7785

			res = glue(glue(ld, USUFFIX), _raw)((uint8_t *)(long)(addr+addend));
#ifdef CONFIG_TRACE_SOFT//wswu
		if(READ_ACCESS_TYPE==0&&start_tracing){//data access read   2->code access
			uint32_t gaddr;
			int prot,ret;
			target_ulong page_size;
			ret=get_phys_addr(env,addr,READ_ACCESS_TYPE,mmu_idx,&gaddr,&prot,&page_size);
			if(!ret){
				//load data (cpu id) (tb pc:0) (phys addr) (mem size:1 2 4 8)
#ifdef CONFIG_TRACEFILE
				fprintf(trace_log_file,"2 %d %u %u %d\n",env->cpu_index,0,gaddr,DATA_SIZE);
#endif
#ifdef CONFIG_SHMEM
				my_trace_log(2);
				my_trace_log(env->cpu_index);
				my_trace_log(0);
				my_trace_log(gaddr);
				my_trace_log(DATA_SIZE);
#endif
#ifdef CONFIG_TRACE_TIME
				sim_time += env->sim_time; 
				env->sim_time = 0;
				//timestamp (cpu id) (timstamp)
#ifdef CONFIG_TRACEFILE
				fprintf(trace_log_file,"4 %d %u\n",env->cpu_index,sim_time);
#endif
#ifdef CONFIG_SHMEM
				my_trace_log(4);
				my_trace_log(env->cpu_index);
				my_trace_log(sim_time);
#endif
#endif
			}
		}
#endif//wswu end
        }
    } else {
        /* the page is not in the TLB : fill it */
        retaddr = GETPC();
#ifdef ALIGNED_ONLY
        if ((addr & (DATA_SIZE - 1)) != 0)
            do_unaligned_access(addr, READ_ACCESS_TYPE, mmu_idx, retaddr);
#endif
        tlb_fill(addr, READ_ACCESS_TYPE, mmu_idx, retaddr);
        goto redo;
    }
    return res;
}

/* handle all unaligned cases */
static DATA_TYPE glue(glue(slow_ld, SUFFIX), MMUSUFFIX)(target_ulong addr,
                                                        int mmu_idx,
                                                        void *retaddr)
{
    DATA_TYPE res, res1, res2;
    int index, shift;
    target_phys_addr_t ioaddr;
    unsigned long addend;
    target_ulong tlb_addr, addr1, addr2;

    index = (addr >> TARGET_PAGE_BITS) & (CPU_TLB_SIZE - 1);
 redo:
    tlb_addr = env->tlb_table[mmu_idx][index].ADDR_READ;
    if ((addr & TARGET_PAGE_MASK) == (tlb_addr & (TARGET_PAGE_MASK | TLB_INVALID_MASK))) {
        if (tlb_addr & ~TARGET_PAGE_MASK) {
            /* IO access */
            if ((addr & (DATA_SIZE - 1)) != 0)
                goto do_unaligned_access;
            ioaddr = env->iotlb[mmu_idx][index];
            res = glue(io_read, SUFFIX)(ioaddr, addr, retaddr);
        } else if (((addr & ~TARGET_PAGE_MASK) + DATA_SIZE - 1) >= TARGET_PAGE_SIZE) {
        do_unaligned_access:
            /* slow unaligned access (it spans two pages) */
            addr1 = addr & ~(DATA_SIZE - 1);
            addr2 = addr1 + DATA_SIZE;
            res1 = glue(glue(slow_ld, SUFFIX), MMUSUFFIX)(addr1,
                                                          mmu_idx, retaddr);
            res2 = glue(glue(slow_ld, SUFFIX), MMUSUFFIX)(addr2,
                                                          mmu_idx, retaddr);
            shift = (addr & (DATA_SIZE - 1)) * 8;
#ifdef TARGET_WORDS_BIGENDIAN
            res = (res1 << shift) | (res2 >> ((DATA_SIZE * 8) - shift));
#else
            res = (res1 >> shift) | (res2 << ((DATA_SIZE * 8) - shift));
#endif
            res = (DATA_TYPE)res;
        } else {
            /* unaligned/aligned access in the same page */
            addend = env->tlb_table[mmu_idx][index].addend;
			
			//printf("ld phy addr:0x%016lx \n", addr+addend); //sue7785

			res = glue(glue(ld, USUFFIX), _raw)((uint8_t *)(long)(addr+addend));
        }
    } else {
        /* the page is not in the TLB : fill it */
        tlb_fill(addr, READ_ACCESS_TYPE, mmu_idx, retaddr);
        goto redo;
    }
    return res;
}

#ifndef SOFTMMU_CODE_ACCESS

static void glue(glue(slow_st, SUFFIX), MMUSUFFIX)(target_ulong addr,
                                                   DATA_TYPE val,
                                                   int mmu_idx,
                                                   void *retaddr);

static inline void glue(io_write, SUFFIX)(target_phys_addr_t physaddr,
                                          DATA_TYPE val,
                                          target_ulong addr,
                                          void *retaddr)
{
    int index;
    index = (physaddr >> IO_MEM_SHIFT) & (IO_MEM_NB_ENTRIES - 1);
    physaddr = (physaddr & TARGET_PAGE_MASK) + addr;
    if (index > (IO_MEM_NOTDIRTY >> IO_MEM_SHIFT)
            && !can_do_io(env)) {
        cpu_io_recompile(env, retaddr);
    }

    env->mem_io_vaddr = addr;
    env->mem_io_pc = (unsigned long)retaddr;
#if SHIFT <= 2
    io_mem_write[index][SHIFT](io_mem_opaque[index], physaddr, val);
#else
#ifdef TARGET_WORDS_BIGENDIAN
    io_mem_write[index][2](io_mem_opaque[index], physaddr, val >> 32);
    io_mem_write[index][2](io_mem_opaque[index], physaddr + 4, val);
#else
    io_mem_write[index][2](io_mem_opaque[index], physaddr, val);
    io_mem_write[index][2](io_mem_opaque[index], physaddr + 4, val >> 32);
#endif
#endif /* SHIFT > 2 */
}

void REGPARM glue(glue(__st, SUFFIX), MMUSUFFIX)(target_ulong addr,
                                                 DATA_TYPE val,
                                                 int mmu_idx)
{
    target_phys_addr_t ioaddr;
    unsigned long addend;
    target_ulong tlb_addr;
    void *retaddr;
    int index;

    index = (addr >> TARGET_PAGE_BITS) & (CPU_TLB_SIZE - 1);
 redo:
    tlb_addr = env->tlb_table[mmu_idx][index].addr_write;
    if ((addr & TARGET_PAGE_MASK) == (tlb_addr & (TARGET_PAGE_MASK | TLB_INVALID_MASK))) {
        if (tlb_addr & ~TARGET_PAGE_MASK) {
            /* IO access */
            if ((addr & (DATA_SIZE - 1)) != 0)
                goto do_unaligned_access;
            retaddr = GETPC();
            ioaddr = env->iotlb[mmu_idx][index];
            glue(io_write, SUFFIX)(ioaddr, val, addr, retaddr);
#ifdef CONFIG_TRACE_SOFT_IO//wswu
                if(start_tracing){//data access write   2->code access  READ_ACCESS_TYPE==1&&
                        uint32_t gaddr;
                        int prot,ret;
                        target_ulong page_size;
                        ret=get_phys_addr(env,addr,READ_ACCESS_TYPE,mmu_idx,&gaddr,&prot,&page_size);
                        if(!ret){
                                //store data (cpu id) (tb pc:0) (phys addr) (mem size:1 2 4 8)
#ifdef CONFIG_TRACEFILE
                                fprintf(trace_log_file,"3 %d %u %u %d\n",env->cpu_index,0,gaddr,DATA_SIZE);
#endif
#ifdef CONFIG_SHMEM
                                my_trace_log(3);
                                my_trace_log(env->cpu_index);
                                my_trace_log(0);
                                my_trace_log(gaddr);
                                my_trace_log(DATA_SIZE);
#endif
#ifdef CONFIG_TRACE_TIME
                                sim_time += env->sim_time;
                                env->sim_time = 0;
                                //timestamp (cpu id) (timstamp)
#ifdef CONFIG_TRACEFILE
                                fprintf(trace_log_file,"4 %d %u\n",env->cpu_index,sim_time);
#endif
#ifdef CONFIG_SHMEM
                                my_trace_log(4);
                                my_trace_log(env->cpu_index);
                                my_trace_log(sim_time);
#endif
#endif
                        }
                }
#endif//wswu end
        } else if (((addr & ~TARGET_PAGE_MASK) + DATA_SIZE - 1) >= TARGET_PAGE_SIZE) {
        do_unaligned_access:
            retaddr = GETPC();
#ifdef ALIGNED_ONLY
            do_unaligned_access(addr, 1, mmu_idx, retaddr);
#endif
            glue(glue(slow_st, SUFFIX), MMUSUFFIX)(addr, val,
                                                   mmu_idx, retaddr);
        } else {
            /* aligned/unaligned access in the same page */
#ifdef ALIGNED_ONLY
            if ((addr & (DATA_SIZE - 1)) != 0) {
                retaddr = GETPC();
                do_unaligned_access(addr, 1, mmu_idx, retaddr);
            }
#endif
            addend = env->tlb_table[mmu_idx][index].addend;
            
			//printf("st phy addr:0x%016lx \n", addr+addend); //sue7785

			glue(glue(st, SUFFIX), _raw)((uint8_t *)(long)(addr+addend), val);
#ifdef CONFIG_TRACE_SOFT//wswu
                if(start_tracing){//data access write   2->code access  READ_ACCESS_TYPE==1&&
                        uint32_t gaddr;
                        int prot,ret;
                        target_ulong page_size;
                        ret=get_phys_addr(env,addr,READ_ACCESS_TYPE,mmu_idx,&gaddr,&prot,&page_size);
                        if(!ret){
                                //store data (cpu id) (tb pc:0) (phys addr) (mem size:1 2 4 8)
#ifdef CONFIG_TRACEFILE
                                fprintf(trace_log_file,"3 %d %u %u %d\n",env->cpu_index,0,gaddr,DATA_SIZE);
#endif
#ifdef CONFIG_SHMEM
                                my_trace_log(3);
                                my_trace_log(env->cpu_index);
                                my_trace_log(0);
                                my_trace_log(gaddr);
                                my_trace_log(DATA_SIZE);
#endif
#ifdef CONFIG_TRACE_TIME
                                sim_time += env->sim_time;
                                env->sim_time = 0;
                                //timestamp (cpu id) (timstamp)
#ifdef CONFIG_TRACEFILE
                                fprintf(trace_log_file,"4 %d %u\n",env->cpu_index,sim_time);
#endif
#ifdef CONFIG_SHMEM
                                my_trace_log(4);
                                my_trace_log(env->cpu_index);
                                my_trace_log(sim_time);
#endif
#endif
                        }
                }
#endif//wswu end
        }
    } else {
        /* the page is not in the TLB : fill it */
        retaddr = GETPC();
#ifdef ALIGNED_ONLY
        if ((addr & (DATA_SIZE - 1)) != 0)
            do_unaligned_access(addr, 1, mmu_idx, retaddr);
#endif
        tlb_fill(addr, 1, mmu_idx, retaddr);
        goto redo;
    }
}

/* handles all unaligned cases */
static void glue(glue(slow_st, SUFFIX), MMUSUFFIX)(target_ulong addr,
                                                   DATA_TYPE val,
                                                   int mmu_idx,
                                                   void *retaddr)
{
    target_phys_addr_t ioaddr;
    unsigned long addend;
    target_ulong tlb_addr;
    int index, i;

    index = (addr >> TARGET_PAGE_BITS) & (CPU_TLB_SIZE - 1);
 redo:
    tlb_addr = env->tlb_table[mmu_idx][index].addr_write;
    if ((addr & TARGET_PAGE_MASK) == (tlb_addr & (TARGET_PAGE_MASK | TLB_INVALID_MASK))) {
        if (tlb_addr & ~TARGET_PAGE_MASK) {
            /* IO access */
            if ((addr & (DATA_SIZE - 1)) != 0)
                goto do_unaligned_access;
            ioaddr = env->iotlb[mmu_idx][index];
            glue(io_write, SUFFIX)(ioaddr, val, addr, retaddr);
        } else if (((addr & ~TARGET_PAGE_MASK) + DATA_SIZE - 1) >= TARGET_PAGE_SIZE) {
        do_unaligned_access:
            /* XXX: not efficient, but simple */
            /* Note: relies on the fact that tlb_fill() does not remove the
             * previous page from the TLB cache.  */
            for(i = DATA_SIZE - 1; i >= 0; i--) {
#ifdef TARGET_WORDS_BIGENDIAN
                glue(slow_stb, MMUSUFFIX)(addr + i, val >> (((DATA_SIZE - 1) * 8) - (i * 8)),
                                          mmu_idx, retaddr);
#else
                glue(slow_stb, MMUSUFFIX)(addr + i, val >> (i * 8),
                                          mmu_idx, retaddr);
#endif
            }
        } else {
            /* aligned/unaligned access in the same page */
            addend = env->tlb_table[mmu_idx][index].addend;
           
			//printf("st phy addr:0x%016lx \n", addr+addend); //sue7785
 
			glue(glue(st, SUFFIX), _raw)((uint8_t *)(long)(addr+addend), val);
        }
    } else {
        /* the page is not in the TLB : fill it */
        tlb_fill(addr, 1, mmu_idx, retaddr);
        goto redo;
    }
}

#endif /* !defined(SOFTMMU_CODE_ACCESS) */

#undef READ_ACCESS_TYPE
#undef SHIFT
#undef DATA_TYPE
#undef SUFFIX
#undef USUFFIX
#undef DATA_SIZE
#undef ADDR_READ
