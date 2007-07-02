// generated by makecode.pl
#ifndef DBUSXESAMLIVESEARCHINTERFACE_H
#define DBUSXESAMLIVESEARCHINTERFACE_H
#include "/home/oever/code/strigi/src/daemon/xesam/xesamlivesearchinterface.h"
#define DBUS_API_SUBJECT_TO_CHANGE 1
#include <dbus/dbus.h>
class DBusObjectInterface;
class DBusXesamLiveSearchInterface : public XesamLiveSearchInterface {
private:
    DBusConnection* const conn;
    std::string object;
    DBusObjectInterface* const iface;
    void GetHitsResponse(void* msg,             const std::vector<std::vector<Strigi::Variant> >& hits);
    void GetHitDataResponse(void* msg,             const std::vector<std::vector<Strigi::Variant> >& v);
    void CountHitsResponse(void* msg, int32_t count);
    void HitsModified(const std::string& search,         const std::vector<int32_t>& hit_ids);
    void HitsRemoved(const std::string& search,         const std::vector<int32_t>& hit_ids);
    void HitsAdded(const std::string& search, const int32_t count);
public:
    DBusXesamLiveSearchInterface(const std::string& objectname, DBusConnection* c, XesamLiveSearchInterface* x);
    ~DBusXesamLiveSearchInterface();
    DBusObjectInterface* interface() { return iface; }
};
#endif
