#ifndef _CPP_PPP_H_
#define _CPP_PPP_H_

#include <rofl/common/protocols/fpppframe.h>

using namespace rofl;
using namespace xdpd::gnu_linux;

inline static
uint16_t get_ppp_prot(void *hdr){
	return ((fpppframe*)hdr)->get_ppp_prot();
}

inline static
void set_ppp_prot(void *hdr, uint16_t prot)
{
	((fpppframe*)hdr)->set_ppp_prot(prot);
}

inline static
uint8_t get_lcp_code(void *hdr)
{
	return ((fpppframe*)hdr)->get_lcp_code();
}

inline static
void set_lcp_code(void *hdr, uint8_t code)
{
	((fpppframe*)hdr)->set_lcp_code(code);
}

inline static
uint8_t get_lcp_ident(void *hdr)
{
	return ((fpppframe*)hdr)->get_lcp_ident();
}

inline static
void set_lcp_ident(void *hdr, uint8_t ident)
{
	((fpppframe*)hdr)->set_lcp_ident(ident);
}

inline static
uint16_t get_lcp_length(void *hdr)
{
	return ((fpppframe*)hdr)->get_lcp_length();
}

inline static
void set_lcp_length(void *hdr, uint16_t len)
{
	((fpppframe*)hdr)->set_lcp_length(len);
}

#if 0
TODO
inline static
cpc_ppp_lcp_option_t* get_lcp_option(void *hdr, enum ppp_lcp_option_t option)
{
	if (lcp_options.find(option) == lcp_options.end()) throw ePPPLcpOptionNotFound();

	return lcp_options[option];
}
#endif

inline static
uint8_t get_ipcp_code(void *hdr)
{
	return ((fpppframe*)hdr)->get_ipcp_code();
}

inline static
void set_ipcp_code(void *hdr, uint8_t code)
{
	((fpppframe*)hdr)->set_ipcp_code(code);
}

inline static
uint8_t get_ipcp_ident(void *hdr)
{
	return ((fpppframe*)hdr)->get_ipcp_ident();
}

inline static
void set_ipcp_ident(void *hdr, uint8_t ident)
{
	((fpppframe*)hdr)->set_ipcp_ident(ident);
}

inline static
uint16_t get_ipcp_length(void *hdr)
{
	return ((fpppframe*)hdr)->get_ipcp_length();
}

inline static
void set_ipcp_length(void *hdr, uint16_t len){
	((fpppframe*)hdr)->set_ipcp_length(len);
}

#if 0
TODO
inline static
fppp_ipcp_option* get_ipcp_option(void *hdr, enum ppp_ipcp_option_t option)
{
	//if (ipcp_options.find(option) == ipcp_options.end()) throw ePPPIpcpOptionNotFound();

	return ipcp_options[option];
}
#endif

#endif //_CPP_PPP_H_
