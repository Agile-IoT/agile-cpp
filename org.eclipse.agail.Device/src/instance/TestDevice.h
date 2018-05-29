#ifndef __AGAIL_TESTDEVICE_H__
#define __AGAIL_TESTDEVICE_H__

#include "../base/DeviceImp.h"

#define AGAIL_TESTDEVICE_METHOD_PROPRIETARY ProprietaryMethod

namespace AGAIL {
    class TestDevice;
}

class AGAIL::TestDevice : public AGAIL::DeviceImp {
    public:
	TestDevice (std::string deviceName, std::string address); // constructor
	TestDevice (AGAIL::DeviceDefinition definition); // constructor

    protected:
	static const std::string TEST_DEVICE;
	static const std::string TEST_PROTOCOL;
	static std::string extraIntrospection;

	// AGILE::Device interface methods
	virtual std::list<AGAIL::RecordObject> AGAIL_DEVICE_METHOD_READALL () override; // reads data from all components
        virtual AGAIL::RecordObject AGAIL_DEVICE_METHOD_READ (std::string componentID) override; // reads data from the component
	virtual void AGAIL_DEVICE_METHOD_WRITE (std::string componentID, std::string payload) override; // writes data to the component

	virtual void AGAIL_TESTDEVICE_METHOD_PROPRIETARY (); // example proprietary method

	// Virtual functions called from g_bus_own_name() function
	virtual void onBusAcquiredCb(GDBusConnection *, const gchar *) override;
        virtual void onNameAcquiredCb(GDBusConnection *, const gchar *) override;
        virtual void onNameLostCb(GDBusConnection *, const gchar *) override;

	// Virtual functions called from g_dbus_connection_register_object() function
        virtual void handleMethodCallCb(GDBusConnection *, const gchar *, const gchar *, const gchar *, const gchar *, GVariant *, GVariant *) override;

};

#endif
