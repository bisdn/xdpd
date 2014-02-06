#include "packet_io.h"

#define FWD_MOD_NAME "netfpga10g"

using namespace xdpd::gnu_linux;

void netpfga_io_read_from_port(switch_port_t* port){

	netfpga_port_t* state = (netfpga_port_t*)port->platform_port_state;

	//ROFL_DEBUG("["FWD_MOD_NAME"] packet_io.cc Got a packet from kernel (PKT_IN) in port %s %p!\n", port->name, state);
	

	//Retrieve an empty buffer
	datapacket_t* pkt=bufferpool::get_free_buffer(false);

	//ROFL_DEBUG(" packet_io.cc pkt->platform_state %p \n",pkt->platform_state);

	datapacketx86* pack=(datapacketx86*)pkt->platform_state;
	
	
	
	//ROFL_DEBUG(" packet_io.cc pack->get_FRAME_SIZE_BYTES() %p \n",pack->get_FRAME_SIZE_BYTES());
	
	//uint8_t* x[pack->get_FRAME_SIZE_BYTES()];
	//uint8_t* buff=(uint8_t*)x;

	
	

	

	struct pcap_pkthdr header;	/* The header that pcap gives us */
	const u_char* packet;		/* The actual packet */

	/* Grab a packet */
	packet = pcap_next(state->pcap_fd, &header);






	if (header.len < 0) 
		ROFL_DEBUG(" packet_io.cc Reading to a socket unsuccessful: %d",errno);
	
	//ROFL_DEBUG(" in the buff %d \n",*buff);
	//ROFL_DEBUG(" read size %d \n",header.len);

	

	pack->init((uint8_t*)packet, header.len, port->attached_sw, port->of_port_num, 0, true, false);

	//ROFL_DEBUG(" packet_io.cc buffer %p \n",pack->get_buffer());

	of1x_switch_t* sw=(of1x_switch_t*)port->attached_sw;

	of_switch_t* lsw;
	lsw = physical_switch_get_logical_switch_by_dpid(sw->dpid);


	datapacket_storage* storage;
	storage=((logical_switch_internals*)lsw->platform_state)->storage;



	storeid storage_id=storage->store_packet(pkt);
	//ROFL_DEBUG(" PACKET_IN storage ID %d for datapacket pkt %d dpid %d  \n", storage_id, pkt,sw->dpid);





	__of1x_init_packet_matches(pkt);// tranform packet->matches into of1x matches
	
	of1x_packet_matches_t* matches = &pkt->matches.of1x;
	
	
	//ROFL_DEBUG(" of1x_switch_t* %d , port->attached_sw %d ", sw, port->attached_sw);
	ROFL_DEBUG("\n in port: %x ", pack->in_port);
	

	afa_result r=cmm_process_of1x_packet_in(
		sw,
		pack->pktin_table_id,
		pack->pktin_reason,
		pack->in_port,
		storage_id,
		pack->get_buffer(),
		header.len,
		pack->get_buffer_length(), 
		*matches );


	if ( AFA_FAILURE == r  ) ROFL_DEBUG(" packet_io.cc cmm packet_in unsuccessful");
	if ( AFA_SUCCESS == r  ) ROFL_DEBUG(" \n packet_io.cc cmm packet_in successful \n");


	//ROFL_DEBUG(" packet_io.cc ENDS \n \n \n \n");
	
}
