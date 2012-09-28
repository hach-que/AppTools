#define _GNU_SOURCE 1
#include <dbus/dbus.h>
#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>

unsigned int inside_objsearch = 0;

#define APPAUDIT_NAME   "appd.Service"
#define APPAUDIT_OPATH  "/appd/Service"
#define APPAUDIT_IPATH  "appd.Service"
#define APPAUDIT_METHOD "resolve_lazy"
#ifdef APPAUDIT_DEBUG
#define DBUS_FAIL(err) { fprintf(stderr, "%s\n", dbus_error_is_set(&error) ? error.message : err); free(linkbuf); return NULL; }
#else
#define DBUS_FAIL(err) { free(linkbuf); return NULL; }
#endif

char* dbus_search(const char* name)
{
    DBusConnection* bus = NULL;
    DBusMessage* msg = NULL;
    DBusMessage* reply = NULL;
    DBusError error;
    ssize_t linksize;
    char* linkbuf = malloc(PATH_MAX);
    char* result = NULL;
    dbus_bool_t success;

    /* Resolve link first. */
    memset(linkbuf, '\0', PATH_MAX);
    linksize = readlink("/proc/self/exe", linkbuf, PATH_MAX);

    /* Now attempt D-Bus communication. */
    dbus_error_init(&error);
    bus = dbus_bus_get(DBUS_BUS_SESSION, &error);
    if (bus == NULL || dbus_error_is_set(&error))
        DBUS_FAIL("bus not found");
    msg = dbus_message_new_method_call(APPAUDIT_NAME,
            APPAUDIT_OPATH,
            APPAUDIT_IPATH,
            APPAUDIT_METHOD);
    if (msg == NULL || dbus_error_is_set(&error))
        DBUS_FAIL("no such method");
    if (!dbus_message_append_args(msg,
                DBUS_TYPE_STRING, &linkbuf,
                DBUS_TYPE_STRING, &name,
                DBUS_TYPE_INVALID))
        DBUS_FAIL("can't append arguments");
    reply = dbus_connection_send_with_reply_and_block(bus, msg, -1, &error);
    if (reply == NULL || dbus_error_is_set(&error))
        DBUS_FAIL("can't send message");
    if (!dbus_message_get_args(reply, &error,
                DBUS_TYPE_BOOLEAN, &success,
                DBUS_TYPE_STRING, &result,
                DBUS_TYPE_INVALID))
        DBUS_FAIL("can't get reply arguments");
    if (dbus_error_is_set(&error))
        DBUS_FAIL("reply arguments invalid");
    result = strdup(result); /* Ensure it is not lost. */
    dbus_message_unref(reply);
    reply = NULL;
    dbus_message_unref(msg);
    msg = NULL;
    dbus_connection_unref(bus);
    bus = NULL;

    /* Now return the result. */
    if (success)
    {
        free(linkbuf);
        return result;
    }
    else
    {
        free(linkbuf);
        free(result);
        return NULL;
    }
}

char* real_search(const char* name)
{
    char* old_audit = NULL;
    char* result = NULL;

    /* If we're already inside objsearch, just return. */
    if (inside_objsearch == 1)
        return NULL;

    /* Save the existing LD_AUDIT value and overwrite
     * it with nothing.  This gives us a safe environment
     * where we can load and run programs without
     * interference. */
    old_audit = getenv("LD_AUDIT");
    unsetenv("LD_AUDIT");

    /* Mark ourselves as being inside objsearch. */
    inside_objsearch = 1;

    /* Search for the library using D-Bus. */
    result = dbus_search(name);

    /* Mark ourselves as being outside objsearch. */
    inside_objsearch = 0;

    /* Restore old value. */
    setenv("LD_AUDIT", old_audit, 1);

    return result;
}

/*
 * See the manual page on rtld-audit(7) for more information
 * about how this hook works.
 */

unsigned int la_version(unsigned int version)
{
    return LAV_CURRENT;
}

uintptr_t* temp_cookie = 0x0;
char* temp_result = NULL;

char* la_objsearch(const char* name, uintptr_t* cookie, unsigned int flag)
{
    /* For some reason, we must leave the original search
     * name intact; we can't just directly replace it with
     * the path.  Thus we must wait until the loader attempts
     * to start creating paths before we can return the actual
     * path to use. */
    if ((flag & LA_SER_ORIG) != 0)
    {
        if (temp_result != NULL)
            free(temp_result);
        temp_result = real_search(name);
        temp_cookie = cookie;
        return (char*)name;
    }
    else
    {
        if (temp_result == NULL)
        {
            char* tmp = real_search(name);
            if (tmp != NULL)
                free(tmp);
            return (char*)name;
        }
        temp_cookie = 0x0;
        return temp_result;
    }
}

