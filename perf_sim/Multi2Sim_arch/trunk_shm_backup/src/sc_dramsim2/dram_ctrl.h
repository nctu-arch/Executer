/*************************************************

SystemC Wrapper for the DRAMSim2 Simulator
(c) 2/10/2013 W. Rhett Davis (rhett_davis@ncsu.edu)

This wrapper was modified from the vu_dram_ctrl 
wrapper by Ojas Bappat (11/7/2011).

The DRAMSim2 simulator itself is (c) 2010-2011
by Elliott Cooper-Balis, Paul Rosenfeld, and
Bruce Jacob of the University of Maryland
(dramninjas [at] gmail [dot] com)

**************************************************/

/********************************************************************
Date:                   2013_08_19
Modified:               Yin-Chi Peng(IC)
Email:                  psycsieic@gmail.com
LAB:                    SoC/ESW Lab EC447
			Institute of Computer Science and Engineering
			National Chiao Tung University
*********************************************************************/

#ifndef DRAM_CTRL_H
#define DRAM_CTRL_H

#include "tlm.h"  
#include <DRAMSim.h>
#include "hier_param.h"
#include "systemc.h" 
#include <Main.h> 

#define DRAMSIM_READ false
#define DRAMSIM_WRITE true

class dram_ctrl : public sc_core::sc_module
   , virtual public tlm::tlm_fw_transport_if<>
{
 
public: 
  typedef tlm::tlm_generic_payload  *gp_ptr;   ///< Generic Payload pointer

  sc_in_clk clock;

  sc_port <sc_fifo_in_if<gp_ptr> > f_in;
  sc_port <sc_fifo_out_if<gp_ptr> > f_out;
	
  DRAMSim::MultiChannelMemorySystem *mem;

  // DRAMSim2 callbacks	
  void read_complete(unsigned, uint64_t, uint64_t);
  void write_complete(unsigned, uint64_t, uint64_t);

  // Thread process
  void update_clock();

  SC_HAS_PROCESS(dram_ctrl);
  dram_ctrl(sc_core::sc_module_name name, hier_param &hierparams);
  ~dram_ctrl();
  
  map<string,string> param;

  sc_fifo <gp_ptr> intra_fifo; 

  double clock_period;	

  virtual void b_transport(tlm::tlm_generic_payload& gp, 
                           sc_core::sc_time& delay);

  // Required by interface but not implemented
  tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& gp, 
    tlm::tlm_phase& phase, sc_core::sc_time& delay);
  bool get_direct_mem_ptr(tlm::tlm_generic_payload& gp, 
    tlm::tlm_dmi& dmi_data) {return 0;}
  unsigned int transport_dbg(tlm::tlm_generic_payload &gp)
        {return 0;}

  void f_in_main (void);
  void f_out_main (void);
  void intra_fifo_get_main (void); 

private:
  gp_ptr *gp_w_read_complete;
  gp_ptr *gp_w_write_complete;
  gp_ptr *gp_w_intra_fifo_get_main;

  unsigned char gp_w_read_complete_index;
  unsigned char gp_w_write_complete_index;
  unsigned char gp_w_intra_fifo_get_main_index;  

};

#endif
