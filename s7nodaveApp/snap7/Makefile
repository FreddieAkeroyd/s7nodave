TOP=../..

include $(TOP)/configure/CONFIG

LIBRARY_IOC += snap7
SRCS += s7_isotcp.cpp
SRCS += s7_micro_client.cpp
SRCS += s7_peer.cpp
SRCS += snap_msgsock.cpp
SRCS += snap_sysutils.cpp
INC += s7_isotcp.h
INC += s7_micro_client.h
INC += s7_peer.h
INC += s7_types.h
INC += snap_msgsock.h
INC += snap_platform.h
INC += snap_sysutils.h

ifdef WIN32
ifeq ($(STATIC_BUILD),NO)
USR_CPPFLAGS += -DSNAP7_EXPORT="__declspec(dllexport)"
else
USR_CPPFLAGS += -DSNAP7_EXPORT
endif
else
USR_CPPFLAGS += -DSNAP7_EXPORT
endif

snap7_SYS_LIBS_WIN32 += ws2_32 Winmm
include $(TOP)/configure/RULES
