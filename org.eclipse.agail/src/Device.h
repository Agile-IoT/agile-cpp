#ifndef __AGAIL_DEVICE_H__
#define __AGAIL_DEVICE_H__

#include <string>
#include <list>

#include "object/RecordObject.h"
#include "object/DeviceComponent.h"
#include "object/DeviceDefinition.h"

#define STR(X) STR2(X)
#define STR2(X) #X
#define AGAIL_DEVICE_METHOD_ID Id
#define AGAIL_DEVICE_METHOD_NAME Name
#define AGAIL_DEVICE_METHOD_STATUS Status
#define AGAIL_DEVICE_METHOD_LASTSEEN LastSeen
#define AGAIL_DEVICE_METHOD_CONFIGURATION Configuration
#define AGAIL_DEVICE_METHOD_PROFILE Profile
#define AGAIL_DEVICE_METHOD_DEFINITION Definition
#define AGAIL_DEVICE_METHOD_LASTUPDATE LastUpdate
#define AGAIL_DEVICE_METHOD_LASTUPDATEALL LastUpdateAll
#define AGAIL_DEVICE_METHOD_DATA Data
#define AGAIL_DEVICE_METHOD_PROTOCOL Protocol
#define AGAIL_DEVICE_METHOD_CONNECT Connect
#define AGAIL_DEVICE_METHOD_DISCONNECT Disconnect
#define AGAIL_DEVICE_METHOD_EXECUTE Execute
#define AGAIL_DEVICE_METHOD_READALL ReadAll
#define AGAIL_DEVICE_METHOD_READ Read
#define AGAIL_DEVICE_METHOD_WRITE Write
#define AGAIL_DEVICE_METHOD_SUBSCRIBE Subscribe
#define AGAIL_DEVICE_METHOD_UNSUBSCRIBE Unsubscribe
#define AGAIL_DEVICE_METHOD_COMMANDS Commands
#define AGAIL_DEVICE_METHOD_STOP Stop
#define AGAIL_DEVICE_SIGNAL_NEWSUBSCRIBEVALUE NewSubscribeValueSignal

namespace AGAIL {
    class Device;
}

class AGAIL::Device {
    public:
	virtual std::string AGAIL_DEVICE_METHOD_ID () = 0; // returns the device ID
	virtual std::string AGAIL_DEVICE_METHOD_NAME () = 0; // returns the device name
	virtual std::string AGAIL_DEVICE_METHOD_STATUS () = 0; // returns the status of the device
	virtual long AGAIL_DEVICE_METHOD_LASTSEEN () = 0; // returns the last time a read operation was done
	virtual std::string AGAIL_DEVICE_METHOD_CONFIGURATION () = 0; // returns the configuration of the device
	virtual std::list<AGAIL::DeviceComponent> AGAIL_DEVICE_METHOD_PROFILE () = 0; // returns the profile of the device
	virtual AGAIL::DeviceDefinition AGAIL_DEVICE_METHOD_DEFINITION () = 0; // returns the definition of the device
	virtual AGAIL::RecordObject AGAIL_DEVICE_METHOD_LASTUPDATE (std::string componentID) = 0; // returns the last data update received from the component
	virtual std::list<AGAIL::RecordObject> AGAIL_DEVICE_METHOD_LASTUPDATEALL () = 0; // returns the last data updates received from all components
	virtual AGAIL::RecordObject AGAIL_DEVICE_METHOD_DATA () = 0; // returns the last data update
	virtual std::string AGAIL_DEVICE_METHOD_PROTOCOL () = 0; // returns the protocol the device supports
	virtual void AGAIL_DEVICE_METHOD_CONNECT () = 0; // sets up and initializes a connection to the device
	virtual void AGAIL_DEVICE_METHOD_DISCONNECT () = 0; // disconnects from the device
	virtual void AGAIL_DEVICE_METHOD_EXECUTE (std::string commandID) = 0; // executes a command on the device
	virtual std::list<AGAIL::RecordObject> AGAIL_DEVICE_METHOD_READALL () = 0; // reads data from all components
	virtual AGAIL::RecordObject AGAIL_DEVICE_METHOD_READ (std::string componentID) = 0; // reads data from the component
	virtual void AGAIL_DEVICE_METHOD_WRITE (std::string componentID, std::string payload) = 0; // writes data to the component
	virtual void AGAIL_DEVICE_METHOD_SUBSCRIBE (std::string componentID) = 0; // enables subscription
	virtual void AGAIL_DEVICE_METHOD_UNSUBSCRIBE (std::string componentID) = 0; // disables subscription
	virtual std::list<std::string> AGAIL_DEVICE_METHOD_COMMANDS () = 0; // returns the list of all commands
	virtual void AGAIL_DEVICE_METHOD_STOP () = 0; // stops the device

    protected:
        // DBUS parameters for AGILE Device classes
        static const std::string BASE_ID; // default bus name
        static const std::string BASE_BUS_PATH; //default bus path
        static const std::string BASE_BUS_IFACE; // default bus interface name
	static const std::string NEW_RECORD_SUBSCRIBE_SIGNAL_PATH; // default new subscribe value signal path
};

#endif
