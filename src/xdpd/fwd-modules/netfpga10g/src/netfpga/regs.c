#include "regs.h"

#include <sys/stat.h>
#include <sys/ioctl.h>

rofl_result_t netfpga_read_reg(netfpga_dev_info_t *nfpga, uint32_t reg_id, uint32_t *value){
	
	netfpga_register_t reg;

	reg.reg_id = reg_id;

	//Calling ioctl
	if(ioctl(nfpga->fd, NF10_IOCTL_CMD_READ_REG, &reg) != 0){
		ROFL_ERR("ioctl failed on reading register fd: %d REG: 0x%x\n", nfpga->fd, reg_id);
		return ROFL_FAILURE;
	}

	//Put value and return	
	*value = reg.reg_val;
	
	return ROFL_SUCCESS;
}

rofl_result_t netfpga_write_reg(netfpga_dev_info_t *nfpga, uint32_t reg_id, uint32_t value){

	nfpgareg_t reg;

	reg.reg_id = reg_id;
	reg.val_val = value;

	//ifreq structure for ioctl call
	if(ioctl(nfpga->fd, NF10_IOCTL_CMD_WRITE_REG, &reg) != 0){
		ROFL_ERR("ioctl failed on writing register fd: %d REG: 0x%x\n", nfpga->fd, reg_id);
		
		return ROFL_FAILURE;
	}

	return ROFL_SUCCESS;
}

