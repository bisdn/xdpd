#ifndef ORCHESTRATOR_CONSTANTS_H_
#define ORCHESTRATOR_CONSTANTS_H_ 1

#define PLUGIN_NAME 	"NodeOrchestrator" 

/*
*	Openflow stuffs
*/
#define NUM_TABLES			8
#define RECONNECT_TIME 		1	//1s
#define OFVERSION 			OF_VERSION_12

/*
*	Connection from the node orchestrator
*/

#define MGMT_PROTOCOL		"tcp"
#define MGMT_ADDR			"127.0.0.1"
#define MGMT_PORT			"2525"

/*
*	Messages and answers from/to the
*	node orchestrator
*/
#define CREATE_LSI				"create-lsi"
#define DESTROY_LSI				"destroy-lsi"

#define ATTACH_PHY_PORTS		"attach-physical-ports"
#define DETACH_PHY_PORTS		"detach-physical-ports"

#define CREATE_NF_PORTS			"create-nfs-ports"
#define DESTROY_NF_PORTS		"destroy-nfs-ports"

#define CREATE_VIRTUAL_LINKS	"create-virtual-link"
#define DESTROY_VIRTUAL_LINKS	"destroy-virtual-link"

#define DISCOVER_PHY_PORTS		"discover-physical-ports"
#define ERROR					"ERROR"

#endif //endif
