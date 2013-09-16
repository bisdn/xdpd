/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * xdpd_cli.cc
 *
 *  Created on: Sep 14, 2012
 *      Author: tobi
 */

#include "xdpd_cli.h"

#include "../../switch_manager.h"
#include "../../port_manager.h"

#include "../../../openflow/openflow_switch.h"

#include <algorithm>
#include <inttypes.h>

enum mode_config {
	MODE_CONFIG_INTERFACE = (MODE_CONFIG + 1),
	MODE_CONFIG_OPENFLOW,
	MODE_CONFIG_OPENFLOW_CONFIGURE
};

using namespace xdpd;


class cli_print_helper {
public:
	cli_print_helper(struct cli_def *cli);

	void
	operator() (std::string s) {
		// sanity check
		if (NULL == cli) {
			return;
		}
		::cli_print(cli, "%s\n", s.c_str());
	};

private:
	struct cli_def *cli;
};
cli_print_helper::cli_print_helper(struct cli_def *cli) : cli(cli) {};



static int
cmd_show_port(struct cli_def *cli, UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
/* todo   if (strcmp(argv[0], "?") == 0)
        cli_print(cli, "  usage: show port [name]");*/

	if (0 < argc && strchr(argv[argc-1], '?')) {
		cli_print(cli, "<cr> show available ports\n");
		return CLI_OK;
	}

	// currently always all port information is printed
	std::list<std::string> port_names(port_manager::list_available_port_names());
	if (port_names.size()) {
		cli_print_helper cph(cli);
		std::for_each(port_names.begin(), port_names.end(), cph);
	} else {
		cli_print(cli, "no ports available\n");
	}

	return CLI_OK;
}

static int
cmd_show_openflow(struct cli_def *cli, UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
	if (0 < argc && strchr(argv[argc-1], '?')) {
		cli_print(cli, "<cr> show openflow datapath entities\n");
		return CLI_OK;
	}

	// todo show more detailed information

	// get logical switch names
	std::list<std::string> sw_names(switch_manager::list_sw_names());
	if (sw_names.size()) {
		cli_print_helper cph(cli);
		std::for_each(sw_names.begin(), sw_names.end(), cph);
	} else {
		cli_print(cli, "no switch available\n");
	}

	return CLI_OK;
}

// todo this partially duplicates code of xdpd
static int
cmd_debuglevel(struct cli_def *cli, const char *command, char *argv[], int argc)
{
	std::string debugclass;
	std::string debuglevel("debug");
	// todo update this function to get library name from cli

	switch (argc) {
	case 2:
		debuglevel.assign(argv[1]);
#ifndef NDEBUG
		cli_print(cli, "new debuglevel set to %s", debuglevel.c_str());
#endif

		/* no break */
	case 1:
		debugclass.assign(argv[0]);
#ifndef NDEBUG
		cli_print(cli, "new debugclass set to %s", debugclass.c_str());
#endif

		/* no break */

	default:
		break;
	}

	/* set debuglevel for debugclass */
	csyslog::set_debug_level(debugclass, debuglevel);

    return CLI_OK;
}

static int
cmd_test(struct cli_def *cli, const char *command, char *argv[], int argc)
{
    cli_print(cli, "command not implemented yet\r\n"
    		"called %s with %s\r\n"
    		"  mode is %d config_desc is %s", __FUNCTION__, command, cli->mode, cli->modestring);
    return CLI_OK;
}

static int
cmd_config_openflow(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
	// check for argv = '...?'
	if (0 < argc && strchr(argv[argc-1], '?')) {
		cli_print(cli, "<cr> enter openflow configuration mode\n");
		return CLI_OK;
	}

    cli_set_configmode(cli, MODE_CONFIG_OPENFLOW, "of");
    return CLI_OK;
}

