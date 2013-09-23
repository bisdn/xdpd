#include "config.h"
#include <rofl/platform/unix/cunixenv.h>

using namespace xdpd;
using namespace rofl;
using namespace libconfig;

void config::init(int args, char** argv){

	//DO something
	std::cerr<<"Initalizing.. config"<<std::endl;
	Config cfg;

	//Read the file. If there is an error, report it and exit.
	try{
		cfg.readFile(cunixenv::getInstance().get_arg("config-file").c_str());
	}catch(const FileIOException &fioex){
		std::cerr << "I/O error while reading file." << std::endl;
		throw fioex;
	}catch(const ParseException &pex){
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
		<< " - " << pex.getError() << std::endl;
		throw pex;
	}
}
