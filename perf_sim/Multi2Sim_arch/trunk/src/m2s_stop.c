#include <lib/util/debug.h>
#include "m2s_stop.h"

struct m2s_stop_st m2s_stop_interface;

void m2s_stop_init(void *class_ptr,
                   void (*upstop_function) (void *class_ptr)
)
{
	m2s_stop_interface.class_ptr=class_ptr;
	m2s_stop_interface.stop_function=upstop_function;

}

void generic_m2s_stop(void)
{
  m2s_stop_interface.stop_function (m2s_stop_interface.class_ptr);
}

void systemc_interface_m2s_stop(void)
{
	return generic_m2s_stop ();
}
