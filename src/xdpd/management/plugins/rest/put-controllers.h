#ifndef PUT_CONTROLLERS_H
#define PUT_CONTROLLERS_H

#include <boost/regex.hpp>
#include "common-controllers.h"

namespace xdpd{
namespace controllers{
namespace put{

	/*
	* Create an LSI
	*/
	void create_lsi(const http::server::request &, http::server::reply &, boost::cmatch&);

	/*
	* Create a vlink
	*/
	void create_vlink(const http::server::request &, http::server::reply &, boost::cmatch&);

} //namespace put
} //namespace controllers
} //namespace xdpd

#endif /* PUT_CONTROLLERS_H_ */
