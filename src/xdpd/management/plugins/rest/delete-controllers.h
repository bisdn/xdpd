#ifndef DELETE_CONTROLLERS_H
#define DELETE_CONTROLLERS_H

#include <boost/regex.hpp>
#include "common-controllers.h"

namespace xdpd{
namespace controllers{
namespace delete_{

	/*
	* Destroy an LSI
	*/
	void destroy_switch(const http::server::request &, http::server::reply &, boost::cmatch&);

} //namespace delete_
} //namespace controllers
} //namespace xdpd

#endif /* DELETE_CONTROLLERS_H_ */
