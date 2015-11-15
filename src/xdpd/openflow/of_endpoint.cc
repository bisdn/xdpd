#include "of_endpoint.h"
#include "../management/system_manager.h"

using namespace xdpd;

of_endpoint::of_endpoint(
			rofl::openflow::cofhello_elem_versionbitmap const& versionbitmap,
			enum xdpd::csocket::socket_type_t socket_type,
			const xdpd::cparams& socket_params) :
				sw(NULL),
				versionbitmap(versionbitmap),
				socket_type(socket_type),
				socket_params(socket_params)
{
	//local address
    rofl::csockaddr laddr;
    //remote address
	rofl::csockaddr raddr;


    int domain = 0, sa_family = 0;
    if (socket_params.get_param(csocket::PARAM_KEY_DOMAIN).get_string() == csocket::PARAM_DOMAIN_VALUE_INET) {
            domain = sa_family = PF_INET;
    } else
    if (socket_params.get_param(csocket::PARAM_KEY_DOMAIN).get_string() == csocket::PARAM_DOMAIN_VALUE_INET6) {
            domain = sa_family = PF_INET6;
    } else
    if (socket_params.get_param(csocket::PARAM_KEY_DOMAIN).get_string() == csocket::PARAM_DOMAIN_VALUE_INET_ANY) {
            domain = sa_family = PF_INET6;
    }

	std::string remote_addr;

	if (not socket_params.get_param(csocket::PARAM_KEY_REMOTE_HOSTNAME).get_string().empty()) {

		rofl::caddrinfos addrinfos;
		addrinfos.set_ai_hints().set_ai_family(domain);
		addrinfos.set_node(socket_params.get_param(csocket::PARAM_KEY_REMOTE_HOSTNAME).get_string());
		addrinfos.resolve();

		if (addrinfos.size() == 0) {
			throw rofl::eInvalid("of_endpoint::of_endpoint() unable to resolve hostname");
		}

		// we take simply the first result returned
		switch (domain) {
		case PF_INET: {
			rofl::caddress_in4 addr;
			addr.set_addr_nbo(addrinfos.get_addr_info(0).get_ai_addr().ca_s4addr->sin_addr.s_addr);
			remote_addr = addr.str();
		} break;
		case PF_INET6: {
			rofl::caddress_in6 addr;
			addr.unpack(addrinfos.get_addr_info(0).get_ai_addr().ca_s6addr->sin6_addr.s6_addr, 16);
			remote_addr = addr.str();
		} break;
		default: {
			// use the first entry and its domain
			switch (addrinfos.get_addr_info(0).get_ai_family()) {
			case PF_INET: {
				domain = PF_INET;
				rofl::caddress_in4 addr;
				addr.set_addr_nbo(addrinfos.get_addr_info(0).get_ai_addr().ca_s4addr->sin_addr.s_addr);
				remote_addr = addr.str();
			} break;
			case PF_INET6: {
				domain = PF_INET6;
				rofl::caddress_in6 addr;
				addr.unpack(addrinfos.get_addr_info(0).get_ai_addr().ca_s6addr->sin6_addr.s6_addr, 16);
				remote_addr = addr.str();
			} break;
			default:
				throw eInvalid("of_endpoint::of_endpoint() unable to resolve remote hostname");
			}
		};
		}

		//remote_addr = params.get_param(csocket::PARAM_KEY_REMOTE_HOSTNAME).get_string();

	} else {
		switch (domain) {
		case PF_INET: {
			remote_addr = std::string("0.0.0.0");
		} break;
		case PF_INET6: {
			remote_addr = std::string("0000:0000:0000:0000:0000:0000:0000:0000");
		} break;
		}
	}

	uint16_t remote_port = 0;

	if (not socket_params.get_param(csocket::PARAM_KEY_REMOTE_PORT).get_string().empty()) {
		remote_port = atoi(socket_params.get_param(csocket::PARAM_KEY_REMOTE_PORT).get_string().c_str());
	} else {
		remote_port = 0;
	}



	std::string local_addr;

	if (not socket_params.get_param(csocket::PARAM_KEY_LOCAL_HOSTNAME).get_string().empty()) {

		rofl::caddrinfos addrinfos;
		addrinfos.set_ai_hints().set_ai_family(domain);
		addrinfos.set_node(socket_params.get_param(csocket::PARAM_KEY_LOCAL_HOSTNAME).get_string());
		addrinfos.resolve();

		if (addrinfos.size() == 0) {
			throw rofl::eInvalid("of_endpoint::of_endpoint() unable to resolve hostname");
		}

		// we take simply the first result returned
		switch (domain) {
		case PF_INET: {
			rofl::caddress_in4 addr;
			addr.set_addr_nbo(addrinfos.get_addr_info(0).get_ai_addr().ca_s4addr->sin_addr.s_addr);
			local_addr = addr.str();
		} break;
		case PF_INET6: {
			rofl::caddress_in6 addr;
			addr.unpack(addrinfos.get_addr_info(0).get_ai_addr().ca_s6addr->sin6_addr.s6_addr, 16);
			local_addr = addr.str();
		} break;
		default: {
			// use the first entry and its domain
			switch (addrinfos.get_addr_info(0).get_ai_family()) {
			case PF_INET: {
				if (domain != PF_INET)
					throw rofl::eInvalid("of_endpoint::of_endpoint() unable to resolve local hostname in domain PF_INET");
				rofl::caddress_in4 addr;
				addr.set_addr_nbo(addrinfos.get_addr_info(0).get_ai_addr().ca_s4addr->sin_addr.s_addr);
				local_addr = addr.str();
			} break;
			case PF_INET6: {
				if (domain != PF_INET6)
					throw rofl::eInvalid("of_endpoint::of_endpoint() unable to resolve local hostname in domain PF_INET6");
				rofl::caddress_in6 addr;
				addr.unpack(addrinfos.get_addr_info(0).get_ai_addr().ca_s6addr->sin6_addr.s6_addr, 16);
				local_addr = addr.str();
			} break;
			default:
				throw rofl::eInvalid("of_endpoint::of_endpoint() unable to resolve remote hostname");
			}
		};
		}

		//local_addr = params.get_param(csocket::PARAM_KEY_LOCAL_HOSTNAME).get_string();

	} else {
		switch (domain) {
		case PF_INET: {
			local_addr = std::string("0.0.0.0");
		} break;
		case PF_INET6: {
			local_addr = std::string("0000:0000:0000:0000:0000:0000:0000:0000");
		} break;
		}
	}

	uint16_t local_port;

	if (not socket_params.get_param(csocket::PARAM_KEY_LOCAL_PORT).get_string().empty()) {
		local_port = atoi(socket_params.get_param(csocket::PARAM_KEY_LOCAL_PORT).get_string().c_str());
	} else {
		local_port = 0;
	}

	laddr = rofl::csockaddr(domain, local_addr, local_port);

	raddr = rofl::csockaddr(domain, remote_addr, remote_port);

	//Connect to the main controller
	crofbase::add_ctl(rofl::cctlid(0)).set_conn(rofl::cauxid(0)).
			set_trace(false).
				set_journal().set_max_entries(64);

	crofbase::set_ctl(rofl::cctlid(0)).set_conn(rofl::cauxid(0)).
			set_trace(false).
				set_tcp_journal().set_max_entries(16);

	crofbase::set_ctl(rofl::cctlid(0)).set_conn(rofl::cauxid(0)).
			set_laddr(laddr).set_raddr(raddr).
				tcp_connect(versionbitmap, rofl::crofconn::MODE_DATAPATH);

	crofbase::set_journal().set_max_entries(16);
};



