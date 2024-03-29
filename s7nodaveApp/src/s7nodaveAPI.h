#ifndef INC_s7nodaveAPI_H
#define INC_s7nodaveAPI_H

#include <epicsVersion.h>

#ifndef VERSION_INT
//! Construct version number constant.
#  define VERSION_INT(V,R,M,P) ( ((V)<<24) | ((R)<<16) | ((M)<<8) | (P))
#endif

#ifndef EPICS_VERSION_INT
#  define EPICS_VERSION_INT VERSION_INT(EPICS_VERSION, EPICS_REVISION, EPICS_MODIFICATION, EPICS_PATCH_LEVEL)
#endif

#if defined(_WIN32) && EPICS_VERSION_INT<VERSION_INT(3,15,0,0) && !defined(EPICS_DLL_NO)
#    define EPICS_BUILD_DLL
#    define EPICS_CALL_DLL
#endif

#if defined(_WIN32) || defined(__CYGWIN__)

#  if !defined(epicsStdCall)
#    define epicsStdCall __stdcall
#  endif

#  if defined(BUILDING_s7nodave_API) && defined(EPICS_BUILD_DLL)
/* Building library as dll */
#    define S7NODAVE_API __declspec(dllexport)
#  elif !defined(BUILDING_s7nodave_API) && defined(EPICS_CALL_DLL)
/* Calling library in dll form */
#    define S7NODAVE_API __declspec(dllimport)
#  endif

#elif __GNUC__ >= 4
#  define S7NODAVE_API __attribute__ ((visibility("default")))
#endif

#if !defined(S7NODAVE_API)
#  define S7NODAVE_API
#endif

#if !defined(epicsStdCall)
#  define epicsStdCall
#endif

#endif /* INC_s7nodaveAPI_H */