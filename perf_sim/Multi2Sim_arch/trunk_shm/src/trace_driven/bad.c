#include <lib/esim/esim.h>
#include <lib/util/file.h>
#include <mem-system/module.h>
#include <arch/x86/timing/cpu.h>

char * mem_trace_driven_file_name_C0="";
char * mem_trace_driven_file_name_C1="";
char * mem_trace_driven_file_name_C2="";
char * mem_trace_driven_file_name_C3="";
FILE *mem_access_trace_f_C0; 
FILE *mem_access_trace_f_C1; 
FILE *mem_access_trace_f_C2; 
FILE *mem_access_trace_f_C3; 

char * inst_trace_driven_file_name_C0="";
char * inst_trace_driven_file_name_C1="";
char * inst_trace_driven_file_name_C2="";
char * inst_trace_driven_file_name_C3="";
FILE *inst_access_trace_f_C0; 
FILE *inst_access_trace_f_C1; 
FILE *inst_access_trace_f_C2; 
FILE *inst_access_trace_f_C3; 
void trace_driven_init()
{
			
	mem_access_trace_f_C0  = file_open_for_read(mem_trace_driven_file_name_C0); 
	mem_access_trace_f_C1  = file_open_for_read(mem_trace_driven_file_name_C1); 
	mem_access_trace_f_C2  = file_open_for_read(mem_trace_driven_file_name_C2); 
	mem_access_trace_f_C3  = file_open_for_read(mem_trace_driven_file_name_C3); 
	inst_access_trace_f_C0  = file_open_for_read(inst_trace_driven_file_name_C0); 
	inst_access_trace_f_C1  = file_open_for_read(inst_trace_driven_file_name_C1); 
	inst_access_trace_f_C2  = file_open_for_read(inst_trace_driven_file_name_C2); 
	inst_access_trace_f_C3  = file_open_for_read(inst_trace_driven_file_name_C3); 
	if(mem_access_trace_f_C0 == NULL )// fatal("trace driven open file fail!");
		printf("trace driven open file mem C0  fail !!!!!!!!!!!!\n");
	if(mem_access_trace_f_C1 == NULL )// fatal("trace driven open file fail!");
		printf("trace driven open file mem C1 fail !!!!!!!!!!!!\n");
	if(mem_access_trace_f_C2 == NULL )// fatal("trace driven open file fail!");
		printf("trace driven open file mem C2 fail !!!!!!!!!!!!\n");
	if(mem_access_trace_f_C3 == NULL )// fatal("trace driven open file fail!");
		printf("trace driven open file mem C3 fail !!!!!!!!!!!!\n");

	if(inst_access_trace_f_C0 == NULL )
	{
		printf("trace driven open file inst C0  fail  %s !!!!!!!!!!!!\n",inst_trace_driven_file_name_C0 );
		fatal("trace driven open file fail!");		
	}
	if(inst_access_trace_f_C1 == NULL )// fatal("trace driven open file fail!");
	{
		printf("trace driven open file inst C1 fail !!!!!!!!!!!!\n");
		fatal("trace driven open file fail!");		
	}
	if(inst_access_trace_f_C2 == NULL )// fatal("trace driven open file fail!");
	{
		printf("trace driven open file inst C2 fail !!!!!!!!!!!!\n");
		fatal("trace driven open file fail!");		
	}
	if(inst_access_trace_f_C3 == NULL )// fatal("trace driven open file fail!");
	{
		printf("trace driven open file inst C3 fail !!!!!!!!!!!!\n");
		fatal("trace driven open file fail!");		
	}
	printf("openfile successful!!!!!\n");
}




