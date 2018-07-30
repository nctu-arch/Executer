/********************************************************************
Date:                   2013_08_19
Modified:               Yin-Chi Peng(IC)
Email:                  psycsieic@gmail.com
LAB:                    SoC/ESW Lab EC447
                        Institute of Computer Science and Engineering
                        National Chiao Tung University
*********************************************************************/

#ifndef __TOP_MODULE_H__
#define __TOP_MODULE_H__

#include "tlm.h"
#include "systemc.h"
#include "m2sSC.h"
#include <sc_dramsim2/dram_ctrl.h>
#include <sc_dramsim2/hier_param.h>

class Top_module : public sc_core::sc_module
{
public:
	
  sc_in_clk cpu_clock; 
  sc_in_clk dram_clock; 
	
  Top_module (sc_core::sc_module_name name, hier_param &hierparams, int *argc_ptr, char **argv); 

  private:

  sc_fifo<gp_ptr> fifo_inst_fw;
  sc_fifo<gp_ptr> fifo_inst_bw;  

  dram_ctrl mem0; //DRAMSim2 instance
  m2sSC iss; //Multi2sim instance 
  
};

#endif /* __TOP_MODULE_H__ */
