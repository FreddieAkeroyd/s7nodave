#include <map>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "S7nodaveRecordAddress.h"

using boost::tuple;
using boost::optional;
using std::map;
using std::string;
using std::vector;

std::string S7nodaveRecordAddress::getPortName() const
{
    return this->portName;
}

S7nodavePlcAddress S7nodaveRecordAddress::getPlcAddress() const
{
    return this->plcAddress;
}
s7nodavePlcDataType S7nodaveRecordAddress::getPlcDataType() const
{
    return this->plcDataType;
}

S7nodaveRecordAddress::DeviceParameters S7nodaveRecordAddress::getDeviceParameters() const
{
    return this->recordParameters;
}

static const std::string whitespace = " \t\n\r\x0b";

static optional<S7nodaveRecordAddress::DeviceParameters> parseDeviceParameters(string parameterString) {
    S7nodaveRecordAddress::DeviceParameters parameterMap;
    vector<string> parameters;
    // Device parameters are separated by commas
    boost::split(parameters, parameterString, boost::is_any_of(","));
    for (vector<string>::iterator i = parameters.begin(); i < parameters.end(); i++) {
        string parameterName;
        optional<string> parameterValue;
        size_t equalPos = i->find_first_of('=');
        if (equalPos == string::npos) {
            // Parameter without value
            parameterName = *i;
            boost::trim(parameterName);
        } else {
            parameterName = i->substr(0, equalPos);
            boost::trim(parameterName);
            parameterValue = i->substr(equalPos + 1);
            boost::trim(*parameterValue);
        }
        // Parameter names are case insensitive, so we normalize them to
        // capital characters.
        boost::to_upper(parameterName);
        if (parameterMap.find(parameterName) != parameterMap.end()) {
            // The same parameter appeared twice
            return optional<S7nodaveRecordAddress::DeviceParameters>();
        }
        // Add parameter to map
        parameterMap.insert(S7nodaveRecordAddress::DeviceParameters::value_type(parameterName, parameterValue));
    }
    return parameterMap;
}

static optional<s7nodavePlcDataType> stringToPlcDataType(string typeString) {
    boost::to_lower(typeString);
    if (typeString == "bool") {
        return plcDataTypeBool;
    } else if (typeString == "int8") {
        return plcDataTypeInt8;
    } else if (typeString == "uint8") {
        return plcDataTypeUint8;
    } else if (typeString == "int16") {
        return plcDataTypeInt16;
    } else if (typeString == "uint16") {
        return plcDataTypeUint16;
    } else if (typeString == "int32") {
        return plcDataTypeInt32;
    } else if (typeString == "uint32") {
        return plcDataTypeUint32;
    } else if (typeString == "float") {
        return plcDataTypeFloat;
    } else {
        return optional<s7nodavePlcDataType>();
    }
}

tuple< string, optional<S7nodaveRecordAddress::DeviceParameters>, optional<S7nodavePlcAddress>, optional<s7nodavePlcDataType>, bool > S7nodaveRecordAddress::parseRecordAddress(std::string addressString)
{
    optional<S7nodavePlcAddress> emptyPlcAddress;
    optional<s7nodavePlcDataType> emptyPlcDataType;
    optional<DeviceParameters> emptyParameterMap;

    boost::trim(addressString);
    size_t pos;
    // Port name ends with opening parenthesis or whitespace
    pos = addressString.find_first_of(whitespace + "(");
    if (pos == std::string::npos) {
        return boost::make_tuple(addressString, emptyParameterMap, emptyPlcAddress, emptyPlcDataType, false);
    }
    std::string portName = addressString.substr(0, pos);
    if (portName.size() == 0) {
        return boost::make_tuple(portName, emptyParameterMap, emptyPlcAddress, emptyPlcDataType, false);
    }
    addressString = addressString.substr(pos);
    boost::trim_left(addressString);
    optional<DeviceParameters> deviceParameterMap;
    // Handle list of optional parameters
    if (addressString.size() >= 1 && addressString[0] == '(') {
        // Find closing parenthesis.
        pos = addressString.find_first_of(')');
        if (pos == string::npos) {
            // Unmatched parenthesis.
            return boost::make_tuple(portName, emptyParameterMap, emptyPlcAddress, emptyPlcDataType, false);
        }
        deviceParameterMap = parseDeviceParameters(addressString.substr(1, pos - 1));
        if (!deviceParameterMap) {
            // Invalid parameter string
            return boost::make_tuple(portName, emptyParameterMap, emptyPlcAddress, emptyPlcDataType, false);
        }
        addressString = addressString.substr(pos + 1);
        boost::trim_left(addressString);
    } else {
        // No device parameters
        deviceParameterMap = DeviceParameters();
    }
    // PLC address ends with whitespace (or end of string)
    pos = addressString.find_first_of(whitespace);
    string plcAddressString;
    if (pos == string::npos) {
        plcAddressString = addressString;
        addressString = "";
    } else {
        plcAddressString = addressString.substr(0, pos);
        addressString = addressString.substr(pos);
        boost::trim_left(addressString);
    }
    optional<S7nodavePlcAddress> plcAddress = S7nodavePlcAddress::create(plcAddressString);
    if (!plcAddress) {
        // Invalid PLC address
        return boost::make_tuple(portName, deviceParameterMap, emptyPlcAddress, emptyPlcDataType, false);
    }
    // The rest of the string is the PLC data type
    optional<s7nodavePlcDataType> plcDataType;
    if (addressString.size() > 0) {
        plcDataType = stringToPlcDataType(addressString);
        if (plcDataType) {
            return boost::make_tuple(portName, deviceParameterMap, plcAddress, plcDataType, true);
        } else {
            return boost::make_tuple(portName, deviceParameterMap, plcAddress, emptyPlcDataType, false);
        }
    } else {
        return boost::make_tuple(portName, deviceParameterMap, plcAddress, emptyPlcDataType, true);
    }
}

optional<S7nodaveRecordAddress> S7nodaveRecordAddress::create(string portName, S7nodavePlcAddress plcAddress, s7nodavePlcDataType plcDataType, DeviceParameters recordParameters)
{
    optional<S7nodaveRecordAddress> emptyResult;
    // Port name must not be empty
    if (portName.size() == 0) {
        return emptyResult;
    }
    // If a PLC data type is specified, its size has to match the data size of
    // the PLC address.
    int dataSize = S7nodavePlcAddress::dataSizeInBits(plcAddress.getDataSize());
    switch (plcDataType) {
    case plcDataTypeBool:
        if (dataSize != 1) {
            return emptyResult;
        }
        break;
    case plcDataTypeInt8:
    case plcDataTypeUint8:
        if (dataSize != 8) {
            return emptyResult;
        }
        break;
    case plcDataTypeInt16:
    case plcDataTypeUint16:
        if (dataSize != 16) {
            return emptyResult;
        }
        break;
    case plcDataTypeInt32:
    case plcDataTypeUint32:
    case plcDataTypeFloat:
        if (dataSize != 32) {
            return emptyResult;
        }
        break;
    }
    return S7nodaveRecordAddress(portName, plcAddress, plcDataType, recordParameters);
}

S7nodaveRecordAddress::S7nodaveRecordAddress(std::string portName, S7nodavePlcAddress plcAddress, s7nodavePlcDataType plcDataType, DeviceParameters recordParameters) :
portName(portName), plcAddress(plcAddress), plcDataType(plcDataType), recordParameters(recordParameters)
{
}
