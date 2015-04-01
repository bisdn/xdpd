// Copyright (c) 2014	Barnstormer Softworks, Ltd.

#include <boost/lexical_cast.hpp>

#include "rest_handler.hpp"
#include "file_handler.hpp"
#include "reply.hpp"
#include "request.hpp"

namespace http{
namespace server{

rest_handler::rest_handler(void){ }


RestFuncT rest_handler::get_handler(const std::string& req_path, boost::cmatch& grps){

	//First check for static matches
	RestFuncT f = this->handler_map[req_path];

	if(f){
		//Empty grps
		grps = boost::cmatch();
		return f;
	}

	//Otherwise apply regular expressions
	std::map<std::string, RestFuncT>::const_iterator it;
	for(it = handler_map.begin(); it != handler_map.end(); ++it){
		boost::regex expression(it->first);
		if(regex_match(req_path.c_str(), grps, expression)){
			return it->second;
		}
	}

	throw std::string("Unknown");
}

void rest_handler::operator()(const request& req, reply& rep){

	boost::cmatch grps;
	std::string request_path;
	if (!file_handler::url_decode(req.uri, request_path)){
		rep = reply::stock_reply(reply::bad_request);
		return;
	}

	try{
		RestFuncT func = get_handler(request_path, grps);
		func(req, rep, grps);
		if(rep.headers.size() == 0){
			rep.headers.resize(2);
			rep.headers[0].name = "Content-Length";
			rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
			rep.headers[1].name = "Content-Type";
			rep.headers[1].value = "application/json";
		}
		rep.status = reply::ok;
		return;
	}catch (...){
		rep = reply::stock_reply(reply::bad_request);
		return;
	}
}

void rest_handler::register_path(std::string path, RestFuncT func){
	this->handler_map[path] = func;
}

} // namespace server
} // namespace http
