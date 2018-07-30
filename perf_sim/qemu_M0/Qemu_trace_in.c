#include <stdio.h>
#include <stdlib.h>
#include "Qemu_trace.h"

static FILE *trc_in;
static TRACE_rec rec_new;
static unsigned long cpu_times_stamp_now[CPU_MAX];
static int trace_buf_read, trace_buf_avail;
static compact_TRACE_rec* get_rec_from_buf(void);

void TRACE_in_init (char *trc_file_name)
{
	if(trc_file_name==0){
		printf("ERROR: no trace file's name\n");
		exit(1);
	}
	else{
		trc_in = fopen(trc_file_name, "r");
		if(trc_in == NULL){
			printf("ERROR: file-%s open faild.\n",trc_file_name);
			exit(1);
		}
	}//else

	//private variable init
	memset(cpu_times_stamp_now, 0, sizeof(cpu_times_stamp_now));
	trace_buf_read = 0;
	trace_buf_avail = 0;
}//TRACE_in_init


void TRACE_in_done (void)
{
	fclose(trc_in);
}//TRACE_in_done

TRACE_rec* get_TRACE_rec(void)
{
	compact_TRACE_rec *rec;

	rec = get_rec_from_buf();
	if(0 == rec){
		return 0;
	}//no rec

	if(256 <= rec->cpu_id){
		printf("ERROR: cpu time_stamp buf is overflow.\n");
		exit(1);
	}

	cpu_times_stamp_now[rec->cpu_id] += rec->time_stamp;
	rec_new.time_stamp = cpu_times_stamp_now[rec->cpu_id];
	rec_new.cpu_id     = rec->cpu_id;
	rec_new.event_type = rec->event_type;
	rec_new.addr       = rec->addr;

	return &rec_new;
}//get_TRACE_rec


//---------------------------------//
//          private function       //
//---------------------------------//

static compact_TRACE_rec trc_buf_in[TRC_BUF_MAX];

static compact_TRACE_rec *get_rec_from_buf(void)
{

	if(trace_buf_avail==0){
		trace_buf_avail = fread(trc_buf_in, sizeof(compact_TRACE_rec), TRC_BUF_MAX, trc_in);
        trace_buf_read=0;
	}//fread from file

	if(trace_buf_avail==0){
		return 0;
	}//file is empty

	trace_buf_avail--;
	return &trc_buf_in[trace_buf_read++];
}//get_rec_from_buf