static int
cmd_config_openflow_configure(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    if (argc < 1 || 1 < argc) {
        cli_print(cli, "Specify a datapath to configure\n"
        		"usage: configure <dpname>");
        return CLI_OK;
    }

    if (strchr(argv[0], '?')) {
    	// get logical switch names
    	std::list<std::string> sw_names(switch_manager::list_sw_names());
    	if (sw_names.size()) {
    		cli_print_helper cph(cli);
    		std::for_each(sw_names.begin(), sw_names.end(), cph);
    	} else {
    		cli_print(cli, "no switch available\n");
    	}
    	return CLI_OK;
    }

    // todo check if there is more than one parameter?

    // set datapath name and find switch
    std::string dpname(argv[0]);
    openflow_switch *sw = switch_manager::find_by_name(dpname);

    if (NULL != sw) { // found?
    	std::string config_mode("of-");
    	config_mode.append(dpname);

    	cli_set_configmode(cli, MODE_CONFIG_OPENFLOW_CONFIGURE, config_mode.c_str());
    } else {
    	cli_print(cli, "Unknown switch name %s", argv[0]);
    }

    return CLI_OK;
}

static inline void
set_ip_helper(char *in, caddress *out)
{
	char *ptr = strchr(in, ':');
	/* for ipv6 sin_port is the same for s6addr so it does not matter which one to take */
	int port = be16toh(out->ca_s4addr->sin_port);  //!< save old port in case it is not set
	// split ip:port if ':' found (ptr points to first number of port)
	if (NULL != ptr) {
		*ptr++ = '\0';
		errno = 0;    /* To distinguish success/failure after call */
		int tmp = strtol(ptr, NULL, 0);
		switch (errno) {
			case ERANGE:
			case EINVAL:
				// todo notify user?
				break;
			default:
				port = tmp;
				break;
		}
	}

	try {
		caddress tmp(AF_INET, in, port);
		// no error... set to out:
		*out = tmp;
	} catch (eAddressInval &e) {
		// don't care
	}
}

static int
cmd_openflow_datapath_create(struct cli_def *cli, const char *command, char *argv[], int argc)
{
	// todo update this function to get library name from cli

	of_version_t version = (of_version_t)0;
	std::string dp_name;
	uint64_t dp_id = 0;
	unsigned int no_tables = 0;
	caddress rpc_ctl_addr(switch_manager::controller_addr);
	caddress rpc_dpt_bind_addr(switch_manager::binding_addr);

	// usage:
	/*   at least 4       question?               max 6 */
	if ( argc < 4 || strchr(argv[argc-1], '?') || 6 < argc ) {
		cli_print(cli, "Create a new datapath\n"
				"usage: create <dpname> <dpid>"
				" <version> <num_tables>"
				" [ctl_ipv4[:port](default:%s:%i)]"
				" [bind_ipv4[:port](default:%s:%i)]",
				rpc_ctl_addr.addr_c_str(),
				be16toh(rpc_ctl_addr.ca_s4addr->sin_port),
				rpc_dpt_bind_addr.addr_c_str(),
				be16toh(rpc_dpt_bind_addr.ca_s4addr->sin_port) );
		return CLI_OK;
	}

	switch (argc) {
	case 6:
		// parse rpc_dpt_addr (check?)
		set_ip_helper(argv[5], &rpc_dpt_bind_addr);
#ifndef NDEBUG
		cli_print(cli, "ctl_addr set to %s:%i", rpc_dpt_bind_addr.addr_c_str(),
				be16toh(rpc_dpt_bind_addr.ca_s4addr->sin_port));
#endif

		/* no break */
	case 5:
		// parse rpc_ctl_addr (check?)
		set_ip_helper(argv[4], &rpc_ctl_addr);
#ifndef NDEBUG
		cli_print(cli, "ctl_addr set to %s:%i", rpc_ctl_addr.addr_c_str(),
				be16toh(rpc_ctl_addr.ca_s4addr->sin_port));
#endif

		/* no break */
	default:
		/* all others are mandatory */

		// check n_tables?
		no_tables = strtol(argv[3], NULL, 0);
#ifndef NDEBUG
		cli_print(cli, "no_tables set to %i", no_tables);
#endif
		// check version?
		version = (of_version_t)strtol(argv[2], NULL, 0);
#ifndef NDEBUG
		cli_print(cli, "no_tables set to %i", no_tables);
#endif

		// todo check dpid
		dp_id = strtol(argv[1], NULL, 0);
#ifndef NDEBUG
		cli_print(cli, "new dp_id set to %"PRIu64, dp_id);
#endif

		// check dpname?
		dp_name.assign(argv[0]);
#ifndef NDEBUG
		cli_print(cli, "new dp_name set to %s", dp_name.c_str());
#endif
		break;

	}

	// std::list<std::string> names = switch_manager::list_matching_algorithms(version);

	int ma_list[256] = { 0 };
	switch_manager::create_switch(version, dp_id, dp_name, no_tables, ma_list, rpc_ctl_addr, rpc_dpt_bind_addr);

    return CLI_OK;
}

