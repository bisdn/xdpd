// Copyright (c) 2014  Barnstormer Softworks, Ltd.

#include <boost/regex.hpp>

namespace http{
namespace server{
	struct request;
	struct reply;
}//namespace server
}//namespace http

namespace endpoints{
	void index(const http::server::request &, http::server::reply &, boost::cmatch&);
	void general_info(const http::server::request &, http::server::reply &, boost::cmatch&);
	void list_plugins(const http::server::request &, http::server::reply &, boost::cmatch&);
	void list_datapaths(const http::server::request &, http::server::reply &, boost::cmatch&);
	void list_ports(const http::server::request &, http::server::reply &, boost::cmatch&);
	void port_detail(const http::server::request &, http::server::reply &, boost::cmatch&);
} // namespace endpoints
