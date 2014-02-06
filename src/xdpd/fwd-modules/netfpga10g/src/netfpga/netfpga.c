#include "netfpga.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <rofl/common/utils/c_logger.h>
#include <assert.h>

#include "regs.h"
#include "flow_entry.h"

#define NETFPGA_DEVNAME "/dev/nf10"

//Pointer to the unique (static) netfpga_device_t
static netfpga_device_t* nfpga=NULL;


/*
 * Internal HW stuff
 */

//Dump wildcard entries
rofl_result_t netfpga_dump_wildcard_hw_entries(){

       unsigned int i,j;
       uint32_t* aux;
       netfpga_flow_entry_t* entry;

       //Create an empty entry
       entry = netfpga_init_flow_entry();      
       
       //Wait for the netfpga to be ready
       netfpga_wait_reg_ready(nfpga);

       //Loop over all entries
       for(j =0; j < NETFPGA_OPENFLOW_WILDCARD_FULL_TABLE_SIZE; j++){

               entry->hw_pos =j;

               //PUT something strange
               memset(entry->matches, 0xEE, sizeof(*(entry->matches)));
               memset(entry->masks, 0xDD, sizeof(*(entry->masks)));
               memset(entry->actions, 0xCC, sizeof(*(entry->actions)));
               
               //Set Row address
               if(netfpga_write_reg(nfpga, NETFPGA_OF_BASE_ADDR_REG, NETFPGA_WILDCARD_BASE + j) != ROFL_SUCCESS)
                       return ROFL_FAILURE;
       
               //Write whatever => Trigger read
               if(netfpga_write_reg(nfpga, NETFPGA_OF_READ_ORDER_REG, 0x1) != ROFL_SUCCESS)
                       return ROFL_FAILURE;

       
               //Read matches
               aux = (uint32_t*)entry->matches;
               for (i = 0; i < NETFPGA_FLOW_ENTRY_MATCHES_WORD_LEN; ++i) {
                       if(netfpga_read_reg(nfpga, NETFPGA_OF_LOOKUP_CMP_BASE_REG + i, (aux+i)) != ROFL_SUCCESS)
                               return ROFL_FAILURE;
               }

               //Read masks
               aux = (uint32_t*)entry->masks;
               for (i = 0; i < NETFPGA_FLOW_ENTRY_WILDCARD_WORD_LEN; ++i) {
                       if(netfpga_read_reg(nfpga, NETFPGA_OF_LOOKUP_CMP_MASK_BASE_REG + i, (aux+i)) != ROFL_SUCCESS)
                               return ROFL_FAILURE;
               }
       
               //Read actions
               aux = (uint32_t*)entry->actions;
               for (i = 0; i < NETFPGA_FLOW_ENTRY_ACTIONS_WORD_LEN; ++i) {
                       if(netfpga_read_reg(nfpga, NETFPGA_OF_LOOKUP_ACTION_BASE_REG + i, (aux+i)) != ROFL_SUCCESS)     
                               return ROFL_FAILURE;
               }
       
               //XXX: dump     
               
       }

       return ROFL_SUCCESS;
}

