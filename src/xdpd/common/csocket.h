/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XDPD_COMMON_CSOCKET_H
#define XDPD_COMMON_CSOCKET_H

#include <string>

#include "xdpd/common/cparams.h"

namespace xdpd {

class csocket
{
public:

	/* supported socket types */
	enum socket_type_t {
		SOCKET_TYPE_UNKNOWN		= 0,
		SOCKET_TYPE_PLAIN 		= 1,
		SOCKET_TYPE_OPENSSL 	= 2,
	};

	/**
	 *
	 */
	static cparams
	get_default_params(
			enum socket_type_t socket_type);


	/**
	 *
	 */
	static bool 
	supports_socket_type(enum socket_type_t socket_type);

	/**
	 *
	 */
	static cparams
	get_default_params_plain();

	/**
	 *
	 */
	static cparams
	get_default_params_tls();

public:

	//Common Keys
	static std::string const 	PARAM_KEY_DO_RECONNECT;
	static std::string const 	PARAM_KEY_REMOTE_HOSTNAME;
	static std::string const 	PARAM_KEY_REMOTE_PORT;
	static std::string const 	PARAM_KEY_LOCAL_HOSTNAME;
	static std::string const 	PARAM_KEY_LOCAL_PORT;
	static std::string const	PARAM_KEY_DOMAIN;
	static std::string const	PARAM_KEY_TYPE;
	static std::string const	PARAM_KEY_PROTOCOL;

	//Common values (non-numeric)
	static std::string const	PARAM_DOMAIN_VALUE_INET_ANY;
	static std::string const	PARAM_DOMAIN_VALUE_INET;
	static std::string const	PARAM_DOMAIN_VALUE_INET6;
	static std::string const	PARAM_TYPE_VALUE_STREAM;
	static std::string const	PARAM_TYPE_VALUE_DGRAM;
	static std::string const	PARAM_PROTOCOL_VALUE_TCP;
	static std::string const	PARAM_PROTOCOL_VALUE_UDP;

	//Socket type specific keys
	static std::string const	PARAM_SSL_KEY_CA_PATH;
	static std::string const	PARAM_SSL_KEY_CA_FILE;
	static std::string const	PARAM_SSL_KEY_CERT;
	static std::string const	PARAM_SSL_KEY_PRIVATE_KEY;
	static std::string const	PARAM_SSL_KEY_PRIVATE_KEY_PASSWORD;
	static std::string const	PARAM_SSL_KEY_VERIFY_MODE;
	static std::string const	PARAM_SSL_KEY_VERIFY_DEPTH;
	static std::string const	PARAM_SSL_KEY_CIPHERS;

public:

	// plain socket
    static bool const           PARAM_DEFAULT_VALUE_DO_RECONNECT;
    static std::string const    PARAM_DEFAULT_VALUE_REMOTE_HOSTNAME;
    static std::string const    PARAM_DEFAULT_VALUE_REMOTE_PORT;
    static std::string const    PARAM_DEFAULT_VALUE_LOCAL_HOSTNAME;
    static std::string const    PARAM_DEFAULT_VALUE_LOCAL_PORT;
    static std::string const    PARAM_DEFAULT_VALUE_DOMAIN;
    static std::string const    PARAM_DEFAULT_VALUE_TYPE;
    static std::string const    PARAM_DEFAULT_VALUE_PROTOCOL;

    // TLS socket
    static std::string const    PARAM_DEFAULT_VALUE_SSL_KEY_CA_PATH;
    static std::string const    PARAM_DEFAULT_VALUE_SSL_KEY_CA_FILE;
    static std::string const    PARAM_DEFAULT_VALUE_SSL_KEY_CERT;
    static std::string const    PARAM_DEFAULT_VALUE_SSL_KEY_PRIVATE_KEY;
    static std::string const    PARAM_DEFAULT_VALUE_SSL_KEY_PRIVATE_KEY_PASSWORD;
    static std::string const    PARAM_DEFAULT_VALUE_SSL_KEY_VERIFY_MODE;
    static std::string const    PARAM_DEFAULT_VALUE_SSL_KEY_VERIFY_DEPTH;
    static std::string const    PARAM_DEFAULT_VALUE_SSL_KEY_CIPHERS;
};

}; // end of namespace xdpd

#endif /* XDPD_COMMON_CSOCKET_H */
