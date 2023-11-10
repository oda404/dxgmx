/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/serialio.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/utils/linkedlist.h>

#define KLOGF_PREFIX "serialio: "

static LinkedList g_serial_drvs;
static LinkedList g_serial_devs;

static SerialIODriver* serialio_new_driver()
{
    SerialIODriver* drv = kcalloc(sizeof(SerialIODriver));
    if (!drv)
        return NULL;

    if (linkedlist_add(drv, &g_serial_drvs) < 0)
    {
        kfree(drv);
        return NULL;
    }

    return drv;
}

static SerialIODevice* serialio_alloc_new_dev()
{
    SerialIODevice* dev = kcalloc(sizeof(SerialIODevice));
    if (!dev)
        return NULL;

    if (linkedlist_add(dev, &g_serial_devs) < 0)
    {
        kfree(dev);
        return NULL;
    }

    return dev;
}

static bool
serialio_check_match(const SerialIODevice* dev, const SerialIODriver* drv)
{
    if (dev->maj == SERIAL_NOMAJ && drv->maj == SERIAL_NOMAJ)
    {
        // We try to match using the name.
        ASSERT(dev->name);
        ASSERT(drv->name);
        return (strcmp(dev->name, drv->name) == 0);
    }
    else if (dev->maj && drv->maj)
    {
        return dev->maj == drv->maj;
    }

    return false;
}

static int serialio_try_dev(SerialIODevice* dev)
{
    ASSERT(!dev->driver);

    FOR_EACH_ENTRY_IN_LL (g_serial_drvs, SerialIODriver*, drv)
    {
        if (serialio_check_match(dev, drv))
        {
            int st = drv->try_link(dev);
            if (st == 0)
                dev->driver = drv;

            return st;
        }
    }
    return 0;
}

static int serialio_try_drv(const SerialIODriver* drv)
{
    FOR_EACH_ENTRY_IN_LL (g_serial_devs, SerialIODevice*, dev)
    {
        if (dev->driver)
            continue; // Skip if the dev already has a driver

        if (serialio_check_match(dev, drv))
        {
            int st = drv->try_link(dev);
            if (st == 0)
                dev->driver = drv;

            return st;
        }
    }

    return 0;
}

static const SerialIODriver* serialio_find_drv_by_major(u32 maj)
{
    FOR_EACH_ENTRY_IN_LL (g_serial_drvs, SerialIODriver*, drv)
    {
        if (drv->maj == maj)
            return drv;
    }
    return NULL;
}

static const SerialIODriver* serialio_find_drv_by_name(const char* name)
{
    FOR_EACH_ENTRY_IN_LL (g_serial_drvs, SerialIODriver*, drv)
    {
        if (drv->name && strcmp(drv->name, name) == 0)
            return drv;
    }
    return NULL;
}

static bool serialio_is_dev_name_dup(const char* name, u32 min)
{
    FOR_EACH_ENTRY_IN_LL (g_serial_devs, SerialIODevice*, dev)
    {
        if (dev->name && strcmp(dev->name, name) == 0 && dev->min == min)
            return true;
    }

    return false;
}

static bool serialio_is_dev_maj_dup(u32 maj, u32 min)
{
    FOR_EACH_ENTRY_IN_LL (g_serial_devs, SerialIODevice*, dev)
    {
        if (dev->maj == maj && dev->min == min)
            return true;
    }

    return false;
}

static int serialio_unlink_dev(SerialIODevice* dev)
{
    ASSERT(dev->driver);

    const SerialIODriver* drv = dev->driver;
    int st = drv->unlink(dev);
    if (st)
        return st;

    dev->driver = NULL;
    return 0;
}