//Specific add command for wildcard entries
static rofl_result_t netfpga_add_entry_hw(netfpga_flow_entry_t* entry){

	unsigned int i;
	uint32_t* aux;

//	ROFL_DEBUG("\n\n  %s : % d  NETFPGA ADD FLOW ENTRY HW !!!!!!!!!!!!!  \n ", __FILE__,__LINE__);
		
	


//////////////LOGS//////////////////////////////////////////////////////////////////////
/*

	netfpga_align_mac_addr_t dst_mac=(netfpga_align_mac_addr_t)entry->matches->eth_dst;
	netfpga_align_mac_addr_t src_mac=(netfpga_align_mac_addr_t)entry->matches->eth_src;

	

	if(entry->type == NETFPGA_FE_FIXED ){
		ROFL_DEBUG("entry type FIXED \n");
	}else{
		ROFL_DEBUG("entry type WILDCARD \n");
	}

	ROFL_DEBUG(" add_entry_hw matches transp_dst: %x, transp_src: %x, ip_proto: %x, ip_dst: %x, ip_src: %x, eth_type: %x, eth_dst: %x:%x:%x:%x:%x:%x, eth_src: %x:%x:%x:%x:%x:%x, src_port: %x, \n hw_pos: %x ",
	entry->matches->transp_dst,
	entry->matches->transp_src,
	entry->matches->ip_proto,
	entry->matches->ip_dst,
	entry->matches->ip_src,
	entry->matches->eth_type,
	dst_mac.addr[0],dst_mac.addr[1],dst_mac.addr[2],dst_mac.addr[3],dst_mac.addr[4],dst_mac.addr[5],
	src_mac.addr[0],src_mac.addr[1],src_mac.addr[2],src_mac.addr[3],src_mac.addr[4],src_mac.addr[5],
	entry->matches->src_port,
	entry->hw_pos
	//sizeof(entry->matches->eth_dst)
	 );


	dst_mac=(netfpga_align_mac_addr_t)entry->actions->eth_dst;
	src_mac=(netfpga_align_mac_addr_t)entry->actions->eth_src;

	ROFL_DEBUG( "\n actions forward_bitmask: %x, action_flag: %x, vlan_id: %x, vlan_pcp: %x, eth_dst: %x:%x:%x:%x:%x:%x, eth_src: %x:%x:%x:%x:%x:%x, ip_src: %x, ip_dst %x, ip_tos %x, transp_src: %x, transp_dst %x ",

	entry->actions->forward_bitmask,
	entry->actions->action_flags,
	entry->actions->vlan_id,
	entry->actions->vlan_pcp,
	dst_mac.addr[0],dst_mac.addr[1],dst_mac.addr[2],dst_mac.addr[3],dst_mac.addr[4],dst_mac.addr[5],
	src_mac.addr[0],src_mac.addr[1],src_mac.addr[2],src_mac.addr[3],src_mac.addr[4],src_mac.addr[5],
	entry->actions->ip_src,
	entry->actions->ip_dst,
	entry->actions->ip_tos,
	entry->actions->transp_src,
	entry->actions->transp_dst
	);








	dst_mac=(netfpga_align_mac_addr_t)entry->masks->eth_dst;
	src_mac=(netfpga_align_mac_addr_t)entry->masks->eth_src;


	ROFL_DEBUG("\n MASK ip_dst: %x, ip_src: %x, eth_type: %x, eth_dst: %x:%x:%x:%x:%x:%x, eth_src: %x:%x:%x:%x:%x:%x, src_port: %x \n\n",
	entry->masks->ip_dst,
	entry->masks->ip_src,
	entry->masks->eth_type,
	dst_mac.addr[0],dst_mac.addr[1],dst_mac.addr[2],dst_mac.addr[3],dst_mac.addr[4],dst_mac.addr[5],
	src_mac.addr[0],src_mac.addr[1],src_mac.addr[2],src_mac.addr[3],src_mac.addr[4],src_mac.addr[5],
	entry->masks->src_port
	 );







*/

//////////////////////////LOGS FINISH////////////////////////////////////////////////////////////////////

	
	
	//Wait for the netfpga to be ready
	netfpga_wait_reg_ready(nfpga);

	//Set Row address
	if(entry->type == NETFPGA_FE_FIXED ){
		//ROFL_DEBUG("\n  %s : % d  FIXED ENTRY  \n ", __FILE__,__LINE__);
		
		if(netfpga_write_reg(nfpga, NETFPGA_OF_BASE_ADDR_REG, NETFPGA_EXACT_BASE + entry->hw_pos) != ROFL_SUCCESS)//NETFPGA_EXACT_BASE			0x0000
			return ROFL_FAILURE;
	}else{
		
		//ROFL_DEBUG("\n  %s : % d  WILD CARD  \n ", __FILE__,__LINE__);
		if(netfpga_write_reg(nfpga, NETFPGA_OF_BASE_ADDR_REG, NETFPGA_WILDCARD_BASE + entry->hw_pos) != ROFL_SUCCESS)
			return ROFL_FAILURE;
	}
	




	//Write matches 
	aux = (uint32_t*)entry->matches;
	for (i = 0; i < NETFPGA_FLOW_ENTRY_MATCHES_WORD_LEN; ++i) {
		if(netfpga_write_reg(nfpga, NETFPGA_OF_LOOKUP_CMP_BASE_REG + i, *(aux+i)) != ROFL_SUCCESS)//NETFPGA_OF_LOOKUP_CMP_BASE_REG		NETFPGA_OF_BASE_ADDR+0x22

			return ROFL_FAILURE;
	}

	if( entry->type == NETFPGA_FE_WILDCARDED){
		aux = (uint32_t*)entry->masks;
		for (i = 0; i < NETFPGA_FLOW_ENTRY_WILDCARD_WORD_LEN; ++i) {
			if(netfpga_write_reg(nfpga, NETFPGA_OF_LOOKUP_CMP_MASK_BASE_REG + i, *(aux+i)) != ROFL_SUCCESS)
				return ROFL_FAILURE;
		}
	}
	//sprawdzić z mapa pamieci!!
	//Write actions
	aux = (uint32_t*)entry->actions;
	for (i = 0; i < NETFPGA_FLOW_ENTRY_ACTIONS_WORD_LEN; ++i) {
		if(netfpga_write_reg(nfpga, NETFPGA_OF_LOOKUP_ACTION_BASE_REG + i, *(aux+i)) != ROFL_SUCCESS)	//NETFPGA_OF_LOOKUP_ACTION_BASE_REG	NETFPGA_OF_BASE_ADDR+0x32
			return ROFL_FAILURE;
	}

	//if( entry->type == NETFPGA_FE_WILDCARDED ){
		//Reset the stats for the pos 
		if(netfpga_write_reg(nfpga, NETFPGA_OF_STATS_BASE_REG, 0x0) != ROFL_SUCCESS)
			return ROFL_FAILURE;
		if(netfpga_write_reg(nfpga, NETFPGA_OF_STATS_BASE_REG+1, 0x0) != ROFL_SUCCESS)
			return ROFL_FAILURE;
	//}

	//Write whatever => Trigger load to table
	if(netfpga_write_reg(nfpga, NETFPGA_OF_WRITE_ORDER_REG, 0x1) != ROFL_SUCCESS) 
		return ROFL_FAILURE; 

	return ROFL_SUCCESS;
}

