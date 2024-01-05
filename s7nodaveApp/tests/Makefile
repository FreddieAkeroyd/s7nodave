TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# Build the test application

GTESTPROD_HOST += runTests

USR_INCLUDES += -I$(TOP)/s7nodaveApp/src

# Specify all source files to be compiled and added to the executable
runTests_SRCS += OptionalTest.cpp
runTests_SRCS += PlcAddressTest.cpp
runTests_SRCS += S7nodaveRecordAddressTest.cpp
runTests_SRCS += utilitiesTest.cpp

# Add all the support libraries needed by this executable
runTests_LIBS += s7nodave asyn snap7 
runTests_LIBS += $(EPICS_BASE_IOC_LIBS)

GTESTS += runTests

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
-include $(GTEST)/cfg/compat.RULES_BUILD
