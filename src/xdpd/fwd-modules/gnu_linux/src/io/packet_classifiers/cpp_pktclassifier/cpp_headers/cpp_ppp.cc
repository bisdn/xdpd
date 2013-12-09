#include "cpp_ppp.h"
#include <rofl/common/protocols/fpppframe.h>

uint16_t get_ppp_prot(void *hdr){
	return ((rofl::fpppframe*)hdr)->get_ppp_prot();
}

void set_ppp_prot(void *hdr, uint16_t prot)
{
	((rofl::fpppframe*)hdr)->set_ppp_prot(prot);
}

uint8_t get_ppp_lcp_code(void *hdr)
{
	return ((rofl::fpppframe*)hdr)->get_lcp_code();
}

void set_ppp_lcp_code(void *hdr, uint8_t code)
{
	((rofl::fpppframe*)hdr)->set_lcp_code(code);
}

uint8_t get_ppp_lcp_ident(void *hdr)
{
	return ((rofl::fpppframe*)hdr)->get_lcp_ident();
}

void set_ppp_lcp_ident(void *hdr, uint8_t ident)
{
	((rofl::fpppframe*)hdr)->set_lcp_ident(ident);
}

uint16_t get_ppp_lcp_length(void *hdr)
{
	return ((rofl::fpppframe*)hdr)->get_lcp_length();
}

void set_ppp_lcp_length(void *hdr, uint16_t len)
{
	((rofl::fpppframe*)hdr)->set_lcp_length(len);
}

#if 0
TODO
cpc_ppp_lcp_option_t* get_lcp_option(void *hdr, enum ppp_lcp_option_t option)
{
	if (lcp_options.find(option) == lcp_options.end()) throw ePPPLcpOptionNotFound();

	return lcp_options[option];
}
#endif

uint8_t get_ppp_ipcp_code(void *hdr)
{
	return ((rofl::fpppframe*)hdr)->get_ipcp_code();
}

void set_ppp_ipcp_code(void *hdr, uint8_t code)
{
	((rofl::fpppframe*)hdr)->set_ipcp_code(code);
}

uint8_t get_ppp_ipcp_ident(void *hdr)
{
	return ((rofl::fpppframe*)hdr)->get_ipcp_ident();
}

void set_ppp_ipcp_ident(void *hdr, uint8_t ident)
{
	((rofl::fpppframe*)hdr)->set_ipcp_ident(ident);
}

uint16_t get_ppp_ipcp_length(void *hdr)
{
	return ((rofl::fpppframe*)hdr)->get_ipcp_length();
}

void set_ppp_ipcp_length(void *hdr, uint16_t len){
	((rofl::fpppframe*)hdr)->set_ipcp_length(len);
}

#if 0
TODO
fppp_ipcp_option* get_ipcp_option(void *hdr, enum ppp_ipcp_option_t option)
{
	//if (ipcp_options.find(option) == ipcp_options.end()) throw ePPPIpcpOptionNotFound();

	return ipcp_options[option];
}
#endif
