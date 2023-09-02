/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

/* Serial in-out subsystem. */

#ifndef _DXGMX_SERIALIO_H
#define _DXGMX_SERIALIO_H

#include <dxgmx/err_or.h>
#include <dxgmx/types.h>

#define SERIAL_NOMAJ 0
#define SERIAL_NONAME NULL

struct SerialIODriver;

typedef struct SerialIODevice
{
    /* Device name. Optional. If maj is SERIAL_NOMAJ then this field will
     * optionally be used to match against drivers' name. If 'maj' is provided
     * this should be set to SERIAL_NONAME */
    const char* name;

    /* Device major id. A number used to match against driver's major_id. Can be
     * set to SERIAL_NOMAJ, in which case the 'name' field will be used to match
     * against drivers. */
    u32 maj;

    /* Number identifying this specific device. Along with the name or maj
     * (whichever was provided) this pair uniquely identifies this device. */
    u32 min;

    /* The driver that is linked to this device. NULL if this device is not
     * being handled by any driver. */
    const struct SerialIODriver* driver;

    /* Extra data about this device. */
    void* data;
} SerialIODevice;

DEFINE_ERR_OR_PTR(SerialIODevice);

typedef struct SerialIODriver
{
    /* Driver name. If 'maj' is provided this should be set to SERIAL_NONAME.
     * Should be unique. */
    const char* name;

    /* Driver major id. If an id is not applicable, this can be set to
     * SERIAL_NOMAJ and 'name' can be set to match devices. Should be unique. */
    u32 maj;

    /* If a device mataches this driver, this function will be called
     * on that device to initialize it. If this function returns 0, the device
     * will be linked to this driver. On error this function should return a
     * negative errno. */
    int (*try_link)(SerialIODevice* dev);

    /* Unlink a device from this driver. */
    int (*unlink)(SerialIODevice* dev);
} SerialIODriver;

DEFINE_ERR_OR_PTR(SerialIODriver);

/** Register a new driver from the provided one. The provided driver is copied
 * and can be safely discarded after this function is done.
 *
 * Returns:
 * value -> A pointer to the newly allocated driver.
 * error -> Errno indicating error.
 */
ERR_OR_PTR(SerialIODriver) serialio_new_drv_from(const SerialIODriver* drv);

/** Delete a registered driver by name. Also unlinks any devices that may have
 * been using this driver.
 *
 * Returns:
 * 0 on success.
 * Negative indicating errno.
 */
int serialio_delete_drv_by_name(const char* name);

/**
 * Register a new device with the given parameters.
 *
 * Returns:
 * value -> A pointer to a newly allocated device.
 * error -> Errno indicating error.
 */
ERR_OR_PTR(SerialIODevice)
serialio_new_dev(const char* name, u32 maj, u32 min, void* data);

/**
 * Delete a registered device by name.
 *
 * Returns:
 * 0 on success
 * Negative errno on error.
 */
int serialio_delete_dev_by_name(const char* name);

#endif // !_DXGMX_SERIALIO_H
