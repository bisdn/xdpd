// Copyright (c) 2014  Barnstormer Softworks, Ltd.

#include <boost/lexical_cast.hpp>

#include <rofl/common/utils/c_logger.h>

#include "server/request.hpp"
#include "server/reply.hpp"

#include "json_spirit/json_spirit_writer_template.h"

#include "../../plugin_manager.h"

#define _LINUX_IF_H
#include "../../switch_manager.h"

namespace endpoints
  {
  void list_plugins (const http::server::request &req, http::server::reply &rep)
    {
    std::vector<xdpd::plugin*> plugin_list = xdpd::plugin_manager::get_plugins();

//    json_spirit::mObject obj;

    std::vector<std::string> plugins;

    for (std::vector<xdpd::plugin*>::iterator i = plugin_list.begin(); i != plugin_list.end(); ++i)
      {
      plugins.push_back((*i)->get_name());
      }

//    obj["plugins"] = json_spirit::Value(plugins.begin(), plugins.end());
    json_spirit::Value pa(plugins.begin(), plugins.end());

    rep.content = json_spirit::write_string(pa);
    }

  void list_ports (const http::server::request &req, http::server::reply &rep)
    {
	  json_spirit::Value tbd(std::string("tdb"));
	  rep.content = json_spirit::write_string(tbd);
    }

  void list_datapaths (const http::server::request &req, http::server::reply &rep)
    {
    std::list<std::string> datapaths = xdpd::switch_manager::list_sw_names();
    json_spirit::Value dl(datapaths.begin(), datapaths.end());
    rep.content = json_spirit::write_string(dl);
    }

  } // namespace endpoints 
