#include "nf_scope.h"

using namespace xdpd;

nf_scope::nf_scope(scope* parent):scope("nf", parent, false){}

void nf_scope::__pre_execute(libconfig::Setting& setting, bool dry_run){
	

}

