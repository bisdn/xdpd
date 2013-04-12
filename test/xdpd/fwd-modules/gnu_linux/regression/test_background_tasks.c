/*lets test the background tasks thread*/
#include <stdio.h>
#include <rofl/datapath/afa/afa.h>
#include <rofl/datapath/afa/fwd_module.h>
#include "src/drivers/x86/background_tasks_manager.h"

int main(int args, char** argv)
{
    
	//create physical switch
	driver_init();

	int matching_algorithm = 0;
	driver_create_switch("switch1",0x1015,OF_VERSION_12,1,&matching_algorithm);

	//add ports
	x86_background_tasks_thread();

	fprintf(stderr,"<%s:%d> Test Passed\n",__func__,__LINE__);
	return 0;

}
