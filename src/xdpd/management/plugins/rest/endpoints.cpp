// Copyright (c) 2014	Barnstormer Softworks, Ltd.

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include "server/request.hpp"
#include "server/reply.hpp"
#include "json_spirit/json_spirit.h"

#include <rofl/common/utils/c_logger.h>

#include "../../port_manager.h"
#include "../../plugin_manager.h"
#include "../../switch_manager.h"
#include "../../system_manager.h"

using namespace xdpd;

namespace endpoints{


static json_spirit::Value get_plugin_list(){
	std::vector<plugin*> plugin_list = plugin_manager::get_plugins();
	std::vector<std::string> plugins;

	for (std::vector<plugin*>::iterator i = plugin_list.begin(); i != plugin_list.end(); ++i){
		plugins.push_back((*i)->get_name());
	}

	return json_spirit::Value(plugins.begin(), plugins.end());
}

//
// General information
//
void general_info(const http::server::request &req, http::server::reply &rep){

	//Prepare object
	json_spirit::Object xdpd_;
	json_spirit::Object info_;

	//Info
	info_.push_back(json_spirit::Pair("system-id", system_manager::get_id()) );
	info_.push_back(json_spirit::Pair("version", XDPD_VERSION));
	info_.push_back(json_spirit::Pair("build", XDPD_BUILD));
	info_.push_back(json_spirit::Pair("detailed-build", XDPD_DESCRIBE));
	info_.push_back(json_spirit::Pair("driver-code-name", system_manager::get_driver_code_name()) );

	info_.push_back(json_spirit::Pair("plugins", get_plugin_list()) );

	//Put header
	xdpd_.push_back(json_spirit::Pair("xdpd", info_) );

	rep.content = json_spirit::write(xdpd_, true);
}

//
// Plugins
//
void list_plugins (const http::server::request &req, http::server::reply &rep){
	//Prepare object
	json_spirit::Object plugins;
	plugins.push_back(json_spirit::Pair("plugins", get_plugin_list()));
	rep.content = json_spirit::write(plugins, true);
}

//
// Ports
//

void port_detail (const http::server::request &req, http::server::reply &rep){

}

void list_ports (const http::server::request &req, http::server::reply &rep){
	//Prepare object
	json_spirit::Object ports;

	std::set<std::string> ports_ = port_manager::list_available_port_names();

	json_spirit::Value pa(ports_.begin(), ports_.end());
	ports.push_back(json_spirit::Pair("ports", pa));

	rep.content = json_spirit::write(ports, true);
}


//
// Datapaths
//

void list_datapaths (const http::server::request &req,
						http::server::reply &rep){

	//Prepare object
	json_spirit::Object dps;

	std::list<std::string> datapaths =
					switch_manager::list_sw_names();

	json_spirit::Value pa(datapaths.begin(), datapaths.end());
	dps.push_back(json_spirit::Pair("lsis", pa));

	rep.content = json_spirit::write(dps, true);
}

} // namespace endpoints
