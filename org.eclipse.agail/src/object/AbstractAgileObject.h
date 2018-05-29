#ifndef __AGAIL_ABSTRACTAGILEOBJECT_H__
#define __AGAIL_ABSTRACTAGILEOBJECT_H__

#include <gio/gio.h>

#include "AgileObjectInterface.h"

namespace AGAIL {
    class AbstractAgileObject;
}

class AGAIL::AbstractAgileObject : public AgileObjectInterface {
    public:
	void dbusConnect(const std::string busName, const std::string busPath, const std::string busIntrospection) final;
	void dbusDisconnect() final;

	// These functions are called from their corresponding callbacks. Since
	// the callbacks are static, these functions are called from outside the
	// object. Therefore, they need to be public

	// Functions called from g_bus_own_name() function
	void onBusAcquired(GDBusConnection *, const gchar *);
        void onNameAcquired(GDBusConnection *, const gchar *);
        void onNameLost(GDBusConnection *, const gchar *);

	// Functions called from g_dbus_connection_register_object() function
	virtual void handleMethodCall(GDBusConnection *, const gchar *, const gchar *, const gchar *, const gchar *, GVariant *, GDBusMethodInvocation *) = 0;
        virtual GVariant* handleGetProperty(GDBusConnection *, const gchar *, const gchar *, const gchar *, const gchar *, GError **) = 0;
        virtual gboolean handleSetProperty(GDBusConnection *, const gchar *, const gchar *, const gchar *, const gchar *, GVariant *, GError **) = 0;


    protected:
	// DBUS parameters for AGILE classes
        static const GBusType DEFAULT_DBUS_CONNECTION; //default bus type (session bus)
        std::string busName;
        std::string busPath;
        std::string busIntrospection;

	// Virtual functions called from g_bus_own_name() function
	virtual void onBusAcquiredCb(GDBusConnection *, const gchar *);
        virtual void onNameAcquiredCb(GDBusConnection *, const gchar *);
        virtual void onNameLostCb(GDBusConnection *, const gchar *);

    private:
	// GNOME GDBUS variables
	GDBusNodeInfo *introspection_data = NULL;
	guint owner_id;
	GMainLoop *mainloop;

	GError* initBus();
	void keepAliveProtocol();

	// These functions need to be static because they are passed as parameter to a C API

	// Callback functions for g_bus_own_name() function
	static void onBusAcquiredCallback(GDBusConnection *, const gchar *, gpointer);
	static void onNameAcquiredCallback(GDBusConnection *, const gchar *, gpointer);
	static void onNameLostCallback(GDBusConnection *, const gchar *, gpointer);

	// Callback functions for g_dbus_connection_register_object() function (passed through interface_vtable struct)
	static const GDBusInterfaceVTable interface_vtable;
	static void handleMethodCallCallback(GDBusConnection *, const gchar *, const gchar *, const gchar *, const gchar *, GVariant *, GDBusMethodInvocation *, gpointer);
	static GVariant* handleGetPropertyCallback(GDBusConnection *, const gchar *, const gchar *, const gchar *, const gchar *, GError **, gpointer);
	static gboolean handleSetPropertyCallback(GDBusConnection *, const gchar *, const gchar *, const gchar *, const gchar *, GVariant *, GError **, gpointer);
};

#endif
