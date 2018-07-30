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

#include "dram_ctrl.h"

using namespace DRAMSim;

dram_ctrl::dram_ctrl (sc_core::sc_module_name name, hier_param &hierparams) 
  : sc_module(name)
{
  //sc_trace(wf, clock, "dram_ctrl_clock");
  
  param=hierparams.get(sc_object::name());
             
  mem = getMemorySystemInstance(param["DRAMSimMemInitFile"], 
				param["DRAMSimSystemInitFile"],
				param["DRAMSimRunDir"],
				param["ResultsDirName"],
				stoi(param["MemSize"])); 

  clock_period=stof(param["cpu_clock"]);
  mem->setCPUClockSpeed((uint64_t)(1.0/clock_period*1000000000)); // ns

  // Register Read & Write Callbacks with DRAMSim2
  TransactionCompleteCB *read_cb = 
    new Callback<dram_ctrl, void, unsigned, uint64_t, uint64_t>
          (this, &dram_ctrl::read_complete);
  TransactionCompleteCB *write_cb = 
    new Callback<dram_ctrl, void, unsigned, uint64_t, uint64_t>
          (this, &dram_ctrl::write_complete);
  mem->RegisterCallbacks(read_cb, write_cb,NULL);

  gp_w_read_complete = new tlm::tlm_generic_payload *[FIFO_SIZE]; /// transaction pointer
  gp_w_write_complete = new tlm::tlm_generic_payload *[FIFO_SIZE]; // transaction pointer
  gp_w_intra_fifo_get_main = new tlm::tlm_generic_payload *[FIFO_SIZE]; // transaction pointer
  
  //Initial
  for (int i=0; i<FIFO_SIZE; i++)
  {
    gp_w_read_complete[i] = new tlm::tlm_generic_payload;
    gp_w_write_complete[i] = new tlm::tlm_generic_payload;
    gp_w_intra_fifo_get_main[i] = new tlm::tlm_generic_payload;
  }

  gp_w_read_complete_index = 0;
  gp_w_write_complete_index = 0;
  gp_w_intra_fifo_get_main_index = 0;
  
  SC_METHOD (update_clock);
  sensitive << clock.pos();

  SC_METHOD(f_in_main);
  sensitive << clock.pos();
  
  SC_THREAD(intra_fifo_get_main);
  sensitive << clock.pos();
}

dram_ctrl::~dram_ctrl ()
{
  for (int i=0; i<FIFO_SIZE; i++)
  {
    delete gp_w_read_complete[i];
    delete gp_w_write_complete[i];
    delete gp_w_intra_fifo_get_main[i];
  }
  delete [] gp_w_read_complete;
  delete [] gp_w_write_complete;
  delete [] gp_w_intra_fifo_get_main;
}

// pass completion of read from DRAMSIM2 to system 
void dram_ctrl::read_complete
  (unsigned id, uint64_t address, uint64_t clock_cycle)
{ 
  gp_w_read_complete[gp_w_read_complete_index]->set_command(tlm::TLM_READ_COMMAND);
  gp_w_read_complete[gp_w_read_complete_index]->set_address(address);
    
  if (intra_fifo.num_free() > 0)
  {
    intra_fifo.nb_write(gp_w_read_complete[gp_w_read_complete_index++]);  

    if (gp_w_read_complete_index == FIFO_SIZE)
    {
      gp_w_read_complete_index = 0;
    }

    #ifdef PRINT_STATUS
    cout << "[DRAMSim2 has succeeded in writing data into intra_fifo.nb_write] "
         << "[Time: " 
         << sc_core::sc_time_stamp() << "] " 
         << "[CMD: " << "READ] " 
         << "[Address: " << address << "] " 
         << endl;
    #endif
      
  }
  else
  {
    cout << "Read_complete error" << endl;
  }  	
}

// pass completion of write from DRAMSIM2 to system
void dram_ctrl::write_complete
   (unsigned id, uint64_t address, uint64_t clock_cycle)
{
  gp_w_write_complete[gp_w_write_complete_index]->set_command(tlm::TLM_WRITE_COMMAND);
  gp_w_write_complete[gp_w_write_complete_index]->set_address(address);
  
  if (intra_fifo.num_free() > 0)
  {
    intra_fifo.nb_write(gp_w_write_complete[gp_w_write_complete_index++]);
   
    if (gp_w_write_complete_index == FIFO_SIZE)
    {
      gp_w_write_complete_index = 0;
    } 
    
    #ifdef PRINT_STATUS
    cout << "[DRAMSim2 has succeeded in writing data into intra_fifo.nb_write] "
         << "[Time: " 
         << sc_core::sc_time_stamp() << "] " 
         << "[CMD: " << "WRITE] " 
         << "[Address: " << address << "] " 
         << endl;
    #endif
  }
  else
  {
    cout << "Write_complete error" << endl;
  }
}

// This function updates cycle in DRAMSIM2 for every systemC clock cycle
void dram_ctrl::update_clock()
{
  mem->update(); 
}

// Required by interface but not implemented
tlm::tlm_sync_enum  dram_ctrl::nb_transport_fw(tlm::tlm_generic_payload& gp, 
   tlm::tlm_phase& phase, sc_core::sc_time& delay)
{	
  return tlm::TLM_COMPLETED;
}


