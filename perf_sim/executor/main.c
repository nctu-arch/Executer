#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>

//cycle info
#include <sys/shm.h>
int *trans_cycle;

enum parse_f{
	NONE,
	SCENARIO,
	TASK,
	CORE,
	MEMORY,
	PERIPHERAL,
	SENSOR,
	SLEEP_MODE
};


enum parse_f parse_flag;

//Perf global Var
int period=0;
int cycle_count;
int total_cycle_count;
int load;
int store;

typedef struct st{
	int loop;
	int total_tasks;
	int total_cores;
	int total_srams;
	int total_peripherals;
	int total_sensors;
}scenario_t;

typedef struct ct{
	int id;
	int speed;
} core_t;

typedef struct mt{
	int id;
	int size;
	int access_cycle;
} mem_t;

typedef struct pt{
	int id;
	char* name;
} periph_t;

typedef struct snt{
	int id;
	char* name;
} sensor_t;

typedef struct sm{
	int*  core_id;
	int*  periph_id;
} sleep_m;

typedef struct tt{
	int id;
	char *name;
	char *axf_file;
	double deadline;
	int previous_task;
	int iteration;
	core_t* assigned_core;
	int core_mode;

	mem_t* assigned_mem;
	int mem_mode;
	int* assigned_periph;
	int periph_mode;
	sleep_m* assigned_sleep_mode;
} task_t;




scenario_t scenario_param;
task_t* task_param;
core_t* core_param;
mem_t* mem_param;
periph_t* periph_param;
sensor_t* sensor_param;
sleep_m* sleep_mode_param;
int task_index;
int core_index;
int mem_index;
int periph_index;
int sensor_index;
int sleep_mode_index;


//Power global Var
int DFVS_mode=0;
typedef struct mp{
	int id;
	float operating_voltage;
	float read_energy;
	float write_energy;
} mem_p;

typedef struct cp{
	int id;
	float operating_voltage;
	float active_current;
	float sleep_current;
	float transition_energy;
	float transition_time;
} core_p;

typedef struct pp{
	int id;
	float operating_voltage;
	float active_current;
	float sleep_current;
	float transition_energy;
	float transition_time;
} periph_p;

typedef struct sp{
	int id;
	float operating_voltage;
	float active_current;
	float sleep_current;
} sensor_p;

typedef struct cdvm{
	int mode;
	core_p* dvm_core_lib;
}c_dvs_t;

typedef struct mdvm{
	int mode;
	mem_p* dvm_mem_lib;
	//periph_p* dvm_periph_lib;
}m_dvs_t;

typedef struct pdvm{
	int mode;
	periph_p* dvm_periph_lib;
}p_dvs_t;

typedef struct sdvm{
	int mode;
	sensor_p* dvm_sensor_lib;
}s_dvs_t;

core_p* core_lib;
mem_p* mem_lib;
periph_p* periph_lib;
sensor_p* sensor_lib;
c_dvs_t* c_dvs_lib; //core dvs mode lib
m_dvs_t* m_dvs_lib; //memory dvs mode lib
p_dvs_t* p_dvs_lib; //periph dvs mode lib
s_dvs_t* s_dvs_lib; //sensor dvs mode lib
int dvs_index;
int core_lib_index;
int mem_lib_index;
int periph_lib_index;
int sensor_lib_index;


//Result global var
typedef struct r{
	double active_time;
	double sleep_time;
	double transition_time;
	
	double active_energy;
	double sleep_energy;
	double transition_energy;


}result_r;


typedef struct res{
	result_r* core_result;
	result_r* mem_result;
	result_r* periph_result;
	result_r* sensor_result;

	int load;
	int store;

	double task_execution_time;
	double task_remaining_time;
	double task_transition_time;
}res_r;

res_r* final_result;
double total_execution_time=0;
double total_active_time=0;
double total_sleep_time=0;
double total_transition_time=0;
double total_active_energy=0;
double total_sleep_energy=0;
double total_transition_energy=0;




