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

	html << "<html>" << std::endl;
	html << "<head>" << std::endl;
	html << "<title> xDPd's REST APIs</title>" << std::endl;
	html << "</head>" << std::endl;
	html << "<body>" << std::endl;
	html << "<h1>xDPd's REST APIs</h1><br>" << std::endl;
	html << "<h3>GET</h3>" << std::endl;
	html << "<ul>" << std::endl;

	//Info
	html << "<li><b><a href=\"/system\">/system</a></b>: general system information" << std::endl;
	html << "<li><b><a href=\"/plugins\">/plugins</a></b>: list of compiled-in plugins" << std::endl;
	html << "<li><b><a href=\"/matching-algorithms\">/matching-algorithms</a></b>: list available OF table matching algorithms<br>" << std::endl;
	html << "<li><b><a href=\"/ports\">/ports</a></b>: list of available ports" << std::endl;
	html << "<li><b>/port/&lt;port_name&gt;</b>: show port information<br>" << std::endl;
	html << "<li><b><a href=\"/lsis\">/lsis</a></b>: list of logical switch instances(LSIs)" << std::endl;
	html << "<li><b>/lsi/&lt;lsi_name&gt</b>: show logical switch instance(LSI) information" << std::endl;
	html << "<li><b>/lsi/&lt;lsi_name&gt/table/&lt;num&gt/flows</b>: list LSI table flow entries" << std::endl;
	html << "<li><b>/lsi/&lt;lsi_name&gt/group-table</b>: list LSI group table entries" << std::endl;
	html << "</ul>" << std::endl;

	html << "<h3>POST</h3><br>" << std::endl;
	html << "None<br><br>" << std::endl;
	html << "<h3>PUT</h3><br>" << std::endl;
	html << "None<br><br>" << std::endl;
	html << "<h3>DELETE</h3><br>" << std::endl;
	html << "None<br><br>" << std::endl;

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

namespace post{

bool authorised(const http::server::request &, http::server::reply &){

#ifndef MGMT_ENABLED
	ss<<"POST disabled";
	rep.content = ss.str();
	rep.status = http::server::reply::unauthorized;
	return false;
#else
	return true;
#endif

}


void enabled(const http::server::request &req, http::server::reply &rep, boost::cmatch& grps){
	authorised(req, rep);
}

} //namespace post
} //namespace controllers
} //namespace xdpd
