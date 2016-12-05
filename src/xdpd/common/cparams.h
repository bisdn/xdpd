/*
 * cparams.h
 *
 *  Created on: 01.04.2014
 *      Author: andreas
 */

#ifndef CPARAMS_H_
#define CPARAMS_H_

#include <map>
#include <string>
#include <iostream>

#include <cparam.h>

namespace xdpd {

class cparams {

	std::map<std::string, cparam>		params;

public:

	/**
	 *
	 */
	cparams();

	/**
	 *
	 */
	virtual
	~cparams();

	/**
	 *
	 */
	cparams(
			cparams const& params);

	/**
	 *
	 */
	cparams&
	operator= (
			cparams const& params);

public:

	/**
	 *
	 */
	void
	clear() { params.clear(); };

	/**
	 *
	 */
	std::map<std::string, cparam>&
	set_params() { return params; };

	/**
	 *
	 */
	std::map<std::string, cparam> const&
	get_params() const { return params; };

	/**
	 *
	 */
	cparam&
	add_param(std::string const& key);

	/**
	 *
	 */
	void
	drop_param(std::string const& key);

	/**
	 *
	 */
	cparam&
	set_param(std::string const& key);

	/**
	 *
	 */
	cparam const&
	get_param(std::string const& key) const;

	/**
	 *
	 */
	bool
	has_param(std::string const& key) const;

public:

	friend std::ostream&
	operator<< (std::ostream& os, cparams const& p) {
		os << "<cparams #params: " << p.params.size() << " >" << std::endl;
		for (std::map<std::string, cparam>::const_iterator
				it = p.params.begin(); it != p.params.end(); ++it) {
			os << "<key: \"" << it->first << "\" >" << std::endl;
			os << it->second;
		}
		return os;
	};
};

}; // end of namespace rofl

#endif /* CPARAMS_H_ */
