/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xdpd/common/csocket.h"
#include "xdpd/common/csocket_strings.h"

using namespace xdpd;

/*
 * socket param keys
 */

// common
std::string const 	csocket::PARAM_KEY_DO_RECONNECT("do-reconnect");
std::string const 	csocket::PARAM_KEY_REMOTE_HOSTNAME("remote-hostname");
std::string const 	csocket::PARAM_KEY_REMOTE_PORT("remote-port");			// "6653"
std::string const 	csocket::PARAM_KEY_LOCAL_HOSTNAME("local-hostname");
std::string const 	csocket::PARAM_KEY_LOCAL_PORT("local-port");			// "0"
std::string const	csocket::PARAM_KEY_DOMAIN("domain"); 					// "inet", "inet6"
std::string const	csocket::PARAM_KEY_TYPE("type");						// "stream", "dgram"
std::string const	csocket::PARAM_KEY_PROTOCOL("protocol");				// "tcp", "udp"

// TLS
std::string const	csocket::PARAM_SSL_KEY_CA_PATH("ca-path");
std::string const	csocket::PARAM_SSL_KEY_CA_FILE("ca-file");
std::string const	csocket::PARAM_SSL_KEY_CERT("cert");
std::string const	csocket::PARAM_SSL_KEY_PRIVATE_KEY("key");
std::string const	csocket::PARAM_SSL_KEY_PRIVATE_KEY_PASSWORD("password");
std::string const	csocket::PARAM_SSL_KEY_VERIFY_MODE("verify-mode");
std::string const	csocket::PARAM_SSL_KEY_VERIFY_DEPTH("verify-depth");
std::string const	csocket::PARAM_SSL_KEY_CIPHERS("ciphers");

//Values (non-numeric)
std::string const 	csocket::PARAM_DOMAIN_VALUE_INET_ANY(__PARAM_DOMAIN_VALUE_INET_ANY);
std::string const 	csocket::PARAM_DOMAIN_VALUE_INET("inet");
std::string const 	csocket::PARAM_DOMAIN_VALUE_INET6("inet6");
std::string const 	csocket::PARAM_TYPE_VALUE_STREAM(__PARAM_TYPE_VALUE_STREAM);
std::string const 	csocket::PARAM_TYPE_VALUE_DGRAM("dgram");
std::string const 	csocket::PARAM_PROTOCOL_VALUE_TCP(__PARAM_PROTOCOL_VALUE_TCP);
std::string const 	csocket::PARAM_PROTOCOL_VALUE_UDP("udp");

/*
 * default values
 */

// plain
bool const          csocket::PARAM_DEFAULT_VALUE_DO_RECONNECT         = false;
std::string const   csocket::PARAM_DEFAULT_VALUE_REMOTE_HOSTNAME(std::string("127.0.0.1"));
std::string const   csocket::PARAM_DEFAULT_VALUE_REMOTE_PORT(std::string("6653"));
std::string const   csocket::PARAM_DEFAULT_VALUE_LOCAL_HOSTNAME;
std::string const   csocket::PARAM_DEFAULT_VALUE_LOCAL_PORT;
std::string const   csocket::PARAM_DEFAULT_VALUE_DOMAIN(__PARAM_DOMAIN_VALUE_INET_ANY);
std::string const   csocket::PARAM_DEFAULT_VALUE_TYPE(__PARAM_TYPE_VALUE_STREAM);
std::string const   csocket::PARAM_DEFAULT_VALUE_PROTOCOL(__PARAM_PROTOCOL_VALUE_TCP);

// TLS
std::string const   csocket::PARAM_DEFAULT_VALUE_SSL_KEY_CA_PATH("");
std::string const   csocket::PARAM_DEFAULT_VALUE_SSL_KEY_CA_FILE("ca.pem");
std::string const   csocket::PARAM_DEFAULT_VALUE_SSL_KEY_CERT("cert.pem");
std::string const   csocket::PARAM_DEFAULT_VALUE_SSL_KEY_PRIVATE_KEY("key.pem");
std::string const   csocket::PARAM_DEFAULT_VALUE_SSL_KEY_PRIVATE_KEY_PASSWORD("");
std::string const   csocket::PARAM_DEFAULT_VALUE_SSL_KEY_VERIFY_MODE("PEER"); // NONE|PEER
std::string const   csocket::PARAM_DEFAULT_VALUE_SSL_KEY_VERIFY_DEPTH("1");
std::string const   csocket::PARAM_DEFAULT_VALUE_SSL_KEY_CIPHERS("EECDH+ECDSA+AESGCM EECDH+aRSA+AESGCM EECDH+ECDSA+SHA256 EECDH+aRSA+RC4 EDH+aRSA EECDH RC4 !aNULL !eNULL !LOW !3DES !MD5 !EXP !PSK !SRP !DSS");



