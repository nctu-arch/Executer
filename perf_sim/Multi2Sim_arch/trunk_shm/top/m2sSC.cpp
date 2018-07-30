// Multi2Sim base wrapper module implementation

/********************************************************************
Date:                   2013_08_19
Author:                 Chien-Chih Chen
Modified:               Yin-Chi Peng(IC), psycsieic@gmail.com
LAB:                    SoC/ESW Lab EC447
                        Institute of Computer Science and Engineering
                        National Chiao Tung University
*********************************************************************/

#include "m2sSC.h"


SC_HAS_PROCESS( m2sSC );

// ----------------------------------------------------------------------------
//! Custom constructor for the Multi2Sim SystemC module
//! Initializes the underlying Multi2Sim ISS
// ----------------------------------------------------------------------------
m2sSC::m2sSC (sc_core::sc_module_name  name,
		int *argc_ptr, 
		char **argv) :
  sc_module (name)
{
  gp_w_readUpcall  = new tlm::tlm_generic_payload *[FIFO_SIZE];
  gp_w_writeUpcall  = new tlm::tlm_generic_payload *[FIFO_SIZE];
  gp_w_f_in_main = new tlm::tlm_generic_payload *[FIFO_SIZE];
  
  //Initial
  for (int i=0; i<FIFO_SIZE; i++)
  {
    gp_w_readUpcall[i] = new tlm::tlm_generic_payload;
    gp_w_writeUpcall[i] = new tlm::tlm_generic_payload;
    gp_w_f_in_main[i] = new tlm::tlm_generic_payload;
  }
  
  gp_w_readUpcall_index = 0;
  gp_w_writeUpcall_index = 0;
  gp_w_f_in_main_index = 0;
 
  m2s_init (argc_ptr, argv, this, staticReadUpcall, staticWriteUpcall, staticm2sStop);

  SC_METHOD (run);		  // Thread to run the ISS
  sensitive << clock.pos();

  SC_METHOD (intra_fifo_get_main);  //get response sent by DRAMSim2 each cycle
  sensitive << clock.pos();
  
  SC_THREAD (f_in_main); // always get response sent by DRAMSim2, and put it into intra_fifo
  sensitive << clock.pos();


}

m2sSC::~m2sSC ()
{
  for (int i=0; i<FIFO_SIZE; i++)
  {
    delete gp_w_readUpcall[i];
    delete gp_w_writeUpcall[i];
    delete gp_w_f_in_main[i];
  }
  delete [] gp_w_readUpcall;
  delete [] gp_w_writeUpcall;
  delete [] gp_w_f_in_main;
}

// ----------------------------------------------------------------------------
//! The SystemC thread running the underlying ISS

//! The thread calls the ISS to run 
// ----------------------------------------------------------------------------
void
m2sSC::run ()
{
  (void)m2s_loop ();
}	

// ----------------------------------------------------------------------------
//! Static upcall for read from the underlying Multi2Sim ISS

//! Static utility routine is used (for easy C linkage!) that will call the
//! class callback routine. Takes standard C types (the underlying ISS is in
//! C) as arguments, but calls the member routine, ::readUpcall(), with fixed
//! width types (from stdint.h).
// ----------------------------------------------------------------------------
int
m2sSC::staticReadUpcall (void              *instancePtr,
			     unsigned long int  addr,
			     unsigned char      mask[],
			     unsigned char      rdata[],
			     int                dataLen)
{
  m2sSC *classPtr = (m2sSC *) instancePtr;
  return  classPtr->readUpcall (addr, mask, rdata, dataLen);

}	// staticReadUpcall()


// ----------------------------------------------------------------------------
//! Static upcall for write to the underlying Multi2Sim ISS

//! Static utility routine is used (for easy C linkage!) that will call the
//! class callback routine. Takes standard C types (the underlying ISS is in
//! C) as arguments, but calls the member routine, ::writeUpcall(), with fixed
//! width types (from stdint.h).
// ----------------------------------------------------------------------------
int
m2sSC::staticWriteUpcall (void              *instancePtr,
			      unsigned long int  addr,
			      unsigned char      mask[],
			      unsigned char      wdata[],
			      int                dataLen)
{
  m2sSC *classPtr = (m2sSC *) instancePtr;
  return  classPtr->writeUpcall (addr, mask, wdata, dataLen);

}	// staticWriteUpcall()


// ----------------------------------------------------------------------------
//! Static upcall for stop to the underlying Multi2Sim ISS

//! Static utility routine is used (for easy C linkage!) that will call the
//! class callback routine.
// ----------------------------------------------------------------------------
void
m2sSC::staticm2sStop (void              *instancePtr)
{
  m2sSC *classPtr = (m2sSC *) instancePtr;
  return  classPtr->m2s_stop_function ();

}

// ----------------------------------------------------------------------------
//! Member function to handle read upcall from the underlying Multi2Sim ISS

//! This is the entry point used by ::staticReadUpcall(). Constructs a Generic
//! transactional payload for the read, then passes it to ::doTrans() (also
//! used by ::writeUpcall()) for transport to the target.
// ----------------------------------------------------------------------------
int
m2sSC::readUpcall (unsigned long int  addr,
                   unsigned char      mask[],
                   unsigned char      rdata[],
                   int                dataLen)
{

  gp_w_readUpcall[gp_w_readUpcall_index]->set_command(tlm::TLM_READ_COMMAND);
  gp_w_readUpcall[gp_w_readUpcall_index]->set_address( (sc_dt::uint64)addr );
  
  while (f_out->num_free() > 0)
  {
    f_out->nb_write(gp_w_readUpcall[gp_w_readUpcall_index++]);
  
    if (gp_w_readUpcall_index == FIFO_SIZE)
    {
      gp_w_readUpcall_index = 0;
    }

    #ifdef PRINT_STATUS
    cout << "[Multi2Sim Perform the READ transaction] "
         << "[Time: " 
         << sc_core::sc_time_stamp() << "] " 
         //<< sc_object::name() << " " 
         << "[CMD: " << "READ" << "] "
         << "[Address: " << addr << "] "
         << endl;
    #endif
    break;
  }
}

