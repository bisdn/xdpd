// Copyright(c) 2014	Barnstormer Softworks, Ltd.

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>


#include "server/request.hpp"
#include "server/reply.hpp"
#include "json_spirit/json_spirit.h"

#include "endpoints.hpp"

#include <rofl/common/utils/c_logger.h>

#include "../../port_manager.h"
#include "../../plugin_manager.h"
#include "../../switch_manager.h"
#include "../../system_manager.h"

using namespace xdpd;

namespace endpoints{

//Utils
static json_spirit::Value get_plugin_list(){
	std::vector<plugin*> plugin_list = plugin_manager::get_plugins();
	std::vector<std::string> plugins;

	for(std::vector<plugin*>::iterator i = plugin_list.begin(); i != plugin_list.end(); ++i){
		plugins.push_back((*i)->get_name());
	}

	return json_spirit::Value(plugins.begin(), plugins.end());
}

//
// Human browsable index
//
void index(const http::server::request &req, http::server::reply &rep, boost::cmatch& grps){
	std::stringstream html;

	html << "<html>" << std::endl;
	html << "<head>" << std::endl;
	html << "<title> xDPd control panel </title>" << std::endl;
	html << "</head>" << std::endl;
	html << "<body>" << std::endl;
	html << "<h1>xDPd control panel</h1><br>" << std::endl;
	html << "Available URLs:<br><br>" << std::endl;
	html << "<ul>" << std::endl;

	//Info
	html << "<li><a href=\"/info\">/info</a>: general system information" << std::endl;
	html << "<li><a href=\"/plugins\">/plugins</a>: list of compiled-in plugins" << std::endl;
	html << "<li><a href=\"/matching-algorithms\">/matching-algorithms</a>: list available OF table matching algorithms" << std::endl;
	html << "<li><a href=\"/ports\">/ports</a>: list of available ports" << std::endl;
	html << "<li>/port/&lt;port_name&gt;: show port information" << std::endl;
	html << "<li><a href=\"/lsis\">/lsis</a>: list of logical switch instances(LSIs)" << std::endl;
	html << "<li>/lsi/&lt;lsi_name&gt\": show logical switch instance(LSI) information" << std::endl;
	html << "</ul>" << std::endl;

	html << "</body>" << std::endl;
	html << "</html>" << std::endl;

	rep.content = html.str();
	rep.headers.resize(2);
	rep.headers[0].name = "Content-Length";
	rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
	rep.headers[1].name = "Content-Type";
	rep.headers[1].value = "text/html";
}

//
// General information
//
void general_info(const http::server::request &req, http::server::reply &rep, boost::cmatch& grps){

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
void list_plugins(const http::server::request &req, http::server::reply &rep, boost::cmatch& grps){
	//Prepare object
	json_spirit::Object plugins;
	plugins.push_back(json_spirit::Pair("plugins", get_plugin_list()));
	rep.content = json_spirit::write(plugins, true);
}

//
// List matching algorithms
//
void list_matching_algorithms(const http::server::request &req, http::server::reply &rep, boost::cmatch& grps){
	//Prepare object
	json_spirit::Object mas;
	std::list<std::string> mas_ =
					switch_manager::list_matching_algorithms(OF_VERSION_10);

	json_spirit::Value pa(mas_.begin(), mas_.end());
	mas.push_back(json_spirit::Pair("matching-algorithms", pa));

	rep.content = json_spirit::write(mas, true);
}

//
// Ports
//

void port_detail(const http::server::request &req, http::server::reply &rep, boost::cmatch& grps){
	json_spirit::Object detail;
	std::string port_name = std::string(grps[1]);
	port_snapshot snapshot;

	//Check if it exists;
	if(!port_manager::exists(port_name)){
		std::stringstream ss;
		ss << "Invalid port: "<<port_name;
		rep.content = ss.str();
		return;
	}

	//Get the snapshot
	port_manager::get_port_info(port_name, snapshot);

	//Fill it in
	detail.push_back(json_spirit::Pair("name", port_name));
	detail.push_back(json_spirit::Pair("is-blacklisted", port_manager::is_blacklisted(port_name)?"yes":"no"));
	detail.push_back(json_spirit::Pair("up", snapshot.up? "yes": "no"));
	detail.push_back(json_spirit::Pair("forward-packets", snapshot.forward_packets? "yes": "no"));
	detail.push_back(json_spirit::Pair("drop-received", snapshot.drop_received? "yes": "no"));
	detail.push_back(json_spirit::Pair("no-flood", snapshot.no_flood? "yes": "no"));

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
	detail.push_back(json_spirit::Pair("link", snapshot.state&PORT_STATE_LINK_DOWN? "down": "up"));

	//Stats
	json_spirit::Object stats;
	stats.push_back(json_spirit::Pair("link", snapshot.state&PORT_STATE_LINK_DOWN? "down": "up"));
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


//
// Datapaths
//

void list_lsis(const http::server::request &req,
						http::server::reply &rep,
						boost::cmatch& grps){
	//Prepare object
	json_spirit::Object dps;

	std::list<std::string> datapaths =
					switch_manager::list_sw_names();

	json_spirit::Value pa(datapaths.begin(), datapaths.end());
	dps.push_back(json_spirit::Pair("lsis", pa));

	rep.content = json_spirit::write(dps, true);
}

void lsi_detail(const http::server::request &req,
						http::server::reply &rep,
						boost::cmatch& grps){
	json_spirit::Object lsi;
	std::string lsi_name = std::string(grps[1]);

	//Check if it exists;
	if(!switch_manager::exists_by_name(lsi_name)){
		std::stringstream ss;
		ss << "Invalid lsi: "<<lsi_name;
		rep.content = ss.str();
		return;
	}

	//Get the snapshot
	uint64_t dpid = switch_manager::get_switch_dpid(lsi_name);
	openflow_switch_snapshot snapshot;

	switch_manager::get_switch_info(dpid, snapshot);

	//Fill in general information
	lsi.push_back(json_spirit::Pair("name", lsi_name));
	lsi.push_back(json_spirit::Pair("dpid", dpid));

	std::string of_version;
	switch(snapshot.version){
		case OF_VERSION_10: of_version = "1.0";
			break;
		case OF_VERSION_12: of_version = "1.2";
			break;
		case OF_VERSION_13: of_version = "1.3";
			break;
		default: of_version = "invalid";
			break;
	}

	lsi.push_back(json_spirit::Pair("of_version", of_version));
	lsi.push_back(json_spirit::Pair("num-of-tables", (int)snapshot.num_of_tables));
	lsi.push_back(json_spirit::Pair("miss-send-len", (int)snapshot.miss_send_len));
	lsi.push_back(json_spirit::Pair("num-of-buffers", (int)snapshot.num_of_buffers));

	//Attached ports
	json_spirit::Object ports;
	lsi.push_back(json_spirit::Pair("ports", ports));
	//TODO add to snapshot

	//Loop over the tables
	json_spirit::Object tables;
	std::list<openflow_switch_table_snapshot>::const_iterator it;
	for(it = snapshot.tables.begin(); it != snapshot.tables.end(); ++it){
		json_spirit::Object t;
		std::stringstream ss;
		//Generics
		t.push_back(json_spirit::Pair("number", (int)it->number));
		ss <<  it->number;

		//TODO: convert this into a string
		t.push_back(json_spirit::Pair("matching-algorithm", (int)it->matching_algorithm));

		t.push_back(json_spirit::Pair("num-of-entries", (uint64_t)it->num_of_entries));
		t.push_back(json_spirit::Pair("max-entries", (uint64_t)it->max_entries));
		//TODO: table miss, capabilities

		//Statistics
		json_spirit::Object s;
		s.push_back(json_spirit::Pair("pkts-looked-up", it->stats_lookup));
		s.push_back(json_spirit::Pair("pkts-matched", it->stats_matched));
		t.push_back(json_spirit::Pair("statistics", s));

		tables.push_back(json_spirit::Pair(ss.str(), t));
	}
	lsi.push_back(json_spirit::Pair("tables", tables));


	rep.content = json_spirit::write(lsi, true);
}

} // namespace endpoints
