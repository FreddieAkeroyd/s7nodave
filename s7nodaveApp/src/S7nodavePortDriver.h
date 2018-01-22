#ifndef S7nodavePortDriver_h
#define S7nodavePortDriver_h

#include <string>

#include <asynDriver.h>
#include <epicsMutex.h>

#include <nodave.h>
#include "s7nodaveAsyn.h"

/**
 * Provides access to a PLC through an asyn port.
 */
class S7nodavePortDriver
{
public:
    /**
     * Constructor. Called by s7nodaveConfigureIsoTcpPort to create a driver for
     * a port.
     */
    S7nodavePortDriver(const char *portName, const char *plcHostname, const int plcPort, const int plcRack, const int plcSlot, const unsigned int priority);

    /**
     * Destructor.
     */
    ~S7nodavePortDriver();

    /**
     * Must be called after construction in order to register the port with the
     * asyn manager. This is separated from construction because it is not
     * guaranteed to succeed.
     */
    asynStatus registerPortDriver();

private:
    /**
     * TRUE if registerPortDriver has been called.
     */
    bool registered;

    /**
     * Name of the port
     */
    std::string portName;

    /**
     * Host name or IP address of PLC (without port number).
     */
    std::string plcHostname;

    /**
     * TCP port number.
     */
    int plcPort;

    /**
     * PLC rack number (needed for connection).
     */
    int plcRack;

    /**
     * PLC slot number (needed for connection).
     */
    int plcSlot;

    /**
     * Priority of the port thread.
     */
    unsigned int priority;

    /**
     * File descriptor of the socket connected to the PLC.
     */
    int socketFd;

    /**
     * Mutex for synchronizing connect/disconnect requests.
     */
    epicsMutex connectMutex;

    /**
     * Interface object that encapsulates the TCP socket connection for
     * libnodave.
     */
    daveInterface *myDaveInterface;

    /**
     * Interface object that encapsulates the connection to a specific PLC
     * for libnodave.
     */
    daveConnection *myDaveConnection;


    /**
     * asynCommon structure registered with the asyn manager.
     */
    asynCommon myAsynCommon;

    /**
     * Interface for the asynCommon structure registered with the asyn manager.
     */
    asynInterface myAsynCommonInterface;

    /**
     * asynS7nodave (driver-specific) structure registered with the asyn
     * manager.
     */
    asynS7nodave myAsynS7nodave;

    /**
     * Interface for the asynS7nodave (driver-specific) structure registered
     * with the asyn manager.
     */
    asynInterface myAsynS7nodaveInterface;


    static void asynReportStatic(void *drvPvt, FILE *fp, int details);

    /**
     * Called by asyn to request a report about the driver status.
     */
    void asynReport(FILE *fp, int details);

    static asynStatus asynConnectStatic(void *drvPvt, asynUser *pasynUser);

    /**
     * Called by asyn to connect the port.
     */
    asynStatus asynConnect(asynUser *pasynUser);

    static asynStatus asynDisconnectStatic(void *drvPvt, asynUser *pasynUser);

    /**
     * Called by asyn to disconnect the port.
     */
    asynStatus asynDisconnect(asynUser *pasynUser);

    /**
     * Internally used for disconnecting without sending out disconnect events.
     */
    asynStatus asynDisconnectCleanup(asynUser *pasynUser);

    static asynStatus asynPlcReadStatic(void *drvPvt, asynUser *pasynUser, s7nodavePlcArea area, int areaNumber, int startByte, int readLength, void *buffer, int *bytesRead);

    /**
     * Used by device support code to read from the PLC.
     */
    asynStatus asynPlcRead(asynUser *paysnUser, s7nodavePlcArea area, int areaNumber, int startByte, int readLength, void *buffer, int *bytesRead);

    static asynStatus asynPlcReadMultipleItemsStatic(void *drvPvt, asynUser *pasynUser, asynS7nodaveReadItem *firstItem);

    /**
     * Used by device support code to read multiple items from the PLC.
     */
    asynStatus asynPlcReadMultipleItems(asynUser *pasynUser, asynS7nodaveReadItem *firstItem);

    static asynStatus asynPlcWriteStatic(void *drvPvt, asynUser *pasynUser, s7nodavePlcArea area, int areaNumber, int startByte, int writeLength, void *buffer, int *bytesWritten);

    /**
     * Used by device support code to write to the PLC.
     */
    asynStatus asynPlcWrite(asynUser *pasynUser, s7nodavePlcArea area, int areaNumber, int startByte, int writeLength, void *buffer, int *bytesWritten);

    static asynStatus asynPlcWriteBitStatic(void *drvPvt, asynUser *pasynUser, s7nodavePlcArea area, int areaNumber, int startByte, int startBit, unsigned char bitValue);

    /**
     * Used by device support code to write a single bit to the PLC.
     */
    asynStatus asynPlcWriteBit(asynUser *pasynUser, s7nodavePlcArea area, int areaNumber, int startByte, int startBit, unsigned char bitValue);
};

#endif
