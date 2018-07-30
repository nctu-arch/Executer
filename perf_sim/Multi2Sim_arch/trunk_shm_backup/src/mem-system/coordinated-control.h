// vim: noexpandtab tabstop=8 shiftwidth=4 softtabstop=4

/*
  #===============================================================================
  #
  #          FILE:  coordinated-control.h
  # 
  #         USAGE:  implement coordinated-control (MICRO2009) 
  # 
  #   DESCRIPTION:  
  # 
  #       OPTIONS:  ---
  #  REQUIREMENTS:  ---
  #          BUGS:  ---
  #         NOTES:  ---
  #        AUTHOR:  IC (Yin-Chi Peng), psycsieic@gmail.com
  #       COMPANY:  ---
  #       VERSION:  1.0
  #       CREATED:  12/07/2012 16:43:02 AM PDT
  #      REVISION:  ---
  #      MODIFIED:  
  #
  #===============================================================================
*/
/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MEM_SYSTEM_COORDINATED_CONTROL_H
#define MEM_SYSTEM_COORDINATED_CONTROL_H

struct coordinated_control_t
{
    float corei_L1_prefetch_accuracy;
    int corei_pollution;
    int bandwidth_consumed_by_corei;
    int bandwidth_needed_by_corei; 
};

struct coordinated_control_t *coordinated_control_create(int cc_cores);
void coordinated_control_free(struct coordinated_control_t *coordi);
void coordinated_control_access(struct mod_stack_t *stack, struct mod_t *mod);


#endif
