/********************************************************************
Date:                   2013_08_19
Modified:               Yin-Chi Peng(IC)
Email:                  psycsieic@gmail.com
LAB:                    SoC/ESW Lab EC447
                        Institute of Computer Science and Engineering
                        National Chiao Tung University
*********************************************************************/

#include <iostream>

#include <tlm.h>
#include "Top_module.h"
#include "Main.h"

#include <sc_dramsim2/hier_param.h>

using std::cerr;
using std::endl;

//sc_trace_file *wf = sc_create_vcd_trace_file("waveform");

int  sc_main( int   argc,
	      char **argv )
{
  
  sc_clock cpu_clk("CPU_CLK", 1, SC_NS, 0.5, 0, SC_NS, true);
  sc_clock dram_clk("DRAM_CLK", 3, SC_NS, 0.5, 0, SC_NS, true);
  
    
  hier_param hierparams("parameters.txt");

  Top_module top("Top_module", hierparams, &argc, argv);
  
  top.cpu_clock(cpu_clk);
  top.dram_clock(dram_clk);
 
  //fsdbDumpfile("dump.fsdb");
  //fsdbDumpvars(0, top); 
  sc_core::sc_start();
  //sc_close_vcd_trace_file(wf);
  
  return 0;			// Should never happen!

}	// sc_main()


// EOF
