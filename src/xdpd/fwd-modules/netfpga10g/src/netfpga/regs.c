#include "regs.h"

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>

rofl_result_t netfpga_read_reg(netfpga_device_t *nfpga, uint32_t reg_id, uint32_t *value){
	
	netfpga_register_t reg;

	reg.reg_id = reg_id;

	//Calling ioctl
	if(ioctl(nfpga->fd, NETFPGA_IOCTL_CMD_READ_REG, &reg) != 0){
		ROFL_ERR("ioctl failed on reading register fd: %d REG: 0x%x\n", nfpga->fd, reg_id);
		return ROFL_FAILURE;
	}

	//Put value and return	
	*value = reg.reg_val;

	ROFL_DEBUG(stderr, "READ on addr: 0x%x value: 0x%x\n",reg_id, reg.reg_val);
	
	return ROFL_SUCCESS;
}

rofl_result_t netfpga_write_reg(netfpga_device_t *nfpga, uint32_t reg_id, uint32_t value){

	netfpga_register_t reg;

	reg.reg_id = reg_id;
	reg.reg_val = value;

	//ifreq structure for ioctl call
	if(ioctl(nfpga->fd, NETFPGA_IOCTL_CMD_WRITE_REG, &reg) != 0){
		ROFL_ERR("ioctl failed on writing register fd: %d REG: 0x%x\n", nfpga->fd, reg_id);
		
		return ROFL_FAILURE;
	}

	ROFL_DEBUG(stderr, "WRITING on addr: 0x%x value: 0x%x\n", reg_id, value);

	return ROFL_SUCCESS;
}

#define NETFPGA_READY_WAIT_TIME_US 50000  //50ms

void netfpga_wait_reg_ready(netfpga_device_t *nfpga){
	
	uint32_t reg_val;
	
	//Wait for the netfpga to be ready
	netfpga_read_reg(nfpga, NETFPGA_OF_ACC_RDY_REG,&reg_val);
	while ( !reg_val&0x01 ){
		//Not ready loop
		usleep(NETFPGA_READY_WAIT_TIME_US);
		netfpga_read_reg(nfpga, NETFPGA_OF_ACC_RDY_REG,&reg_val);
	}
}