//Specific delete commands for wildcarded entries
static rofl_result_t netfpga_delete_entry_hw(unsigned int pos){

	rofl_result_t result;

	//Creating empty tmp entry	
	netfpga_flow_entry_t* hw_entry = netfpga_init_flow_entry();
	hw_entry->hw_pos=pos;

	if(!hw_entry)
		return ROFL_FAILURE; 
	
	//Perform the add (delete actually)
	result = netfpga_add_entry_hw(hw_entry);

	//Destroy tmp entry
	netfpga_destroy_flow_entry(hw_entry);	
	
	return result;
}

//Setup the catch-all DMA entries (packet_in) and packet out entries
static rofl_result_t netfpga_init_dma_mechanism(){




	unsigned int i;
	netfpga_flow_entry_t* entry;

	//Create an empty entry
	entry = netfpga_init_flow_entry();	

	//Wildcard ALL except ports
	memset(entry->masks, 0xFF, sizeof(*entry->masks));
	entry->masks->src_port = 0x0;
	entry->type = NETFPGA_FE_WILDCARDED; 

	//Insert catch all entries (PKT_IN)
	for (i = 0; i < 4; ++i) {
		entry->matches->src_port = 0x1 << (i * 2);
		entry->actions->forward_bitmask = 0x1 << ((i * 2) + 1);
		entry->hw_pos =(NETFPGA_OPENFLOW_WILDCARD_FULL_TABLE_SIZE - 4) + i; 				 	
		
		//Install entry
		if(netfpga_add_entry_hw(entry) != ROFL_SUCCESS)
			return ROFL_FAILURE;
	}

	//Insert output entries (PKT_OUT)  
	for (i = 0; i < 4; ++i) {
		entry->matches->src_port = 0x1 << ((i * 2) + 1);
		entry->actions->forward_bitmask = 0x1 << (i * 2);
		entry->hw_pos =(NETFPGA_OPENFLOW_WILDCARD_FULL_TABLE_SIZE - 8) + i; 	 	
	
		//Install entry
		if(netfpga_add_entry_hw(entry) != ROFL_SUCCESS)
			return ROFL_FAILURE;
	}

	//Destroy tmp entry
	netfpga_destroy_flow_entry(entry);	
	

///TEST///////injection of fake flowmod//////////////////////////////////////////////////////    TEST   ///////////////////
/*

ROFL_DEBUG(" \n BEGIN OF FAKE ENTRY \n");
	entry = netfpga_init_flow_entry();
	

	for (i = 0; i < 4; ++i) {
		entry = netfpga_init_flow_entry();		
		entry->matches->src_port = 0x1 << ((i * 2) );
		entry->actions->forward_bitmask = 0x10;
		entry->hw_pos = i; 	
		//entry->type = NETFPGA_FE_FIXED;  	
		entry->type = NETFPGA_FE_WILDCARDED; 
		//entry->masks->src_port=0xFF;



		
		netfpga_align_mac_addr_t dst_mac;
		netfpga_align_mac_addr_t src_mac;





		entry->masks->transp_dst=0xFFFF;
		entry->masks->transp_src=0xFFFF;
		entry->masks->ip_proto=0xFF;
		entry->masks->ip_dst=0xFFFFFFFF;
		entry->masks->ip_src=0xFFFFFFFF;
		entry->masks->eth_type=0xFFFF;
		dst_mac.addr[0]=0xFF;dst_mac.addr[1]=0xFF;dst_mac.addr[2]=0xFF;dst_mac.addr[3]=0xFF;dst_mac.addr[4]=0xFF;dst_mac.addr[5]=0xFF;
		src_mac.addr[0]=0xFF;src_mac.addr[1]=0xFF;src_mac.addr[2]=0xFF;src_mac.addr[3]=0xFF;src_mac.addr[4]=0xFF;src_mac.addr[5]=0xFF;
		entry->masks->src_port=0x00;
		entry->masks->ip_tos=0xFF;
		entry->masks->vlan_id=0xFFFF;
		entry->masks->pad=0xFF;

		entry->masks->eth_dst=dst_mac;
		entry->masks->eth_src=src_mac;
	
		//Install entry
		if(netfpga_add_entry_hw(entry) != ROFL_SUCCESS)
			return ROFL_FAILURE;

		netfpga_destroy_flow_entry(entry);
	} 
///////works for wildcard entry
*/

	
/*
		uint64_t tmp_mac;
		
		entry = netfpga_init_flow_entry();		
	//	entry->matches->src_port = 0x1 << ((i * 2) );//////DZIALA
		

		
		entry->actions->forward_bitmask = 0x04;
		entry->hw_pos = 0x01; 	
		//entry->type = NETFPGA_FE_FIXED;  	
		entry->type = NETFPGA_FE_WILDCARDED; 
		//entry->masks->src_port=0xFF;
//		tmp_mac=0x0000011033333334;
		tmp_mac=0x0000343333331031;
		memcpy(&(entry->matches->eth_dst), &tmp_mac, 6);

		
		//netfpga_align_mac_addr_t dst_mac;
		//netfpga_align_mac_addr_t src_mac;


		memset((entry->masks),0xFF,sizeof(*(entry->masks)));


		memset(&(entry->masks->eth_dst),0x00,sizeof(entry->masks->eth_dst)); 
		//Install entry
		if(netfpga_add_entry_hw(entry) != ROFL_SUCCESS)
			return ROFL_FAILURE;

		netfpga_destroy_flow_entry(entry);

////////////////////////2/////////////////////////////////


		entry = netfpga_init_flow_entry();		
	//	entry->matches->src_port = 0x1 << ((i * 2) );
		

		
		entry->actions->forward_bitmask = 0x01;
		entry->hw_pos = 02; 	
		//entry->type = NETFPGA_FE_FIXED;  	
		entry->type = NETFPGA_FE_WILDCARDED; 
		//entry->masks->src_port=0xFF;
//		tmp_mac=0x0000111044444444;
		tmp_mac=0x0000444444441011;
		memcpy(&(entry->matches->eth_dst), &tmp_mac, 6);

		
		//netfpga_align_mac_addr_t dst_mac;
		//netfpga_align_mac_addr_t src_mac;


		memset((entry->masks),0xFF,sizeof(*(entry->masks)));

		
		memset(&(entry->masks->eth_dst),0x00,sizeof(entry->masks->eth_dst)); 
		//Install entry
		if(netfpga_add_entry_hw(entry) != ROFL_SUCCESS)
			return ROFL_FAILURE;

		netfpga_destroy_flow_entry(entry);

		ROFL_DEBUG(" \n END OF FAKE ENTRY \n");

//////DZIALA
*/

////////////////////////////////////////////////////////////wildcard///////////
/*

		entry = netfpga_init_flow_entry();		
	//	entry->matches->src_port = 0x1 << ((i * 2) );
		

		memset((entry->matches),0xFF,sizeof(*(entry->matches)));
		

		entry->actions->forward_bitmask = 0x04;
		entry->hw_pos = 0x10; 	
		entry->type = NETFPGA_FE_FIXED;  	
		//entry->type = NETFPGA_FE_WILDCARDED; 
		//entry->masks->src_port=0xFF;
//		tmp_mac=0x0000011033333334;
		tmp_mac=0x0000343333331001;
		memcpy(&(entry->matches->eth_dst), &tmp_mac, 6);

		
		//netfpga_align_mac_addr_t dst_mac;
		//netfpga_align_mac_addr_t src_mac;


		memset((entry->masks),0xFF,sizeof(*(entry->masks)));


		//memset(&(entry->masks->eth_dst),0x00,sizeof(entry->masks->eth_dst)); 
		//Install entry
		if(netfpga_add_entry_hw(entry) != ROFL_SUCCESS)
			return ROFL_FAILURE;

		netfpga_destroy_flow_entry(entry);

////////////////////////2/////////////////////////////////


		entry = netfpga_init_flow_entry();		
	//	entry->matches->src_port = 0x1 << ((i * 2) );
		

		memset((entry->matches),0xFF,sizeof(*(entry->matches)));		
		entry->actions->forward_bitmask = 0x01;
		entry->hw_pos = 20; 	
		entry->type = NETFPGA_FE_FIXED;  	
		//entry->type = NETFPGA_FE_WILDCARDED; 
		//entry->masks->src_port=0xFF;
//		tmp_mac=0x0000111044444444;
		tmp_mac=0x0000444444441011;
		memcpy(&(entry->matches->eth_dst), &tmp_mac, 6);

		
		//netfpga_align_mac_addr_t dst_mac;
		//netfpga_align_mac_addr_t src_mac;


		memset((entry->masks),0xFF,sizeof(*(entry->masks)));

		
		//memset(&(entry->masks->eth_dst),0x00,sizeof(entry->masks->eth_dst)); 
		//Install entry
		if(netfpga_add_entry_hw(entry) != ROFL_SUCCESS)
			return ROFL_FAILURE;

		netfpga_destroy_flow_entry(entry);

		ROFL_DEBUG(" \n END OF FAKE ENTRY \n");



*/





/*
//exact 
//one way   1
 	entry = netfpga_init_flow_entry();
		uint64_t tmp_mac;
		entry = netfpga_init_flow_entry();		
		
		entry->actions->forward_bitmask = 0x4;
		
		entry->hw_pos = 0x10; 	
		entry->type = NETFPGA_FE_FIXED;  		
		//netfpga_align_mac_addr_t dst_mac;
		//netfpga_align_mac_addr_t src_mac;

		entry->matches->transp_dst=0x0;
		entry->matches->transp_src=0x0;
		entry->matches->ip_proto=0x02;
		entry->matches->ip_dst=0xc0a8001f;
		entry->matches->ip_src=0xc0a80028;
		entry->matches->eth_type=0x0806;
		tmp_mac=0x0000343333331001;
		memcpy(&(entry->matches->eth_dst), &tmp_mac, 6);
//dst_mac.addr[0]=0x11;dst_mac.addr[1]=0x10;dst_mac.addr[2]=0x44;dst_mac.addr[3]=0x44;dst_mac.addr[4]=0x44;dst_mac.addr[5]=0x44;
		tmp_mac=0x0000444444441011;
		memcpy(&(entry->matches->eth_src), &tmp_mac, 6);
//src_mac.addr[0]=0x01;src_mac.addr[1]=0x10;src_mac.addr[2]=0x33;src_mac.addr[3]=0x33;src_mac.addr[4]=0x33;src_mac.addr[5]=0x34;
		entry->matches->src_port=0x01;
		entry->matches->ip_tos=0x00;
		entry->matches->vlan_id=0xFFFF;
		entry->matches->pad=0x80;

		//entry->matches->eth_dst=dst_mac;
		//entry->matches->eth_src=src_mac;
	
		//Install entry
		if(netfpga_add_entry_hw(entry) != ROFL_SUCCESS)
			return ROFL_FAILURE;

		netfpga_destroy_flow_entry(entry);
	


////////////  2    ////////////
		entry = netfpga_init_flow_entry();

		entry->actions->forward_bitmask=0x0001;  	
		entry->hw_pos = 0x20;
		entry->type = NETFPGA_FE_FIXED;  	
		


		entry->matches->transp_dst=0x0;
		entry->matches->transp_src=0x8;
		entry->matches->ip_proto=0x01;
		entry->matches->ip_dst=0xc0a80028;
		entry->matches->ip_src=0xc0a8001f;
		entry->matches->eth_type=0x0800;

		tmp_mac=0x0000444444441011;
		memcpy(&(entry->matches->eth_dst), &tmp_mac, 6);
		tmp_mac=0x0000343333331001;
		memcpy(&(entry->matches->eth_src), &tmp_mac, 6);

		//dst_mac.addr[0]=0x11;dst_mac.addr[1]=0x10;dst_mac.addr[2]=0x44;dst_mac.addr[3]=0x44;dst_mac.addr[4]=0x44;dst_mac.addr[5]=0x44;
		//src_mac.addr[0]=0x01;src_mac.addr[1]=0x10;src_mac.addr[2]=0x33;src_mac.addr[3]=0x33;src_mac.addr[4]=0x33;src_mac.addr[5]=0x34;
		entry->matches->src_port=0x04;
		entry->matches->ip_tos=0x00;
		entry->matches->vlan_id=0xFFFF;
		entry->matches->pad=0x80;

		//entry->matches->eth_dst=dst_mac;
		//entry->matches->eth_src=src_mac;
	
		//Install entry
		if(netfpga_add_entry_hw(entry) != ROFL_SUCCESS)
			return ROFL_FAILURE;

		netfpga_destroy_flow_entry(entry);



//			3
//entry = netfpga_init_flow_entry();

	
		entry = netfpga_init_flow_entry();		
		 
		entry->type = NETFPGA_FE_FIXED;  				
		entry->actions->forward_bitmask = 0x04;
		entry->hw_pos = 0x30; 	
		entry->type = NETFPGA_FE_FIXED;  




		entry->matches->transp_dst=0x0;
		entry->matches->transp_src=0x0;
		entry->matches->ip_proto=0x01;
		entry->matches->ip_src=0xc0a80028;
		entry->matches->ip_dst=0xc0a8001f;
		entry->matches->eth_type=0x0800;

		tmp_mac=0x0000343333331001;
		memcpy(&(entry->matches->eth_dst), &tmp_mac, 6);
		tmp_mac=0x0000444444441011;
		memcpy(&(entry->matches->eth_src), &tmp_mac, 6);

		entry->matches->src_port=0x01;
		entry->matches->ip_tos=0x00;
		entry->matches->vlan_id=0xFFFF;
		entry->matches->pad=0x80;

		//entry->matches->eth_dst=dst_mac;
		//entry->matches->eth_src=src_mac;
	
		//Install entry
		if(netfpga_add_entry_hw(entry) != ROFL_SUCCESS)
			return ROFL_FAILURE;

		netfpga_destroy_flow_entry(entry);	




/////////////////////////////////////////////////   4    ///////////////////////////////////////////////////////////////////////////////////////////



		entry = netfpga_init_flow_entry();		
		
 
		entry->type = NETFPGA_FE_FIXED;  	
		


		entry->actions->forward_bitmask = 0x04;
		entry->hw_pos = 0x50; 	
		entry->type = NETFPGA_FE_FIXED;  	
		



		entry->matches->transp_dst=0x0;
		entry->matches->transp_src=0x0;
		entry->matches->ip_proto=0x01;
		entry->matches->ip_src=0xc0a80028;
		entry->matches->ip_dst=0xc0a8001f;
		entry->matches->eth_type=0x0806;

		tmp_mac=0x0000343333331001;
		memcpy(&(entry->matches->eth_dst), &tmp_mac, 6);
		tmp_mac=0x0000444444441011;
		memcpy(&(entry->matches->eth_src), &tmp_mac, 6);
		
		entry->matches->src_port=0x01;
		entry->matches->ip_tos=0x00;
		entry->matches->vlan_id=0xFFFF;
		entry->matches->pad=0x80;

		//entry->matches->eth_dst=dst_mac;
		//entry->matches->eth_src=src_mac;
	
		//Install entry
		if(netfpga_add_entry_hw(entry) != ROFL_SUCCESS)
			return ROFL_FAILURE;

		netfpga_destroy_flow_entry(entry);
		


//ARP w druga strone            5
entry = netfpga_init_flow_entry();
	


		entry = netfpga_init_flow_entry();		
		
		entry->actions->forward_bitmask = 0x01;
		entry->hw_pos = 0x40;
		entry->type = NETFPGA_FE_FIXED;  	


		entry->matches->transp_dst=0x0;
		entry->matches->transp_src=0x0;
		entry->matches->ip_proto=0x02;
		entry->matches->ip_dst=0xc0a80028;
		entry->matches->ip_src=0xc0a8001f;
		entry->matches->eth_type=0x0806;

		tmp_mac=0x0000444444441011;
		memmove((entry->matches->eth_dst.addr), &tmp_mac, sizeof(entry->matches->eth_dst));
		tmp_mac=0x0000343333331001;
		memmove((entry->matches->eth_src.addr), &tmp_mac, sizeof(entry->matches->eth_dst));

		//src_mac.addr[0]=0x11;src_mac.addr[1]=0x10;src_mac.addr[2]=0x44;src_mac.addr[3]=0x44;src_mac.addr[4]=0x44;src_mac.addr[5]=0x44;
		//dst_mac.addr[0]=0x01;dst_mac.addr[1]=0x10;dst_mac.addr[2]=0x33;dst_mac.addr[3]=0x33;dst_mac.addr[4]=0x33;dst_mac.addr[5]=0x34;
		entry->matches->src_port=0x04;
		entry->matches->ip_tos=0x00;
		entry->matches->vlan_id=0xFFFF;
		entry->matches->pad=0x80;

		//entry->matches->eth_dst=dst_mac;
		//entry->matches->eth_src=src_mac;
	
		//Install entry
		if(netfpga_add_entry_hw(entry) != ROFL_SUCCESS)
			return ROFL_FAILURE;

		netfpga_destroy_flow_entry(entry);
		

*/


////////////////////////            TEST finish     //////////////////////////////////////////////////////////////////////////////




	return ROFL_SUCCESS;
}


