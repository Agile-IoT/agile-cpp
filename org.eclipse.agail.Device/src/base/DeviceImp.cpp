#include "DeviceImp.h"

#include <iterator>

#include <iostream> // TODO: substitute by log library

std::string AGAIL::DeviceImp::generateDbusIntrospection(std::string extraIntrospection) {
    return "<node name='" + BASE_BUS_PATH + "'>" +
    "  <interface name='" + BASE_BUS_IFACE + "'>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_ID) + "'>" +
    "    <arg name='return' type='s' direction='out'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_NAME) + "'>" +
    "    <arg name='return' type='s' direction='out'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_STATUS) + "'>" +
    "    <arg name='return' type='s' direction='out'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_LASTSEEN) + "'>" +
    "    <arg name='return' type='i' direction='out'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_CONFIGURATION) + "'>" +
    "    <arg name='return' type='s' direction='out'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_PROFILE) + "'>" +
    "    <arg name='return' type='a(ss)' direction='out'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_DEFINITION) + "'>" +
    "    <arg name='return' type='(ssssssa(ss))' direction='out'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_LASTUPDATE) + "'>" +
    "    <arg name='componentID' type='s' direction='in'/>" +
    "    <arg name='return' type='(sssssi)' direction='out'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_LASTUPDATEALL) + "'>" +
    "    <arg name='return' type='a(sssssi)' direction='out'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_DATA) + "'>" +
    "    <arg name='return' type='(sssssi)' direction='out'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_PROTOCOL) + "'>" +
    "    <arg name='return' type='s' direction='out'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_CONNECT) + "'>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_DISCONNECT) + "'>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_EXECUTE) + "'>" +
    "    <arg name='commandID' type='s' direction='in'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_READALL) + "'>" +
    "    <arg name='return' type='a(sssssi)' direction='out'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_READ) + "'>" +
    "    <arg name='componentID' type='s' direction='in'/>" +
    "    <arg name='return' type='(sssssi)' direction='out'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_WRITE) + "'>" +
    "    <arg name='componentID' type='s' direction='in'/>" +
    "    <arg name='payload' type='s' direction='in'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_SUBSCRIBE) + "'>" +
    "    <arg name='componentID' type='s' direction='in'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_UNSUBSCRIBE) + "'>" +
    "    <arg name='componentID' type='s' direction='in'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_COMMANDS) + "'>" +
    "    <arg name='return' type='as' direction='out'/>" +
    "  </method>" +
    "  <method name='" + STR(AGAIL_DEVICE_METHOD_STOP) + "'>" +
    "  </method>" +
    "  <signal name='" + STR(AGAIL_DEVICE_SIGNAL_NEWSUBSCRIBEVALUE) + "'>"
    "    <arg name='record' type='(sssssi)'/>"
    "  </signal>" +
    extraIntrospection +
    "  </interface>" +
    "</node>";
}

AGAIL::DeviceImp::DeviceImp (std::string deviceName, std::string protocol, std::string address, std::string path, std::list<AGAIL::DeviceComponent> profile) : deviceName(deviceName), deviceID(protocol + address), protocol(protocol), address(address), path(path), profile(profile), deviceAgileID(BASE_ID) {}

AGAIL::DeviceImp::DeviceImp (AGAIL::DeviceDefinition definition) : deviceName(definition.name), deviceID(definition.deviceID), protocol(definition.protocol), address(definition.address), path(definition.path), profile(definition.profile), deviceAgileID(BASE_ID) {}

/////////////////////////////////////
// AGILE::Device interface methods //
/////////////////////////////////////

std::string AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_ID () {
    return deviceID;
}

std::string AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_NAME () {
    return deviceName;
}

std::string AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_STATUS () {
    std::cout << STR(AGAIL_DEVICE_METHOD_STATUS) << "() method not implemented!" << std::endl;
    return std::string();
}

long AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_LASTSEEN () {
    if (data == NULL)
	return 0;
    return data->lastUpdate;
}

std::string AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_CONFIGURATION () {
    std::cout << STR(AGAIL_DEVICE_METHOD_CONFIGURATION) << "() method not implemented!" << std::endl;
    return std::string();
}

std::list<AGAIL::DeviceComponent> AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_PROFILE () {
    return profile;
}

