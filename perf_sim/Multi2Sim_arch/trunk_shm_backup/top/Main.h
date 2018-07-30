/********************************************************************
Date:                   2013_08_19
Modified:               Yin-Chi Peng(IC)
Email:                  psycsieic@gmail.com
LAB:                    SoC/ESW Lab EC447
                        Institute of Computer Science and Engineering
                        National Chiao Tung University
*********************************************************************/

#ifndef __MAIN_H__
#define __MAIN_H__

#include "tlm.h"
#include "systemc.h"
#include "./sc_dramsim2/hier_param.h"
#include "tlm_utils/simple_initiator_socket.h"
//#include "fsdb_trace_file.h" //IC_S_2013_08_22
extern "C" {
	#include "m2s.h"
	#include "mem-system/dram-handler.h"
	//#include "trace_driven/trace_driven.h"
}

#define FIFO_SIZE 16 //default sc_fifo size

//extern sc_trace_file *wf;

typedef tlm::tlm_generic_payload  *gp_ptr;   //< Generic Payload pointer
   
#endif //__MAIN_H__
