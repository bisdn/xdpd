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
* 
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
    virtual void init(int args, char** argv);

    virtual std::string get_name(void){
      return std::string("REST plugin");
    };
  };

}// namespace xdpd 

#endif /* REST_PLUGIN_H_ */