void read_power_lib(char *argv){

	FILE *fp=NULL;
	yaml_parser_t parser;
	yaml_parser_t parser2;
	yaml_token_t token;
	yaml_event_t event;
	unsigned char *key;
	//0 is key
	//1 is value
	int state=0;
	int temp=0;	
	int i=0;
	
	fp = fopen(argv,"r");

	if(!yaml_parser_initialize(&parser))
		fputs("Failed to initialize parser!\n", stderr);
	//if(!yaml_parser_initialize(&parser2))
	//	fputs("Failed to initialize parser2!\n", stderr);
	if(fp == NULL)
		fputs("Failed to open file!\n", stderr);
	
	/* Set input file */
	yaml_parser_set_input_file(&parser, fp);
	//yaml_parser_set_input_file(&parser2, fp);

	/* CODE HERE */
	/* BEGIN new code */
	do {
		yaml_parser_scan(&parser, &token);
		yaml_parser_parse(&parser2, &event);
		switch(event.type){
		case YAML_SEQUENCE_START_EVENT: puts("sequence start event");break;
		case YAML_MAPPING_START_EVENT: puts("mapping start event");break;
		}
		switch(token.type)
		{
		/* Stream start/end */
		case YAML_STREAM_START_TOKEN: break;
		case YAML_STREAM_END_TOKEN:   break;
		/* Block delimeters */
		case YAML_BLOCK_SEQUENCE_START_TOKEN: break;
		case YAML_BLOCK_ENTRY_TOKEN:          break;
		case YAML_BLOCK_END_TOKEN:            break;
		/* Data */
		case YAML_BLOCK_MAPPING_START_TOKEN:  break;
		/* Token types (read before actual token) */
		case YAML_KEY_TOKEN:   state=0; break;
		case YAML_VALUE_TOKEN: 
			state=1;
			//printf("%s\n",key);
			//printf("\n");
			if(!strcmp(key, "core_param")){
				parse_flag = CORE;
				printf("%s\n",key);
			}
			else if(!strcmp(key, "memory_param")){
				parse_flag = MEMORY;
				printf("%s\n",key);
			}
			else if(!strcmp(key, "peripheral_param")){
				parse_flag = PERIPHERAL;
				printf("%s\n",key);
			}
			else if(!strcmp(key, "sensor_param")){
				parse_flag = SENSOR;
				printf("%s\n",key);
			}
			else{
				//do nothing
			}
			break;
		case YAML_SCALAR_TOKEN:  
			if(state){
			if(parse_flag==CORE){
			    if(!strcmp(key, "DFVS_mode")){
					DFVS_mode = atoi(token.data.scalar.value);
					printf("\tDFVS_mode = %d\n",DFVS_mode);
					c_dvs_lib = (c_dvs_t*) malloc(sizeof(c_dvs_t)*DFVS_mode);

					for(i=0;i<DFVS_mode;i++)
					c_dvs_lib[i].dvm_core_lib = (core_p*) malloc(sizeof(core_p)*scenario_param.total_cores);
					
				}
			    else if(!strcmp(key, "mode")){
					dvs_index = atoi(token.data.scalar.value);
					c_dvs_lib[dvs_index].mode = dvs_index;
					printf("\tmode %d\n",c_dvs_lib[dvs_index].mode);
				}
				else if(!strcmp(key, "core_id")){
					core_lib_index = atoi(token.data.scalar.value);
					c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].id = core_lib_index;
        			printf("\t\tcore id = %d\n",c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].id);

				}
				else if(!strcmp(key, "operating_voltage")){
					c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].operating_voltage = atof(token.data.scalar.value);
        			printf("\t\toperating voltage of mode %d of core %d = %.3f V\n",dvs_index,c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].id,c_dvs_lib[dvs_index].dvm_core_lib[c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].id].operating_voltage);
					
				}
				else if(!strcmp(key, "active_current")){
					c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].active_current = atof(token.data.scalar.value);
        			printf("\t\tactive_current of mode %d of core %d = %.3f mA\n",dvs_index,c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].id,c_dvs_lib[dvs_index].dvm_core_lib[c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].id].active_current);
					
				}
				else if(!strcmp(key, "sleep_current")){
					c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].sleep_current = atof(token.data.scalar.value);
        			printf("\t\tsleep_current of mode %d of core %d = %.3f mA\n",dvs_index,c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].id,c_dvs_lib[dvs_index].dvm_core_lib[c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].id].sleep_current);
					
				}
				else if(!strcmp(key, "transition_energy")){
					c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].transition_energy = atof(token.data.scalar.value);
        			printf("\t\ttransition_energy of mode %d of core %d  = %.3f nJ\n",dvs_index,c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].id,c_dvs_lib[dvs_index].dvm_core_lib[c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].id].transition_energy);
					
				}
				else if(!strcmp(key, "transition_time")){
					c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].transition_time = atof(token.data.scalar.value);
        			printf("\t\ttransition_time of mode %d of core %d = %.3f us\n\n",dvs_index,c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].id,c_dvs_lib[dvs_index].dvm_core_lib[c_dvs_lib[dvs_index].dvm_core_lib[core_lib_index].id].transition_time);
					
				}
				else{
					//do nothing
				}
			}
			else if(parse_flag==MEMORY){
			    if(!strcmp(key, "DFVS_mode")){
					DFVS_mode = atoi(token.data.scalar.value);
					printf("\tDFVS_mode = %d\n",DFVS_mode);
					dvs_index = -1;	
					m_dvs_lib = (m_dvs_t*) malloc(sizeof(m_dvs_t)*DFVS_mode);

					for(i=0;i<DFVS_mode;i++)
					m_dvs_lib[i].dvm_mem_lib = (mem_p*) malloc(sizeof(mem_p)*scenario_param.total_srams);
					
				}
			    else if(!strcmp(key, "mode")){
					dvs_index = atoi(token.data.scalar.value);
					m_dvs_lib[dvs_index].mode = dvs_index;
					printf("\tmode %d\n",m_dvs_lib[dvs_index].mode);
				}
				else if(!strcmp(key, "mem_id")){
					mem_lib_index = atoi(token.data.scalar.value);
					m_dvs_lib[dvs_index].dvm_mem_lib[mem_lib_index].id = mem_lib_index;
        			printf("\t\tmem id = %d\n",m_dvs_lib[dvs_index].dvm_mem_lib[mem_lib_index].id);

				}
				else if(!strcmp(key, "operating_voltage")){
					m_dvs_lib[dvs_index].dvm_mem_lib[mem_lib_index].operating_voltage = atof(token.data.scalar.value);
        			printf("\t\toperating voltage of mode %d of mem %d = %.3f V\n",dvs_index,m_dvs_lib[dvs_index].dvm_mem_lib[mem_lib_index].id,m_dvs_lib[dvs_index].dvm_mem_lib[m_dvs_lib[dvs_index].dvm_mem_lib[mem_lib_index].id].operating_voltage);
					
				}
				else if(!strcmp(key, "read_energy")){
					m_dvs_lib[dvs_index].dvm_mem_lib[mem_lib_index].read_energy = atof(token.data.scalar.value);
        			printf("\t\tread energy of mode %d of mem %d = %.3f nJ\n",dvs_index,m_dvs_lib[dvs_index].dvm_mem_lib[mem_lib_index].id,m_dvs_lib[dvs_index].dvm_mem_lib[m_dvs_lib[dvs_index].dvm_mem_lib[mem_lib_index].id].read_energy);
					
				}
				else if(!strcmp(key, "write_energy")){
					m_dvs_lib[dvs_index].dvm_mem_lib[mem_lib_index].write_energy = atof(token.data.scalar.value);
        			printf("\t\twrite energy of mode %d of mem %d= %.3f nJ\n",dvs_index,m_dvs_lib[dvs_index].dvm_mem_lib[mem_lib_index].id,m_dvs_lib[dvs_index].dvm_mem_lib[m_dvs_lib[dvs_index].dvm_mem_lib[mem_lib_index].id].write_energy);
					
				}
				else{
					//do nothing
				}
			}
			else if(parse_flag==PERIPHERAL){
			    if(!strcmp(key, "DFVS_mode")){
					DFVS_mode = atoi(token.data.scalar.value);
					printf("\tDFVS_mode = %d\n",DFVS_mode);
					dvs_index = -1;	
					p_dvs_lib = (p_dvs_t*) malloc(sizeof(p_dvs_t)*DFVS_mode);

					for(i=0;i<DFVS_mode;i++)
					p_dvs_lib[i].dvm_periph_lib = (periph_p*) malloc(sizeof(periph_p)*scenario_param.total_peripherals);
					
				}
			    else if(!strcmp(key, "mode")){
					dvs_index = atoi(token.data.scalar.value);
					p_dvs_lib[dvs_index].mode = dvs_index;
					printf("\tmode %d\n",p_dvs_lib[dvs_index].mode);
				}
				else if(!strcmp(key, "periph_id")){
					periph_lib_index = atoi(token.data.scalar.value);
					p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].id = periph_lib_index;
        			printf("\t\tperiph id = %d\n",p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].id);

				}
				else if(!strcmp(key, "operating_voltage")){
					p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].operating_voltage = atof(token.data.scalar.value);
        			printf("\t\toperating voltage of mode %d of periph %d  = %.3f V\n",dvs_index,p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].id,p_dvs_lib[dvs_index].dvm_periph_lib[p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].id].operating_voltage);
					
				}
				else if(!strcmp(key, "active_current")){
					p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].active_current = atof(token.data.scalar.value);
        			printf("\t\tactive current of mode %d of periph %d = %.3f mA\n",dvs_index,p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].id,p_dvs_lib[dvs_index].dvm_periph_lib[p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].id].active_current);
					
				}
				else if(!strcmp(key, "sleep_current")){
					p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].sleep_current = atof(token.data.scalar.value);
        			printf("\t\tsleep current of mode %d of periph %d = %.3f mA\n",dvs_index,p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].id,p_dvs_lib[dvs_index].dvm_periph_lib[p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].id].sleep_current);
					
				}
				else if(!strcmp(key, "transition_energy")){
					p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].transition_energy = atof(token.data.scalar.value);
        			printf("\t\ttransition energy of mode %d of periph %d  = %.3f uJ\n",dvs_index,p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].id,p_dvs_lib[dvs_index].dvm_periph_lib[p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].id].transition_energy);
					
				}
				else if(!strcmp(key, "transition_time")){
					p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].transition_time = atof(token.data.scalar.value);
        			printf("\t\ttransition time of mode %d of periph %d  = %.3f us\n\n",dvs_index,p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].id,p_dvs_lib[dvs_index].dvm_periph_lib[p_dvs_lib[dvs_index].dvm_periph_lib[periph_lib_index].id].transition_time);
					
				}
				else{
					//do nothing
				}
			}
			else if(parse_flag==SENSOR){
			    if(!strcmp(key, "DFVS_mode")){
					DFVS_mode = atoi(token.data.scalar.value);
					printf("\tDFVS_mode = %d\n",DFVS_mode);
					dvs_index = -1;	
					s_dvs_lib = (s_dvs_t*) malloc(sizeof(s_dvs_t)*DFVS_mode);

					for(i=0;i<DFVS_mode;i++)
					s_dvs_lib[i].dvm_sensor_lib = (sensor_p*) malloc(sizeof(sensor_p)*scenario_param.total_sensors);
					
				}
			    else if(!strcmp(key, "mode")){
					dvs_index = atoi(token.data.scalar.value);
					s_dvs_lib[dvs_index].mode = dvs_index;
					printf("\tmode %d\n",s_dvs_lib[dvs_index].mode);
				}
				else if(!strcmp(key, "sensor_id")){
					sensor_lib_index = atoi(token.data.scalar.value);
					s_dvs_lib[dvs_index].dvm_sensor_lib[sensor_lib_index].id = sensor_lib_index;
        			printf("\t\tsensor id = %d\n",s_dvs_lib[dvs_index].dvm_sensor_lib[sensor_lib_index].id);

				}
				else if(!strcmp(key, "operating_voltage")){
					s_dvs_lib[dvs_index].dvm_sensor_lib[sensor_lib_index].operating_voltage = atof(token.data.scalar.value);
        			printf("\t\toperating voltage of mode %d of sensor %d = %.3f V\n",dvs_index,s_dvs_lib[dvs_index].dvm_sensor_lib[sensor_lib_index].id,s_dvs_lib[dvs_index].dvm_sensor_lib[s_dvs_lib[dvs_index].dvm_sensor_lib[sensor_lib_index].id].operating_voltage);
					
				}
				else if(!strcmp(key, "active_current")){
					s_dvs_lib[dvs_index].dvm_sensor_lib[sensor_lib_index].active_current = atof(token.data.scalar.value);
        			printf("\t\tactive current of mode %d of sensor %d = %.3f mA\n",dvs_index,s_dvs_lib[dvs_index].dvm_sensor_lib[sensor_lib_index].id,s_dvs_lib[dvs_index].dvm_sensor_lib[s_dvs_lib[dvs_index].dvm_sensor_lib[sensor_lib_index].id].active_current);
					
				}
				else if(!strcmp(key, "sleep_current")){
					s_dvs_lib[dvs_index].dvm_sensor_lib[sensor_lib_index].sleep_current = atof(token.data.scalar.value);
        			printf("\t\tsleep current of mode %d of sensor %d = %.3f mA\n\n",dvs_index,s_dvs_lib[dvs_index].dvm_sensor_lib[sensor_lib_index].id,s_dvs_lib[dvs_index].dvm_sensor_lib[s_dvs_lib[dvs_index].dvm_sensor_lib[sensor_lib_index].id].sleep_current);
					
				}
				else{
					//do nothing
				}
			}
			else{
				//do nothing
			}
			}
			else{
			  key = strdup(token.data.scalar.value);
			}//end state
			break;
		/* Others */
		//default:
		//printf("Got token of type %d\n", token.type);
		}
		if(token.type != YAML_STREAM_END_TOKEN)
		yaml_token_delete(&token);
	} while(token.type != YAML_STREAM_END_TOKEN);
	yaml_token_delete(&token);
	/* END new code */
	
	yaml_parser_delete(&parser);
	//yaml_parser_delete(&parser2);
	fclose(fp);
}


