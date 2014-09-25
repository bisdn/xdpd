/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _NF_DPDK_EXTENSIONS_H_
#define _NF_DPDK_EXTENSIONS_H_

#include <rofl/datapath/hal/driver.h>

//C++ extern C
ROFL_BEGIN_DECLS


hal_result_t hal_driver_dpdk_nf_create_nf_port(const char *nf_name, const char *nf_port_name, port_type_t nf_port_type);

hal_result_t hal_driver_dpdk_nf_destroy_nf_port(const char *nf_port_name);

//C++ extern C
ROFL_END_DECLS

#endif //_NF_DPDK_EXTENSIONS_H_
