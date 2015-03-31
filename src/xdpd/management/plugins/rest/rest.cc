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

#include "endpoints.hpp"

namespace xdpd{

void srvthread (){
	boost::asio::io_service io_service;

	try{
		http::server::rest_handler handler;

		handler.register_path("/", boost::bind(endpoints::index, _1, _2));
		handler.register_path("/index.htm", boost::bind(endpoints::index, _1, _2));
		handler.register_path("/index.html", boost::bind(endpoints::index, _1, _2));
		handler.register_path("/info", boost::bind(endpoints::general_info, _1, _2));
		handler.register_path("/plugins", boost::bind(endpoints::list_plugins, _1, _2));
		handler.register_path("/lsis", boost::bind(endpoints::list_datapaths, _1, _2));
		handler.register_path("/ports", boost::bind(endpoints::list_ports, _1, _2));

		http::server::server(io_service, "0.0.0.0", "80", handler)();
		boost::asio::signal_set signals(io_service);
		signals.add(SIGINT);
		signals.add(SIGTERM);
		signals.async_wait(boost::bind(&boost::asio::io_service::stop, &io_service));

		io_service.run();
	}catch(boost::thread_interrupted&){
		ROFL_INFO("[xdpd][rest] REST Server shutting down\n");
		return;
	}
}

void rest::init(){
	ROFL_INFO("[xdpd][rest] Starting REST server\n");
	boost::thread t(&srvthread);

	return;
}

} // namespace xdpd