#if 0
static int
cmd_config_openflow_configure_connect(struct cli_def *cli, const char *command, char *argv[], int argc)
{
	caddress controller_address = cconfigfwdelem::default_controller_address;

	// usage:
	if ( (argc && strchr(argv[argc-1], '?')) || 1 < argc ) {
		cli_print(cli, "Connect to controller\n"
				"usage: connect [ctrl_ip[:port](default:%s:%i)]",
				controller_address.addr_c_str(),
				be16toh(controller_address.ca_s4addr->sin_port) );
		return CLI_OK;
	}

	if (1 == argc) {
		set_ip_helper(argv[0], &controller_address);
#ifndef NDEBUG
		cli_print(cli, "ctl_addr set to %s:%i",
				controller_address.addr_c_str(),
				be16toh(controller_address.ca_s4addr->sin_port));
#endif
	}

	// get dp_name
	std::string dp_name(&cli->modestring[strlen("(config-of-")]);
	dp_name.erase(dp_name.size()-1, 1); // remove ')'

	cconfigfwdelem::getInstance().controller_connect(dp_name, controller_address);

	cli_print(cli, "connecting...");

    return CLI_OK;
}
#endif

static int
cmd_config_openflow_configure_attach_port(struct cli_def *cli, UNUSED(const char *command), char *argv[], int argc)
{
    if (!argc || strchr(argv[argc-1], '?') || 2 < argc) {
        cli_print(cli, "usage: attach_port <portname> <port_number>\r\n");

        std::list<std::string> port_names(port_manager::list_available_port_names());
    	if (port_names.size()) {
        	cli_print(cli, "Available ports:");
    		cli_print_helper cph(cli);
    		std::for_each(port_names.begin(), port_names.end(), cph);
    	} else {
    		cli_print(cli, "no ports available\n");
    	}

        return CLI_OK;
    }

	// todo check if there is more than one parameter?
	std::string portname(argv[0]);

	// todo  uint32_t of_port_no = 0; /* invalid port number */ (need interface change)
#if 0
	/* check if second parameter (port_number) is set */
	if (2 == argc) {
		errno = 0;
		unsigned long int tmp = strtoul(argv[1], NULL, 10);

		switch (errno) {
			case EINVAL:
			case ERANGE:
				break;
			default:
				of_port_no = tmp;
				break;
		}
	}
#endif

	std::string dpname(&cli->modestring[strlen("(config-of-")]);
	dpname.erase(dpname.size()-1, 1); // remove ')'

	openflow_switch* sw = switch_manager::find_by_name(dpname);

	if (NULL == sw) {
		cli_print(cli, "internal error");
	} else {
		try {
			port_manager::attach_port_to_switch(sw->dpid, portname);
			port_manager::enable_port(portname);
			cli_print(cli, "attached port %s", argv[0]);
		} catch (eOfSmDoesNotExist &e) {
			// should never happen, since we already found the switch
			cli_print(cli, "internal error");
		} catch (ePmInvalidPort &e) {
			cli_print(cli, "wrong port name %s", argv[0]);
		} catch (eOfSmGeneralError &e) {
			cli_print(cli, "unknown error");
		}
	}

	return CLI_OK;
}

static int
cmd_config_subcmd_exit(struct cli_def *cli, UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
	int next_mode;

	// todo could use a map to get next state
	switch (cli->mode) {
	case MODE_CONFIG_OPENFLOW_CONFIGURE:
		next_mode = MODE_CONFIG_OPENFLOW;
		break;
	case MODE_CONFIG_OPENFLOW:
	case MODE_CONFIG_INTERFACE:
	default:
		next_mode = MODE_CONFIG;
		break;
	}

    cli_set_configmode(cli, next_mode, NULL);
    return CLI_OK;
}

