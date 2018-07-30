

#ifndef MEM_SYSTEM_ADDRESS_ANALYSIS_H
#define MEM_SYSTEM_ADDRESS_ANALYSIS_H


#define ADDR_ANAL_LOG_PAGE_SIZE 12

struct addr_anal_items{
	unsigned int index;
	int pc;
	long long timestamp;
};

struct addr_anal_entry{
	struct linked_list_t item_list;
};

struct addr_anal_pages{
	unsigned int page_index;
	struct addr_anal_entry *entry;
};


extern struct linked_list_t addr_anal_page_list;
extern char *addr_anal_file_name;

void addr_anal_insert(int which_core,int addr,int pc);

struct addr_anal_pages * addr_anal_page_create(unsigned int page_index);
struct addr_anal_items * addr_anal_item_create(unsigned int index,long long timestamp,int pc);
void addr_anal_free();
void addr_anal_dump();

#endif
