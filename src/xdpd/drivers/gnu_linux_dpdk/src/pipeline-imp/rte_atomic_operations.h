//Guards used only when inlining
#ifndef ATOMIC_OPS_IMPL_INLINE
#define ATOMIC_OPS_IMPL_INLINE

#include <rofl.h> 
#include <inttypes.h>

//Define inline or not depending if the pipeline was compiled with inlined functions
#if !defined(ROFL_PIPELINE_INLINE_PP_PLATFORM_FUNCS)
	#include <rofl/datapath/pipeline/platform/atomic_operations.h>
	#define STATIC_ATOMIC_INLINE__ 
#else
	#define STATIC_ATOMIC_INLINE__ inline
	#include <rofl/datapath/pipeline/util/pp_guard.h> // Do not forget the guard 
	#include <rofl/datapath/pipeline/platform/atomic_operations.h>
	#include "rte_atomic_operations.c" //Yes, nasty :)	
#endif //Defined inline
#endif //Guards used only when inlining
