# vim: set ts=4 sw=4 tw=0 et ai:

class Client(object):
    def __new__(cls):
        import dbus
        try:
            remote = dbus.SystemBus().get_object("appd.Service", "/appd/Service")
        except:
            # Fallback to a local session if no system-wide service is available.
            try:
                remote = dbus.SessionBus().get_object("appd.Service", "/appd/Service")
            except:
                raise RuntimeError("The appd service is not available at this time.")
        return remote

