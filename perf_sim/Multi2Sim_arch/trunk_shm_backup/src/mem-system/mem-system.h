/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MEM_SYSTEM_H
#define MEM_SYSTEM_H

#define throttle_debug(...) debug(throttle_debug_category, __VA_ARGS__)
extern int throttle_debug_category;
#define pf_debug(...) debug(pf_debug_category, __VA_ARGS__)
extern int pf_debug_category;//cjc_20121210 pf_debug
extern int dram_debug_category;//cjc_20130317 dram_debug
#define dram_debug(...) debug(dram_debug_category, __VA_ARGS__)
//IC_S_2013_07_29
extern int dram_log_category;
#define dram_log(...) debug(dram_log_category, __VA_ARGS__)
//IC_E
//IC_S_2013_09_24
//FILE *fp;
//IC_E
extern int cache_debug_category;
#define cache_debug(...) debug(cache_debug_category, __VA_ARGS__)

extern char *mem_report_file_name;
extern char *mem_profiling_file_name; //porshe:20130220 mem profile report
#define mem_debugging() debug_status(mem_debug_category)
#define mem_debug(...) debug(mem_debug_category, __VA_ARGS__)
extern int mem_debug_category;

//IC_S_2013_10_29
//#define mem_debug_2(...) debug(mem_debug_category_2, __VA_ARGS__)
#define mem_debug_2(...) debug(mem_debug_2_start, __VA_ARGS__)
extern int mem_debug_category_2;
extern int mem_debug_2_start;
//IC_E

#define mem_tracing() trace_status(mem_trace_category)
#define mem_trace(...) trace(mem_trace_category, __VA_ARGS__)
#define mem_trace_header(...) trace_header(mem_trace_category, __VA_ARGS__)
extern int mem_trace_category;

extern int mem_system_peer_transfers;

struct mem_system_t
{
	/* List of modules and networks */
	struct list_t *mod_list;
	struct list_t *net_list;
};

extern struct mem_system_t *mem_system;


void mem_system_init(void);
void mem_system_done(void);

void mem_system_dump_report(FILE *f);
void mem_system_profiling_report(FILE *f); //porshe:20130220 profiling report dump

struct mod_t *mem_system_get_mod(char *mod_name);
struct net_t *mem_system_get_net(char *net_name);

extern struct mod_t * memory_mod;


#endif

