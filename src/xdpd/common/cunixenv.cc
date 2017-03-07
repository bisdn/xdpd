/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cunixenv.h"

using namespace xdpd;

extern char* optarg;
extern int opterr;
extern int optind;

/* static */const std::string daemon_dir("/var/tmp");

/* Carg stuff */
coption::coption(bool optional, 
		int value_type,
		char shortcut, 
		std::string full_name, 
		std::string description, 
		std::string default_value):
		optional(optional), 
		value_type(value_type), 
		shortcut(shortcut), 
		full_name(full_name), 
		description(description), 
		default_value(default_value)
{
	current_value = default_value;
	present = false;
	
	if(!(this->value_type == NO_ARGUMENT ||
		this->value_type == REQUIRED_ARGUMENT|| 
		this->value_type == OPTIONAL_ARGUMENT )) 
			throw std::runtime_error("Unknown value type");
}	

std::string coption::parse_argument(char* optarg){
	if(value_type != NO_ARGUMENT)
		this->current_value = std::string(optarg);
	present = true;
	
	return this->current_value;	
}

/*
*
* cunixenv
*
*/

/* Constructor */ 
cunixenv::cunixenv(int argc, char** argv)
{
	std::stringstream ss("");

	for (int i = 0; i < argc; i++) {
		cargs.push_back(std::string(argv[i]));
	}

	/*
	* Default arguments are debug and help ONLY
	*/
	
	//Prepare debug debug level
	coption debug = coption(true,REQUIRED_ARGUMENT,'d',"debug","debug level",ss.str());
	arguments.push_back(debug);
	
	coption help = coption(true,NO_ARGUMENT,'h',"help","Help message","");
	arguments.push_back(help);
	
	parsed = false;
}

std::string 
cunixenv::get_usage(char *argv0)
{
	std::stringstream ss;
	ss << "usage: " << std::string(argv0)<<" {parameters}"<<std::endl; 
	
	std::string mandatory = "";
	std::string optional = "";

	for(std::vector<coption>::iterator it = this->arguments.begin(); it != this->arguments.end(); ++it){
		std::string tmp(""); 
		if(it->optional)
			tmp+="[";
		tmp+="--"+it->full_name+"|-"+it->shortcut;
		
		if(it->optional)
			tmp+="]";

		//Tabulate
		for(int i= 30 - tmp.length();i>0;i--){
			tmp +=" ";
		}

		tmp +=" <"+it->description;
		if(it->default_value != "")
			tmp+=". default("+it->default_value+")";
		tmp+=">\n";
		
		if(!it->optional)
			mandatory += tmp; 
		else
			optional += tmp; 
			
	}
	if(mandatory!= "")
		ss<<"\nMandatory parameters:"<<std::endl<<mandatory;	
	if(optional!= "")
		ss<<"\nOptional parameters:"<<std::endl<<optional;	
	
	return ss.str();
}

void
cunixenv::update_default_option(const std::string &option_name, const std::string &value)
{
	for(std::vector<coption>::iterator it = this->arguments.begin(); it != this->arguments.end(); ++it) {
		if ( 0 == (*it).full_name.compare(option_name) ) {
			(*it).current_value = value;
			(*it).default_value = value;
		}
	}
}

void
cunixenv::parse_args()
{
	int c;
	int option_index;
	std::string format;
	struct option* long_options = (struct option*)calloc(1,sizeof(struct option)*(this->arguments.size()+1));

	//Construct getopts stuff
	for(std::vector<coption>::iterator it = this->arguments.begin(); it != this->arguments.end(); ++it){
		format += it->shortcut;
		if(it->value_type == REQUIRED_ARGUMENT)
			format+=":"; 
		else if(it->value_type == OPTIONAL_ARGUMENT)
			format+="::"; 

		//Fill long_options row
		struct option* tmp = &long_options[std::distance(this->arguments.begin(), it)];
		tmp->has_arg = it->value_type;
		tmp->name = it->full_name.c_str();
		tmp->val = it->shortcut;
	
	}

	// recreate argc/argv, as it might get altered by getopt
	int argc = cargs.size();
	char** argv = (char**)calloc(1, sizeof(char*) * cargs.size()+1);
	for (int i = 0; i < argc; i++) {
		argv[i] = (char*)cargs[i].c_str();
	}

	opterr = 0; //Disable message "invalid option"
	optind = 1;

	while(true){
			
		c = getopt_long(argc, argv, format.c_str(), long_options, &option_index);
		if (c == -1)
			break;
		//TODO: might be better to have a map	
		for(std::vector<coption>::iterator it = this->arguments.begin(); it != this->arguments.end(); ++it){
			if(it->shortcut == c){
				it->parse_argument(optarg);
			}
		}
	}
	
	//free calloc
	free(argv);
	free(long_options);	
	parsed = true;

}

void cunixenv::add_option(const coption &arg){

	std::stringstream error_str;
	error_str << "Duplicated command line argument: ";

	//Check if it exists
	std::vector<coption>::iterator it;
	for(it = this->arguments.begin(); it != this->arguments.end(); ++it){
		
		if( (*it).full_name == arg.full_name ){
			error_str << arg.full_name;
			throw std::runtime_error(error_str.str());
		}
			
		if( (*it).shortcut == arg.shortcut ){
			error_str << arg.shortcut;
			throw std::runtime_error(error_str.str());
		}
	}

	arguments.push_back(arg);	
}

std::string cunixenv::get_arg(const std::string &name){
	if(!parsed)
		throw std::runtime_error("Args not yet parsed. use parse_args()");
	std::vector<coption>::iterator it;
	for(it = this->arguments.begin(); it != this->arguments.end(); ++it){
		if(it->full_name == name)
			return it->current_value;
	}
	throw std::runtime_error("Unknown argument");
}

std::string cunixenv::get_arg(char shortcut){

	if(!parsed)
		throw std::runtime_error("Args not yet parsed. use parse_args()");
	
	std::vector<coption>::iterator it;
	for(it = this->arguments.begin(); it != this->arguments.end(); ++it){
		if(it->shortcut == shortcut)
			return it->current_value;
	}
	throw std::runtime_error("Unknown argument");
}

bool
cunixenv::is_arg_set(const std::string &name)
{
	if(!parsed)
		throw std::runtime_error("Args not yet parsed. use parse_args()");

	for(std::vector<coption>::iterator it = this->arguments.begin(); it != this->arguments.end(); ++it){
		if( 0 == (*it).full_name.compare(name)) {
			return (*it).is_present();
		}
	}

	return false;
}

cunixenv::~cunixenv() {
	// everything destroyed anyway...
}
