#ifndef __AGAIL_DEVICECALLER_H__
#define __AGAIL_DEVICECALLER_H__

#include <gio/gio.h>

#include "Device.h"

namespace AGAIL {
    class DeviceCaller;
}

class AGAIL::DeviceCaller : public Device {
    public:
	DeviceCaller(std::string address, std::string devicePath);

	bool isConnected();

	// AGILE::Device interface methods
        virtual std::string AGAIL_DEVICE_METHOD_ID () override; // returns the device ID
        virtual std::string AGAIL_DEVICE_METHOD_NAME () override; // returns the device name
        virtual std::string AGAIL_DEVICE_METHOD_STATUS () override; // returns the status of the device
	virtual long AGAIL_DEVICE_METHOD_LASTSEEN () override; // returns the last time a read operation was done
	virtual std::string AGAIL_DEVICE_METHOD_CONFIGURATION () override; // returns the configuration of the device
	virtual std::list<AGAIL::DeviceComponent> AGAIL_DEVICE_METHOD_PROFILE () override; // returns the profile of the device
	virtual AGAIL::DeviceDefinition AGAIL_DEVICE_METHOD_DEFINITION () override; // returns the definition of the device
	virtual AGAIL::RecordObject AGAIL_DEVICE_METHOD_LASTUPDATE (std::string componentID) override; // returns the last data update received from the component
	virtual std::list<AGAIL::RecordObject> AGAIL_DEVICE_METHOD_LASTUPDATEALL () override; // returns the last data updates received from all components
	virtual AGAIL::RecordObject AGAIL_DEVICE_METHOD_DATA () override; // returns the last data update
	virtual std::string AGAIL_DEVICE_METHOD_PROTOCOL () override; // returns the protocol the device supports
        virtual void AGAIL_DEVICE_METHOD_CONNECT () override; // sets up and initializes a connection to the device
        virtual void AGAIL_DEVICE_METHOD_DISCONNECT () override; // disconnects from the device
	virtual void AGAIL_DEVICE_METHOD_EXECUTE (std::string commandID) override; // executes a command on the device
	virtual std::list<AGAIL::RecordObject> AGAIL_DEVICE_METHOD_READALL () override; // reads data from all components
        virtual AGAIL::RecordObject AGAIL_DEVICE_METHOD_READ (std::string componentID) override; // reads data from the component
	virtual void AGAIL_DEVICE_METHOD_WRITE (std::string componentID, std::string payload) override; // writes data to the component
	virtual void AGAIL_DEVICE_METHOD_SUBSCRIBE (std::string componentID) override; // enables subscription
        virtual void AGAIL_DEVICE_METHOD_UNSUBSCRIBE (std::string componentID) override; // disables subscription
	virtual std::list<std::string> AGAIL_DEVICE_METHOD_COMMANDS () override; // returns the list of all commands
	virtual void AGAIL_DEVICE_METHOD_STOP () override; // stops the device

    protected:
	std::string devicePath;

	GVariant* genericMethod(std::string methodName, GVariant* input, const GVariantType* outputType);

    private:
	GDBusConnection *connection;
	bool connected = false;

	// DBUS parameters
        static const GBusType DEFAULT_DBUS_CONNECTION = G_BUS_TYPE_SESSION; //default bus type (session bus)
};

#endif
