/********************************************************************
Date:                   2013_08_19
Modified:               Yin-Chi Peng(IC)
Email:                  psycsieic@gmail.com
LAB:                    SoC/ESW Lab EC447
                        Institute of Computer Science and Engineering
                        National Chiao Tung University
*********************************************************************/

//SystemC Top Module

#include "Top_module.h"

Top_module::Top_module(sc_core::sc_module_name name, hier_param &hierparams, int *argc_ptr, char **argv)
  : sc_core::sc_module(name)
  , mem0("mem0", hierparams)
  , iss("m2s", argc_ptr, argv)
{
  //sc_trace(wf, clock, "top_clock");
  
  iss.clock(cpu_clock);
  mem0.clock(dram_clock);

  iss.f_out (fifo_inst_fw);
  iss.f_in (fifo_inst_bw);
  mem0.f_in (fifo_inst_fw);
  mem0.f_out (fifo_inst_bw);  

}