void read_scenario_param(char *argv){

	FILE *fp=NULL;
	yaml_parser_t parser;
	yaml_token_t token;
	yaml_event_t event;
	unsigned char *key;
	//0 is key
	//1 is value
	int state=0;
	int temp=0;	
	int i,j;
	
	fp = fopen(argv,"r");

	if(!yaml_parser_initialize(&parser))
		fputs("Failed to initialize parser!\n", stderr);
	if(fp == NULL)
		fputs("Failed to open file!\n", stderr);
	
	/* Set input file */
	yaml_parser_set_input_file(&parser, fp);

	/* CODE HERE */
	/* BEGIN new code */
	do {
		yaml_parser_scan(&parser, &token);
		switch(event.type){
		case YAML_SEQUENCE_START_EVENT: puts("sequence start event");break;
		case YAML_MAPPING_START_EVENT: puts("mapping start event");break;
		}
		switch(token.type)
		{
		/* Stream start/end */
		case YAML_STREAM_START_TOKEN: break;
		case YAML_STREAM_END_TOKEN:   break;
		/* Block delimeters */
		case YAML_BLOCK_SEQUENCE_START_TOKEN: break;
		case YAML_BLOCK_ENTRY_TOKEN:          break;
		case YAML_BLOCK_END_TOKEN:            break;
		/* Data */
		case YAML_BLOCK_MAPPING_START_TOKEN:  break;
		/* Token types (read before actual token) */
		case YAML_KEY_TOKEN:   state=0; break;
		case YAML_VALUE_TOKEN: 
			state=1;
			//printf("%s\n",key);
			//printf("\n");
			if(!strcmp(key, "scenario_param")){
				parse_flag = SCENARIO;
				printf("%s\n",key);
			}
			else if(!strcmp(key, "core_param")){
				parse_flag = CORE;
				printf("%s\n",key);
			}
			else if(!strcmp(key, "task_param")){
				parse_flag = TASK;
				printf("%s\n",key);
			}
			else if(!strcmp(key, "sram_param")){
				parse_flag = MEMORY;
				printf("%s\n",key);
			}
			else if(!strcmp(key, "peripheral_param")){
				parse_flag = PERIPHERAL;
				printf("%s\n",key);
			}
			else if(!strcmp(key, "sensor_param")){
				parse_flag = SENSOR;
				printf("%s\n",key);
			}
			else if(!strcmp(key, "sleep_mode_param")){
				parse_flag = SLEEP_MODE;
				printf("%s\n",key);
			}
			else{
				//do nothing
			}
			break;
		case YAML_SCALAR_TOKEN:  
			if(state){
			if(parse_flag==SCENARIO){
			    if(!strcmp(key, "loop")){
					scenario_param.loop = atoi(token.data.scalar.value);
					printf("\tloop = %d\n",scenario_param.loop);
				}
				else if(!strcmp(key, "total tasks")){
					scenario_param.total_tasks = atoi(token.data.scalar.value);	
        			printf("\ttotal tasks = %d\n",scenario_param.total_tasks);

					task_param = (task_t*) malloc(sizeof(task_t)*scenario_param.total_tasks);
					task_index= 0;

					final_result = (res_r*) malloc(sizeof(res_r)*scenario_param.total_tasks);
				}
				else if(!strcmp(key, "total cores")){
					 scenario_param.total_cores = atoi(token.data.scalar.value);	
        			 printf("\ttotal cores = %d\n",scenario_param.total_cores);
					
					core_param = (core_t*) malloc(sizeof(core_t)*scenario_param.total_cores);
					//core_lib = (core_p*) malloc(sizeof(core_p)*scenario_param.total_cores);
					core_index=0;
				}
				else if(!strcmp(key, "total srams")){
					 scenario_param.total_srams = atoi(token.data.scalar.value);	
        			 printf("\ttotal srams = %d\n",scenario_param.total_srams);
					
					mem_param = (mem_t*) malloc(sizeof(mem_t)*scenario_param.total_srams);
					mem_index=0;
				}
				else if(!strcmp(key, "total peripherals")){
					 scenario_param.total_peripherals = atoi(token.data.scalar.value);	
        			 printf("\ttotal peripherals = %d\n",scenario_param.total_peripherals);
					
					periph_param = (periph_t*) malloc(sizeof(periph_t)*scenario_param.total_peripherals);
					periph_index=0;

					for(i=0;i<scenario_param.total_tasks;i++){
					task_param[i].assigned_periph = (int*) malloc(sizeof(int)*scenario_param.total_peripherals);
						for(j=0;j<scenario_param.total_peripherals;j++){
						task_param[i].assigned_periph[j] = 0;
						}
					}
				}
				else if(!strcmp(key, "total sensors")){
					 scenario_param.total_sensors = atoi(token.data.scalar.value);	
        			 printf("\ttotal peripherals = %d\n",scenario_param.total_sensors);
					
					sensor_param = (sensor_t*) malloc(sizeof(sensor_t)*scenario_param.total_sensors);
					sensor_index=0;
				}
				else{
					//do nothing
				}
			}
			else if(parse_flag==CORE){
			    if(!strcmp(key, "id")){
				    core_index = atoi(token.data.scalar.value);
					core_param[core_index].id = core_index;
					printf("\tcore%d_id = %d\n",core_index,core_param[core_index].id);
				}
			    else if(!strcmp(key, "speed")){
					core_param[core_index].speed = atoi(token.data.scalar.value);
					printf("\tcore%d_speed = %d\n\n",core_index,core_param[core_index].speed);
				}
				else{
					//do nothing
				}
			}
			else if(parse_flag==MEMORY){
			    if(!strcmp(key, "id")){
					mem_index = atoi(token.data.scalar.value);
					mem_param[mem_index].id = mem_index;
					printf("\tsram%d_id = %d\n",mem_index,mem_param[mem_index].id);
				}
			    else if(!strcmp(key, "size")){
					mem_param[mem_index].size = atoi(token.data.scalar.value);
					printf("\tsram%d_size = %d\n",mem_index,mem_param[mem_index].size);
				}
			    else if(!strcmp(key, "access cycle")){
					mem_param[mem_index].access_cycle = atoi(token.data.scalar.value);
					printf("\tsram%d access cycle = %d\n\n",mem_index,mem_param[mem_index].access_cycle);
				}
				else{
					//do nothing
				}
			}
			else if(parse_flag==PERIPHERAL){
			    if(!strcmp(key, "id")){
					periph_index = atoi(token.data.scalar.value);
					periph_param[periph_index].id = periph_index;
					printf("\tperiph_id = %d\n",periph_param[periph_index].id);
				}
			    else if(!strcmp(key, "name")){
					periph_param[periph_index].name = strdup(token.data.scalar.value);
					printf("\tperiph_name = %s\n\n",periph_param[periph_index].name);
				}
				else{
					//do nothing
				}
			}
			else if(parse_flag==SENSOR){
			    if(!strcmp(key, "id")){
					sensor_index = atoi(token.data.scalar.value);
					sensor_param[sensor_index].id = sensor_index;
					printf("\tsensor_id = %d\n",sensor_param[sensor_index].id);
				}
			    else if(!strcmp(key, "name")){
					sensor_param[sensor_index].name = strdup(token.data.scalar.value);
					printf("\tsensor_name = %s\n\n",sensor_param[sensor_index].name);
				}
				else{
					//do nothing
				}
			}
			else if(parse_flag==SLEEP_MODE){
			    if(!strcmp(key, "total mode")){
					temp = atoi(token.data.scalar.value);
					sleep_mode_param = (sleep_m*) malloc(sizeof(sleep_m)*temp);
					printf("\ttotal sleep mode = %d\n",temp);

					for(i=0;i<temp;i++){
					 sleep_mode_param[i].core_id = (int*) calloc(scenario_param.total_cores,sizeof(int));
					 sleep_mode_param[i].periph_id = (int*) calloc(scenario_param.total_peripherals,sizeof(int));
					}
				}
			    else if(!strcmp(key, "mode")){
					sleep_mode_index = atoi(token.data.scalar.value);
					printf("\n");
				}
			    else if(!strcmp(key, "core id")){
				    temp = atoi(token.data.scalar.value);
					sleep_mode_param[sleep_mode_index].core_id[temp] = 1;
					printf("\tsleep mode %d turn off core %d\n",sleep_mode_index, temp);
				}
			    else if(!strcmp(key, "periph id")){
				    temp = atoi(token.data.scalar.value);
					sleep_mode_param[sleep_mode_index].periph_id[temp] = 1;
					printf("\tsleep mode %d turn off %s\n",sleep_mode_index, periph_param[temp].name);
				}
				else{
					//do nothing
				}
			}
			else if(parse_flag==TASK){
			    if(!strcmp(key,"id")){
					task_index = atoi(token.data.scalar.value);
					task_param[task_index].id = atoi(token.data.scalar.value);
					printf("\n\ttask%d_id = %d\n",task_index,task_param[task_index].id);
				}
			    else if(!strcmp(key,"axf file")){
					task_param[task_index].axf_file = strdup(token.data.scalar.value);
					printf("\ttask%d axf_file = %s\n",task_index,task_param[task_index].axf_file);
				}
			    else if(!strcmp(key,"name")){
					task_param[task_index].name = strdup(token.data.scalar.value);
					printf("\ttask%d name = %s\n",task_index,task_param[task_index].name);
				}
			    else if(!strcmp(key,"deadline")){
					task_param[task_index].deadline = atof(token.data.scalar.value);
					printf("\t%s deadline = %lf\n",task_param[task_index].name,task_param[task_index].deadline);
				}
			    else if(!strcmp(key,"previous task id")){
					task_param[task_index].previous_task = atoi(token.data.scalar.value);
					printf("\t%s previous task id = %d\n",task_param[task_index].name,task_param[task_index].previous_task);
				}
			    else if(!strcmp(key,"iteration")){
					task_param[task_index].iteration = atoi(token.data.scalar.value);
					printf("\t%s iteration = %d\n",task_param[task_index].name,task_param[task_index].iteration);
				}
			    else if(!strcmp(key,"assigned sleep mode")){
					temp = atoi(token.data.scalar.value);
					task_param[task_index].assigned_sleep_mode = &sleep_mode_param[temp];
					printf("\t%s use sleep mode %d\n",task_param[task_index].name,temp);
				}
			    else if(!strcmp(key,"assigned core id")){
					temp = atoi(token.data.scalar.value);
					task_param[task_index].assigned_core = &core_param[temp];
					printf("\t%s assigned to core %d\n",task_param[task_index].name,task_param[task_index].assigned_core->id);
				}
			    else if(!strcmp(key,"assigned core mode")){
					temp = atoi(token.data.scalar.value);
					task_param[task_index].core_mode = temp;
					printf("\t%s running with core mode %d\n",task_param[task_index].name,task_param[task_index].core_mode);
				}
			    else if(!strcmp(key,"assigned mem id")){
					temp = atoi(token.data.scalar.value);
					task_param[task_index].assigned_mem = &mem_param[temp];
					printf("\t%s use mem %d\n",task_param[task_index].name,task_param[task_index].assigned_mem->id);
				}
			    else if(!strcmp(key,"assigned mem mode")){
					temp = atoi(token.data.scalar.value);
					task_param[task_index].mem_mode = temp;
					printf("\t%s running with mem mode %d\n",task_param[task_index].name,task_param[task_index].mem_mode);
				}
			    else if(!strcmp(key,"assigned periph id")){
					temp = atoi(token.data.scalar.value);
					task_param[task_index].assigned_periph[temp] = 1;
					printf("\t%s use periph %s\n",task_param[task_index].name,periph_param[temp].name);
				}
			    else if(!strcmp(key,"assigned periph mode")){
					temp = atoi(token.data.scalar.value);
					task_param[task_index].periph_mode = temp;
					printf("\t%s running with periph mode %d\n",task_param[task_index].name,task_param[task_index].periph_mode);
				}
				else{
					//do nothing
				}
			}
			else{
				//do nothing
			}
			}
			else{
			  key = strdup(token.data.scalar.value);
			}//end state
			break;
		/* Others */
		//default:
		//printf("Got token of type %d\n", token.type);
		}
		if(token.type != YAML_STREAM_END_TOKEN)
		yaml_token_delete(&token);
	} while(token.type != YAML_STREAM_END_TOKEN);
	yaml_token_delete(&token);
	/* END new code */
	
	yaml_parser_delete(&parser);
	fclose(fp);
}



