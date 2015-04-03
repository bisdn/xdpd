// Copyright (c) 2014	Barnstormer Softworks, Ltd.

#include <iostream>

#include "rest.h"
#include <rofl/common/utils/c_logger.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <signal.h>

#include "server/server.hpp"
#include "server/rest_handler.hpp"
#include "../../system_manager.h"

#include "get-controllers.h"
#include "post-controllers.h"
#include "put-controllers.h"
#include "delete-controllers.h"

namespace xdpd{

const std::string rest::BIND_ADDR_OPT_FULL_NAME="bind-rest";
const std::string rest::MGMT_OPT_FULL_NAME="mgmt-rest";
const std::string rest::name="rest";


class eInvalidBindAddrRest : public rofl::RoflException {};

static void parse_bind_addr(std::string& host, std::string& port){

	if(!system_manager::is_option_set(std::string(rest::BIND_ADDR_OPT_FULL_NAME)))
		return;

	std::string tmp = system_manager::get_option_value(std::string(rest::BIND_ADDR_OPT_FULL_NAME));

	//Check if : char is there
	if(tmp.find(std::string(":")) == std::string::npos){
		//TODO: use CRIT
		fprintf(stderr, "[xdpd][rest] CRITICAL ERROR: could not parse bind address '%s'. REST server cannot be started.\n", tmp.c_str());
		throw eInvalidBindAddrRest();
	}

	//Split the string and recover the parameters
	std::string r;
	std::istringstream ss_(tmp);

	//Recover parameter
	std::getline(ss_, r, ':');
	host = r;
	std::getline(ss_, r, ':');
	port = r;
}

static void srvthread (){
	boost::asio::io_service io_service;
	std::string host="0.0.0.0";
	std::string port="5757";

	try{
		http::server::rest_handler handler;

		//
		// GET
		//
		handler.register_get_path("/", boost::bind(controllers::get::index, _1, _2, _3));
		handler.register_get_path("/index.htm", boost::bind(controllers::get::index, _1, _2, _3));
		handler.register_get_path("/index.html", boost::bind(controllers::get::index, _1, _2, _3));

		//General information
		handler.register_get_path("/system", boost::bind(controllers::get::system_info, _1, _2, _3));
		handler.register_get_path("/plugins", boost::bind(controllers::get::list_plugins, _1, _2, _3));
		handler.register_get_path("/matching-algorithms", boost::bind(controllers::get::list_matching_algorithms, _1, _2, _3));

		//Ports
		handler.register_get_path("/ports", boost::bind(controllers::get::list_ports, _1, _2, _3));
		handler.register_get_path("/port/(\\w+)", boost::bind(controllers::get::port_detail, _1, _2, _3));

		handler.register_get_path("/lsis", boost::bind(controllers::get::list_lsis, _1, _2, _3));
		handler.register_get_path("/lsi/(\\w+)", boost::bind(controllers::get::lsi_detail, _1, _2, _3));
		handler.register_get_path("/lsi/(\\w+)/table/([0-9]+)/flows", boost::bind(controllers::get::lsi_table_flows, _1, _2, _3));
		handler.register_get_path("/lsi/(\\w+)/group-table", boost::bind(controllers::get::lsi_groups, _1, _2, _3));

		//
		// POST
		//
		handler.register_post_path("/", boost::bind(controllers::mgmt_enabled, _1, _2, _3));

		//Ports
		handler.register_post_path("/port/(\\w+)/up", boost::bind(controllers::post::port_up, _1, _2, _3));
		handler.register_post_path("/port/(\\w+)/down", boost::bind(controllers::post::port_down, _1, _2, _3));
		handler.register_post_path("/attach/port/(\\w+)/(\\w+)", boost::bind(controllers::post::attach_port, _1, _2, _3));
		handler.register_post_path("/detach/port/(\\w+)/(\\w+)", boost::bind(controllers::post::detach_port, _1, _2, _3));

		//
		// PUT
		//
		handler.register_put_path("/create/vlink/(\\w+)/(\\w+)", boost::bind(controllers::put::create_vlink, _1, _2, _3));

		//
		//DELETE
		//
		handler.register_delete_path("/destroy/lsi/(\\w+)", boost::bind(controllers::delete_::destroy_switch, _1, _2, _3));

		//Recover host and port
		parse_bind_addr(host, port);

		http::server::server(io_service, host, port, handler)();
		boost::asio::signal_set signals(io_service);
		signals.async_wait(boost::bind(&boost::asio::io_service::stop, &io_service));

		io_service.run();
	}catch(boost::thread_interrupted&){
		ROFL_INFO("[xdpd][rest] REST Server shutting down\n");
		return;
	}catch(eInvalidBindAddrRest& e){
		//Already logged
	}catch(...){
		//TODO: we could try to re_load ourselves

		//Recover plugin
		rest* rest_plugin = (rest*)plugin_manager::get_plugin_by_name(rest::name);
		std::string mgmt_enabled = "unknown";
		if(!rest_plugin){
			assert(0);
		}else{
			if(!rest_plugin->is_mgmt_enabled())
				mgmt_enabled = "no";
			else
				mgmt_enabled = "yes";
		}

		//TODO use CRIT
		fprintf(stderr, "[xdpd][rest] CRITICAL ERROR: the REST server thrown an uncatched error and will be shutdown. \n"
			  "[xdpd][rest] parameters {bind-address: '%s:%s', management enabled: '%s'}\n", host.c_str(), port.c_str(), mgmt_enabled.c_str());
		return;
	}
}

void rest::init(){

	//See if -m option is there
	mgmt_enabled = system_manager::is_option_set(std::string(MGMT_OPT_FULL_NAME));

	ROFL_INFO("[xdpd][rest] Starting REST server\n");
	if(mgmt_enabled){
		ROFL_INFO("[xdpd][rest] Enabling mgmt routines (write mode)\n");
		ROFL_WARN("[xdpd][rest] WARNING: please note that REST plugin does not provide authentication nor encryption services yet (HTTPs).\n");
	}

	t = boost::thread(&srvthread);
}

rest::~rest(){
	t.interrupt();
}

} // namespace xdpd