/*
 * External interfaces
 */

//Getter only used internally in the netfpga code
netfpga_device_t* netfpga_get(){
	return nfpga;
}

//Initializes the netfpga shared state, including appropiate state of registers and bootstrap. 
rofl_result_t netfpga_init(){

	if(nfpga){
		ROFL_DEBUG("Double call to netfpga_init()\n");
		assert(0);
		return ROFL_SUCCESS; //Skip
	}

	nfpga = (netfpga_device_t*)malloc(sizeof(*nfpga));
	memset(nfpga, 0, sizeof(*nfpga));


	//Open fd
	nfpga->fd = open(NETFPGA_DEVNAME, O_RDWR);
	if( ( nfpga->fd) < 0)
		return ROFL_FAILURE;

	//Reset counters
	nfpga->num_of_wildcarded_entries= nfpga->num_of_exact_entries = 0;

	//Init mutex
	pthread_mutex_init(&nfpga->mutex, NULL);

	//Delete all entries
	if(netfpga_delete_all_entries() != ROFL_SUCCESS)
		return ROFL_FAILURE;

	//Init DMA
	if(netfpga_init_dma_mechanism() != ROFL_SUCCESS)
		return ROFL_FAILURE;

	//FIXME: set registers	
/*
	// cleaning statistisc imposible due to read only stats registers in OFSWITCH
	uint32_t misc_stats[NETFPGA_NUMBER_OF_MISC_STATS];
	netfpga_read_misc_stats(misc_stats);
	displacy_misc_stats(misc_stats);
	
	//clean misculenious stats
	if(netfpga_clean_misc_stats() != ROFL_SUCCESS)
		return ROFL_FAILURE;
	netfpga_read_misc_stats(misc_stats);
	displacy_misc_stats(misc_stats);
*/
	ROFL_DEBUG("\n   END OF NETFPGA INITIALIZATION !!!!!!!!!!!!!!!!!!!!!!!! \n");
	return ROFL_SUCCESS;
}