void sim_result(char * str){
	char line[256];
	FILE *fd = popen(str,"r");
	while((fgets(line, 256, fd))!=NULL);
	printf("%s\n",line);
	sscanf(line,"tb total cycle cnt = %d, load = %d, store = %d\n",&cycle_count,&load,&store);
	pclose(fd);

}



void runtask(int task_id, int core_id, int mem_id,int* periph_array, sleep_m* sleep_mode,res_r* f_result, char* axf ){
    char str[256];
    int i;
    FILE *fd;
	double time_per_cycle;
	double execution_time = 0;
	double task_deadline;
	int task_iteration;
 
	time_per_cycle = (1 / (double) core_param[core_id].speed); //in us
	task_deadline = (task_param[task_id].deadline * 1000); //in us
	task_iteration = task_param[task_id].iteration;


	if(core_id==1){   
	sprintf(str, "/home/cws/perf_sim/executor/run_m4.sh %s\n", axf);
	}
	else if(core_id==0){
	sprintf(str, "/home/cws/perf_sim/executor/run_m0.sh %s\n", axf);
	}
	fd = popen(str,"r");
	pclose(fd);

	sim_result(str);

	printf("SIMULATOR RESULT:");
	printf("%s run with core %d : cycles %d load %d store %d\n",task_param[task_id].name,core_id,cycle_count,load,store);


	f_result[task_id].load = load;
	f_result[task_id].store = store;

	//*********CALCULATE ACTIVE TIME****************//
	execution_time = (cycle_count * time_per_cycle) + ((load+store)*mem_param[mem_id].access_cycle*time_per_cycle);
	//printf("execution time %f\n",execution_time);
	//check task deadline (not scenario deadline)
	if(execution_time>task_deadline)
	{
	  printf("------------------WARNING---------------------\n");
	  printf("%s TASK HAS DEADLINE VIOLATION\n",task_param[task_id].name);

	}
	

	//core active time
	f_result[task_id].core_result[core_id].active_time = execution_time; //in us
	f_result[task_id].task_execution_time = execution_time; //in us

	//mem active time
	f_result[task_id].mem_result[mem_id].active_time =  ((load+store)*mem_param[mem_id].access_cycle*time_per_cycle); //in us

	//periph active time
	for(i=0;i<scenario_param.total_peripherals;i++){
		if(periph_array[i]==1){
		  f_result[task_id].periph_result[i].active_time = execution_time;
		 // printf("EXECUTION TIME %lf\n",execution_time);
		  }
	}


	if(task_param[task_id].previous_task >=0) 
	  f_result[task_id].task_remaining_time = task_deadline - (f_result[task_param[task_id].previous_task].task_execution_time + execution_time);
	else
	  f_result[task_id].task_remaining_time = task_deadline - execution_time;

	//printf("cycle count = %d\n",cycle_count);
	//printf("load  = %d, store = %d\n",load,store);
    

}

