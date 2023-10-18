/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/devfs.h>
#include <dxgmx/errno.h>
#include <dxgmx/interrupts.h>
#include <dxgmx/klog.h>
#include <dxgmx/module.h>
#include <dxgmx/ps2io.h>
#include <dxgmx/serialio.h>
#include <dxgmx/todo.h>

#define KLOGF_PREFIX "ps2kbd: "
#define DRV_NAME "ps2kbd"

static void kbd_isr()
{
    u8 data = ps2io_read_data_byte_nochk();

    KLOGF(INFO, "%X", data);

    interrupts_irq_done();
}

static int ps2kbd_vnode_open(VirtualNode* vnode, int flags)
{
    return 0;
}

static ssize_t ps2kbd_vnode_read(const VirtualNode*, void*, size_t, off_t)
{
    return -1;
}

static ssize_t ps2kbd_vnode_write(
    VirtualNode* vnode, const void* _USERPTR buf, size_t n, off_t off)
{
    return 0;
}

static int ps2kbd_vnode_ioctl(VirtualNode* vnode, int req, void* data)
{
    return 0;
}

static void* ps2kbd_vnode_mmap(
    VirtualNode* vnode, void* addr, size_t len, int prot, int flags, off_t off)
{
    return NULL;
}

static VirtualNodeOperations g_ps2kbd_vnode_ops = {
    .open = ps2kbd_vnode_open,
    .read = ps2kbd_vnode_read,
    .write = ps2kbd_vnode_write,
    .ioctl = ps2kbd_vnode_ioctl,
    .mmap = ps2kbd_vnode_mmap};

static int ps2kbd_try(SerialIODevice* dev)
{
    if (dev->min == 2)
        TODO_FATAL();

    ASSERT(dev->min == 1 || dev->min == 2);

    int st = devfs_register(
        "kbd0", S_IFCHR | (S_IRUSR | S_IRGRP), 0, 0, &g_ps2kbd_vnode_ops, NULL);
    if (st < 0)
    {
        KLOGF(ERR, "Failed to create devfs entry for kbd.");
        return st;
    }

    interrupts_reqister_irq_isr(0x21, kbd_isr);
    ps2io_set_interrupts(true, dev->min);
    /* We don't check for the return code, since it may have fired
     inside the IRQ, and this functin reads it straight from the port */
    ps2io_set_scanning(true, dev->min);
    return 0;
}

static int ps2kbd_remove(SerialIODevice* dev)
{
    ps2io_set_interrupts(false, dev->min);
    ps2io_set_scanning(false, dev->min);
    return 0;
}

static int ps2kbd_main()
{
    SerialIODriver driver = {
        .name = DRV_NAME, // match against devs named "ps2kbd"
        .try_link = ps2kbd_try,
        .unlink = ps2kbd_remove};

    return serialio_new_drv_from(&driver).error;
}

static int ps2kbd_exit()
{
    return serialio_delete_drv_by_name(DRV_NAME);
}

MODULE g_ps2kbd_module = {
    .name = "ps2kbd",
    .main = ps2kbd_main,
    .exit = ps2kbd_exit,
    .stage = MODULE_STAGE3};
