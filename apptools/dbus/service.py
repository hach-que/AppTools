# vim: set ts=4 sw=4 tw=0 et ai:

import os
import dbus
import dbus.service
from elftools.elf.elffile import ELFFile
from apptools.resolve import ELFHasher, LibraryResolver

class PickledDB():
    def __init__(self, path):
        import cPickle
        import os
        self._path = path
        if (os.path.exists(self._path)):
            try:
                with file(self._path) as f:
                    self._data = cPickle.load(f)
            except EOFError:
                # The database wasn't saved correctly and is empty.
                self._data = list()
        else:
            self._data = list()

    def add_entry(self, uid, binary_hash, library_hash, library_path):
        self._data.append({
            "uid": int(uid),
            "binary_hash": str(binary_hash),
            "library_hash": str(library_hash),
            "library_path": str(library_path)
            })

    def iter_entries(self):
        for i in self._data:
            yield i

    def save(self):
        import cPickle
        try:
            with open(self._path, 'w') as f:
                cPickle.dump(self._data, f)
        except:
            print self._data
            import traceback
            traceback.print_exc()

class Service(dbus.service.Object):
    def __init__(self):
        if (os.getuid() == 0):
            self._bus = dbus.SystemBus()
            self._system = True
        else:
            print "warning: appd is running on the session bus and"
            print "         will only be available to the current user."
            self._bus = dbus.SessionBus()
            self._system = False

        bus_name = dbus.service.BusName("appd.Service", bus=self._bus)
        dbus.service.Object.__init__(self, bus_name, "/appd/Service")

        # TODO: We should have another class that manages
        # the state of the server, rather than just lumping it
        # all in here.
        self._system_db = PickledDB("/lib/.db")
        self._user_db = PickledDB(os.path.expanduser("~/.libraries.db"))
        if (self._system):
            self._writable_db = self._system_db
        else:
            self._writable_db = self._user_db

    def _get_user(self, sender):
        return int(self._bus.get_unix_user(sender))

    @dbus.service.method(dbus_interface='appd.Service',
                         in_signature='', out_signature='i',
                         sender_keyword='sender')
    def version(self, sender=None):
        return 1

    @dbus.service.method(dbus_interface='appd.Service',
                         in_signature='sss', out_signature='',
                         sender_keyword='sender')
    def store(self, binary_hash, library_hash, library_path, sender=None):
        # TODO: if (self.get_user(sender) == 0 and self._system):
        print "Storing entry for UID " + str(self._get_user(sender)) + " to resolve " + library_path + "."
        self._writable_db.add_entry(self._get_user(sender), binary_hash, library_hash, library_path)
        self._writable_db.save()

    def _resolve_iter(self, i, binary_hash, library_name):
        if (i["binary_hash"] == binary_hash and
                os.path.basename(i["library_path"]) == library_name):
            # Match by source hash and destination name.  Check to
            # see if the destination file matches our hash.
            if (not os.path.exists(i["library_path"])):
                return None
            with file(i["library_path"]) as dest_f:
                dest_elf = ELFFile(dest_f)
                dest_hash = ELFHasher().get_readable_hash(dest_elf)
                if (i["library_hash"] == dest_hash):
                    # Absolute match by destination hash.
                    print "Resolved library " + library_name + " -> " + i["library_path"] + "."
                    return i["library_path"]
        return None

    @dbus.service.method(dbus_interface='appd.Service',
                         in_signature='ss', out_signature='bs',
                         sender_keyword='sender')
    def resolve(self, binary_hash, library_name, sender=None):
        for i in self._system_db.iter_entries():
            if (i["uid"] == 0):
                # Entries set by root always take precedence.
                result = self._resolve_iter(i, binary_hash, library_name)
                if (result != None):
                    return True, result
        # FIXME: We really should prioritize here on out by the
        # last entry date, since we could have an entry in the
        # system database and an entry in the local database and
        # there's no way to tell which one is 'better'.
        for i in self._system_db.iter_entries():
            if (i["uid"] != 0):
                result = self._resolve_iter(i, binary_hash, library_name)
                if (result != None):
                    return True, result
        for i in self._user_db.iter_entries():
            if (i["uid"] != 0):
                result = self._resolve_iter(i, binary_hash, library_name)
                if (result != None):
                    return True, result

        print "No resolution found for " + library_name + " for specified application."
        return False, ""

    @dbus.service.method(dbus_interface='appd.Service',
                         in_signature='ss', out_signature='bs',
                         sender_keyword='sender')
    def resolve_lazy(self, binary_path, library_name, sender=None):
        def _local_resolve(a, b):
            return self.resolve(a, b, sender)
        def _local_store(a, b, c):
            self.store(a, b, c, sender)
        resolver = LibraryResolver(_local_resolve, _local_store)
        result = resolver.fast_resolve(binary_path, library_name)
        if (result == None):
            return False, ""
        else:
            return True, result

def run_daemon():
    import dbus.mainloop.glib
    import gobject
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    ex = Service()
    loop = gobject.MainLoop()
    loop.run()

