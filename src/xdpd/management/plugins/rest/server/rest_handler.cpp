// Copyright (c) 2014  Barnstormer Softworks, Ltd.

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
        return;
        }
      catch (...)
        {
        rep = reply::stock_reply(reply::bad_request);
        return;
        }
      }

    }
  }
