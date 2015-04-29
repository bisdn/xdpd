#include "of_endpoint.h"
#include "../management/system_manager.h"

using namespace xdpd;

of_endpoint::of_endpoint(
			rofl::openflow::cofhello_elem_versionbitmap const& versionbitmap,
			enum rofl::csocket::socket_type_t socket_type,
			const rofl::cparams& socket_params) :
				crofbase(versionbitmap, system_manager::ciosrv_thread),
				sw(NULL),
				versionbitmap(versionbitmap),
				socket_type(socket_type),
				socket_params(socket_params) {};
