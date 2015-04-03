#ifndef REST_PLUGIN_H
#define REST_PLUGIN_H 

#include <string>
#include <boost/thread.hpp>

#include "../../plugin_manager.h"
#include "server/request.hpp"
#include "server/reply.hpp"

/**
* @file rest.h
* @author Nick Bastin<nick (at) bssoftworks.com>
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Rest plugin
*/

namespace xdpd {

/**
* @brief Dummy management plugin rest
* @ingroup cmm_mgmt_plugins
*/
class rest:public plugin {

public:
	virtual void init(void);
	virtual ~rest(void);

	virtual std::vector<rofl::coption> get_options(void){
		std::vector<rofl::coption> vec;
		//Add mandatory optiona argument -m to enable management
		//routines
		vec.push_back(rofl::coption(true, NO_ARGUMENT, MGMT_OPT_CODE,  MGMT_OPT_FULL_NAME, "Enable REST management routines (write mode)",""));
		//vec.push_back(rofl::coption(true, NO_ARGUMENT, MGMT_OPT_CODE,  MGMT_OPT_FULL_NAME, "Enable REST management routines (write mode)",""));
		return vec;
	};

	virtual std::string get_name(void){
		return name;
	};

	bool is_mgmt_enabled(void){
		return mgmt_enabled;
	};

	//Plugin name
	static const std::string name;
private:
	boost::thread t;
	static const std::string MGMT_OPT_FULL_NAME;
	static const char MGMT_OPT_CODE = 'm';
	bool mgmt_enabled;
};

}// namespace xdpd 

#endif /* REST_PLUGIN_H_ */


