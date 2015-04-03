#ifndef COMMON_CONTROLLERS_H
#define COMMON_CONTROLLERS_H

namespace http{
namespace server{
	struct request;
	struct reply;
}//namespace server
}//namespace http

namespace xdpd{
namespace controllers{

	/*
	* Is active management enabled
	*/
	void mgmt_enabled(const http::server::request &, http::server::reply &, boost::cmatch&);

	/*
	* Util to check if the user is allowed to perform management calls
	*/
	bool authorised(const http::server::request &, http::server::reply &);

} //namespace controllers
} //namespace xdpd

#endif /* COMMON_CONTROLLERS_H_ */
