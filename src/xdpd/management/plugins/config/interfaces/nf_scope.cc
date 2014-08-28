#include "nf_scope.h"

using namespace xdpd;

nf_scope::nf_scope(std::string name, bool mandatory):scope(name, mandatory){}

void nf_scope::__pre_execute(libconfig::Setting& setting, bool dry_run){
	

}