//Destroys state of the netfpga, and restores it to the original state (state before init) 
rofl_result_t netfpga_destroy(){

	if(!nfpga){
		ROFL_DEBUG("netfpga_destroy() called without netfpga being initialized!\n");
		assert(0);
		return ROFL_SUCCESS; //Skip
	}	

	//Destroy mutex
	pthread_mutex_destroy(&nfpga->mutex);


	//FIXME set registers

	close(nfpga->fd);
	free(nfpga);	
	
	return ROFL_SUCCESS;
}

//Lock netfpga so that other threads cannot do operations of it. 
void netfpga_lock(){
	pthread_mutex_lock(&nfpga->mutex);
}

//Unlock netfpga so that other threads can do operations of it. 
void netfpga_unlock(){
	pthread_mutex_unlock(&nfpga->mutex);
}




//Set table behaviour
rofl_result_t netfpga_set_table_behaviour(void){
	//FIXME
	return ROFL_SUCCESS;
}

//Add flow entry to table 
rofl_result_t netfpga_add_flow_entry(of1x_flow_entry_t* entry){
	ROFL_DEBUG("%s : %d GOT FLOW ENTRY! \n", __FILE__,__LINE__);
	//Map entry to a hw entry

	//ROFL_DEBUG("",entry->matchs->    ,            ,             );
	netfpga_flow_entry_t* hw_entry = netfpga_generate_hw_flow_entry(nfpga, entry);


	if(!hw_entry)
		return ROFL_FAILURE;
	
	//Do the association with the netfpga state
	entry->platform_state = (of1x_flow_entry_platform_state_t*) hw_entry;

	//Write to hw
	if( netfpga_add_entry_hw(hw_entry) != ROFL_SUCCESS ){
		//Remove reference (release slot) in the table
		//Acquired in netfpga_generate_hw_flow_entry()
		if( hw_entry->type == NETFPGA_FE_WILDCARDED )
			nfpga->hw_wildcard_table[hw_entry->hw_pos] = NULL;
		else
			nfpga->hw_exact_table[hw_entry->hw_pos] = NULL;
			
		return ROFL_FAILURE;
	}

	//Increment counters
	if( hw_entry->type == NETFPGA_FE_WILDCARDED )
		nfpga->num_of_wildcarded_entries++;	
	else
		nfpga->num_of_exact_entries++;	

	return ROFL_SUCCESS;
}