void calculate_transition(int task_id, int core_id, int mem_id,int* periph_array, sleep_m* sleep_mode, res_r* f_result){
	int i;
	int max=0;
	
		if(sleep_mode->core_id[core_id] == 1){
			f_result->core_result[core_id].transition_time = c_dvs_lib[task_param[task_id].core_mode].dvm_core_lib[core_id].transition_time;
			f_result->task_transition_time += f_result->core_result[core_id].transition_time;
		}
		else{
			f_result->core_result[core_id].transition_time = 0;
		}
	//memory transition is not yet supported

	for(i=0;i<scenario_param.total_peripherals;i++){
		if((sleep_mode->periph_id[i] == 1) && periph_array[i]==1){
			f_result->periph_result[i].transition_time = p_dvs_lib[task_param[task_id].periph_mode].dvm_periph_lib[i].transition_time;
			f_result->task_transition_time += f_result->periph_result[i].transition_time;
		}
		else{
			f_result->periph_result[i].transition_time = 0;
		}
	}
	
	f_result->task_remaining_time -= f_result->task_transition_time;
}
void calculate_sleep(int task_id, int core_id, int mem_id,int* periph_array, sleep_m* sleep_mode, res_r* f_result){
	int i;

	double task_remaining_time = f_result->task_remaining_time;
	
	if(task_remaining_time<=0){
	  printf("------------------WARNING---------------------\n");
	  printf("NO TIME TO SLEEP FOR %s; FIX SCENARIO!\n",task_param[task_id].name);
	  //return;
	}
	
	if(f_result->task_transition_time>task_remaining_time){
	  printf("------------------WARNING---------------------\n");
	  printf("TRANSITION TIME LARGER THAN SLEEP TIME FOR %s; FIX SCENARIO!\n",task_param[task_id].name);
	  //return;
	}

	//this part need to be fixed
	if(task_param[task_id].previous_task >=0) 
	  f_result->task_remaining_time += (task_param[task_param[task_id].previous_task].deadline*(task_param[task_param[task_id].previous_task].iteration-1)*1000);


	task_remaining_time = f_result->task_remaining_time;


	for(i=0;i<scenario_param.total_cores;i++){
		if(sleep_mode->core_id[i] == 1){
			f_result->core_result[i].sleep_time = task_remaining_time;
		}
		else{
			f_result->core_result[i].active_time = (task_param[task_id].deadline * 1000) - f_result->task_transition_time;
		}
		//printf("CORE %d SLEEP TIME %d\n",i,f_result->core_result[i].sleep_time);
	}

	//memory sleep is not yet supported

	for(i=0;i<scenario_param.total_peripherals;i++){
		if((sleep_mode->periph_id[i] == 1) && periph_array[i]==1){
			f_result->periph_result[i].sleep_time = task_remaining_time;
		}
		else{
			//f_result->periph_result[i].sleep_time = task_remaining_time; //this part need to be fixed
			f_result->periph_result[i].sleep_time = task_remaining_time + f_result->task_transition_time; //this part need to be fixed
			//f_result->periph_result[i].sleep_time = 0;
			//f_result->periph_result[i].active_time += task_remaining_time;
		}
	}
	
	total_sleep_time = task_remaining_time;
}

