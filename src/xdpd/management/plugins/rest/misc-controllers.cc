// Copyright(c) 2014	Barnstormer Softworks, Ltd.

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>


#include "rest.h"
#include "server/request.hpp"
#include "server/reply.hpp"
#include "json_spirit/json_spirit.h"

#include "get-controllers.h"

#include <rofl/common/utils/c_logger.h>

#include "../../port_manager.h"
#include "../../plugin_manager.h"
#include "../../switch_manager.h"
#include "../../system_manager.h"

namespace xdpd{
namespace controllers{
namespace get{

//
// Human browsable index
//
void index(const http::server::request &req, http::server::reply &rep, boost::cmatch& grps){
	std::stringstream html;

	bool mgmt_enabled = authorised(req,rep);
	std::string mgmt_class = mgmt_enabled? "class='enabled'":"class='disabled'";
	html << "<html>" << std::endl;
	html << "<head>" << std::endl;
	html << "<title> xDPd's REST APIs</title>" << std::endl;
	html << "<style> .enabled{color:green} .disabled{color:red}</style>" << std::endl;
	html << "</head>" << std::endl;
	html << "<body>" << std::endl;
	html << "<h1>xDPd's REST APIs</h1><br>" << std::endl;
	std::stringstream mgmt;
	if(mgmt_enabled)
		mgmt << "<b "<< mgmt_class <<">enabled</b>";
	else
		mgmt << "<b "<< mgmt_class <<">disabled</b>";
	html << "Management APIs status (-m): " <<  mgmt.str() << "<br>" << std::endl;
	html << "<h3 class='enabled'>GET</h3>" << std::endl;
	html << "<ul>" << std::endl;

	//GET
	html << "<li><b><a href=\"/info/system\">/info/system</a></b>: general system information" << std::endl;
	html << "<li><b><a href=\"/info/plugins\">/info/plugins</a></b>: list of compiled-in plugins" << std::endl;
	html << "<li><b><a href=\"/info/matching-algorithms\">/info/matching-algorithms</a></b>: list available OF table matching algorithms<br>" << std::endl;
	html << "<li><b><a href=\"/info/ports\">/info/ports</a></b>: list of available ports" << std::endl;
	html << "<li><b>/info/port/&lt;port_name&gt;</b>: show port information<br>" << std::endl;
	html << "<li><b><a href=\"/info/lsis\">/info/lsis</a></b>: list of logical switch instances(LSIs)" << std::endl;
	html << "<li><b>/info/lsi/&lt;lsi_name&gt</b>: show logical switch instance(LSI) information" << std::endl;
	html << "<li><b>/info/lsi/&lt;lsi_name&gt/table/&lt;num&gt/flows</b>: list LSI table flow entries" << std::endl;
	html << "<li><b>/info/lsi/&lt;lsi_name&gt/group-table</b>: list LSI group table entries" << std::endl;
	html << "</ul>" << std::endl;

	//POST
	html << "<h3 "<< mgmt_class <<">POST</h3>" << std::endl;
	html << "<ul>" << std::endl;

	html << "<li><b>/mgmt/port/&lt;lsi_name&gt/up</b>: bring port administratively up" << std::endl;
	html << "<li><b>/mgmt/port/&lt;lsi_name&gt/down</b>: bring port administratively down" << std::endl;
	html << "<li><b>/mgmt/attach/port/&lt;port_name&gt/&lt;lsi_name&gt</b>: attach a port to an LSI" << std::endl;
	html << "<li><b>/mgmt/detach/port/&lt;port_name&gt/&lt;lsi_name&gt</b>: detach a port from an LSI" << std::endl;

	html << "</ul>" << std::endl;

	//PUT
	html << "<h3 "<< mgmt_class << ">PUT</h3>" << std::endl;
	html << "<ul>" << std::endl;

	html << "<li><b>/mgmt/create/vlink/&lt;lsi1_name&gt/&lt;lsi2_name&gt</b>: create a virtual link between two LSIs" << std::endl;

	html << "</ul>" << std::endl;

	//DELETE
	html << "<h3 "<< mgmt_class << ">DELETE</h3>" << std::endl;
	html << "<ul>" << std::endl;

	html << "<li><b>/mgmt/destroy/lsi/&lt;lsi_name&gt</b>: destroy an LSIs" << std::endl;

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

} //namespace get

bool authorised(const http::server::request &req, http::server::reply &rep){

	//Recover plugin
	rest* rest_plugin = (rest*)plugin_manager::get_plugin_by_name(rest::name); 
	if(!rest_plugin){
		assert(0);
		return false;
	}

	if(!rest_plugin->is_mgmt_enabled()){;
		std::stringstream ss;
		ss<<"REST management routines disabled";
		rep.content = ss.str();
		rep.status = http::server::reply::unauthorized;
	}
	return rest_plugin->is_mgmt_enabled();

}


void mgmt_enabled(const http::server::request &req, http::server::reply &rep, boost::cmatch& grps){
	authorised(req, rep);
}

} //namespace controllers
} //namespace xdpd
