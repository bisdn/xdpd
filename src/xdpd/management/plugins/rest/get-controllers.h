#ifndef GET_CONTROLLERS_H
#define GET_CONTROLLERS_H

// Copyright (c) 2014  Barnstormer Softworks, Ltd.

#include <boost/regex.hpp>

namespace http{
namespace server{
	struct request;
	struct reply;
}//namespace server
}//namespace http

namespace xdpd{
namespace controllers{
namespace get{
	/**
	* Human readable index
	*/
	void index(const http::server::request &, http::server::reply &, boost::cmatch&);

	/**
	* general info
	*/
	void system_info(const http::server::request &, http::server::reply &, boost::cmatch&);
	void list_plugins(const http::server::request &, http::server::reply &, boost::cmatch&);
	void list_matching_algorithms(const http::server::request &, http::server::reply &, boost::cmatch&);

	/**
	* LSI
	*/
	void list_lsis(const http::server::request &, http::server::reply &, boost::cmatch&);
	void lsi_detail(const http::server::request &, http::server::reply &, boost::cmatch&);
	void lsi_table_flows(const http::server::request &, http::server::reply &, boost::cmatch&);
	void lsi_groups(const http::server::request &, http::server::reply &, boost::cmatch&);

	/**
	* Ports
	*/
	void list_ports(const http::server::request &, http::server::reply &, boost::cmatch&);
	void port_detail(const http::server::request &, http::server::reply &, boost::cmatch&);

} //namespace get
} //namespace controllers
} //namespace xdpd

#endif /* GET_CONTROLLERS_H_ */
