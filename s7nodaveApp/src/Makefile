TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += s7nodave

# install s7nodave.dbd into <top>/dbd
DBD += s7nodave.dbd

#s7nodave_DBD = asyn.dbd

# specify all source files to be compiled and added to the library
s7nodave_SRCS += S7nodaveAnalogSupport.cc
s7nodave_SRCS += S7nodaveArraySupport.cc
s7nodave_SRCS += S7nodaveBinarySupport.cc
s7nodave_SRCS += S7nodaveInputRecord.cc
s7nodave_SRCS += S7nodaveOutputRecord.cc
s7nodave_SRCS += S7nodaveLongSupport.cc
s7nodave_SRCS += S7nodaveMultiBinarySupport.cc
s7nodave_SRCS += S7nodavePlcAddress.cc
s7nodave_SRCS += S7nodavePollGroup.cc
s7nodave_SRCS += S7nodavePortDriver.cc
s7nodave_SRCS += S7nodavePortDriverReadItem.cc
s7nodave_SRCS += S7nodavePortDriverReadOptimizer.cc
s7nodave_SRCS += S7nodaveRecord.cc
s7nodave_SRCS += S7nodaveRecordAddress.cc
s7nodave_SRCS += S7nodaveStringSupport.cc
s7nodave_SRCS += s7nodaveAsyn.cc
s7nodave_SRCS += s7nodaveRecordDefinitions.c
s7nodave_SRCS += s7nodaveRegistrar.c

s7nodave_LIBS += asyn
s7nodave_LIBS += nodave
s7nodave_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

