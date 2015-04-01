// Copyright (c) 2014	Barnstormer Softworks, Ltd.

#ifndef HTTP_SERVER_REST_HANDLER_HPP
#define HTTP_SERVER_REST_HANDLER_HPP

#include <map>
#include <string>

#include <boost/regex.hpp>
#include <boost/function.hpp>

namespace http {
namespace server {

struct reply;
struct request;

typedef boost::function<void (const request&, reply&, boost::cmatch& grps)> RestFuncT;

class rest_handler{

public:
	explicit rest_handler();

	void operator()(const request&, reply&);

	//Register routines
	void register_get_path(std::string path, RestFuncT f);
	void register_post_path(std::string path, RestFuncT f);
	void register_put_path(std::string path, RestFuncT f);
	void register_delete_path(std::string path, RestFuncT f);

private:
	RestFuncT get_handler(std::map<std::string, RestFuncT>& handler_map, const std::string& req_path, boost::cmatch& grps);

	std::map<std::string, RestFuncT> get_handlers;
	std::map<std::string, RestFuncT> post_handlers;
	std::map<std::string, RestFuncT> put_handlers;
	std::map<std::string, RestFuncT> delete_handlers;

	//(supported) HTTP methods
	enum http_methods{
		GET = 0,
		POST = 1,
		PUT = 2,
		DELETE = 3,
	};

	//Helper
	std::map<std::string, enum http_methods> methods_to_enum;
};

} //namespace server
} //namespace http

#endif
