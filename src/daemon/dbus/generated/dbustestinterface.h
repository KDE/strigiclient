// generated by makecode.pl
#ifndef DBUSTESTINTERFACE_H
#define DBUSTESTINTERFACE_H
#include "/home/oever/code/strigi/src/daemon/dbus/testinterface.h"
#define DBUS_API_SUBJECT_TO_CHANGE 1
#include <dbus/dbus.h>
class DBusObjectInterface;
class DBusTestInterface : public TestInterface {
private:
    DBusConnection* const conn;
    std::string object;
    DBusObjectInterface* const iface;
public:
    DBusTestInterface(const std::string& objectname, DBusConnection* c);
    ~DBusTestInterface();
    DBusObjectInterface* interface() { return iface; }
};
#endif