int trace_driven_run_inst()
{
	static unsigned int phy_addr_0,phy_addr_1,phy_addr_2,phy_addr_3;
	static int time_diff_0 = 0,time_diff_1 = 0, time_diff_2 = 0, time_diff_3 = 0;
	static int state0 = 0,state1 = 0,state2 = 0, state3 = 0;
//	int total_state = 0;
    //struct mod_client_info_t *client_info; 
	//client_info = mod_client_info_create( (*((*x86_cpu).core[0]).thread).data_mod);
	//client_info = mod_client_info_create( *(((x86_cpu->core[0]).thread).data_mod);
    //client_info->prefetcher_eip = load->eip;

	
	while(state0 != -1){

		if(time_diff_0 == 0)
		{
			state0 = fscanf(inst_access_trace_f_C0, "%d%x",&time_diff_0,&phy_addr_0);
			if(time_diff_0 == 0)
			{
				mod_access( (*((*x86_cpu).core[0]).thread).inst_mod  , mod_access_load,phy_addr_0, NULL, NULL , NULL , NULL);
				continue;
			}
			time_diff_0++;
		}
		
		time_diff_0--;
		if(time_diff_0 == 0)
				mod_access( (*((*x86_cpu).core[0]).thread).inst_mod  , mod_access_load,phy_addr_0, NULL, NULL , NULL , NULL);
		else break;
	}

	while(state1 != -1){

		if(time_diff_1 == 0)
		{
			state1 = fscanf(inst_access_trace_f_C1, "%d%x",&time_diff_1,&phy_addr_1);
			
			if(time_diff_1 == 0)
			{
				mod_access( (*((*x86_cpu).core[1]).thread).inst_mod  , mod_access_load,phy_addr_1, NULL, NULL , NULL , NULL);
				continue;
			}
			time_diff_1++;
		}
		time_diff_1--;
        
		if(time_diff_1 == 0)
			mod_access( (*((*x86_cpu).core[1]).thread).inst_mod  , mod_access_load,phy_addr_1, NULL, NULL , NULL , NULL);
		else break;
	}
	while(state2 != -1){

		if(time_diff_2 == 0)
		{
			state2 = fscanf(inst_access_trace_f_C2, "%d%x",&time_diff_2,&phy_addr_2);
			if(time_diff_2 == 0)
			{
				mod_access( (*((*x86_cpu).core[2]).thread).inst_mod  , mod_access_load,phy_addr_2, NULL, NULL , NULL , NULL);
				continue;
			}
			time_diff_2++;
		}
		time_diff_2--;
        
		if(time_diff_2 == 0)
			mod_access( (*((*x86_cpu).core[2]).thread).inst_mod  , mod_access_load,phy_addr_2, NULL, NULL , NULL , NULL);
		else break;
	}

	while(state3 != -1){

		if(time_diff_3 == 0)
		{
			state3 = fscanf(inst_access_trace_f_C3, "%d%x",&time_diff_3,&phy_addr_3);
			if(time_diff_3 == 0)
			{
				mod_access( (*((*x86_cpu).core[3]).thread).inst_mod  , mod_access_load,phy_addr_3, NULL, NULL , NULL , NULL);
				continue;
			}
			time_diff_3++;
		}
		time_diff_3--;
        
		if(time_diff_3 == 0)
			mod_access( (*((*x86_cpu).core[3]).thread).inst_mod  , mod_access_load,phy_addr_3, NULL, NULL , NULL , NULL);
		else break;
	}
	if( state0 == -1 && state1 == -1 && state2 == -1 && state3 == -1   ) return 0;
	else return 1;
}


