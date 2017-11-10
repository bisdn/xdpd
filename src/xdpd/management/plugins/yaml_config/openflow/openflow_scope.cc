#include "openflow_scope.h"
#include "lsi_scope.h"

#include "../yaml_config.h"

using namespace xdpd::plugins::yaml_config;

openflow_scope::openflow_scope(scope* parent):scope("openflow", parent, true){

	//Register parameters
	//None for the moment

	//Register subscopes
	//Subscopes are logical switch elements so will be captured on pre_validate hook
	register_subscope(new of_lsis_scope(this));	
}


of_lsis_scope::of_lsis_scope(scope* parent):scope("lsis", parent, true){

	//Register subscopes
	//Subscopes are logical switch elements so will be captured on pre_validate hook
}


void of_lsis_scope::pre_validate(YAML::Node& node, bool dry_run){

	if (node.IsMap()) {
		if (node.size() == 0) {
			XDPD_ERR(YAML_PLUGIN_ID "%s: No logical switches found!\n", get_path().c_str());
			throw eYamlConfParseError();
		}
		for (auto lsi : node) {
			std::string lsi_name = lsi.first.as<std::string>();
			XDPD_DEBUG_VERBOSE(YAML_PLUGIN_ID "[%s] Found logical switch: %s\n", get_path().c_str(), lsi_name.c_str());
			register_subscope(lsi_name, new lsi_scope(lsi_name, this));
		}
	}
}
