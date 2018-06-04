#include "DeviceCaller.h"

#include <iostream> // TODO: substitute by log library

AGAIL::DeviceCaller::DeviceCaller(std::string address, std::string devicePath) : devicePath(devicePath) {
    GError* error = NULL;

    connection = g_bus_get_sync (DEFAULT_DBUS_CONNECTION, NULL, &error);
    if (connection == NULL) {
          std::cout << "Error connecting to D-Bus address " << error->message << std::endl;
        g_error_free (error);
	return;
    }

    connected = true;
}

bool AGAIL::DeviceCaller::isConnected() {
    return connected;
}

GVariant* AGAIL::DeviceCaller::genericMethod(std::string methodName, GVariant* input, const GVariantType* outputType) {
    GError* error = NULL;

    GVariant* gVar = g_dbus_connection_call_sync (connection,
                             BASE_ID.c_str(),
                             devicePath.c_str(),
                             BASE_BUS_IFACE.c_str(),
                             methodName.c_str(),
                             input,
                             outputType,
                             G_DBUS_CALL_FLAGS_NONE,
                             -1,
                             NULL,
                             &error);
    if (gVar == NULL) {
          std::cout << "Error invoking " << methodName << "(): " << error->message << std::endl;
          g_error_free (error);
    }

    return gVar;
}

std::string AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_ID () {
    GVariant* gVar = genericMethod(STR(AGAIL_DEVICE_METHOD_ID),
				   NULL,
				   G_VARIANT_TYPE ("(s)"));
    if (gVar == NULL)
          return std::string();

    gchar* id;
    g_variant_get (gVar, "(&s)", &id);

    return id;
}

std::string AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_NAME () {
    GVariant* gVar = genericMethod (STR(AGAIL_DEVICE_METHOD_NAME),
				    NULL,
                    		    G_VARIANT_TYPE ("(s)"));
    if (gVar == NULL)
          return std::string();

    gchar* name;
    g_variant_get (gVar, "(&s)", &name);

    return name;
}

std::string AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_STATUS () {
    GVariant* gVar = genericMethod (STR(AGAIL_DEVICE_METHOD_STATUS),
                             	    NULL,
                             	    G_VARIANT_TYPE ("(s)"));
    if (gVar == NULL)
	return std::string();

    gchar* status;
    g_variant_get (gVar, "(&s)", &status);

    return status;
}

long AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_LASTSEEN () {
    GVariant* gVar = genericMethod (STR(AGAIL_DEVICE_METHOD_LASTSEEN),
                                    NULL,
                                    G_VARIANT_TYPE ("(i)"));
    if (gVar == NULL)
        return 0;

    gint32 lastSeen;
    g_variant_get (gVar, "(i)", &lastSeen);

    return lastSeen;
}

std::string AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_CONFIGURATION () {
    GVariant* gVar = genericMethod (STR(AGAIL_DEVICE_METHOD_CONFIGURATION),
                                    NULL,
                                    G_VARIANT_TYPE ("(s)"));
    if (gVar == NULL)
        return std::string();

    gchar* configuration;
    g_variant_get (gVar, "(&s)", &configuration);

    return configuration;
}

std::list<AGAIL::DeviceComponent> AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_PROFILE () {
    GVariant* gVar = genericMethod (STR(AGAIL_DEVICE_METHOD_PROFILE),
                                    NULL,
                                    G_VARIANT_TYPE ("(a(ss))"));
    if (gVar == NULL)
        return std::list<AGAIL::DeviceComponent>();

    GVariantIter* iter;
    gchar *id, *unit;
    std::list<AGAIL::DeviceComponent> profile;
    g_variant_get (gVar, "(a(ss))", &iter);
    while (g_variant_iter_loop (iter, "(&s&s)", &id, &unit))
	profile.push_back (AGAIL::DeviceComponent (id, unit));
    g_variant_iter_free (iter);
    g_variant_unref (gVar);

    return profile;
}

AGAIL::DeviceDefinition AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_DEFINITION () {
    GVariant* gVar = genericMethod (STR(AGAIL_DEVICE_METHOD_DEFINITION),
                                    NULL,
                                    G_VARIANT_TYPE ("((ssssssa(ss)))"));
    if (gVar == NULL)
        return AGAIL::DeviceDefinition("", "", "", "", "", "", std::list<AGAIL::DeviceComponent>());

    GVariantIter* iter;
    gchar *deviceID, *address, *name, *description, *protocol, *path, *id, *unit;
    std::list<AGAIL::DeviceComponent> profile;
    g_variant_get (gVar, "((&s&s&s&s&s&sa(ss)))", &deviceID, &address, &name, &description, &protocol, &path, &iter);
    while (g_variant_iter_loop (iter, "(&s&s)", &id, &unit))
        profile.push_back (DeviceComponent (id, unit));
    AGAIL::DeviceDefinition definition (deviceID, address, name, description, protocol, path, profile);
    g_variant_iter_free (iter);
    g_variant_unref (gVar);

    return definition;
}

