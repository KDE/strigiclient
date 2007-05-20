#include "dbusserialization.h"
#include "variant.h"
#include "dbusmessagewriter.h"
#include "dbusmessagereader.h"
#include "textutils.h"
#include <iostream>
using namespace std;

/* yes this is ugly, i have to change the interface */
DBusMessageWriter&
operator<<(DBusMessageWriter& w,
        const ClientInterface::Hits& s) {
    DBusMessageIter sub;
    DBusMessageIter ssub;
    DBusMessageIter sssub;
    DBusMessageIter ssssub;
    DBusMessageIter sssssub;
    dbus_message_iter_open_container(&w.it, DBUS_TYPE_ARRAY,
        "(sdsssxxa{sas})", &sub);
    vector<Strigi::IndexedDocument>::const_iterator i;
    for (i=s.hits.begin(); i!=s.hits.end(); ++i) {
        dbus_message_iter_open_container(&sub, DBUS_TYPE_STRUCT, 0, &ssub);
        const char* c = i->uri.c_str();
        dbus_message_iter_append_basic(&ssub, DBUS_TYPE_STRING, &c);
        double d = i->score;
        dbus_message_iter_append_basic(&ssub, DBUS_TYPE_DOUBLE, &d);
        // quick bug fix on what is probably a clucene bug
        // the fragments as stored are sometimes not properly recoded back
        // from usc2 to utf8 which causes dbus to close the connection,
        // whereupon the strigidaemon quits
        if (Strigi::checkUtf8(i->fragment.c_str(), i->fragment.size())) {
            c = i->fragment.c_str();
        } else {
            c = "";
        }
        dbus_message_iter_append_basic(&ssub, DBUS_TYPE_STRING, &c);
        c = i->mimetype.c_str();
        dbus_message_iter_append_basic(&ssub, DBUS_TYPE_STRING, &c);
        c = i->sha1.c_str();
        dbus_message_iter_append_basic(&ssub, DBUS_TYPE_STRING, &c);
        int64_t n = i->size;
        dbus_message_iter_append_basic(&ssub, DBUS_TYPE_INT64, &n);
        n = i->mtime;
        dbus_message_iter_append_basic(&ssub, DBUS_TYPE_INT64, &n);
        dbus_message_iter_open_container(&ssub, DBUS_TYPE_ARRAY, "{sas})",
            &sssub);
        multimap<string, string>::const_iterator j;
        for (j = i->properties.begin(); j != i->properties.end(); ++j) {
            dbus_message_iter_open_container(&sssub, DBUS_TYPE_DICT_ENTRY, 0,
                &ssssub);
            const char* c = j->first.c_str();
            dbus_message_iter_append_basic(&ssssub, DBUS_TYPE_STRING, &c);
            dbus_message_iter_open_container(&ssssub, DBUS_TYPE_ARRAY, "s",
                &sssssub);
            c = j->second.c_str();
            dbus_message_iter_append_basic(&sssssub, DBUS_TYPE_STRING, &c);
            dbus_message_iter_close_container(&ssssub, &sssssub);
            dbus_message_iter_close_container(&sssub, &ssssub);
        }
        dbus_message_iter_close_container(&ssub, &sssub);
        dbus_message_iter_close_container(&sub, &ssub);
    }
    dbus_message_iter_close_container(&w.it, &sub);
    return w;
}

DBusMessageWriter&
operator<<(DBusMessageWriter& w, const Variant& v) {
    DBusMessageWriter sub;
    switch (v.type()) {
    case Variant::b_val:
        dbus_message_iter_open_container(&w.it, DBUS_TYPE_VARIANT, "b",&sub.it);
        sub << v.b();
        break;
    case Variant::i_val:
        dbus_message_iter_open_container(&w.it, DBUS_TYPE_VARIANT, "i",&sub.it);
        sub << v.i();
        break;
    case Variant::s_val:
        dbus_message_iter_open_container(&w.it, DBUS_TYPE_VARIANT, "s",&sub.it);
        sub << v.s();
        break;
    case Variant::as_val:
        dbus_message_iter_open_container(&w.it, DBUS_TYPE_VARIANT,"as",&sub.it);
        sub << v.as();
        break;
    }
    dbus_message_iter_close_container(&w.it, &sub.it);
    return w;
}
DBusMessageWriter&
operator<<(DBusMessageWriter& w, const vector<vector<Variant> >& v) {
    DBusMessageWriter sub, ssub;
    dbus_message_iter_open_container(&w.it, DBUS_TYPE_ARRAY, "av", &sub.it);
    vector<vector<Variant> >::const_iterator i;
    vector<Variant>::const_iterator j;
    for (i = v.begin(); i != v.end(); ++i) {
        dbus_message_iter_open_container(&sub.it,DBUS_TYPE_ARRAY,"v",&ssub.it);
        for (j = i->begin(); j != i->end(); ++j) {
            ssub << *j;
        }
        dbus_message_iter_close_container(&sub.it, &ssub.it);
    }
    dbus_message_iter_close_container(&w.it, &sub.it);
    return w;
}
DBusMessageReader&
operator>>(DBusMessageReader& r, Variant& v) {
    if (!r.isOk()) return r;
    if (dbus_message_iter_get_arg_type(&r.it) != DBUS_TYPE_VARIANT) {
        r.close();
        return r;
    }
    DBusMessageReader sub(r);
    dbus_message_iter_recurse(&r.it, &sub.it);
    dbus_message_iter_next(&r.it);
    int t = dbus_message_iter_get_arg_type(&sub.it);
    if (t == DBUS_TYPE_BOOLEAN) {
        dbus_bool_t b;
        sub >> b;
        v = (bool)b;
    } else if (t == DBUS_TYPE_INT32) {
        dbus_int32_t i;
        sub >> i;
        v = i;
    } else if (t == DBUS_TYPE_STRING) {
        string s;
        sub >> s;
        v = s;
    } else if (t == DBUS_TYPE_ARRAY) {
        vector<string> as;
        sub >> as;
        v = as;
    } else {
        r.close();
    }
    return r;
}