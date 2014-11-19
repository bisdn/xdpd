#include "packet_io.h"
#include "datapacketx86.h"

#define FWD_MOD_NAME "netfpga10g"

using namespace xdpd::gnu_linux;

void netpfga_io_read_from_port(switch_port_t* port){

	struct pcap_pkthdr header;	/* The header that pcap gives us */
	const u_char* packet;		/* The actual packet */
	packet_matches_t matches;
	netfpga_port_t* state = (netfpga_port_t*)port->platform_port_state;

	//Retrieve an empty buffer
	datapacket_t* pkt=bufferpool::get_free_buffer_nonblocking();
	
	if(!pkt)
		return;

	datapacketx86* pack=(datapacketx86*)pkt->platform_state;
	
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

	//Fill matches
	fill_packet_matches(pkt, &matches);
	//Process packet in
	hal_result r=hal_cmm_process_of1x_packet_in(
		sw->dpid,
		pack->pktin_table_id,
		pack->pktin_reason,
		pack->clas_state.port_in,
		storage_id,
		pkt->__cookie,
		pack->get_buffer(),
		pack->pktin_send_len,
		pack->get_buffer_length(), 
		&matches );


	if ( HAL_FAILURE == r  ) ROFL_DEBUG(" packet_io.cc cmm packet_in unsuccessful");
	if ( HAL_SUCCESS == r  ) ROFL_DEBUG(" \n packet_io.cc cmm packet_in successful \n");


	//ROFL_DEBUG(" packet_io.cc ENDS \n \n \n \n");
	
}
