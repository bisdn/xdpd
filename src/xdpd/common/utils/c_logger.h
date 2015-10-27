/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ROFL_LOG_H_
#define ROFL_LOG_H_

#include <stdlib.h>
#include <stdio.h>
#include <rofl_common.h>

/**
* @file c_logger.h
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>, Marc Sune<marc.sune (at) bisdn.de>  
* @brief Lightweight platform-indepedent C logger
*/

//Define debug levels
enum rofl_debug_levels {
	UNDEF_DEBUG_LEVEL = -1,		/* Undefined debug level */
	//EMERGENCY_LEVEL,			/* system is unusable */
	//ALERT_LEVEL,			/* action must be taken immediately */
	//CRITICAL_LEVEL,			/* critical conditions */
	ERROR_LEVEL,				/* error conditions */
	WARN_LEVEL,				/* warning conditions */
	//NOTICE_LEVEL,			/* normal but significant condition */
	INFO_LEVEL,				/* informational */
	DBG_LEVEL,				/* debug-level messages */
	DBG_VERBOSE_LEVEL,			/* debug-level messages */
	MAX_DEBUG_LEVEL			/* DO NOT USE */

	/* do not put anything beyond MAX_DEBUG_LEVEL! */
};

//Debug classes (not used currently)
enum rofl_debug_class {
	UNDEF_DEBUG_CLASS = -1,		/* Undefined debug level */
	DEFAULT = 0,			/* todo name it correct */
	MAX_DEBUG_CLASS			/* DO NOT USE */

	/* do not put anything beyond MAX_DEBUG_CLASS! */
};

//Todo if ever use groups/classes modify this
#ifndef LOGGING_LEVEL
	//If not defined default value DBG
	#define ROFL_DEFAULT_LEVELS { DBG_LEVEL } /* default for each class */
#else
	#define ROFL_DEFAULT_LEVELS { LOGGING_LEVEL }
#endif

//Fwd declarations
extern enum rofl_debug_levels rofl_debug_level[MAX_DEBUG_CLASS];
extern int (*rofl_debug_print)(FILE *stream, const char *format, ...);

//Define macros
#define ROFL_DEBUG_CHECK(cn, level)  \
    ( rofl_debug_level[cn] >= level )
#define ROFL_DEBUG_PRINT(fd, cn, level, stuff, ...)  \
    do{\
	if (ROFL_DEBUG_CHECK(cn, level) && *rofl_debug_print != NULL){ \
		rofl_debug_print(fd,stuff, ##__VA_ARGS__);\
	}\
    }while(0)

#define ROFL_WARN(stuff,...) \
	ROFL_DEBUG_PRINT(stderr, DEFAULT, WARN_LEVEL, stuff, ##__VA_ARGS__)

#define ROFL_ERR(stuff, ...)          \
	ROFL_DEBUG_PRINT(stderr, DEFAULT, ERROR_LEVEL, stuff, ##__VA_ARGS__)

#define ROFL_INFO(stuff,...) \
	ROFL_DEBUG_PRINT(stderr, DEFAULT, INFO_LEVEL, stuff, ##__VA_ARGS__)


#ifdef DEBUG
	#define ROFL_DEBUG(stuff, ...)        \
		ROFL_DEBUG_PRINT(stderr, DEFAULT, DBG_LEVEL, stuff, ##__VA_ARGS__)

	#define ROFL_DEBUG_VERBOSE(stuff, ...)        \
		ROFL_DEBUG_PRINT(stderr, DEFAULT, DBG_VERBOSE_LEVEL, stuff, ##__VA_ARGS__)
#else
	//No logging
	//#define ROFL_DEBUG_CHECK(stuff, ...) do{}while(0)
	//#define ROFL_DEBUG_PRINT(stuff, ...) do{}while(0)             /* ROFL_DEBUG_CHECK */
	//#define ROFL_WARN(stuff, ...) do{}while(0)
	//#define ROFL_ERR(stuff, ...) do{}while(0)
	//#define ROFL_INFO(stuff,...) do{}while(0)
	#define ROFL_DEBUG(stuff, ...) do{}while(0)
	#define ROFL_DEBUG_VERBOSE(stuff, ...) do{}while(0)
#endif //ROFL_NO_LOGGING

//C++ extern C
#ifdef __cplusplus
	extern "C"{
#endif

//API to capture logging events of the logger 
void rofl_set_logging_function(int (*logging_func)(FILE *stream, const char *format, ...));

//API to capture logging events of the logger 
void rofl_set_logging_level(/*cn,*/ enum rofl_debug_levels level);


//C++ extern C
#ifdef __cplusplus
	}
#endif

#endif /* ROFL_LOG_H_ */
