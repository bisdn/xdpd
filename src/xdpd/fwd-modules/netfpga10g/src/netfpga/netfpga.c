#include "netfpga.h"

#include <sys/stat.h>
#include <sys/ioctl.h>

#define NETFPGA_DEVNAME "/dev/nf10"

//Pointer to the unique (static) netfpga_dev_info_t
static netfpga_dev_info_t* nfpga=NULL;

//Initializes the netfpga shared state, including appropiate state of registers and bootstrap.
rofl_result_t netfpga_init(){

	if(nfpga){
		ROFL_DBG("Double call to netfpga_init()\n");
		assert(0);
		return ROFL_SUCCESS; //Skip
	}

	nfpga = (netfpga_dev_info_t*)malloc(sizeof(*nfpga));
	//memset(*nfpga, 0, sizeof(*nfpga));


	//Open fd
	nfpga->fd = open(NETFPGA_DEVNAME, O_RDWR);
	if( ( nfpga->fd) < 0)
		return ROFL_FAILURE;

	//FIXME: set registers	
	

	return ROFL_SUCCESS;
}

//Destroys state of the netfpga, and restores it to the original state (state before init) 
rofl_result_t netfpga_destroy(){

	if(!nfpga){
		ROFL_DBG("netfpga_destroy() called without netfpga being initialized!\n");
		assert(0);
		return ROFL_SUCCESS; //Skip
	}	

	//FIXME set registers

	close(nfpga->fd)
	free(nfpga);	
	
	return ROFL_SUCCESS;
}