//Deletes an specific entry defined by *entry 
rofl_result_t netfpga_delete_flow_entry(of1x_flow_entry_t* entry){

	//Recover the position
	netfpga_flow_entry_t* hw_entry = (netfpga_flow_entry_t*)entry->platform_state;

	//Check
	if(!hw_entry)
		return ROFL_FAILURE;

	//Delete entry in HW
	if( netfpga_delete_entry_hw(hw_entry->hw_pos) != ROFL_SUCCESS )
		return ROFL_FAILURE;
	
	//Destroy
	netfpga_destroy_flow_entry(hw_entry);

	//Decrement counters
	if( hw_entry->type == NETFPGA_FE_WILDCARDED )
		nfpga->num_of_wildcarded_entries--;	
	else
		nfpga->num_of_exact_entries--;	

	//Unset platform state
	entry->platform_state = NULL;	

	return ROFL_SUCCESS;
}

//Deletes all entries within a table 
rofl_result_t netfpga_delete_all_entries(void){

	unsigned int i;	
	netfpga_flow_entry_t* entry;

	//Create an empty entry
	entry = netfpga_init_flow_entry();	

	//Attempt to delete all entries in the table
	//for fixed flow entries
	entry->type = NETFPGA_FE_FIXED;
	for(i=0; i< NETFPGA_OPENFLOW_EXACT_TABLE_SIZE; ++i){
		entry->hw_pos = i;
		netfpga_delete_entry_hw(entry->hw_pos);	
	}
	
	//Attempt to delete all entries in the table
	//for wildcarded flow entries
	entry->type = NETFPGA_FE_WILDCARDED;
	for(i=NETFPGA_WILDCARD_BASE ; i< (NETFPGA_WILDCARD_BASE + NETFPGA_OPENFLOW_WILDCARD_FULL_TABLE_SIZE - NETFPGA_RESERVED_FOR_CPU2NETFPGA) ; ++i){ 
		entry->hw_pos = i;
		netfpga_delete_entry_hw(entry->hw_pos);	
	}

	//Create an empty entry
	netfpga_destroy_flow_entry(entry);	

	return ROFL_SUCCESS;
}

