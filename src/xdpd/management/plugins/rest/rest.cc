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

#include "controllers.h" 

namespace xdpd{

#define XDPD_REST_PORT "5757"

static void srvthread (){
	boost::asio::io_service io_service;

	try{
		http::server::rest_handler handler;

		handler.register_get_path("/", boost::bind(controllers::index, _1, _2, _3));
		handler.register_get_path("/index.htm", boost::bind(controllers::index, _1, _2, _3));
		handler.register_get_path("/index.html", boost::bind(controllers::index, _1, _2, _3));

		//General information
		handler.register_get_path("/info", boost::bind(controllers::general_info, _1, _2, _3));
		handler.register_get_path("/plugins", boost::bind(controllers::list_plugins, _1, _2, _3));
		handler.register_get_path("/matching-algorithms", boost::bind(controllers::list_matching_algorithms, _1, _2, _3));

		//Ports
		handler.register_get_path("/ports", boost::bind(controllers::list_ports, _1, _2, _3));
		handler.register_get_path("/port/(\\w+)", boost::bind(controllers::port_detail, _1, _2, _3));

		handler.register_get_path("/lsis", boost::bind(controllers::list_lsis, _1, _2, _3));
		handler.register_get_path("/lsi/(\\w+)", boost::bind(controllers::lsi_detail, _1, _2, _3));
		handler.register_get_path("/lsi/(\\w+)/table/([0-9]+)/flows", boost::bind(controllers::lsi_table_flows, _1, _2, _3));
		handler.register_get_path("/lsi/(\\w+)/group-table", boost::bind(controllers::lsi_groups, _1, _2, _3));

		http::server::server(io_service, "0.0.0.0", XDPD_REST_PORT, handler)();
		boost::asio::signal_set signals(io_service);
		/*signals.add(SIGINT);
		signals.add(SIGTERM);*/
		signals.async_wait(boost::bind(&boost::asio::io_service::stop, &io_service));

		io_service.run();
	}catch(boost::thread_interrupted&){
		ROFL_INFO("[xdpd][rest] REST Server shutting down\n");
		return;
	}
}

void rest::init(){
	ROFL_INFO("[xdpd][rest] Starting REST server\n");
	t = boost::thread(&srvthread);
}

rest::~rest(){
	t.interrupt();
}

} // namespace xdpd