AGAIL::DeviceDefinition AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_DEFINITION () {
    return DeviceDefinition (deviceID, address, deviceName, "", protocol, path, profile);
}

AGAIL::RecordObject AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_LASTUPDATE (std::string componentID) {
    std::map<std::string,AGAIL::RecordObject>::iterator it = lastReadStore.find(componentID);
    if (it == lastReadStore.end())
	return RecordObject ("", "", "", "", "", 0);
    return it->second;
}

std::list<AGAIL::RecordObject> AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_LASTUPDATEALL () {
    std::list<AGAIL::RecordObject> lastUpdateAll;
    for (std::map<std::string,AGAIL::RecordObject>::iterator it = lastReadStore.begin(); it != lastReadStore.end(); it++)
	lastUpdateAll.push_back (it->second);
    return lastUpdateAll;
}

AGAIL::RecordObject AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_DATA () {
    if (data == NULL)
	return RecordObject ("", "", "", "", "", 0);
    return *data;
}

std::string AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_PROTOCOL () {
    return protocol;
}

void AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_CONNECT () {
    std::cout << STR(AGAIL_DEVICE_METHOD_CONNECT) << "() method not implemented!" << std::endl;
}

void AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_DISCONNECT () {
    std::cout << STR(AGAIL_DEVICE_METHOD_DISCONNECT) << "() method not implemented!" << std::endl;
}

void AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_EXECUTE (std::string commandID) {
    std::cout << STR(AGAIL_DEVICE_METHOD_EXECUTE) << "() method not implemented!" << std::endl;
}

std::list<AGAIL::RecordObject> AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_READALL () {
    std::cout << STR(AGAIL_DEVICE_METHOD_READALL) << "() method not implemented!" << std::endl;
    return std::list<AGAIL::RecordObject>();
}

AGAIL::RecordObject AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_READ (std::string componentID) {
    std::cout << STR(AGAIL_DEVICE_METHOD_READ) << "() method not implemented!" << std::endl;
    return RecordObject ("", "", "", "", "", 0);
}

void AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_WRITE (std::string componentID, std::string payload) {
    std::cout << STR(AGAIL_DEVICE_METHOD_WRITE) << "() method not implemented!" << std::endl;
}

void AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_SUBSCRIBE (std::string componentID) {
    std::cout << STR(AGAIL_DEVICE_METHOD_SUBSCRIBE) << "() method not implemented!" << std::endl;
}

void AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_UNSUBSCRIBE (std::string componentID) {
    std::cout << STR(AGAIL_DEVICE_METHOD_UNSUBSCRIBE) << "() method not implemented!" << std::endl;
}

std::list<std::string> AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_COMMANDS () {
    std::cout << STR(AGAIL_DEVICE_METHOD_COMMANDS) << "() method not implemented!" << std::endl;
    return std::list<std::string>();
}

void AGAIL::DeviceImp::AGAIL_DEVICE_METHOD_STOP () {
    std::cout << STR(AGAIL_DEVICE_METHOD_STOP) << "() method not implemented!" << std::endl;
}

std::string AGAIL::DeviceImp::getMeasurementUnit (std::string componentID) {
    std::string unit;
    for (std::list<AGAIL::DeviceComponent>::iterator it = profile.begin(); it != profile.end(); it++)
        if (componentID == it->componentID)
                unit = it->unit;
    return unit;
}

//////////////////////////////////////////////////////////////////////////////////
// GDBusInterfaceVTable struct for g_dbus_connection_register_object() function //
//////////////////////////////////////////////////////////////////////////////////