void calculate_energy(int task_id, int core_id, int mem_id,int* periph_array, sleep_m* sleep_mode, res_r* f_result){

	int i;
	
	//core active energy in mJ
	f_result->core_result[core_id].active_energy =	(f_result->core_result[core_id].active_time * //us
														c_dvs_lib[task_param[task_id].core_mode].dvm_core_lib[core_id].active_current * //mA
														c_dvs_lib[task_param[task_id].core_mode].dvm_core_lib[core_id].operating_voltage)/1000000; //V
	f_result->core_result[core_id].active_energy =	f_result->core_result[core_id].active_energy * task_param[task_id].iteration;

	//mem active energy in mJ
	f_result->mem_result[mem_id].active_energy = ((f_result->load*m_dvs_lib[task_param[task_id].mem_mode].dvm_mem_lib[mem_id].read_energy)+
													(f_result->store*m_dvs_lib[task_param[task_id].mem_mode].dvm_mem_lib[mem_id].write_energy))/1000000;
	f_result->mem_result[mem_id].active_energy =	f_result->mem_result[mem_id].active_energy * task_param[task_id].iteration;
	
	//periph active energy in mJ
	for(i=0;i<scenario_param.total_peripherals;i++){
		if(periph_array[i]==1){
		  f_result->periph_result[i].active_energy = (f_result->periph_result[i].active_time * //us
														p_dvs_lib[task_param[task_id].periph_mode].dvm_periph_lib[i].active_current * //mA
														p_dvs_lib[task_param[task_id].periph_mode].dvm_periph_lib[i].operating_voltage)/1000000; //V
		  f_result->periph_result[i].active_energy = f_result->periph_result[i].active_energy * task_param[task_id].iteration;
		  }
	}


	
	//core sleep energy in mJ
	f_result->core_result[core_id].sleep_energy =	(f_result->core_result[core_id].sleep_time * //us
														c_dvs_lib[task_param[task_id].core_mode].dvm_core_lib[core_id].sleep_current * //mA
														c_dvs_lib[task_param[task_id].core_mode].dvm_core_lib[core_id].operating_voltage)/1000000; //V
	f_result->core_result[core_id].sleep_energy =	f_result->core_result[core_id].sleep_energy * task_param[task_id].iteration;

	//printf("DEBUG core %d sleep time %lf\n",core_id,f_result->core_result[core_id].sleep_time);
	//printf("DEBUG core %d sleep current %lf\n",core_id,c_dvs_lib[task_param[task_id].core_mode].dvm_core_lib[core_id].sleep_current);
	//printf("DEBUG core %d voltage %lf\n",core_id,c_dvs_lib[task_param[task_id].core_mode].dvm_core_lib[core_id].operating_voltage);

	//mem sleep is not supported yet

	//periph sleep energy in mJ
	for(i=0;i<scenario_param.total_peripherals;i++){
		if((sleep_mode->periph_id[i] == 1) && periph_array[i]==1){
		  f_result->periph_result[i].sleep_energy = (f_result->periph_result[i].sleep_time * //us
														p_dvs_lib[task_param[task_id].periph_mode].dvm_periph_lib[i].sleep_current * //mA
														p_dvs_lib[task_param[task_id].periph_mode].dvm_periph_lib[i].operating_voltage)/1000000; //V
		  f_result->periph_result[i].sleep_energy = f_result->periph_result[i].sleep_energy * task_param[task_id].iteration;
		  }
		else{
		  f_result->periph_result[i].sleep_energy = (f_result->periph_result[i].sleep_time * //us
														p_dvs_lib[task_param[task_id].periph_mode].dvm_periph_lib[i].active_current * //mA
														p_dvs_lib[task_param[task_id].periph_mode].dvm_periph_lib[i].operating_voltage)/1000000; //V
		  f_result->periph_result[i].sleep_energy = f_result->periph_result[i].sleep_energy * task_param[task_id].iteration;



		}
	}



	//consider iteration instead of  transition time
	//core transition energy in mJ
	f_result->core_result[core_id].transition_energy =	(task_param[task_id].iteration *
															c_dvs_lib[task_param[task_id].core_mode].dvm_core_lib[core_id].transition_energy)/1000000; //nJ

	//mem sleep is not supported yet
	

	//consider iteration instead of  transition time
	//periph transition energy in mJ
	for(i=0;i<scenario_param.total_peripherals;i++){
		if((sleep_mode->periph_id[i] == 1) && periph_array[i]==1){
		  f_result->periph_result[i].transition_energy = (task_param[task_id].iteration*
															p_dvs_lib[task_param[task_id].periph_mode].dvm_periph_lib[i].transition_energy)/1000 ; //uJ
		  }
	}
}
void execute(){
	int i=0;


	for(i=0;i<scenario_param.total_tasks;i++){
		runtask(task_param[i].id,
				task_param[i].assigned_core->id,
				task_param[i].assigned_mem->id,
				task_param[i].assigned_periph,
				task_param[i].assigned_sleep_mode,
				final_result,
				task_param[i].axf_file);
	}
	
	for(i=0;i<scenario_param.total_tasks;i++){
		calculate_transition(task_param[i].id,
						task_param[i].assigned_core->id,
						task_param[i].assigned_mem->id,
						task_param[i].assigned_periph,
						task_param[i].assigned_sleep_mode,
						&final_result[task_param[i].id]
						);
	}
	//sleep time is calculated after active and transition calculated; because the rest remaining time is in sleep mode	
	for(i=0;i<scenario_param.total_tasks;i++){
		calculate_sleep(task_param[i].id,
						task_param[i].assigned_core->id,
						task_param[i].assigned_mem->id,
						task_param[i].assigned_periph,
						task_param[i].assigned_sleep_mode,
						&final_result[task_param[i].id]
						);
	}
	
	

	for(i=0;i<scenario_param.total_tasks;i++){
		calculate_energy(task_param[i].id,
						task_param[i].assigned_core->id,
						task_param[i].assigned_mem->id,
						task_param[i].assigned_periph,
						task_param[i].assigned_sleep_mode,
						&final_result[task_param[i].id]
						);
	}
}
void print_performance(int task_id, int core_id, int mem_id,int* periph_array, sleep_m* sleep_mode, res_r* f_result){
	
	int i;

	printf("\n-----------------------TASK %d-------------------\n",task_id);
	printf("Task %s (task id %d) running with core %d and mem %d \n",task_param[task_id].name,task_id,core_id,mem_id);
	
	printf("\t task execution time is %.5f us\n",final_result[task_id].task_execution_time);
	printf("\t task remaining time is %.5f us\n",final_result[task_id].task_remaining_time);
	printf("\t task transition time is %.5f us\n",final_result[task_id].task_transition_time);
	
	
	printf("------CORE %d-----\n",core_id);
	printf("\t core active time is %.5f us\n",final_result[task_id].core_result[core_id].active_time);
	printf("\t core sleep time is %.5f us\n",final_result[task_id].core_result[core_id].sleep_time);
	printf("\t core transition time is %.5f us\n",final_result[task_id].core_result[core_id].transition_time);

	printf("------MEM %d-----\n",mem_id);
	printf("\t mem active time is %.5f us\n",final_result[task_id].mem_result[mem_id].active_time);
	printf("\t mem sleep time is %.5f us\n",final_result[task_id].mem_result[mem_id].sleep_time);
	printf("\t mem transition time is %.5f us\n",final_result[task_id].mem_result[mem_id].transition_time);

	
	for(i=0;i<scenario_param.total_peripherals;i++){
		if(periph_array[i]==1){
			printf("-------%s-------\n",periph_param[i].name);
			printf("\t %s active time is %.5f us\n",periph_param[i].name,final_result[task_id].periph_result[i].active_time);
			printf("\t %s sleep time is %.5f us\n",periph_param[i].name,final_result[task_id].periph_result[i].sleep_time);
			printf("\t %s transition time is %.5f us\n",periph_param[i].name,final_result[task_id].periph_result[i].transition_time);
		}
	}

	printf("----------------------END OF TASK %d-------------------\n",task_id);
    
}

