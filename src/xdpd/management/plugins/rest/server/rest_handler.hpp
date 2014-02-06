// Copyright (c) 2014  Barnstormer Softworks, Ltd.

#ifndef HTTP_SERVER_REST_HANDLER_HPP
#define HTTP_SERVER_REST_HANDLER_HPP

#include <map>
#include <string>

#include <boost/function.hpp>

namespace http {
  namespace server {

    struct reply;
    struct request;

    typedef boost::function<void (const request&, reply&)>  RestFuncT;

    class rest_handler
      {
    public:
      explicit rest_handler();

      void  operator()(const request&, reply&);
      void  register_path (std::string, RestFuncT);

    private:
      std::map<std::string, RestFuncT>  handler_map;
      };

  }
}

#endif
