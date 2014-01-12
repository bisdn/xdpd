#include <iostream>

#include "rest.h"
#include <rofl/common/utils/c_logger.h>

#include "../../plugin_manager.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <signal.h>

#include "server/server.hpp"
#include "server/file_handler.hpp"

using namespace xdpd;

void srvthread ()
  {
  boost::asio::io_service io_service;
  
  try
    {
    http::server::server(io_service, "0.0.0.0", "80", http::server::file_handler("/tmp"))();
    boost::asio::signal_set signals(io_service);
    signals.add(SIGINT);
    signals.add(SIGTERM);
    signals.async_wait(boost::bind(
          &boost::asio::io_service::stop, &io_service));

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
  std::vector<plugin*> plugin_list = plugin_manager::get_plugins();

  for (std::vector<plugin*>::iterator i = plugin_list.begin(); i != plugin_list.end(); ++i)
    {
    ROFL_INFO("Plugin: %s\n", (*i)->get_name().c_str());
    }

  ROFL_INFO("Starting REST server");
  boost::thread t(&srvthread);

  return;
  };