// Receive transaction from Multi2Sim and pass it to DRAMSim2
void dram_ctrl::f_in_main (void)
{
  gp_ptr gp_r;
  uint64_t addr;
  tlm::tlm_command cmd;  
  
  if (f_in->num_available() > 0)
  {
    // receive transaction from Multi2sim
    f_in->nb_read(gp_r);

    cmd = gp_r->get_command();
    addr = gp_r->get_address();
    gp_r = NULL;
    
    if(cmd  == tlm::TLM_READ_COMMAND)
    {
      // Add transaction to DRAMSim2, let it simulates
      if (!mem->addTransaction(DRAMSIM_READ, addr))   
      {
        cout << "Time: "
             << sc_core::sc_time_stamp()
             << " Read addTransaction error"
             << endl;
      }
      else
      {
        #ifdef PRINT_STATUS
        cout << "[DRAMSim2's wrapper has succeeded in getting a addTransaction] "
             << "[Time: " 
             << sc_core::sc_time_stamp() << "] " 
             << "[CMD: " << cmd << "] " 
             << "[Address: " << addr << "] "
             << endl;
        #endif
        
      }
    }
    else if(cmd  == tlm::TLM_WRITE_COMMAND)
    {
      // Add transaction to DRAMSim2, let it simulates 
      if (!mem->addTransaction(DRAMSIM_WRITE,addr)) 
      {
        cout << "Time: "
             << sc_core::sc_time_stamp()
             << " Write addTransaction error" 
             << endl;
      }
      else
      {
        #ifdef PRINT_STATUS
        cout << "[DRAMSim2's wrapper has succeeded in getting a addTransaction] "
             << "[Time: " 
             << sc_core::sc_time_stamp() << "] " 
             << "[CMD: " << cmd << "] " 
             << "[Address: " << addr << "] " 
             << endl;
        #endif
      }
    }
    else
    {
      ;
    }
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

// Required by interface but not implemented
void dram_ctrl::b_transport(tlm::tlm_generic_payload& gp, 
  sc_core::sc_time& delay)
{
  ;
}

// When DRAMSim2 simulated a request completely, it put the request's
// address to the intra-fifo in DRAMSim2's wrapper,
// and it pass to Multi2sim
void dram_ctrl::intra_fifo_get_main (void)
{
  gp_ptr gp_r;
  
  unsigned int addr;
  tlm::tlm_command cmd_complete;
      
  while (1)
  { 
    // Get the transaction that simulated completely by DRAMSim2 
    intra_fifo.read(gp_r);
    cmd_complete = gp_r->get_command();
    addr = gp_r->get_address();
    gp_r = NULL;
    
    gp_w_intra_fifo_get_main[gp_w_intra_fifo_get_main_index]->set_command(cmd_complete);
    gp_w_intra_fifo_get_main[gp_w_intra_fifo_get_main_index]->set_address(addr);  
    
    if (cmd_complete == tlm::TLM_READ_COMMAND)
    {
      #ifdef PRINT_STATUS
      cout << "[DRAMSim2's intra_fifo.read succeeded. READ transaction completed, respond to Multi2Sim] "
           << "[Time: " 
           << sc_core::sc_time_stamp() << "] " 
           << "[CMD: " << cmd_complete << "] " 
           << "[Address: " << addr << "] "
           << endl;
      #endif
      
      // pass to Multisim       
      f_out->write(gp_w_intra_fifo_get_main[gp_w_intra_fifo_get_main_index++]);
      
      if (gp_w_intra_fifo_get_main_index == FIFO_SIZE)
      {
        gp_w_intra_fifo_get_main_index = 0;
      }  
           
      #ifdef PRINT_STATUS
      cout << "[DRAMSim2 has succeeded in sending a response of READ transaction to Multi2Sim] "
           << "[Time: " 
           << sc_core::sc_time_stamp() << "] " 
           << "[CMD: " << cmd_complete << "] " 
           << "[Address: " << addr << "] "
           << endl;
      #endif  

    }
    else if (cmd_complete == tlm::TLM_WRITE_COMMAND)
    {
      #ifdef PRINT_STATUS
      cout << "[DRAMSim2's intra_fifo.read succeeded. WRITE transaction completed, respond to Multi2Sim] "
           << "[Time: " 
           << sc_core::sc_time_stamp() << "] " 
           << "[CMD: " << cmd_complete << "] " 
           << "[Address: " << addr << "] "
           << endl;
      #endif

      // pass to Multisim
      f_out->write(gp_w_intra_fifo_get_main[gp_w_intra_fifo_get_main_index++]);
  
      if (gp_w_intra_fifo_get_main_index == FIFO_SIZE)
      {
        gp_w_intra_fifo_get_main_index = 0;
      }
      
      #ifdef PRINT_STATUS
      cout << "[DRAMSim2 has succeeded in sending a response of WRITE transaction to Multi2Sim] "
           << "[Time: " 
           << sc_core::sc_time_stamp() << "] " 
           << "[CMD: " << cmd_complete << "] " 
           << "[Address: " << addr << "] "
           << endl;
      #endif
    }
    else
    {
      cout << "Wrong TLM_COMMAND error"
           << endl;
    }
  }
}