AGAIL::RecordObject AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_LASTUPDATE (std::string componentID) {
    GVariant* gVar = genericMethod(STR(AGAIL_DEVICE_METHOD_LASTUPDATE),
                                   g_variant_new ("(s)", componentID.c_str()),
                                   G_VARIANT_TYPE ("((ssayssi))"));
    if (gVar == NULL)
          return AGAIL::RecordObject("", "", "", "", "", 0);

    GVariantIter* iter;
    gchar *deviceID, *outComponentID, *unit, *format;
    gint32 lastUpdate;
    gchar valueByte;
    std::string value;
    g_variant_get (gVar, "((&s&say&s&si))", &deviceID, &outComponentID, &iter, &unit, &format, &lastUpdate);
    while (g_variant_iter_loop (iter, "y", &valueByte))
	value.push_back (valueByte);
    g_variant_iter_free (iter);
    AGAIL::RecordObject object (deviceID, outComponentID, value, unit, format, lastUpdate);
    g_variant_unref (gVar);

    return object;
}

std::list<AGAIL::RecordObject> AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_LASTUPDATEALL () {
    GVariant* gVar = genericMethod (STR(AGAIL_DEVICE_METHOD_LASTUPDATEALL),
                                    NULL,
                                    G_VARIANT_TYPE ("(a(ssayssi))"));
    if (gVar == NULL)
        return std::list<AGAIL::RecordObject>();

    GVariantIter* iter;
    gchar *deviceID, *componentID, *unit, *format;
    gint32 lastUpdate;
    GVariantIter* iterValue;
    gchar valueByte;
    std::list<AGAIL::RecordObject> lastUpdateAll;
    g_variant_get (gVar, "(a(ssayssi))", &iter);
    while (g_variant_iter_loop (iter, "(&s&say&s&si)", &deviceID, &componentID, &iterValue, &unit, &format, &lastUpdate)) {
	std::string value;
	while (g_variant_iter_loop (iterValue, "y", &valueByte))
	    value.push_back (valueByte);
	//g_variant_iter_free (iterValue);
        lastUpdateAll.push_back (AGAIL::RecordObject (deviceID, componentID, value, unit, format, lastUpdate));
    }
    g_variant_iter_free (iter);
    g_variant_unref (gVar);

    return lastUpdateAll;
}

AGAIL::RecordObject AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_DATA () {
    GVariant* gVar = genericMethod(STR(AGAIL_DEVICE_METHOD_DATA),
                                   NULL,
                                   G_VARIANT_TYPE ("((ssayssi))"));
    if (gVar == NULL)
          return AGAIL::RecordObject("", "", "", "", "", 0);

    GVariantIter* iter;
    gchar *deviceID, *outComponentID, *unit, *format;
    gint32 lastUpdate;
    gchar valueByte;
    std::string value;
    g_variant_get (gVar, "((&s&say&s&si))", &deviceID, &outComponentID, &iter, &unit, &format, &lastUpdate);
    while (g_variant_iter_loop (iter, "y", &valueByte))
	value.push_back (valueByte);
    g_variant_iter_free (iter);
    AGAIL::RecordObject data (deviceID, outComponentID, value, unit, format, lastUpdate);
    g_variant_unref (gVar);

    return data;
}

std::string AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_PROTOCOL () {
    GVariant* gVar = genericMethod (STR(AGAIL_DEVICE_METHOD_PROTOCOL),
                                    NULL,
                                    G_VARIANT_TYPE ("(s)"));
    if (gVar == NULL)
        return std::string();

    gchar* protocol;
    g_variant_get (gVar, "(&s)", &protocol);

    return protocol;
}

void AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_CONNECT () {
    GVariant* gVar = genericMethod (STR(AGAIL_DEVICE_METHOD_CONNECT),
                                    NULL,
                                    NULL);
}

void AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_DISCONNECT () {
    GVariant* gVar = genericMethod (STR(AGAIL_DEVICE_METHOD_DISCONNECT),
                                    NULL,
                                    NULL);
}

void AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_EXECUTE (std::string commandID) {
    GVariant* gVar = genericMethod(STR(AGAIL_DEVICE_METHOD_EXECUTE),
                                   g_variant_new ("(s)", commandID.c_str()),
                                   NULL);
}

std::list<AGAIL::RecordObject> AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_READALL () {
    GVariant* gVar = genericMethod (STR(AGAIL_DEVICE_METHOD_READALL),
                                    NULL,
                                    G_VARIANT_TYPE ("(a(ssayssi))"));
    if (gVar == NULL)
        return std::list<AGAIL::RecordObject>();

    GVariantIter* iter;
    gchar *deviceID, *componentID, *unit, *format;
    gint32 lastUpdate;
    GVariantIter* iterValue;
    gchar valueByte;
    std::list<AGAIL::RecordObject> readAll;
    g_variant_get (gVar, "(a(ssayssi))", &iter);
    while (g_variant_iter_loop (iter, "(&s&say&s&si)", &deviceID, &componentID, &iterValue, &unit, &format, &lastUpdate)) {
	std::string value;
	while (g_variant_iter_loop (iterValue, "y", &valueByte))
	    value.push_back (valueByte);
	//g_variant_iter_free (iterValue);
        readAll.push_back (AGAIL::RecordObject (deviceID, componentID, value, unit, format, lastUpdate));
    }
    g_variant_iter_free (iter);
    g_variant_unref (gVar);

    return readAll;
}

AGAIL::RecordObject AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_READ (std::string componentID) {
    GVariant* gVar = genericMethod(STR(AGAIL_DEVICE_METHOD_READ),
                                   g_variant_new ("(s)", componentID.c_str()),
                                   G_VARIANT_TYPE ("((ssayssi))"));
    if (gVar == NULL)
          return AGAIL::RecordObject("", "", "", "", "", 0);

    GVariantIter* iter;
    gchar *deviceID, *outComponentID, *unit, *format;
    gint32 lastUpdate;
    gchar valueByte;
    std::string value;
    g_variant_get (gVar, "((&s&say&s&si))", &deviceID, &outComponentID, &iter, &unit, &format, &lastUpdate);
    while (g_variant_iter_loop (iter, "y", &valueByte))
        value.push_back (valueByte);
    g_variant_iter_free (iter);
    AGAIL::RecordObject read (deviceID, outComponentID, value, unit, format, lastUpdate);
    g_variant_unref (gVar);

    return read;
}

void AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_WRITE (std::string componentID, std::string payload) {
    GVariantBuilder* builder = g_variant_builder_new (G_VARIANT_TYPE ("ay"));
    for (std::string::iterator it = payload.begin(); it != payload.end(); it++)
        g_variant_builder_add (builder, "y", *it);
    GVariant* gVar = genericMethod(STR(AGAIL_DEVICE_METHOD_WRITE),
                                   g_variant_new ("(say)", componentID.c_str(), builder),
                                   NULL);
	g_variant_builder_unref (builder);
}

void AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_SUBSCRIBE (std::string componentID) {
    GVariant* gVar = genericMethod(STR(AGAIL_DEVICE_METHOD_SUBSCRIBE),
                                   g_variant_new ("(s)", componentID.c_str()),
                                   NULL);
}

void AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_UNSUBSCRIBE (std::string componentID) {
    GVariant* gVar = genericMethod(STR(AGAIL_DEVICE_METHOD_UNSUBSCRIBE),
                                   g_variant_new ("(s)", componentID.c_str()),
                                   NULL);
}

std::list<std::string> AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_COMMANDS () {
    GVariant* gVar = genericMethod (STR(AGAIL_DEVICE_METHOD_COMMANDS),
                                    NULL,
                                    G_VARIANT_TYPE ("(as)"));
    if (gVar == NULL)
        return std::list<std::string>();

    GVariantIter* iter;
    gchar *command;
    std::list<std::string> commands;
    g_variant_get (gVar, "(as)", &iter);
    while (g_variant_iter_loop (iter, "(&s)", &command))
        commands.push_back (command);
    g_variant_iter_free (iter);
    g_variant_unref (gVar);

    return commands;
}

void AGAIL::DeviceCaller::AGAIL_DEVICE_METHOD_STOP () {
    GVariant* gVar = genericMethod (STR(AGAIL_DEVICE_METHOD_STOP),
                                    NULL,
                                    NULL);
}
