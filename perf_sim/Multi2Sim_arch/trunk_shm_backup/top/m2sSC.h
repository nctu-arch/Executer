// Multi2Sim base wrapper module definition

/********************************************************************
Date:                   2013_08_19
Author:                 Chien-Chih Chen
Modified:               Yin-Chi Peng(IC), psycsieic@gmail.com
LAB:                    SoC/ESW Lab EC447
                        Institute of Computer Science and Engineering
                        National Chiao Tung University
*********************************************************************/

#ifndef M2S_SC__H
#define M2S_SC__H

#include <stdint.h>
#include "Main.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"

extern "C" {
	#include "m2s.h"
	#include "mem-system/dram-handler.h"
	//#include "trace_driven/trace_driven.h"
}
#include "systemc.h"

using namespace std;

// ----------------------------------------------------------------------------
//! SystemC module class wrapping Multi2Sim ISS
//! Provides a single thread (::run) which runs the underlying Multi2Sim ISS.
// ----------------------------------------------------------------------------

class m2sSC
  : public sc_core::sc_module
  , virtual public tlm::tlm_bw_transport_if<>  
{
public:

  sc_in_clk clock; 
   
  sc_port <sc_fifo_out_if<gp_ptr> > f_out;
  sc_port <sc_fifo_in_if<gp_ptr> > f_in;      

  sc_fifo <gp_ptr> intra_fifo;
    
  m2sSC (sc_core::sc_module_name  name, int *argc_ptr, char **argv);
  ~m2sSC ();
  
  void f_in_main (void);
  void intra_fifo_get_main (void);
  
protected:

  // Thread which will run the model. This will be reimplemented in later
  // derived classes to deal with timing.
  virtual void  run ();

  // The common thread to make the transport calls. This will be reimplemented
  // in later derived classes to deal with timing.
  virtual void  doTrans (tlm::tlm_generic_payload &trans);


private:

  //! The transactional payload on the bus.

  //! The instinct is to declare this locally in the transaction method where
  //! it will be used. However the underlying SystemC data structures are
  //! apparently large with complex initialization. Although not documented in
  //! the LRM, it is far more efficient to declare the payload once here.

  //! Only one upcall is ever in use at a time, so there is no danger of a
  //! clash in usage.
  
  //! Since this is only used by the upcall methods, it can be private to this
  //! base class.

  //tlm::tlm_generic_payload  trans;

  // I/O upcalls from Multi2Sim
  static int  staticReadUpcall (void              *instancePtr,
				unsigned long int  addr,
				unsigned char      mask[],
				unsigned char      rdata[],
				int                dataLen);

  static int  staticWriteUpcall (void              *instancePtr,
				 unsigned long int  addr,
				 unsigned char      mask[],
				 unsigned char      wdata[],
				 int                dataLen);
  
  static void  staticm2sStop (void              *instancePtr); //IC_S_2013_08_01
  
  int         readUpcall (unsigned long int  addr,
			  unsigned char      mask[],
			  unsigned char      rdata[],
			  int                dataLen);

  int         writeUpcall (unsigned long int  addr,
			   unsigned char      mask[],
			   unsigned char      wdata[],
			   int                dataLen);
			   
  void        m2s_stop_function (void); //IC_S_2013_08_01
	
	
  /// Not Implemented for this example but required by the initiator socket
  void invalidate_direct_mem_ptr (sc_dt::uint64 start_range, sc_dt::uint64 end_range);
  tlm::tlm_sync_enum nb_transport_bw (tlm::tlm_generic_payload  &gp, 
                                           tlm::tlm_phase &phase, 
                                           sc_core::sc_time &delay);

  gp_ptr *gp_w_readUpcall;  // Generic Payload pointer
  gp_ptr *gp_w_writeUpcall; // Generic Payload pointer
  gp_ptr *gp_w_f_in_main;   // Generic Payload pointer
  
  unsigned char gp_w_readUpcall_index;
  unsigned char gp_w_writeUpcall_index; 
  unsigned char gp_w_f_in_main_index; 

};

#endif	// M2S_SC__H

