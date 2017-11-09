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
#define LSI_CONNECTION_SSL_PRIVATE_KEY_FILE_PASSWORD "ssl-key-file-password"
#define LSI_CONNECTION_SSL_CA_PATH "ssl-ca-path"
#define LSI_CONNECTION_SSL_CA_FILE_FILE "ssl-ca-file"
#define LSI_CONNECTION_SSL_VERIFY_MODE "ssl-verify-mode"
#define LSI_CONNECTION_SSL_VERIFY_DEPTH "ssl-verify-depth"
#define LSI_CONNECTION_SSL_CIPHER "ssl-cipher"

//namespace
using namespace xdpd::plugins::yaml_config;

const std::string lsi_connections_scope::SCOPE_NAME = "controller-connections";

//Connection
lsi_connection_scope::lsi_connection_scope(std::string name, scope* parent):scope(name, parent, false){

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
	register_parameter(LSI_CONNECTION_SSL_PRIVATE_KEY_FILE_PASSWORD);
	register_parameter(LSI_CONNECTION_SSL_CA_PATH);
	register_parameter(LSI_CONNECTION_SSL_CA_FILE_FILE);
	register_parameter(LSI_CONNECTION_SSL_VERIFY_MODE);
	register_parameter(LSI_CONNECTION_SSL_VERIFY_DEPTH);
	register_parameter(LSI_CONNECTION_SSL_CIPHER);
}

//Connections
lsi_connections_scope::lsi_connections_scope(scope* parent):scope(lsi_connections_scope::SCOPE_NAME, parent, true){

}

void lsi_connections_scope::parse_connection_params(YAML::Node& node, lsi_connection& con){

	//Defaults
	std::string hostname = "127.0.0.1";
	std::string port = "6653";

	//Auxiliary
	std::string aux;
	std::stringstream ss;
	int _port;

	//Parse hostname
	if(node[LSI_CONNECTION_CONTROLLER_HOSTNAME]){
		hostname = node[LSI_CONNECTION_CONTROLLER_HOSTNAME].as<std::string>();
	}
	con.params.set_param(xdpd::csocket::PARAM_KEY_REMOTE_HOSTNAME) = hostname;

	//Parse port
	if(node[LSI_CONNECTION_CONTROLLER_PORT]){
		_port = node[LSI_CONNECTION_CONTROLLER_PORT].as<int>();

		if(_port < 1 || _port > 65535){
			XDPD_ERR(YAML_PLUGIN_ID "%s: invalid controller port number %u. Must be [1-65535]\n", name.c_str(), _port);
			throw eYamlConfParseError();
		}

		//Convert to string
		ss << _port;
		port = ss.str();
	}
	con.params.set_param(xdpd::csocket::PARAM_KEY_REMOTE_PORT) = port;

	//Parse family
	if(node[LSI_CONNECTION_CONTROLLER_FAMILY]){
		aux = node[LSI_CONNECTION_CONTROLLER_FAMILY].as<std::string>();
		con.params.set_param(xdpd::csocket::PARAM_KEY_DOMAIN) = aux;
	}
	//Bind address
	if(node[LSI_CONNECTION_CONTROLLER_BIND_ADDRESS]){
		aux = node[LSI_CONNECTION_CONTROLLER_BIND_ADDRESS].as<std::string>();
		con.params.set_param(xdpd::csocket::PARAM_KEY_LOCAL_HOSTNAME) = aux;
	}

	//Parse bind port
	if(node[LSI_CONNECTION_CONTROLLER_BIND_PORT]){
		_port = node[LSI_CONNECTION_CONTROLLER_BIND_PORT].as<int>();
		ss.clear();
		ss << _port;
		con.params.set_param(xdpd::csocket::PARAM_KEY_LOCAL_PORT) = ss.str();
	}

}

