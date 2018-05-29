#include "TestDevice.h"

#include <chrono>

#include <iostream> // TODO: substitute by log library

const std::string AGAIL::TestDevice::TEST_DEVICE = "test";
const std::string AGAIL::TestDevice::TEST_PROTOCOL = "org.eclipse.agail.protocol.test";
std::string AGAIL::TestDevice::extraIntrospection = std::string() + "<method name='" + STR(AGAIL_TESTDEVICE_METHOD_PROPRIETARY) + "'></method>";

AGAIL::TestDevice::TestDevice (std::string deviceName, std::string address) : DeviceImp (deviceName, TEST_PROTOCOL, address, BASE_BUS_PATH + "/" + TEST_DEVICE + address, std::list<AGAIL::DeviceComponent> ({AGAIL::DeviceComponent ("testComponent", "units"), AGAIL::DeviceComponent ("otherComponent", "otherUnits")})) {
    dbusConnect (deviceAgileID, path, generateDbusIntrospection (extraIntrospection));
}

AGAIL::TestDevice::TestDevice (AGAIL::DeviceDefinition definition) : DeviceImp (definition) {
    dbusConnect (deviceAgileID, path, generateDbusIntrospection (extraIntrospection));
}

/////////////////////////////////////
// AGILE::Device interface methods //
/////////////////////////////////////

std::list<AGAIL::RecordObject> AGAIL::TestDevice::AGAIL_DEVICE_METHOD_READALL () {
    std::list<AGAIL::RecordObject> readAll;
    for (std::map<std::string,AGAIL::RecordObject>::iterator it = lastReadStore.begin(); it != lastReadStore.end(); it++)
        readAll.push_back (it->second);
    if (!readAll.empty()) {
	if (data != NULL)
	    delete data;
	data = new RecordObject ((lastReadStore.rbegin())->second);
    }
    return readAll;
}

AGAIL::RecordObject AGAIL::TestDevice::AGAIL_DEVICE_METHOD_READ (std::string componentID) {
    std::map<std::string,AGAIL::RecordObject>::iterator it = lastReadStore.find(componentID);
    if (it == lastReadStore.end())
        return RecordObject ("", "", "", "", "", 0);
    if (data != NULL)
	delete data;
    data = new RecordObject (it->second);
    return it->second;
}

void AGAIL::TestDevice::AGAIL_DEVICE_METHOD_WRITE (std::string componentID, std::string payload) {
    lastReadStore.erase (componentID);
    lastReadStore.insert (std::map<std::string,AGAIL::RecordObject>::value_type(componentID,RecordObject(deviceID, componentID, payload, getMeasurementUnit (componentID), "", std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count())));
}

void AGAIL::TestDevice::AGAIL_TESTDEVICE_METHOD_PROPRIETARY () {
    std::cout << "Proprietary method has been executed" << std::endl;
}

//////////////////////////////////////////////////////
// Callback functions for g_bus_own_name() function //
//////////////////////////////////////////////////////

void AGAIL::TestDevice::onBusAcquiredCb(GDBusConnection *conn, const gchar *name) {
    std::cout << "D-Bus bus acquired by test device" << std::endl;
}

void AGAIL::TestDevice::onNameAcquiredCb(GDBusConnection *conn, const gchar *name) {
    std::cout << "D-bus name acquired by test device" << std::endl;
}

void AGAIL::TestDevice::onNameLostCb(GDBusConnection *conn, const gchar *name) {
    std::cout << "D-bus name lost by test device" << std::endl;
}

//////////////////////////////////////////////////////////////////////////////////
// GDBusInterfaceVTable struct for g_dbus_connection_register_object() function //
//////////////////////////////////////////////////////////////////////////////////

void AGAIL::TestDevice::handleMethodCallCb (GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GVariant * out) {
    // Method ProprietaryMethod
    if (g_strcmp0 (method_name, STR(AGAIL_TESTDEVICE_METHOD_PROPRIETARY)) == 0) {
        AGAIL_TESTDEVICE_METHOD_PROPRIETARY();
    }
    // Unknown method
    else {
	onUnknownMethod (method_name);
    }
}
