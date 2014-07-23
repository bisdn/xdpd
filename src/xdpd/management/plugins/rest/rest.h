/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef REST_PLUGIN_H
#define REST_PLUGIN_H 

#include <string>

#include "../../plugin_manager.h"

#include "server/request.hpp"
#include "server/reply.hpp"

/**
* @file rest.h
* @author Nick Bastin<nick (at) bssoftworks.com>
*
* @brief Simple rest plugin
* @warning: this is just sketch plugin with limited functionality 
*/

namespace xdpd {

  void list_plugins (const http::server::request&, http::server::reply&);
  void srvthread (void);

  /**
  * @brief Dummy management plugin rest
  * @ingroup cmm_mgmt_plugins
  */
  class rest:public plugin {
    
  public:
    virtual void init();

    virtual std::string get_name(void){
      return std::string("rest");
    };
  };

}// namespace xdpd 

#endif /* REST_PLUGIN_H_ */


