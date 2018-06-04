#ifndef __AGAIL_DEVICEIMP_H__
#define __AGAIL_DEVICEIMP_H__

#include "Device.h"
#include "object/AbstractAgileObject.h"

#include <map>

namespace AGAIL {
    class DeviceImp;
}

class AGAIL::DeviceImp : public AGAIL::Device, public AGAIL::AbstractAgileObject {
    public:
	DeviceImp (std::string deviceName, std::string protocol, std::string address, std::string path, std::list<AGAIL::DeviceComponent> profile); // constructor
	DeviceImp (AGAIL::DeviceDefinition definition); // constructor

	// These functions are called from their corresponding callbacks. Since
	// the callbacks are static, these functions are called from outside the
	// object. Therefore, they need to be public

	// Functions called from g_dbus_connection_register_object() function
	void handleMethodCall(GDBusConnection *, const gchar *, const gchar *, const gchar *, const gchar *, GVariant *, GDBusMethodInvocation *) final;
        GVariant* handleGetProperty(GDBusConnection *, const gchar *, const gchar *, const gchar *, const gchar *, GError **) final;
        gboolean handleSetProperty(GDBusConnection *, const gchar *, const gchar *, const gchar *, const gchar *, GVariant *, GError **) final;

    protected:
        static std::string generateDbusIntrospection(std::string extraIntrospection);

	// AGILE::Device interface methods
        virtual std::string AGAIL_DEVICE_METHOD_ID () override; // returns the device ID
	virtual std::string AGAIL_DEVICE_METHOD_NAME () override; // returns the device name
	virtual std::string AGAIL_DEVICE_METHOD_STATUS () override; // returns the status of the device
	virtual long AGAIL_DEVICE_METHOD_LASTSEEN () override; // returns the last time a read operation was done
	virtual std::string AGAIL_DEVICE_METHOD_CONFIGURATION () override; // returns the configuration of the device
	virtual std::list<AGAIL::DeviceComponent> AGAIL_DEVICE_METHOD_PROFILE () override; //  returns the profile of the device
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

	std::string getMeasurementUnit (std::string componentID);

	std::string deviceAgileID; // AGILE specific device ID
	std::string deviceName; // device name
	std::string deviceID; // device ID
	std::string protocol; // the protocol the device supports
	std::string address; // protocol specific address
	std::string path; // device D-Bus object path
	std::list<AGAIL::DeviceComponent> profile; // 
	RecordObject* data = NULL; // data
	std::map<std::string,AGAIL::RecordObject> lastReadStore; // map to store the last reads of each component of the device

	// Virtual functions called from g_dbus_connection_register_object() function
        virtual void handleMethodCallCb(GDBusConnection *, const gchar *, const gchar *, const gchar *, const gchar *, GVariant *, GVariant *);
        virtual GVariant* handleGetPropertyCb(GDBusConnection *, const gchar *, const gchar *, const gchar *, const gchar *, GError **);
        virtual gboolean handleSetPropertyCb(GDBusConnection *, const gchar *, const gchar *, const gchar *, const gchar *, GVariant *, GError **);

	virtual void onUnknownMethod(std::string);
};

#endif