void AGAIL::DeviceImp::handleMethodCall(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GDBusMethodInvocation *invocation) {
    GVariant * out;
    out = NULL;

    // Method Id
    if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_ID)) == 0) {
        std::string ret = AGAIL_DEVICE_METHOD_ID();
        out = g_variant_new ("(s)", ret.c_str());
    }
    // Method Name
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_NAME)) == 0) {
        std::string ret = AGAIL_DEVICE_METHOD_NAME();
        out = g_variant_new ("(s)", ret.c_str());
    }
    // Method Status
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_STATUS)) == 0) {
        std::string ret = AGAIL_DEVICE_METHOD_STATUS();
        out = g_variant_new ("(s)", ret.c_str());
    }
    // Method LastSeen
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_LASTSEEN)) == 0) {
        long ret = AGAIL_DEVICE_METHOD_LASTSEEN();
        out = g_variant_new ("(i)", ret);
    }
    // Method Configuration
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_CONFIGURATION)) == 0) {
        std::string ret = AGAIL_DEVICE_METHOD_CONFIGURATION();
        out = g_variant_new ("(s)", ret.c_str());
    }
    // Method Profile
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_PROFILE)) == 0) {
        std::list<AGAIL::DeviceComponent> ret = AGAIL_DEVICE_METHOD_PROFILE();
	GVariantBuilder* builder = g_variant_builder_new (G_VARIANT_TYPE ("a(ss)"));
	for (std::list<AGAIL::DeviceComponent>::iterator it = ret.begin(); it != ret.end(); it++)
	    g_variant_builder_add (builder, "(ss)", it->componentID.c_str(), it->unit.c_str());
        out = g_variant_new ("(a(ss))", builder);
	g_variant_builder_unref (builder);
    }
    // Method Definition
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_DEFINITION)) == 0) {
        AGAIL::DeviceDefinition ret = AGAIL_DEVICE_METHOD_DEFINITION();
        GVariantBuilder* builder = g_variant_builder_new (G_VARIANT_TYPE ("a(ss)"));
        for (std::list<AGAIL::DeviceComponent>::const_iterator it = ret.profile.begin(); it != ret.profile.end(); it++)
            g_variant_builder_add (builder, "(ss)", it->componentID.c_str(), it->unit.c_str());
        out = g_variant_new ("((ssssssa(ss)))", ret.deviceID.c_str(), ret.address.c_str(), ret.name.c_str(), ret.description.c_str(), ret.protocol.c_str(), ret.path.c_str(), builder);
        g_variant_builder_unref (builder);
    }
    // Method LastUpdate
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_LASTUPDATE)) == 0) {
        const gchar *componentID;
        g_variant_get (parameters, "(&s)", &componentID);
        AGAIL::RecordObject ret = AGAIL_DEVICE_METHOD_LASTUPDATE(componentID);
        out = g_variant_new ("((sssssi))", ret.deviceID.c_str(), ret.componentID.c_str(), ret.value.c_str(), ret.unit.c_str(), ret.format.c_str(), ret.lastUpdate);
    }
    // Method LastUpdateAll
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_LASTUPDATEALL)) == 0) {
        std::list<AGAIL::RecordObject> ret = AGAIL_DEVICE_METHOD_LASTUPDATEALL();
        GVariantBuilder* builder = g_variant_builder_new (G_VARIANT_TYPE ("a(sssssi)"));
        for (std::list<AGAIL::RecordObject>::iterator it = ret.begin(); it != ret.end(); it++)
            g_variant_builder_add (builder, "(sssssi)", it->deviceID.c_str(), it->componentID.c_str(), it->value.c_str(), it->unit.c_str(), it->format.c_str(), it->lastUpdate);
        out = g_variant_new ("(a(sssssi))", builder);
        g_variant_builder_unref (builder);
    }
    // Method Data
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_DATA)) == 0) {
        AGAIL::RecordObject ret = AGAIL_DEVICE_METHOD_DATA();
        out = g_variant_new ("((sssssi))", ret.deviceID.c_str(), ret.componentID.c_str(), ret.value.c_str(), ret.unit.c_str(), ret.format.c_str(), ret.lastUpdate);
    }
    // Method Protocol
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_PROTOCOL)) == 0) {
        std::string ret = AGAIL_DEVICE_METHOD_PROTOCOL();
        out = g_variant_new ("(s)", ret.c_str());
    }
    // Method Connect
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_CONNECT)) == 0) {
        AGAIL_DEVICE_METHOD_CONNECT();
    }
    // Method Disconnect
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_DISCONNECT)) == 0) {
        AGAIL_DEVICE_METHOD_DISCONNECT();
    }
    // Method Execute
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_EXECUTE)) == 0) {
        const gchar *commandID;
        g_variant_get (parameters, "(&s)", &commandID);
        AGAIL_DEVICE_METHOD_EXECUTE(commandID);
    }
    // Method ReadAll
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_READALL)) == 0) {
        std::list<AGAIL::RecordObject> ret = AGAIL_DEVICE_METHOD_READALL();
        GVariantBuilder* builder = g_variant_builder_new (G_VARIANT_TYPE ("a(sssssi)"));
        for (std::list<AGAIL::RecordObject>::iterator it = ret.begin(); it != ret.end(); it++)
            g_variant_builder_add (builder, "(sssssi)", it->deviceID.c_str(), it->componentID.c_str(), it->value.c_str(), it->unit.c_str(), it->format.c_str(), it->lastUpdate);
        out = g_variant_new ("(a(sssssi))", builder);
        g_variant_builder_unref (builder);
    }
    // Method Read
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_READ)) == 0) {
        const gchar *componentID;
        g_variant_get (parameters, "(&s)", &componentID);
        AGAIL::RecordObject ret = AGAIL_DEVICE_METHOD_READ(componentID);
        out = g_variant_new ("((sssssi))", ret.deviceID.c_str(), ret.componentID.c_str(), ret.value.c_str(), ret.unit.c_str(), ret.format.c_str(), ret.lastUpdate);
    }
    // Method Write
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_WRITE)) == 0) {
        const gchar *componentID, *payload;
        g_variant_get (parameters, "(&s&s)", &componentID, &payload);
        AGAIL_DEVICE_METHOD_WRITE(componentID, payload);
    }
    // Method Subscribe
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_SUBSCRIBE)) == 0) {
        const gchar *componentID;
        g_variant_get (parameters, "(&s)", &componentID);
        AGAIL_DEVICE_METHOD_SUBSCRIBE(componentID);
    }
    // Method Unsubscribe
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_UNSUBSCRIBE)) == 0) {
        const gchar *componentID;
        g_variant_get (parameters, "(&s)", &componentID);
        AGAIL_DEVICE_METHOD_UNSUBSCRIBE(componentID);
    }
    // Method Commands
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_COMMANDS)) == 0) {
        std::list<std::string> ret = AGAIL_DEVICE_METHOD_COMMANDS();
        GVariantBuilder* builder = g_variant_builder_new (G_VARIANT_TYPE ("as"));
        for (std::list<std::string>::iterator it = ret.begin(); it != ret.end(); it++)
            g_variant_builder_add (builder, "s", it->c_str());
        out = g_variant_new ("(as)", builder);
        g_variant_builder_unref (builder);
    }
    // Method Stop
    else if (g_strcmp0 (method_name, STR(AGAIL_DEVICE_METHOD_STOP)) == 0) {
        AGAIL_DEVICE_METHOD_STOP();
    }
    // Unknown method
    else {
        handleMethodCallCb (connection, sender, object_path, interface_name, method_name, parameters, out);
    }

    g_dbus_method_invocation_return_value(invocation, out);
}        

