//Guards used only when inlining
#ifndef PACKET_IMPL_INLINE
#define PACKET_IMPL_INLINE

#include <rofl.h>

//Define inline or not depending if the pipeline was compiled with inlined functions
#if !defined(ROFL_PIPELINE_INLINE_PP_PLATFORM_FUNCS)
	#define STATIC_PACKET_INLINE__ 
	#include <rofl/datapath/pipeline/platform/packet.h>
#else
	#define STATIC_PACKET_INLINE__ static inline 
	#include <rofl/datapath/pipeline/util/pp_guard.h> // Do not forget the guard 
	#include "packet.cc" //Yes, nasty :)	
#endif //Define inline
#endif //Guards used only when inlining
