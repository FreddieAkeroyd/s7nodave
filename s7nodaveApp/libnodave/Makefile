TOP=../..

include $(TOP)/configure/CONFIG

LIBRARY_IOC += nodave
SRCS += nodave.c
SRCS_WIN32 += openS7online.c
INC += nodave.h
nodave_SYS_LIBS_WIN32 += ws2_32
USR_CPPFLAGS_WIN32 += -DDOEXPORT -DDAVE_DOEXPORT

include $(TOP)/configure/RULES
