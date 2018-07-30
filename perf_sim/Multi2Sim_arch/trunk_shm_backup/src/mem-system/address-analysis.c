

#include <assert.h>

#include <lib/mhandle/mhandle.h>

#include <lib/esim/esim.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>

#include <arch/x86/timing/cpu.h>

#include "address-analysis.h"

struct linked_list_t addr_anal_page_list;
char *addr_anal_file_name="";

void addr_anal_insert(int which_core,int addr, int pc){
	
	unsigned int page_index,item_index;	
	struct addr_anal_pages * temp_page;
	if(which_core<0||which_core>=x86_cpu_num_cores)
		fatal("Invalid Core-id\n");
	
	page_index = addr>>(ADDR_ANAL_LOG_PAGE_SIZE);
	item_index = addr&((1<<ADDR_ANAL_LOG_PAGE_SIZE)-1);

	
	if(linked_list_count(&addr_anal_page_list)==0)
	{
		linked_list_insert(&addr_anal_page_list,addr_anal_page_create(page_index));		
		linked_list_insert(&(((struct addr_anal_pages*)linked_list_get(&addr_anal_page_list))->entry[which_core].item_list),addr_anal_item_create(item_index,esim_cycle,pc));
	}
	else
	{	
		LINKED_LIST_FOR_EACH(&addr_anal_page_list)
		{
			temp_page = linked_list_get(&addr_anal_page_list);
			if(temp_page->page_index == page_index)
			{
				linked_list_insert(&(temp_page->entry[which_core].item_list),addr_anal_item_create(item_index,esim_cycle,pc));
				return;
			}
		}
		linked_list_insert(&addr_anal_page_list,addr_anal_page_create(page_index));
                linked_list_insert(&(((struct addr_anal_pages*)linked_list_get(&addr_anal_page_list))->entry[which_core].item_list),addr_anal_item_create(item_index,esim_cycle,pc));
	}

}

void addr_anal_dump(FILE *f)
{
	int i;
	struct addr_anal_pages * cur_page;
	struct addr_anal_items * cur_item;
	
	LINKED_LIST_FOR_EACH(&addr_anal_page_list)
	{	
		cur_page = linked_list_get(&addr_anal_page_list);
		for( i = 0; i < x86_cpu_num_cores; i++)
		{
			LINKED_LIST_FOR_EACH(&(cur_page->entry[i].item_list))
			{
				cur_item = linked_list_get(&(cur_page->entry[i].item_list));
				fprintf(f,"page_index=%ud core_id=%d addr=%d time=%lld pc=%d\n",cur_page->page_index,i,cur_item->index,cur_item->timestamp,cur_item->pc);
			}
		}
	}
}

void addr_anal_free()
{
	int i;
        struct addr_anal_pages * cur_page;
	
	LINKED_LIST_FOR_EACH(&addr_anal_page_list)
        {
		cur_page = linked_list_get(&addr_anal_page_list);
                for( i = 0; i < x86_cpu_num_cores; i++)
                {
			linked_list_clear(&(cur_page->entry[i].item_list));
		}
	}
	
	linked_list_clear(&addr_anal_page_list);
}

struct addr_anal_pages * addr_anal_page_create(unsigned int page_index)
{
	struct addr_anal_pages *page;

        /* Return */
        page = xcalloc(1, sizeof(struct addr_anal_pages));
        page->page_index = page_index;
	page->entry = xcalloc(x86_cpu_num_cores, sizeof(struct addr_anal_entry));
	
	return page;

}

struct addr_anal_items * addr_anal_item_create(unsigned int index,long long timestamp,int pc)
{
	struct addr_anal_items * item;
	item = xcalloc(1, sizeof(struct addr_anal_items));
	item->index = index;
	item->pc = pc;
	item->timestamp = timestamp;
	
	return item;
}
