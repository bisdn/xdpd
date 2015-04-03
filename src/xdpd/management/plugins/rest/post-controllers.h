#ifndef POST_CONTROLLERS_H
#define POST_CONTROLLERS_H

// Copyright (c) 2014  Barnstormer Softworks, Ltd.

#include <boost/regex.hpp>
#include "common-controllers.h"

namespace xdpd{
namespace controllers{
namespace post{

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
