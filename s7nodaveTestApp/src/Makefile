TOP=../..

include $(TOP)/configure/CONFIG

PROD_IOC = s7nodaveTest

# s7nodaveTest.dbd will be created and installed
DBD += s7nodaveTest.dbd

s7nodaveTest_DBD += base.dbd
s7nodaveTest_DBD += s7nodave.dbd

s7nodaveTest_LIBS += s7nodave
s7nodaveTest_LIBS += asyn
s7nodaveTest_LIBS += snap7
s7nodaveTest_LIBS += $(EPICS_BASE_IOC_LIBS)

s7nodaveTest_SYS_LIBS_WIN32 += ws2_32

# s7nodaveTest_registerRecordDeviceDriver.cpp derives from s7nodaveTest.dbd
s7nodaveTest_SRCS += s7nodaveTest_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
s7nodaveTest_SRCS_DEFAULT += s7nodaveTestMain.cpp
s7nodaveTest_SRCS_vxWorks += -nil-

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

