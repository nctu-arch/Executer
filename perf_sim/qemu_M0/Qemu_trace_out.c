#include <stdio.h>
#include <stdlib.h>
#include "Qemu_trace.h"

static FILE *trc_out;
static compact_TRACE_rec trc_out_buf[TRC_BUF_MAX];
static int trc_buf_fill;

int flush_trc_buf(void);
static compact_TRACE_rec* get_rec_from_buf(void);

void TRACE_out_init (char *trc_file_name)
{
	if(0 == trc_file_name){
		printf("ERROR: no output file name.\n");
		exit(1);
	}//file name = 0
	else{
		trc_out = fopen(trc_file_name,"w");
		if(NULL == trc_out){
			printf("ERROR: open file-%s failed,\n",trc_file_name);
			exit(1);
		}//open error
	}//open file

	trc_buf_fill = 0;
}//TRACE_out_init

void TRACE_out_done (void)
{
	flush_trc_buf();
	fclose(trc_out);
}//TRACE_out_done

int TRACE_out_rec(unsigned long timeoffset, int cpuid, TRACE_type type, uint64_t addr)
{
	compact_TRACE_rec *rec;

	rec = get_rec_from_buf();
	
	rec->time_stamp = timeoffset;
	rec->cpu_id     = cpuid;
	rec->event_type = type;
	rec->addr       = addr;

	return 0;	
}//TRACE_out_rec

//---------------------------------//
//          private function       //
//---------------------------------//

int flush_trc_buf(void)
{
	fwrite(trc_out_buf, trc_buf_fill, sizeof(compact_TRACE_rec), trc_out);
	trc_buf_fill = 0;
	return 0;	
}//flush out buf to file


static compact_TRACE_rec* get_rec_from_buf(void)
{
	if( trc_buf_fill == TRC_BUF_MAX ){
		flush_trc_buf();
	}//flush buf to file

	return &trc_out_buf[trc_buf_fill++];
}//get_rec_from_buf
