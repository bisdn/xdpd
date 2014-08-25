#include "group_table_snapshot.h"
#include <rofl/datapath/pipeline/openflow/of_switch.h>

//Translators for matches and actions
//#include "../../openflow/openflow10/of10_translation_utils.h"
//#include "../../openflow/openflow12/of12_translation_utils.h"
//#include "../../openflow/openflow13/of13_translation_utils.h"

using namespace xdpd;

bucket_snapshot::bucket_snapshot(of_version_t ver,ff){

	switch(ver){
			case OF_VERSION_10:
				assert(0);
				break;
			case OF_VERSION_12:
				of12_translation_utils::of12_map_reverse_bucket_list();
				break;
			case OF_VERSION_13:
				of13_translation_utils::of13_map_reverse_bucket_list();
				break;
			default:
				assert(0);
				break;
	}
}