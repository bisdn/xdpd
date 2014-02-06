// Copyright (c) 2014  Barnstormer Softworks, Ltd.

#include <boost/lexical_cast.hpp>

#include "rest_handler.hpp"
#include "file_handler.hpp"
#include "reply.hpp"
#include "request.hpp"

namespace http
  {
  namespace server
    {
    rest_handler::rest_handler(void)
      { }

    void rest_handler::operator()(const request& req, reply& rep)
      {
      std::string request_path;
      if (!file_handler::url_decode(req.uri, request_path))
        {
        rep = reply::stock_reply(reply::bad_request);
        return;
        }

      try
        {
        RestFuncT func = this->handler_map[request_path];
        func(req, rep);
        rep.headers.resize(2);
        rep.headers[0].name = "Content-Length";
        rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
        rep.headers[1].name = "Content-Type";
        rep.headers[1].value = "application/json";
        rep.status = reply::ok;
        return;
        }
      catch (...)
        {
        rep = reply::stock_reply(reply::bad_request);
        return;
        }
      }

    void rest_handler::register_path (std::string path, RestFuncT func)
      {
      this->handler_map[path] = func;
      }

    } // namespace server
  } // namespace http