GVariant* AGAIL::DeviceImp::handleGetProperty(GDBusConnection *connection, const gchar *sender, const gchar *obj_path, const gchar *interface_name, const gchar *property_name, GError **error) {
    return handleGetPropertyCb(connection, sender, obj_path, interface_name, property_name, error);
}

gboolean AGAIL::DeviceImp::handleSetProperty(GDBusConnection *connection, const gchar *sender, const gchar *obj_path, const gchar *interface_name, const gchar *property_name, GVariant *value, GError **error) {
    return handleSetPropertyCb(connection, sender, obj_path, interface_name, property_name, value, error);
}

void AGAIL::DeviceImp::handleMethodCallCb (GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GVariant * out) {
    onUnknownMethod(method_name);
}

GVariant* AGAIL::DeviceImp::handleGetPropertyCb (GDBusConnection *connection, const gchar *sender, const gchar *obj_path, const gchar *interface_name, const gchar *property_name, GError **error) {
    return NULL;
}

gboolean AGAIL::DeviceImp::handleSetPropertyCb (GDBusConnection *connection, const gchar *sender, const gchar *obj_path, const gchar *interface_name, const gchar *property_name, GVariant *value, GError **error) {
    return false;
}

void AGAIL::DeviceImp::onUnknownMethod(std::string method) {
    std::cout << method << "() method unknown!" << std::endl;
}