int trace_driven_run_mem()
{
	static unsigned int phy_addr_0,phy_addr_1,phy_addr_2,phy_addr_3;
	static int kind0,kind1,kind2,kind3;                                          // 1 load  , 2 store
	static int time_diff_0 = 0,time_diff_1 = 0, time_diff_2 = 0, time_diff_3 = 0;
	static int state0 = 0,state1 = 0,state2 = 0, state3 = 0;
//	int total_state = 0;
    //struct mod_client_info_t *client_info; 
	//client_info = mod_client_info_create( (*((*x86_cpu).core[0]).thread).data_mod);
	//client_info = mod_client_info_create( *(((x86_cpu->core[0]).thread).data_mod);
    //client_info->prefetcher_eip = load->eip;

	
	while(state0 != -1){

		if(time_diff_0 == 0)
		{
			state0 = fscanf(mem_access_trace_f_C0, "%d%d%x",&kind0,&time_diff_0,&phy_addr_0);
			if(time_diff_0 == 0)
			{
				if(kind0 == 1)mod_access( (*((*x86_cpu).core[0]).thread).data_mod  , mod_access_load,phy_addr_0, NULL, NULL , NULL , NULL);
				else if(kind0 == 2) mod_access( (*((*x86_cpu).core[0]).thread).data_mod  , mod_access_store,phy_addr_0, NULL, NULL , NULL , NULL);
				continue;
			}
			time_diff_0++;
		}
		time_diff_0--;
        
		if(time_diff_0 == 0)
		{
			if(kind0 == 1)mod_access( (*((*x86_cpu).core[0]).thread).data_mod  , mod_access_load,phy_addr_0, NULL, NULL , NULL , NULL);
			else if(kind0 == 2) mod_access( (*((*x86_cpu).core[0]).thread).data_mod  , mod_access_store,phy_addr_0, NULL, NULL , NULL , NULL);
		}
		else break;
	}

	while(state1 != -1){

		if(time_diff_1 == 0)
		{
			state1 = fscanf(mem_access_trace_f_C1, "%d%d%x",&kind1,&time_diff_1,&phy_addr_1);
			if(time_diff_1 == 0)
			{
				if(kind1 == 1)mod_access( (*((*x86_cpu).core[1]).thread).data_mod  , mod_access_load,phy_addr_1, NULL, NULL , NULL , NULL);
				else if(kind1 == 2) mod_access( (*((*x86_cpu).core[1]).thread).data_mod  , mod_access_store,phy_addr_1, NULL, NULL , NULL , NULL);
				continue;
			}
			time_diff_1++;
		}
		time_diff_1--;
        
		if(time_diff_1 == 0)
		{
			if(kind1 == 1)mod_access( (*((*x86_cpu).core[1]).thread).data_mod  , mod_access_load,phy_addr_1, NULL, NULL , NULL , NULL);
			else if(kind1 == 2) mod_access( (*((*x86_cpu).core[1]).thread).data_mod  , mod_access_store,phy_addr_1, NULL, NULL , NULL , NULL);
		}
		else break;
	}
	while(state2 != -1){

		if(time_diff_2 == 0)
		{
			state2 = fscanf(mem_access_trace_f_C2, "%d%d%x",&kind2,&time_diff_2,&phy_addr_2);
			if(time_diff_2 == 0)
			{
				if(kind2 == 1)mod_access( (*((*x86_cpu).core[2]).thread).data_mod  , mod_access_load,phy_addr_2, NULL, NULL , NULL , NULL);
				else if(kind2 == 2) mod_access( (*((*x86_cpu).core[2]).thread).data_mod  , mod_access_store,phy_addr_2, NULL, NULL , NULL , NULL);
				continue;
			}
			time_diff_2++;
		}
		time_diff_2--;
        
		if(time_diff_2 == 0)
		{
			if(kind2 == 1)mod_access( (*((*x86_cpu).core[2]).thread).data_mod  , mod_access_load,phy_addr_2, NULL, NULL , NULL , NULL);
			else if(kind2 == 2) mod_access( (*((*x86_cpu).core[2]).thread).data_mod  , mod_access_store,phy_addr_2, NULL, NULL , NULL , NULL);
		}
		else break;
	}

	while(state3 != -1){

		if(time_diff_3 == 0)
		{
			state3 = fscanf(mem_access_trace_f_C3, "%d%d%x",&kind3,&time_diff_3,&phy_addr_3);
			if(time_diff_3 == 0)
			{
				if(kind3 == 1)mod_access( (*((*x86_cpu).core[3]).thread).data_mod  , mod_access_load,phy_addr_3, NULL, NULL , NULL , NULL);
				else if(kind3 == 2) mod_access( (*((*x86_cpu).core[3]).thread).data_mod  , mod_access_store,phy_addr_3, NULL, NULL , NULL , NULL);
				continue;
			}
			time_diff_3++;
		}
		time_diff_3--;
        
		if(time_diff_3 == 0)
		{
			if(kind3 == 1)mod_access( (*((*x86_cpu).core[3]).thread).data_mod  , mod_access_load,phy_addr_3, NULL, NULL , NULL , NULL);
			else if(kind3 == 2) mod_access( (*((*x86_cpu).core[3]).thread).data_mod  , mod_access_store,phy_addr_3, NULL, NULL , NULL , NULL);
		}
		else break;
	}
	if( state0 == -1 && state1 == -1 && state2 == -1 && state3 == -1   ) return 0;
	else return 1;
}
void trace_driven_done()
{
		printf("close mem trace FILE * \n");
		fclose( mem_access_trace_f_C0 );
		fclose( mem_access_trace_f_C1 );
		fclose( mem_access_trace_f_C2 );
		fclose( mem_access_trace_f_C3 );
		fclose( inst_access_trace_f_C0 );
		fclose( inst_access_trace_f_C1 );
		fclose( inst_access_trace_f_C2 );
		fclose( inst_access_trace_f_C3 );
}
