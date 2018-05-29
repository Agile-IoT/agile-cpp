#include <iostream>

#include "TestDeviceCaller.h"

int main() {
    AGAIL::TestDeviceCaller myClient("0000");
    if (!myClient.isConnected())
	return 1;

    // Call to Id method
    std::cout << "Id: " << myClient.AGAIL_DEVICE_METHOD_ID() << std::endl;

    // Call to Name method
    std::cout << "Name: " << myClient.AGAIL_DEVICE_METHOD_NAME() << std::endl;

    // Call to Status method
    std::cout << "Status: " << myClient.AGAIL_DEVICE_METHOD_STATUS() << std::endl;

    // Call to LastSeen method
    std::cout << "Last seen: " << myClient.AGAIL_DEVICE_METHOD_LASTSEEN() << std::endl;

    // Call to Configuration method
    std::cout << "Configuration: " << myClient.AGAIL_DEVICE_METHOD_CONFIGURATION() << std::endl;

    // Call to Profile method
    std::list<AGAIL::DeviceComponent> profile = myClient.AGAIL_DEVICE_METHOD_PROFILE();
    std::cout << "Profile:" << std::endl;
    for (std::list<AGAIL::DeviceComponent>::iterator it = profile.begin(); it != profile.end(); it++)
	std::cout << "\tcomponentID: " << it->componentID << "\tunit: " << it->unit << std::endl;

    // Call to Definition method
    AGAIL::DeviceDefinition definition = myClient.AGAIL_DEVICE_METHOD_DEFINITION();
    std::cout << "Definition:" << std::endl;
    std::cout << "\tdeviceID: " << definition.deviceID << std::endl;
    std::cout << "\taddress: " << definition.address << std::endl;
    std::cout << "\tname: " << definition.name << std::endl;
    std::cout << "\tdescription: " << definition.description << std::endl;
    std::cout << "\tprotocol: " << definition.protocol << std::endl;
    std::cout << "\tpath: " << definition.path << std::endl;
    std::cout << "\tprofile:" << std::endl;
    for (std::list<AGAIL::DeviceComponent>::const_iterator it = definition.profile.begin(); it != definition.profile.end(); it++)
        std::cout << "\t\tcomponentID: " << it->componentID << "\tunit: " << it->unit << std::endl;

    // Call to LastUpdate method
    std::string componentID = "testComponent";
    AGAIL::RecordObject lastUpdate = myClient.AGAIL_DEVICE_METHOD_LASTUPDATE (componentID);
    std::cout << "Last Update \"" << componentID << "\":" << std::endl;
    std::cout << "\tdeviceID: " << lastUpdate.deviceID << "\tcomponentID: " << lastUpdate.componentID << "\tvalue: " << lastUpdate.value << "\tunit: " << lastUpdate.unit << "\tformat: " << lastUpdate.format << "\tlastUpdate: " << lastUpdate.lastUpdate << std::endl;

    // Call to LastUpdateAll method
    std::list<AGAIL::RecordObject> lastUpdateAll = myClient.AGAIL_DEVICE_METHOD_LASTUPDATEALL();
    std::cout << "Last Update All:" << std::endl;
    for (std::list<AGAIL::RecordObject>::iterator it = lastUpdateAll.begin(); it != lastUpdateAll.end(); it++)
        std::cout << "\tdeviceID: " << it->deviceID << "\tcomponentID: " << it->componentID << "\tvalue: " << it->value << "\tunit: " << it->unit << "\tformat: " << it->format << "\tlastUpdate: " << it->lastUpdate << std::endl;

    // Call to Data method
    AGAIL::RecordObject data = myClient.AGAIL_DEVICE_METHOD_DATA ();
    std::cout << "Data:" << std::endl;
    std::cout << "\tdeviceID: " << data.deviceID << "\tcomponentID: " << data.componentID << "\tvalue: " << data.value << "\tunit: " << data.unit << "\tformat: " << data.format << "\tlastUpdate: " << data.lastUpdate << std::endl;

    // Call to Protocol method
    std::cout << "Protocol: " << myClient.AGAIL_DEVICE_METHOD_PROTOCOL() << std::endl;

    // Call to Connect method
    myClient.AGAIL_DEVICE_METHOD_CONNECT();
    std::cout << "Connect" << std::endl;

    // Call to Disconnect method
    myClient.AGAIL_DEVICE_METHOD_DISCONNECT();
    std::cout << "Disconnect" << std::endl;

    // Call to Execute method
    std::string commandID = "testCommand";
    myClient.AGAIL_DEVICE_METHOD_EXECUTE (commandID);
    std::cout << "Execute \"" << commandID << "\"" << std::endl;

    // Call to ReadAll method
    std::list<AGAIL::RecordObject> readAll = myClient.AGAIL_DEVICE_METHOD_READALL();
    std::cout << "Read All:" << std::endl;
    for (std::list<AGAIL::RecordObject>::iterator it = readAll.begin(); it != readAll.end(); it++)
        std::cout << "\tdeviceID: " << it->deviceID << "\tcomponentID: " << it->componentID << "\tvalue: " << it->value << "\tunit: " << it->unit << "\tformat: " << it->format << "\tlastUpdate: " << it->lastUpdate << std::endl;

    // Call to Read method
    componentID = "testComponent";
    AGAIL::RecordObject read = myClient.AGAIL_DEVICE_METHOD_READ (componentID);
    std::cout << "Read \"" << componentID << "\":" << std::endl;
    std::cout << "\tdeviceID: " << read.deviceID << "\tcomponentID: " << read.componentID << "\tvalue: " << read.value << "\tunit: " << read.unit << "\tformat: " << read.format << "\tlastUpdate: " << read.lastUpdate << std::endl;

    // Call to Write method
    componentID = "testComponent";
    std::string payload = "testPayload";
    myClient.AGAIL_DEVICE_METHOD_WRITE (componentID, payload);
    std::cout << "Write \"" << componentID << "\" \"" << payload << "\"" << std::endl;

    // Call to Subscribe method
    componentID = "testComponent";
    myClient.AGAIL_DEVICE_METHOD_SUBSCRIBE (componentID);
    std::cout << "Subscribe \"" << componentID << "\"" << std::endl;

    // Call to Unsubscribe method
    componentID = "testComponent";
    myClient.AGAIL_DEVICE_METHOD_UNSUBSCRIBE (componentID);
    std::cout << "Unsubscribe \"" << componentID << "\"" << std::endl;

    // Call to Commands method
    std::list<std::string> commands = myClient.AGAIL_DEVICE_METHOD_COMMANDS();
    std::cout << "Commands:";
    for (std::list<std::string>::iterator it = commands.begin(); it != commands.end(); it++)
        std::cout << "\t" << *it;
    std::cout << std::endl;

    // Call to Stop method
    myClient.AGAIL_DEVICE_METHOD_STOP();
    std::cout << "Stop" << std::endl;

    // Call to ProprietaryMethod method
    myClient.AGAIL_TESTDEVICE_METHOD_PROPRIETARY();
    std::cout << "Proprietary Method" << std::endl;

    return 0;
}
