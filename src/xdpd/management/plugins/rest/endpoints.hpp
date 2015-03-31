// Copyright (c) 2014  Barnstormer Softworks, Ltd.

namespace http{
namespace server{
	struct request;
	struct reply;
}//namespace server
}//namespace http

namespace endpoints{
	void general_info(const http::server::request &, http::server::reply &);
	void list_plugins(const http::server::request &, http::server::reply &);
	void list_datapaths(const http::server::request &, http::server::reply &);
	void list_ports(const http::server::request &, http::server::reply &);
} // namespace endpoints
