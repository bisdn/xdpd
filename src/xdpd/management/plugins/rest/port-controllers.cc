// Copyright(c) 2014	Barnstormer Softworks, Ltd.

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>


#include "server/request.hpp"
#include "server/reply.hpp"
#include "json_spirit/json_spirit.h"

#include "get-controllers.h"
#include "post-controllers.h"

#include <rofl/common/utils/c_logger.h>

#include "../../port_manager.h"
#include "../../plugin_manager.h"
#include "../../switch_manager.h"
#include "../../system_manager.h"

namespace xdpd{
namespace controllers{
namespace get{

void port_detail(const http::server::request &req, http::server::reply &rep, boost::cmatch& grps){
	json_spirit::Object detail;
	std::string port_name = std::string(grps[1]);
	port_snapshot snapshot;

	//Check if it exists;
	if(!port_manager::exists(port_name)){
		//Throw 404
		std::stringstream ss;
		ss<<"Invalid port '"<<port_name<<"'";
		rep.content = ss.str();
		rep.status = http::server::reply::not_found;
		return;
	}

	//Get the snapshot
	port_manager::get_port_info(port_name, snapshot);

	//Fill it in
	detail.push_back(json_spirit::Pair("name", port_name));
	std::string p_type;
	switch(snapshot.type){
		case PORT_TYPE_PHYSICAL: p_type = "physical";
					break;
		case PORT_TYPE_VIRTUAL: p_type = "virtual";
					break;
		case PORT_TYPE_TUNNEL: p_type = "tunnel";
					break;
		case PORT_TYPE_NF_NATIVE: p_type = "nf-native";
					break;
		case PORT_TYPE_NF_SHMEM: p_type = "nf-shmem";
					break;
		case PORT_TYPE_NF_EXTERNAL: p_type = "nf-external";
					break;
		default: p_type = "unknown";
			break;
	}
	detail.push_back(json_spirit::Pair("type", p_type));
	detail.push_back(json_spirit::Pair("is-vlink", port_manager::is_vlink(port_name)? "yes": "no"));
	if(port_manager::is_vlink(port_name))
		detail.push_back(json_spirit::Pair("vlink-pair", port_manager::get_vlink_pair(port_name)));
	std::stringstream mac;
	mac << snapshot.hw_address.str();
	detail.push_back(json_spirit::Pair("mac-address", mac.str()));

	detail.push_back(json_spirit::Pair("up", snapshot.up? "yes": "no"));

	detail.push_back(json_spirit::Pair("link", snapshot.state&PORT_STATE_LINK_DOWN? "down": "detected"));

	detail.push_back(json_spirit::Pair("forward-packets", snapshot.forward_packets? "yes": "no"));
	detail.push_back(json_spirit::Pair("drop-received", snapshot.drop_received? "yes": "no"));
	detail.push_back(json_spirit::Pair("no-flood", snapshot.no_flood? "yes": "no"));
	detail.push_back(json_spirit::Pair("is-blacklisted", port_manager::is_blacklisted(port_name)?"yes":"no"));


	//Stats
	json_spirit::Object stats;
	stats.push_back(json_spirit::Pair("rx_packets", snapshot.stats.rx_packets));
	stats.push_back(json_spirit::Pair("tx_packets", snapshot.stats.tx_packets));
	stats.push_back(json_spirit::Pair("rx_bytes", snapshot.stats.rx_bytes));
	stats.push_back(json_spirit::Pair("tx_bytes", snapshot.stats.tx_bytes));
	stats.push_back(json_spirit::Pair("rx_dropped", snapshot.stats.rx_dropped));
	stats.push_back(json_spirit::Pair("tx_dropped", snapshot.stats.tx_dropped));
	stats.push_back(json_spirit::Pair("rx_errors", snapshot.stats.rx_errors));
	stats.push_back(json_spirit::Pair("tx_errors", snapshot.stats.tx_errors));
	stats.push_back(json_spirit::Pair("rx_frame_err", snapshot.stats.rx_frame_err));
	stats.push_back(json_spirit::Pair("rx_over_err", snapshot.stats.rx_over_err));
	stats.push_back(json_spirit::Pair("rx_crc_err", snapshot.stats.rx_crc_err));
	stats.push_back(json_spirit::Pair("collisions", snapshot.stats.collisions));

	detail.push_back(json_spirit::Pair("statistics", stats));

	//Openflow
	if(snapshot.attached_sw_dpid > 0){
		json_spirit::Object of;
		of.push_back(json_spirit::Pair("attached-dpid", snapshot.attached_sw_dpid));
		of.push_back(json_spirit::Pair("generate-pkt-in", snapshot.of_generate_packet_in? "yes":"no"));
		of.push_back(json_spirit::Pair("port-num", (uint64_t)snapshot.of_port_num));
		detail.push_back(json_spirit::Pair("openflow", of));
	}

	json_spirit::Object queues;
	std::list<port_queue_snapshot>::const_iterator it;
	for(it = snapshot.queues.begin(); it != snapshot.queues.end(); ++it){
		std::stringstream ss;

		//General info
		json_spirit::Object q;
		q.push_back(json_spirit::Pair("id", (uint64_t)it->id));
		ss << it->id;
		q.push_back(json_spirit::Pair("length", (uint64_t)it->length));

		//Stats
		json_spirit::Object s;
		s.push_back(json_spirit::Pair("tx-pkts", it->stats_tx_pkts));
		s.push_back(json_spirit::Pair("tx-bytes", it->stats_tx_bytes));
		q.push_back(json_spirit::Pair("statistics", s));

		queues.push_back(json_spirit::Pair(ss.str(), q));
	}
	detail.push_back(json_spirit::Pair("queues", queues));

	rep.content = json_spirit::write(detail, true);
}

void list_ports(const http::server::request &req, http::server::reply &rep, boost::cmatch& grps){
	//Prepare object
	json_spirit::Object ports;

	std::set<std::string> ports_ = port_manager::list_available_port_names();

	json_spirit::Value pa(ports_.begin(), ports_.end());
	ports.push_back(json_spirit::Pair("ports", pa));

	rep.content = json_spirit::write(ports, true);
}

} //namespace get

namespace post{

void port_up(const http::server::request &req, http::server::reply &rep, boost::cmatch& grps){

	//Perform security checks
	if(!authorised(req,rep)) return;

	std::string port_name = std::string(grps[1]);

	//Check if it exists;
	if(!port_manager::exists(port_name)){
		//Throw 404
		std::stringstream ss;
		ss<<"Invalid port '"<<port_name<<"'";
		rep.content = ss.str();
		rep.status = http::server::reply::not_found;
		return;
	}

	try{
		port_manager::bring_up(port_name);
	}catch(...){
		//Something went wrong
		std::stringstream ss;
		ss<<"Unable to bring port'"<<port_name<<"' up";
		rep.content = ss.str();
		rep.status = http::server::reply::internal_server_error;
		return;
	}
}

void port_down(const http::server::request &req, http::server::reply &rep, boost::cmatch& grps){

	//Perform security checks
	if(!authorised(req,rep)) return;

	std::string port_name = std::string(grps[1]);

	//Check if it exists;
	if(!port_manager::exists(port_name)){
		//Throw 404
		std::stringstream ss;
		ss<<"Invalid port '"<<port_name<<"'";
		rep.content = ss.str();
		rep.status = http::server::reply::not_found;
		return;
	}

	try{
		port_manager::bring_down(port_name);
	}catch(...){
		//Something went wrong
		std::stringstream ss;
		ss<<"Unable to bring port'"<<port_name<<"' down";
		rep.content = ss.str();
		rep.status = http::server::reply::internal_server_error;
		return;
	}
}

} //namespace post
} //namespace controllers
} //namespace xdpd
