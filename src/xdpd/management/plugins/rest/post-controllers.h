#ifndef POST_CONTROLLERS_H
#define POST_CONTROLLERS_H

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
namespace post{

	/*
	* Util to check if the user is allowed to perform management calls
	*/
	bool authorised(const http::server::request &, http::server::reply &);

	/*
	* Is active management enabled
	*/
	void enabled(const http::server::request &, http::server::reply &, boost::cmatch&);

	/*
	* Ports
	*/
	void port_up(const http::server::request &, http::server::reply &, boost::cmatch&);
	void port_down(const http::server::request &, http::server::reply &, boost::cmatch&);

	/*
	* Port attachment
	*/
	void attach_port(const http::server::request &, http::server::reply &, boost::cmatch&);
	void detach_port(const http::server::request &, http::server::reply &, boost::cmatch&);

} //namespace post
} //namespace controllers
} //namespace xdpd

#endif /* POST_CONTROLLERS_H_ */