void lsi_connections_scope::parse_ssl_connection_params(YAML::Node& node, lsi_connection& con, bool dry_run){

	bool weak_ssl_config=true;
	int mandatory_params_found=0;
	std::string tmp;

	//SSL specific
	if (node[LSI_CONNECTION_SSL_CERTIFICATE_FILE]) {
		con.params.set_param(xdpd::csocket::PARAM_SSL_KEY_CERT) = node[LSI_CONNECTION_SSL_CERTIFICATE_FILE].as<std::string>();
		mandatory_params_found++;
	}
	if (node[LSI_CONNECTION_SSL_PRIVATE_KEY_FILE]) {
		con.params.set_param(xdpd::csocket::PARAM_SSL_KEY_PRIVATE_KEY) = node[LSI_CONNECTION_SSL_PRIVATE_KEY_FILE].as<std::string>();
		mandatory_params_found++;
	}
	if (node[LSI_CONNECTION_SSL_PRIVATE_KEY_FILE_PASSWORD]) {
		con.params.set_param(xdpd::csocket::PARAM_SSL_KEY_PRIVATE_KEY_PASSWORD) = node[LSI_CONNECTION_SSL_PRIVATE_KEY_FILE_PASSWORD].as<std::string>();
	}
	if (node[LSI_CONNECTION_SSL_CA_FILE_FILE]) {
		con.params.set_param(xdpd::csocket::PARAM_SSL_KEY_CA_FILE) = node[LSI_CONNECTION_SSL_CA_FILE_FILE].as<std::string>();
		mandatory_params_found++;
	}
	if (node[LSI_CONNECTION_SSL_CA_PATH]) {
		con.params.set_param(xdpd::csocket::PARAM_SSL_KEY_CA_PATH) = node[LSI_CONNECTION_SSL_CA_PATH].as<std::string>();
		mandatory_params_found++;
	}
	if (node[LSI_CONNECTION_SSL_VERIFY_MODE]) {
		con.params.set_param(xdpd::csocket::PARAM_SSL_KEY_VERIFY_MODE) = node[LSI_CONNECTION_SSL_VERIFY_MODE].as<std::string>();
	}
	if (node[LSI_CONNECTION_SSL_VERIFY_DEPTH]) {
		con.params.set_param(xdpd::csocket::PARAM_SSL_KEY_VERIFY_DEPTH) = node[LSI_CONNECTION_SSL_VERIFY_DEPTH].as<std::string>();
	}
	if (node[LSI_CONNECTION_SSL_CIPHER]) {
		con.params.set_param(xdpd::csocket::PARAM_SSL_KEY_CIPHERS) = node[LSI_CONNECTION_SSL_CIPHER].as<std::string>();
	}


	//Issue warning
	if(weak_ssl_config){
		if(dry_run)
			XDPD_WARN(YAML_PLUGIN_ID "%s: WARNING the connection only provide encryption but no authentication. \n", name.c_str());
	}else{
		if(mandatory_params_found != 3){
			XDPD_ERR(YAML_PLUGIN_ID "%s: ERROR the connection does not provide the necessary SSL parameters. Required parameters are: %s, %s, (%s or %s). \n", name.c_str(), LSI_CONNECTION_SSL_CERTIFICATE_FILE, LSI_CONNECTION_SSL_PRIVATE_KEY_FILE, LSI_CONNECTION_SSL_CA_PATH, LSI_CONNECTION_SSL_CA_FILE_FILE );
			throw eYamlConfParseError();
		}
	}
}

lsi_connection lsi_connections_scope::parse_connection(YAML::Node& node, bool dry_run){

	lsi_connection con;

	bool enable_ssl = false;
	if (node[LSI_CONNECTION_SSL]) {
		enable_ssl = node[LSI_CONNECTION_SSL].as<bool>();
	}

	if (enable_ssl) {
		con.type = xdpd::csocket::SOCKET_TYPE_OPENSSL;
		//Generate list of empty parameters for this socket
		con.params = xdpd::csocket::get_default_params(con.type);

		//Parse specific stuff for SSL
		parse_ssl_connection_params(node, con, dry_run);
	}else{
		con.type = xdpd::csocket::SOCKET_TYPE_PLAIN;
		//Generate list of empty parameters for this socket
		con.params = xdpd::csocket::get_default_params(con.type);
	}

	//Fill common parameters
	parse_connection_params(node, con);

	return con;
}


void lsi_connections_scope::pre_validate(YAML::Node& node, bool dry_run){

	if (node.IsMap() && (node.size() == 0)) {
		XDPD_ERR(YAML_PLUGIN_ID "%s: No controller connections found! At least one connection is mandatory\n", name.c_str());
		throw eYamlConfParseError();
	}

	//Detect existing subscopes (logical switches) and register
	for (auto it : node) {
		std::string conn_name = it.first.as<std::string>();
		XDPD_DEBUG_VERBOSE(YAML_PLUGIN_ID "[%s] Found controller connection named: %s\n", get_path().c_str(), conn_name.c_str());

		//Pre-Parse and add to the list of connections
		parsed_connections.push_back(parse_connection(it.second, dry_run));

		//Register subscope
		register_subscope(std::string(conn_name), new lsi_connection_scope(conn_name, this));
	}
}