// ----------------------------------------------------------------------------
//! Member function to handle write upcall from the underlying Multi2Sim ISS

//! This is the entry point used by ::staticWriteUpcall(). Constructs a
//! Generic transactional payload for the write, then passes it to ::doTrans()
//! (also used by ::readUpcall()) for transport to the target.
// ----------------------------------------------------------------------------
int
m2sSC::writeUpcall (unsigned long int  addr,
                    unsigned char      mask[],
                    unsigned char      wdata[],
                    int                dataLen)
{
  
  gp_w_writeUpcall[gp_w_writeUpcall_index]->set_command(tlm::TLM_WRITE_COMMAND);
  gp_w_writeUpcall[gp_w_writeUpcall_index]->set_address( (sc_dt::uint64)addr );
 
  while (f_out->num_free() > 0)
  { 
    f_out->nb_write(gp_w_writeUpcall[gp_w_writeUpcall_index++]);
    
    if (gp_w_writeUpcall_index == FIFO_SIZE)
    {
      gp_w_writeUpcall_index = 0;
    }
    
    #ifdef PRINT_STATUS
    cout << "[Multi2Sim Perform the WRITE transaction] "
         << "[Time: " 
         << sc_core::sc_time_stamp() << "] "
         << "[CMD: " << "WRITE" << "] "
         //<< sc_object::name() << " " 
         << "[Address: " << addr << "] " 
         << endl;
    #endif
    break;
  }    
}

void
m2sSC::m2s_stop_function (void)
{
  sc_stop();
}

// ----------------------------------------------------------------------------
//! TLM transport to the target
// ----------------------------------------------------------------------------

//This function does not be called
void
m2sSC::doTrans( tlm::tlm_generic_payload &trans )
{
  ;
}	

//This function does not be called
tlm::tlm_sync_enum  m2sSC::nb_transport_bw( tlm::tlm_generic_payload &gp,
                           tlm::tlm_phase &phase, sc_core::sc_time &delay)
{
  tlm::tlm_sync_enum status;
  status = tlm::TLM_ACCEPTED;
  return status;
}

//This function does not be called
void m2sSC::invalidate_direct_mem_ptr					
  (sc_dt::uint64 start_range, sc_dt::uint64 end_range)
{  
    return;
}

// Multi2sim_intra-fifo <- FIFO <- DRAMSim2 
void m2sSC::f_in_main (void)
{
  gp_ptr gp_r; 

  sc_dt::uint64 addr;
  tlm::tlm_command cmd;  

  while (1)
  {    
    f_in->read(gp_r); 

    cmd = gp_r->get_command();
    addr = gp_r->get_address();
    gp_r = NULL;

    #ifdef PRINT_STATUS
    cout << "[Multi2Sim's wrapper received DRAMSim2's response] "
         << "[Time: " 
         << sc_core::sc_time_stamp() << "] " 
         << "[CMD: " << cmd << "] " 
         << "[Address: " << addr << "] "
         << endl;
    #endif
            
    gp_w_f_in_main[gp_w_f_in_main_index]->set_command(cmd);                
    gp_w_f_in_main[gp_w_f_in_main_index]->set_address(addr);                 

    intra_fifo.write(gp_w_f_in_main[gp_w_f_in_main_index++]);
    
    if (gp_w_f_in_main_index == FIFO_SIZE)
    {
      gp_w_f_in_main_index = 0;
    }
   
    #ifdef PRINT_STATUS
    cout << "[Multi2Sim's wrapper write responsed data into intra_fifo] "
         << "[Time: " 
         << sc_core::sc_time_stamp() << "] " 
         << "[CMD: " << cmd << "] " 
         << "[Address: " << addr << "] "
         << endl;
    #endif
  }
}


// Multi2Sim event schedule <- Multi2Sim_intra-fifo
void m2sSC::intra_fifo_get_main (void)
{
  gp_ptr gp_r; 

  sc_dt::uint64 addr;
  tlm::tlm_command cmd;  

  if (intra_fifo.num_available() > 0)
  {
    intra_fifo.nb_read(gp_r);

    cmd = gp_r->get_command();
    addr = gp_r->get_address();
    gp_r = NULL;
    
    #ifdef PRINT_STATUS
    cout << "[Multi2Sim has succeeded in getting response from intra_fifo.nb_read] "
         << "[Time: " 
         << sc_core::sc_time_stamp() << "] " 
         << "[CMD: " << cmd << "] " 
         << "[Address: " << addr << "] "
         << endl;
    #endif
    
    // ----------------------------------------------------------------------------     
    // IC_S_2013_08_15
    // Call dramsim2_access_finish
    // The data(stack) has simulated by DRAMSim2,
    // and it must put into the m2s.   
    // We put the data(stack) into m2s's event queue,
    // let the stack be simulated in m2s
    // Parameter must be addr and access type
    //IC_E
    // ----------------------------------------------------------------------------   
    
    dramsim2_access_finish(addr, 0);   //0: DRAM Controller ID
  }
  else
  {
    #ifdef PRINT_STATUS
    //cout << "[Time: " 
    //     << sc_time_stamp().to_string() << "] "
    //     << "[" << sc_object::name() << "] "
    //     << "[No item is in fifo] "
    //     << endl;
    #endif
  }
}
