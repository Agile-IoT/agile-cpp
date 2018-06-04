#include "AbstractAgileObject.h"

const GBusType AGAIL::AbstractAgileObject::DEFAULT_DBUS_CONNECTION = G_BUS_TYPE_SESSION;

void AGAIL::AbstractAgileObject::dbusConnect(const std::string busName, const std::string busPath, const std::string busIntrospection) {
    this->busName = busName;
    this->busPath = busPath;
    this->busIntrospection = busIntrospection;

    if(initBus() != NULL)
        return;

    keepAliveProtocol();
}

GError* AGAIL::AbstractAgileObject::initBus() {
    GError *g_error;

    introspection_data = g_dbus_node_info_new_for_xml (busIntrospection.c_str(), &g_error);
    if(introspection_data == NULL)
        return g_error;

    owner_id = g_bus_own_name(DEFAULT_DBUS_CONNECTION,
                             busName.c_str(),
                             G_BUS_NAME_OWNER_FLAGS_NONE,
                             &onBusAcquiredCallback,
                             &onNameAcquiredCallback,
                             &onNameLostCallback,
                             this,
                             NULL);

    mainloop = g_main_loop_new (NULL, FALSE);

    return NULL;
}

void AGAIL::AbstractAgileObject::keepAliveProtocol() {
    g_main_loop_run(mainloop);
    g_bus_unown_name(owner_id);
    g_dbus_node_info_unref(introspection_data);
}

void AGAIL::AbstractAgileObject::dbusDisconnect(){
    g_bus_unown_name(owner_id);
    g_dbus_node_info_unref(introspection_data);
}

//////////////////////////////////////////////////////
// Callback functions for g_bus_own_name() function //
//////////////////////////////////////////////////////

void AGAIL::AbstractAgileObject::onBusAcquiredCallback(GDBusConnection *conn, const gchar *name, gpointer user_data) {
    ((AbstractAgileObject*)user_data)->onBusAcquired(conn, name);
}

void AGAIL::AbstractAgileObject::onNameAcquiredCallback(GDBusConnection *conn, const gchar *name, gpointer user_data) {
    ((AbstractAgileObject*)user_data)->onNameAcquired(conn, name);
}

void AGAIL::AbstractAgileObject::onNameLostCallback(GDBusConnection *conn, const gchar *name, gpointer user_data) {
    ((AbstractAgileObject*)user_data)->onNameLost(conn, name);
}

void AGAIL::AbstractAgileObject::onBusAcquired(GDBusConnection *conn, const gchar *name) {
    g_dbus_connection_register_object(conn,
                                      busPath.c_str(),
                                      introspection_data->interfaces[0],
                                      &interface_vtable,
                                      this,  /* user_data */
                                      NULL,  /* user_data_free_func */
                                      NULL); /* GError** */

    onBusAcquiredCb(conn, name);
}

void AGAIL::AbstractAgileObject::onNameAcquired(GDBusConnection *conn, const gchar *name) {
    onNameAcquiredCb(conn, name);
}

void AGAIL::AbstractAgileObject::onNameLost(GDBusConnection *conn, const gchar *name) {
    onNameLostCb(conn, name);
}

void AGAIL::AbstractAgileObject::onBusAcquiredCb(GDBusConnection *conn, const gchar *name) {
    // If not overloaded, this function does nothing
}

void AGAIL::AbstractAgileObject::onNameAcquiredCb(GDBusConnection *conn, const gchar *name) {
    // If not overloaded, this function does nothing
}

void AGAIL::AbstractAgileObject::onNameLostCb(GDBusConnection *conn, const gchar *name) {
    // If not overloaded, this function does nothing
}

//////////////////////////////////////////////////////////////////////////////////
// GDBusInterfaceVTable struct for g_dbus_connection_register_object() function //
//////////////////////////////////////////////////////////////////////////////////

const GDBusInterfaceVTable AGAIL::AbstractAgileObject::interface_vtable = {
    AGAIL::AbstractAgileObject::handleMethodCallCallback,
    AGAIL::AbstractAgileObject::handleGetPropertyCallback,
    AGAIL::AbstractAgileObject::handleSetPropertyCallback
};

void AGAIL::AbstractAgileObject::handleMethodCallCallback(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GDBusMethodInvocation *invocation, gpointer user_data) {
    ((AbstractAgileObject*)user_data)->handleMethodCall(connection, sender, object_path, interface_name, method_name, parameters, invocation);
}

GVariant* AGAIL::AbstractAgileObject::handleGetPropertyCallback(GDBusConnection *connection, const gchar *sender, const gchar *obj_path, const gchar *interface_name, const gchar *property_name, GError **error, gpointer user_data) {
    return ((AbstractAgileObject*)user_data)->handleGetProperty(connection, sender, obj_path, interface_name, property_name, error);
}

gboolean AGAIL::AbstractAgileObject::handleSetPropertyCallback(GDBusConnection *connection, const gchar *sender, const gchar *obj_path, const gchar *interface_name, const gchar *property_name, GVariant *value, GError **error, gpointer user_data) {
    return ((AbstractAgileObject*)user_data)->handleSetProperty(connection, sender, obj_path, interface_name, property_name, value, error);
}
