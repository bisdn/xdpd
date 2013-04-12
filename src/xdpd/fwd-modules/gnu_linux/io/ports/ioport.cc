/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ioport.h"

#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

//Constructor and destructor
ioport::ioport(switch_port_t* of_ps, unsigned int q_num)
{
	//Input queue
	input_queue = new ringbuffer();

	//Output queues
	num_of_queues = q_num;	
	output_queues = new ringbuffer[num_of_queues];

	//of_port_state
	of_port_state = of_ps;
}
ioport::~ioport(){

	delete input_queue;
	delete [] output_queues;
}

//Get buffer status
ringbuffer_state_t ioport::get_input_queue_state(){
	return input_queue->get_buffer_state();
} 

ringbuffer_state_t ioport::get_output_queue_state(unsigned int q_id){
	if(q_id<num_of_queues)
		return output_queues[q_id].get_buffer_state();
	else{
		//TODO: print trace
		return RB_BUFFER_INVALID;
	} 
}

/**
 * Sets the port receiving behaviour. This MUST change the of_port_state appropiately
 */
rofl_result_t ioport::set_drop_received_config(bool drop_received){
	of_port_state->drop_received = drop_received;
	return ROFL_SUCCESS;
}

/**
 * Sets the port output behaviour. This MUST change the of_port_state appropiately
 */
rofl_result_t ioport::set_forward_config(bool forward_packets){
	of_port_state->forward_packets = forward_packets;
	return ROFL_SUCCESS;
}

/**
 * Sets the port Openflow specific behaviour for non matching packets (PACKET_IN). This MUST change the of_port_state appropiately
 */
rofl_result_t ioport::set_generate_packet_in_config(bool generate_pkt_in){
	of_port_state->of_generate_packet_in = generate_pkt_in;
	return ROFL_SUCCESS;
}

/**
 * Sets the port advertised features. This MUST change the of_port_state appropiately
 */
rofl_result_t ioport::set_advertise_config(uint32_t advertised){
	of_port_state->advertised = (port_features_t)advertised;
	return ROFL_SUCCESS;
}

