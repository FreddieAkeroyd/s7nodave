#!../../bin/win32-x86/s7nodaveTest

## You may have to change s7nodaveTest to something else
## everywhere it appears in this file

< envPaths

## Register all support components
dbLoadDatabase("../../dbd/s7nodaveTest.dbd",0,0)
s7nodaveTest_registerRecordDeviceDriver(pdbbase) 

s7nodaveConfigureIsoTcpPort("NBIPLC", "172.24.10.171", 0, 2, 0)

## Load record instances
dbLoadRecords("../../db/s7nodaveTest.db")

iocInit()

## Start any sequence programs
#seq sncs7nodaveTest,"user=faa59"