xdpd_cli::xdpd_cli(caddress addr) : ccli(addr)
{
	/** UNPRIVILEGED COMMANDS **/

	struct cli_command *c1;
	struct cli_command *c2;

	// find show
	c1 = find_command("show");
	// show port
	cli_register_command(cli, c1, "port", cmd_show_port, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, NULL);
	// show openflow
	c2 = cli_register_command(cli, c1, "openflow", cmd_show_openflow, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, NULL);
	// show openflow flow <all|#id>
	cli_register_command(cli, c2, "flow", cmd_test, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, NULL);
	// show openflow stats
	cli_register_command(cli, c2, "stat", cmd_test, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, NULL);

	/** PRIVILEGED COMMANDS  "enable" **/

	// clear <one of the following>
	c1 = cli_register_command(cli, NULL, "clear", NULL, PRIVILEGE_PRIVILEGED, MODE_EXEC, NULL);
	// clear flowtable <entryid>
	cli_register_command(cli, c1, "flowtable", cmd_test, PRIVILEGE_PRIVILEGED, MODE_EXEC, NULL);
	// clear log
	cli_register_command(cli, c1, "log", cmd_test, PRIVILEGE_PRIVILEGED, MODE_EXEC, NULL);


	/** CONFIG COMMANDS  config terminal **/

	// debuglevel [category] [level]
	cli_register_command(cli, NULL, "debuglevel", cmd_debuglevel, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "set debuglevel for a C++ class");
	/* interface <name>			(todo not yet implemented)  */
	cli_register_command(cli, NULL, "interface", cmd_test, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Configure an interface");
	/* openflow 				(entry function) */
	cli_register_command(cli, NULL, "openflow", cmd_config_openflow, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Configure OpenFlow");


	/** CONFIG OPENFLOW COMMANDS  openflow **/

	// create
	cli_register_command(cli, NULL, "create", cmd_openflow_datapath_create, PRIVILEGE_PRIVILEGED, MODE_CONFIG_OPENFLOW, "Create an OpenFlow datapath");
	// configure <dpname>
	cli_register_command(cli, NULL, "configure", cmd_config_openflow_configure, PRIVILEGE_PRIVILEGED, MODE_CONFIG_OPENFLOW, "Configure an OpenFlow datapath (param: <dpname>)");

	// show
	c1 = cli_register_command(cli, NULL, "show", NULL, PRIVILEGE_PRIVILEGED, MODE_CONFIG_OPENFLOW, NULL);
	// show datapath
	cli_register_command(cli, c1, "openflow", cmd_show_openflow, PRIVILEGE_PRIVILEGED, MODE_CONFIG_OPENFLOW, NULL);
	// show resources
	cli_register_command(cli, c1, "port", cmd_show_port, PRIVILEGE_PRIVILEGED, MODE_CONFIG_OPENFLOW, NULL);

	// exit (back to config)
	cli_register_command(cli, NULL, "exit", cmd_config_subcmd_exit, PRIVILEGE_PRIVILEGED, MODE_CONFIG_OPENFLOW, "Exit from OpenFlow configuration");


	/** CONFIG OPENFLOW CONFIGURE COMMANDS  configure **/


	// attach_port
	cli_register_command(cli, NULL, "attach_port", cmd_config_openflow_configure_attach_port, PRIVILEGE_PRIVILEGED, MODE_CONFIG_OPENFLOW_CONFIGURE, "Attach port to OpenFlow datapath <>");
	// connect
	// todo cli_register_command(cli, NULL, "connect", cmd_config_openflow_configure_connect, PRIVILEGE_PRIVILEGED, MODE_CONFIG_OPENFLOW_CONFIGURE, "connect to controller");
	// exit (back to config)
	cli_register_command(cli, NULL, "exit", cmd_config_subcmd_exit, PRIVILEGE_PRIVILEGED, MODE_CONFIG_OPENFLOW_CONFIGURE, "Exit from OpenFlow configuration");

}

xdpd_cli::~xdpd_cli() {
}

