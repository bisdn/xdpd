#include "lsi_connections.h"
#include <sstream>

//Default
#define LSI_CONNECTION_CONTROLLER_HOSTNAME "remote-hostname"
#define LSI_CONNECTION_CONTROLLER_PORT "remote-port"
#define LSI_CONNECTION_CONTROLLER_FAMILY "address-family"
#define LSI_CONNECTION_CONTROLLER_BIND_ADDRESS "bind-address"
#define LSI_CONNECTION_CONTROLLER_BIND_PORT "bind-port"

//SSL stuff
#define LSI_CONNECTION_SSL "ssl"
#define LSI_CONNECTION_SSL_CERTIFICATE_FILE "ssl-certificate-file"
#define LSI_CONNECTION_SSL_PRIVATE_KEY_FILE "ssl-key-file"
#define LSI_CONNECTION_SSL_CA_PATH_FILE "ssl-ca-path"
#define LSI_CONNECTION_SSL_CA_FILE_FILE "ssl-ca-file"

//namespace
using namespace xdpd;

//static elements
const std::string lsi_connections_scope::SCOPE_NAME = "controller-connections";

//Connection
lsi_connection_scope::lsi_connection_scope(std::string name):scope(name, false){

	//Common stuff	
	register_parameter(LSI_CONNECTION_CONTROLLER_HOSTNAME);
	register_parameter(LSI_CONNECTION_CONTROLLER_PORT); 
	register_parameter(LSI_CONNECTION_CONTROLLER_FAMILY);
	register_parameter(LSI_CONNECTION_CONTROLLER_BIND_ADDRESS); 
	register_parameter(LSI_CONNECTION_CONTROLLER_BIND_PORT); 

	//SSL stuff
	register_parameter(LSI_CONNECTION_SSL);
	register_parameter(LSI_CONNECTION_SSL_CERTIFICATE_FILE);
	register_parameter(LSI_CONNECTION_SSL_PRIVATE_KEY_FILE);
	register_parameter(LSI_CONNECTION_SSL_CA_PATH_FILE);
	register_parameter(LSI_CONNECTION_SSL_CA_FILE_FILE);
}

//Connections
lsi_connections_scope::lsi_connections_scope(std::string name, bool mandatory):scope(name, mandatory){

}

void lsi_connections_scope::parse_connection_params(libconfig::Setting& setting, lsi_connection& con){

	//Defaults
	std::string hostname = "127.0.0.1";
	std::string port = "6633";
	
	//Auxiliary
	std::string aux; 
	std::stringstream ss;
	int _port;

	//Parse hostname 
	if(setting.exists(LSI_CONNECTION_CONTROLLER_HOSTNAME)){
		setting.lookupValue(LSI_CONNECTION_CONTROLLER_HOSTNAME, hostname);
	}
	con.params.set_param(rofl::csocket::PARAM_KEY_REMOTE_HOSTNAME) = hostname;

	//Parse port 
	if(setting.exists(LSI_CONNECTION_CONTROLLER_PORT)){
		_port = setting[LSI_CONNECTION_CONTROLLER_PORT];

		if(_port < 1 || _port > 65535){
			ROFL_ERR(CONF_PLUGIN_ID "%s: invalid controller port number %u. Must be [1-65535]\n", setting.getPath().c_str(), _port);
			throw eConfParseError(); 	
				
		}
			
		//Convert to string
		ss << _port;
		port = ss.str();
	}
	con.params.set_param(rofl::csocket::PARAM_KEY_REMOTE_PORT) = port; 
	
	//Parse family
	if(setting.exists(LSI_CONNECTION_CONTROLLER_FAMILY)){
		setting.lookupValue(LSI_CONNECTION_CONTROLLER_FAMILY, aux);
		con.params.set_param(rofl::csocket::PARAM_KEY_DOMAIN) = aux;
	}
	//Bind address
	if(setting.exists(LSI_CONNECTION_CONTROLLER_BIND_ADDRESS)){
		setting.lookupValue(LSI_CONNECTION_CONTROLLER_BIND_ADDRESS, aux);
		con.params.set_param(rofl::csocket::PARAM_KEY_LOCAL_HOSTNAME) = aux;
	}

	//Parse bind port
	if(setting.exists(LSI_CONNECTION_CONTROLLER_BIND_PORT)){
		_port = setting[LSI_CONNECTION_CONTROLLER_BIND_PORT];
		ss.clear();
		ss << _port;
		con.params.set_param(rofl::csocket::PARAM_KEY_LOCAL_PORT) = ss.str();
	}

}

void lsi_connections_scope::parse_ssl_connection_params(libconfig::Setting& setting, lsi_connection& con){
	
	//TODO
	
#if 0
	//SSL specific
	std::string cert_and_key_file("");

	if (not setting.lookupValue(LSI_CONNECTION_SSL_CERTIFICATE_FILE, cert_and_key_file)) {
		ROFL_ERR(CONF_PLUGIN_ID "%s: Unable to recover certificate file path\n", setting.getPath().c_str());
		throw eConfParseError();
	}

	//Fill in
	p.set_param(rofl::csocket::PARAM_SSL_KEY_CERT) = cert_and_key_file;

	//TODO: add private-key, ca-path, ca-file
#endif
}

lsi_connection lsi_connections_scope::parse_connection(libconfig::Setting& setting){ 

	lsi_connection con;

	if (setting.exists(LSI_CONNECTION_SSL)) {
		con.type = rofl::csocket::SOCKET_TYPE_OPENSSL;
		//Generate list of empty parameters for this socket
		con.params = rofl::csocket::get_default_params(con.type);

		//Parse specific stuff for SSL
		parse_ssl_connection_params(setting, con);
	}else{
		con.type = rofl::csocket::SOCKET_TYPE_PLAIN;
		//Generate list of empty parameters for this socket
		con.params = rofl::csocket::get_default_params(con.type);
	}

	//Fill common parameters
	parse_connection_params(setting, con);

	return con;
}


void lsi_connections_scope::pre_validate(libconfig::Setting& setting, bool dry_run){

	if(setting.getLength() == 0){
		ROFL_ERR(CONF_PLUGIN_ID "%s: No controller connections found! At least one connection is mandatory\n", setting.getPath().c_str());
		throw eConfParseError(); 	
		
	}
	
	//Detect existing subscopes (logical switches) and register
 	for(int i = 0; i<setting.getLength(); ++i){
		ROFL_DEBUG_VERBOSE(CONF_PLUGIN_ID "Found controller connection named: %s\n", setting[i].getName());

		//Pre-Parse and add to the list of connections
		parsed_connections.push_back(parse_connection(setting[i]));
		
		//Register subscope	
		register_subscope(std::string(setting[i].getName()), new lsi_connection_scope(setting[i].getName()));
	}
		
}
