// Copyright (c) 2014  Barnstormer Softworks, Ltd.

#include <iostream>

#include "rest.h"
#include <rofl/common/utils/c_logger.h>

#include "../../plugin_manager.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <signal.h>

#include "server/server.hpp"
#include "server/rest_handler.hpp"
#include "server/request.hpp"
#include "server/reply.hpp"

#include "json_spirit/json_spirit_writer_template.h"

namespace xdpd
  {
  void list_plugins (const http::server::request &req, http::server::reply &rep)
    {
    std::vector<plugin*> plugin_list = plugin_manager::get_plugins();

//    json_spirit::mObject obj;

    std::vector<std::string> plugins;

    for (std::vector<plugin*>::iterator i = plugin_list.begin(); i != plugin_list.end(); ++i)
      {
      plugins.push_back((*i)->get_name());
      }

//    obj["plugins"] = json_spirit::Value(plugins.begin(), plugins.end());
    json_spirit::Value pa(plugins.begin(), plugins.end());

    rep.content = json_spirit::write_string(pa);
    }

  void list_ports (const http::server::request &req, http::server::reply &rep)
    {
    }

  void srvthread ()
    {
    boost::asio::io_service io_service;
    
    try
      {
      http::server::rest_handler  handler;

      handler.register_path("/plugins", boost::bind(list_plugins, _1, _2));

      http::server::server(io_service, "0.0.0.0", "80", handler)();
      boost::asio::signal_set signals(io_service);
      signals.add(SIGINT);
      signals.add(SIGTERM);
      signals.async_wait(boost::bind(&boost::asio::io_service::stop, &io_service));

      io_service.run();
      }
    catch(boost::thread_interrupted&)
      {
      ROFL_INFO("REST Server shutting down");
      return;
      }
    }

  void rest::init(int args, char** argv)
    {
    ROFL_INFO("Starting REST server");
    boost::thread t(&srvthread);

    return;
    };
  } // namespace xdpd