ERR_OR_PTR(SerialIODriver) serialio_new_drv_from(const SerialIODriver* drv)
{
    if (drv->maj == SERIAL_NOMAJ && drv->name == SERIAL_NONAME)
    {
        KLOGF(ERR, "Tried to register new driver with no maj and name.");
        return ERR_PTR(SerialIODriver, -EINVAL);
    }

    const SerialIODriver* existing_drv =
        drv->maj == SERIAL_NOMAJ ? NULL : serialio_find_drv_by_major(drv->maj);
    if (existing_drv)
    {
        KLOGF(
            ERR,
            "Failed to register driver: '%s'. Driver '%s' has already registered maj: %d.",
            drv->name,
            existing_drv->name,
            drv->maj);
        return ERR_PTR(SerialIODriver, -EEXIST);
    }

    existing_drv = drv->name == SERIAL_NONAME
                       ? NULL
                       : serialio_find_drv_by_name(drv->name);
    if (existing_drv)
    {
        KLOGF(
            ERR, "Failed to register driver: '%s', already exists.", drv->name);
        return ERR_PTR(SerialIODriver, -EEXIST);
    }

    SerialIODriver* new_drv = serialio_new_driver();
    if (!new_drv)
        return ERR_PTR(SerialIODriver, -ENOMEM);

    *new_drv = *drv;
    int st = serialio_try_drv(new_drv);
    if (st)
        return ERR_PTR(SerialIODriver, st);

    return VALUE_PTR(SerialIODriver, new_drv);
}

int serialio_delete_drv(const SerialIODriver* drv)
{
    /* Remove all of this driver's devices */
    FOR_EACH_ENTRY_IN_LL (g_serial_devs, SerialIODevice*, dev)
    {
        if (dev->driver != drv)
            continue;

        int st = serialio_unlink_dev(dev);
        if (st < 0)
            return -EBUSY;
    }

    if (linkedlist_remove_by_data((void*)drv, &g_serial_drvs) < 0)
    {
        if (drv->maj)
            KLOGF(ERR, "Driver (%u) was not found for deletion!", drv->maj);
        else
            KLOGF(ERR, "Driver (%s) was not found for deletion!", drv->name);
    }

    kfree((void*)drv);
    return 0;
}

int serialio_delete_drv_by_name(const char* name)
{
    const SerialIODriver* drv = serialio_find_drv_by_name(name);
    if (!drv)
        return -ENOENT;

    return serialio_delete_drv(drv);
}

ERR_OR_PTR(SerialIODevice)
serialio_new_dev(const char* name, u32 maj, u32 min, void* data)
{
    if (maj == SERIAL_NOMAJ && name == SERIAL_NONAME)
    {
        KLOGF(ERR, "Tried to register new device with no maj and name.");
        return ERR_PTR(SerialIODevice, -EINVAL);
    }

    /* Check if a device with the same ids already exists */
    if (name && serialio_is_dev_name_dup(name, min))
    {
        KLOGF(ERR, "Device (\"%s\", %d) already exists.", name, min);
        return ERR_PTR(SerialIODevice, -EEXIST);
    }

    if (maj && serialio_is_dev_maj_dup(maj, min))
    {
        KLOGF(ERR, "Device (%d, %d) already exists.", maj, min);
        return ERR_PTR(SerialIODevice, -EEXIST);
    }

    SerialIODevice* dev = serialio_alloc_new_dev();
    if (!dev)
        return ERR_PTR(SerialIODevice, -ENOMEM);

    dev->name = name;
    dev->maj = maj;
    dev->min = min;
    dev->data = data;

    int st = serialio_try_dev(dev);
    if (st)
        return ERR_PTR(SerialIODevice, st);

    return VALUE_PTR(SerialIODevice, dev);
}

int serialio_delete_dev_by_name(const char* name)
{
    SerialIODevice* found_dev = NULL;
    FOR_EACH_ENTRY_IN_LL (g_serial_devs, SerialIODevice*, dev)
    {
        if (dev->name && strcmp(dev->name, name) == 0)
        {
            found_dev = dev;
            BREAK_LL(dev);
        }
    }

    if (!found_dev)
        return -ENOENT;

    /* Unlink device if it has been linked to a driver. */
    if (found_dev->driver)
    {
        int st = serialio_unlink_dev(found_dev);
        if (st)
            return st;
    }

    if (linkedlist_remove_by_data(found_dev, &g_serial_devs) < 0)
        return -ENOMEM;

    kfree(found_dev);
    return 0;
}
