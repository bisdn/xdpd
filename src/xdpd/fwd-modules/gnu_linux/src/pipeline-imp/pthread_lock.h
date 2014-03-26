//Guards used only when inlining
#ifndef PTHREAD_LOCK_IMPL_INLINE
#define PTHREAD_LOCK_IMPL_INLINE

#include <rofl.h>
#include <pthread.h>

//Define inline or not depending if the pipeline was compiled with inlined functions
#if !defined(ROFL_PIPELINE_INLINE_PP_PLATFORM_FUNCS)
	#include <rofl/datapath/pipeline/platform/lock.h>
	#define STATIC_LOCK_INLINE__ 
#else
	#define STATIC_LOCK_INLINE__ static inline 
	#include <rofl/datapath/pipeline/util/pp_guard.h> // Do not forget the guard 
	#include <rofl/datapath/pipeline/platform/lock.h>
	#include "pthread_lock.c" //Yes, nasty :)	
#endif //Define inline
#endif //Guards used only when inlining
