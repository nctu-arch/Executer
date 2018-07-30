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
int load;
int store;

typedef struct st{
	int loop;
	int deadline;
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
	int period;
	int iteration;
	core_t* assigned_core;
	mem_t* assigned_mem;
	int* assigned_periph;
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
typedef struct rcore{
	int id;
	double active_time;
	double sleep_time;
	double transition_time;
	
	double active_energy;
	double sleep_energy;
	double transition_energy;


}core_r;

typedef struct rperiph{
	int id;
	double active_time;
	double sleep_time;
	double transition_time;
	
	double active_energy;
	double sleep_energy;
	double transition_energy;
	

}periph_r;

typedef struct res{
	core_r* core_result;
	periph_r* periph_result;

}result_r;

result_r final_result;

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
					dvs_index = -1;	
					c_dvs_lib = (c_dvs_t*) malloc(sizeof(c_dvs_t)*DFVS_mode);

					for(i=0;i<DFVS_mode;i++)
					c_dvs_lib[i].dvm_core_lib = (core_p*) malloc(sizeof(core_p)*scenario_param.total_cores);
					
				}
			    else if(!strcmp(key, "mode")){
					dvs_index++;
					core_lib_index = -1;
					c_dvs_lib[dvs_index].mode = atoi(token.data.scalar.value);
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
					dvs_index++;
					mem_lib_index = -1;
					m_dvs_lib[dvs_index].mode = atoi(token.data.scalar.value);
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
					dvs_index++;
					periph_lib_index = -1;
					p_dvs_lib[dvs_index].mode = atoi(token.data.scalar.value);
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
					dvs_index++;
					sensor_lib_index = -1;
					s_dvs_lib[dvs_index].mode = atoi(token.data.scalar.value);
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
			    else if(!strcmp(key, "deadline")){
					scenario_param.deadline = atoi(token.data.scalar.value);
					printf("\tdeadline = %d\n",scenario_param.deadline);
				}
				else if(!strcmp(key, "total tasks")){
					scenario_param.total_tasks = atoi(token.data.scalar.value);	
        			printf("\ttotal tasks = %d\n",scenario_param.total_tasks);

					task_param = (task_t*) malloc(sizeof(task_t)*scenario_param.total_tasks);
					task_index= 0;
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
					printf("\tsram%d_size = %d\n\n",mem_index,mem_param[mem_index].size);
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
			    else if(!strcmp(key,"period")){
					task_param[task_index].period = atoi(token.data.scalar.value);
					printf("\t%s period = %d\n",task_param[task_index].name,task_param[task_index].period);
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
			    else if(!strcmp(key,"assigned mem id")){
					temp = atoi(token.data.scalar.value);
					task_param[task_index].assigned_mem = &mem_param[temp];
					printf("\t%s use mem %d\n",task_param[task_index].name,task_param[task_index].assigned_mem->id);
				}
			    else if(!strcmp(key,"assigned periph id")){
					temp = atoi(token.data.scalar.value);
					task_param[task_index].assigned_periph[temp] = 1;
					printf("\t%s use periph %s\n",task_param[task_index].name,periph_param[temp].name);
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


void reset_clock(){
	//key_t shm_key = 263;
	//int trans_cycle_id;
	//trans_cycle_id = shmget(shm_key, sizeof(int), 0666);
	/*if((trans_cycle = (int *)shmat(trans_cycle_id, NULL, 0)) == -1){
		printf("shmatl error: %x \n", trans_cycle);
		perror("shmat");
		shmctl(trans_cycle, IPC_RMID, 0);
		exit(-1);
	}*/
	//trans_cycle[0] = 0;
	//*trans_cycle = 0;
	int i=0;
	memcpy(trans_cycle,&i,sizeof(int));
	printf("(reset)trans_cycle : %d\n",*trans_cycle);
}
void clock_init(){
	key_t shm_key = 263;
	int trans_cycle_id;
	int i=0;
	trans_cycle_id = shmget(shm_key, sizeof(int), 0666);
	if((trans_cycle = (int *)shmat(trans_cycle_id, NULL, 0)) == -1){
		printf("shmatl error: %x \n", trans_cycle);
		perror("shmat");
		shmctl(trans_cycle, IPC_RMID, 0);
		exit(-1);
	}
	//*trans_cycle = 0;
	memcpy(trans_cycle,&i,sizeof(int));
	printf("(init)trans_cycle : %d\n",*trans_cycle);
}



int runtask(int core_id, int mem_id,int* periph_array, sleep_m* sleep_mode, char* axf ){
    char str[256];
    int i;
    int total_cycle_cnt=0;
    FILE *fd;
 
	if(core_id==1){   
	sprintf(str, "/home/cws/perf_sim/executor/run_m4.sh %s\n", axf);
	}
	else if(core_id==0){
	sprintf(str, "/home/cws/perf_sim/executor/run_m0.sh %s\n", axf);
	}
	fd = popen(str,"r");
	pclose(fd);

	sim_result(str);

	printf("cycle count = %d\n",cycle_count);
	printf("load  = %d, store = %d\n",load,store);
    
    return 0;//system(str);

}

void execute(){
	int i=0;

	for(i=0;i<scenario_param.total_tasks;i++){
		runtask(task_param[i].assigned_core->id,
				task_param[i].assigned_mem->id,
				task_param[i].assigned_periph,
				task_param[i].assigned_sleep_mode,
				task_param[i].axf_file);
	}
	



}
int main(int argc, char **argv){
	
	printf("-------------------SCENARIO DESCRIPTION!------------------\n");
	read_scenario_param(argv[1]);
	printf("----------------------POWER TABLE!------------------------\n");
	read_power_lib(argv[2]);


	execute();
	
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