void print_energy(int task_id, int core_id, int mem_id,int* periph_array, sleep_m* sleep_mode, res_r* f_result){
	
	int i;

	printf("\n-----------------------TASK %d-------------------\n",task_id);
	printf("Task %s (task id %d) running with core %d and mem %d ",task_param[task_id].name,task_id,core_id,mem_id);
	printf("with peripherals:\n");
	for(i=0;i<scenario_param.total_peripherals;i++){
		if(periph_array[i]==1)printf("- %s\n",periph_param[i].name);
	}
	printf("------CORE %d-----\n",core_id);
	printf("\t core active energy is %.5f mJ\n",final_result[task_id].core_result[core_id].active_energy);
	printf("\t core sleep energy is %.5f mJ\n",final_result[task_id].core_result[core_id].sleep_energy);
	printf("\t core transition energy is %.5f mJ\n",final_result[task_id].core_result[core_id].transition_energy);

	printf("------MEM %d-----\n",mem_id);
	printf("\t mem active energy is %.5f mJ\n",final_result[task_id].mem_result[mem_id].active_energy);
	printf("\t mem sleep energy is %.5f mJ\n",final_result[task_id].mem_result[mem_id].sleep_energy);
	printf("\t mem transition energy is %.5f mJ\n",final_result[task_id].mem_result[mem_id].transition_energy);

	
	for(i=0;i<scenario_param.total_peripherals;i++){
		if(periph_array[i]==1){
			printf("-------%s-------\n",periph_param[i].name);
			printf("\t %s active energy is %.5f mJ\n",periph_param[i].name,final_result[task_id].periph_result[i].active_energy);
			printf("\t %s sleep energy is %.5f mJ\n",periph_param[i].name,final_result[task_id].periph_result[i].sleep_energy);
			printf("\t %s transition energy is %.5f mJ\n",periph_param[i].name,final_result[task_id].periph_result[i].transition_energy);
		}
	}

	printf("----------------------END OF TASK %d-------------------\n",task_id);
    
}