cparams
csocket::get_default_params(
		enum socket_type_t socket_type)
{
	switch (socket_type) {
	case SOCKET_TYPE_PLAIN: {
		return get_default_params_plain();
	} break;
	case SOCKET_TYPE_OPENSSL: {
		return get_default_params_tls();
	} break;
	default:
		throw xdpd::exception("invalid socket type");
	}

}

bool
csocket::supports_socket_type(enum socket_type_t socket_type)
{
	switch (socket_type) {

		case SOCKET_TYPE_PLAIN: {
			return true;
		} break;

		case SOCKET_TYPE_OPENSSL: {
			return true; 
		} break;

		default:
			return false;
	}

}


/*static*/
cparams
csocket::get_default_params_plain()
{
        /*
         * fill in cparams structure and fill in default values
         */

        cparams p;
        p.add_param(csocket::PARAM_KEY_DO_RECONNECT).set_bool(PARAM_DEFAULT_VALUE_DO_RECONNECT);
        p.add_param(csocket::PARAM_KEY_REMOTE_HOSTNAME).set_string(PARAM_DEFAULT_VALUE_REMOTE_HOSTNAME);
        p.add_param(csocket::PARAM_KEY_REMOTE_PORT).set_string(PARAM_DEFAULT_VALUE_REMOTE_PORT);
        p.add_param(csocket::PARAM_KEY_LOCAL_HOSTNAME).set_string(PARAM_DEFAULT_VALUE_LOCAL_HOSTNAME);
        p.add_param(csocket::PARAM_KEY_LOCAL_PORT).set_string(PARAM_DEFAULT_VALUE_LOCAL_PORT);
        p.add_param(csocket::PARAM_KEY_DOMAIN).set_string(PARAM_DEFAULT_VALUE_DOMAIN);
        p.add_param(csocket::PARAM_KEY_TYPE).set_string(PARAM_DEFAULT_VALUE_TYPE);
        p.add_param(csocket::PARAM_KEY_PROTOCOL).set_string(PARAM_DEFAULT_VALUE_PROTOCOL);
        return p;
}



/*static*/
cparams
csocket::get_default_params_tls()
{
        cparams p = get_default_params_plain();
        p.add_param(csocket::PARAM_SSL_KEY_CA_PATH).set_string(PARAM_DEFAULT_VALUE_SSL_KEY_CA_PATH);
        p.add_param(csocket::PARAM_SSL_KEY_CA_FILE).set_string(PARAM_DEFAULT_VALUE_SSL_KEY_CA_FILE);
        p.add_param(csocket::PARAM_SSL_KEY_CERT).set_string(PARAM_DEFAULT_VALUE_SSL_KEY_CERT);
        p.add_param(csocket::PARAM_SSL_KEY_PRIVATE_KEY).set_string(PARAM_DEFAULT_VALUE_SSL_KEY_PRIVATE_KEY);
        p.add_param(csocket::PARAM_SSL_KEY_PRIVATE_KEY_PASSWORD).set_string(PARAM_DEFAULT_VALUE_SSL_KEY_PRIVATE_KEY_PASSWORD);
        p.add_param(csocket::PARAM_SSL_KEY_VERIFY_MODE).set_string(PARAM_DEFAULT_VALUE_SSL_KEY_VERIFY_MODE);
        p.add_param(csocket::PARAM_SSL_KEY_VERIFY_DEPTH).set_string(PARAM_DEFAULT_VALUE_SSL_KEY_VERIFY_DEPTH);
        p.add_param(csocket::PARAM_SSL_KEY_CIPHERS).set_string(PARAM_DEFAULT_VALUE_SSL_KEY_CIPHERS);
        return p;
}


