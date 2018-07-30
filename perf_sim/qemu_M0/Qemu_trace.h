#ifndef QEMU_TRACE_H
#define QEMU_TRACE_H

#include <inttypes.h>

#define TRC_BUF_MAX 40960
#define CPU_MAX     256

typedef enum{
	TR_NULL,
	TR_READ,
	TR_WRITE,
	TR_TB    //TB's start
} TRACE_type;

typedef struct trace_struct{
	unsigned long long time_stamp; //total instruction count
	int cpu_id;
	TRACE_type event_type;
	uint64_t addr;
} TRACE_rec;

typedef struct compact_trace_struct{
	unsigned short  time_stamp; //TB's instruction count
	unsigned short  cpu_id;
	unsigned short  event_type;
	uint64_t addr;
} compact_TRACE_rec;

/* public function */

void TRACE_in_init (char *trc_file_name);
void TRACE_in_done (void);
TRACE_rec* get_TRACE_rec(void);

void TRACE_out_init (char *trc_file_name);
void TRACE_out_done (void);
int TRACE_out_rec(unsigned long timeoffset, int cpuid, TRACE_type type, uint64_t addr);

#endif //QEMU_TRACE_H