int main(int argc, char **argv){
	int i,j;

	printf("-------------------SCENARIO DESCRIPTION!------------------\n");
	read_scenario_param(argv[1]);
	printf("----------------------POWER TABLE!------------------------\n");
	read_power_lib(argv[2]);

	//*********************initialize**************//
	//remaining_time = (double) (scenario_param.deadline*1000); //in us
	total_execution_time = 0;
	for(j=0;j<scenario_param.total_tasks;j++){
		final_result[j].core_result = (result_r*) malloc(sizeof(result_r)*scenario_param.total_cores);
		final_result[j].mem_result = (result_r*) malloc(sizeof(result_r)*scenario_param.total_srams);
		final_result[j].periph_result = (result_r*) malloc(sizeof(result_r)*scenario_param.total_peripherals);
		final_result[j].sensor_result = (result_r*) malloc(sizeof(result_r)*scenario_param.total_sensors);
		
		final_result[j].load = 0;	
		final_result[j].store= 0;	

		final_result[j].task_execution_time = 0;	
		final_result[j].task_remaining_time = 0;	
		final_result[j].task_transition_time = 0;	

		for(i=0;i<scenario_param.total_cores;i++){
			final_result[j].core_result[i].active_time = 0;
			final_result[j].core_result[i].sleep_time = 0;
			final_result[j].core_result[i].transition_time = 0;

			final_result[j].core_result[i].active_energy = 0;
			final_result[j].core_result[i].sleep_energy = 0;
			final_result[j].core_result[i].transition_energy = 0;

		}
		for(i=0;i<scenario_param.total_srams;i++){
			final_result[j].mem_result[i].active_time = 0;
			final_result[j].mem_result[i].sleep_time = 0;
			final_result[j].mem_result[i].transition_time = 0;

			final_result[j].mem_result[i].active_energy = 0;
			final_result[j].mem_result[i].sleep_energy = 0;
			final_result[j].mem_result[i].transition_energy = 0;

		}
		for(i=0;i<scenario_param.total_peripherals;i++){
			final_result[j].periph_result[i].active_time = 0;
			final_result[j].periph_result[i].sleep_time = 0;
			final_result[j].core_result[i].transition_time = 0;

			final_result[j].periph_result[i].active_energy = 0;
			final_result[j].periph_result[i].sleep_energy = 0;
			final_result[j].periph_result[i].transition_energy = 0;

		}
		for(i=0;i<scenario_param.total_sensors;i++){
			final_result[j].sensor_result[i].active_time = 0;
			final_result[j].sensor_result[i].sleep_time = 0;
			final_result[j].sensor_result[i].transition_time = 0;

			final_result[j].sensor_result[i].active_energy = 0;
			final_result[j].sensor_result[i].sleep_energy = 0;
			final_result[j].sensor_result[i].transition_energy = 0;

		}
	}
	//*********************************************//


	execute();
	
	printf("--------------------PERFORMANCE RESULT!--------------------\n");
	for(i=0;i<scenario_param.total_tasks;i++){
		print_performance(task_param[i].id,
						task_param[i].assigned_core->id,
						task_param[i].assigned_mem->id,
						task_param[i].assigned_periph,
						task_param[i].assigned_sleep_mode,
						&final_result[task_param[i].id]
						);
		
	}


	printf("----------------------ENERGY RESULT!--------------------\n");
	for(i=0;i<scenario_param.total_tasks;i++){
		print_energy(task_param[i].id,
						task_param[i].assigned_core->id,
						task_param[i].assigned_mem->id,
						task_param[i].assigned_periph,
						task_param[i].assigned_sleep_mode,
						&final_result[task_param[i].id]
						);
		
	}
	

	//printf("the clock rate is %d\n",clock_rate);
	//printf("the task deadline is %d ms\n",task_deadline);
	//for(i=0;i<func_cnt;i++)
		//printf("the running function is %s\n",funcname[i]);
	//runFunc(0,func_cnt,funcname);
	//ret=(int)system("./script 2 20");
	//ret = WEXITSTATUS(ret);
	//printf("return value from script is %d\n",ret);
	/* Cleanup */
	//reset_clock();
	return 0;
}
