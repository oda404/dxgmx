/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/devfs.h>
#include <dxgmx/errno.h>
#include <dxgmx/input/input.h>
#include <dxgmx/input/user@kbd.h>
#include <dxgmx/interrupts.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/module.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/ps2io.h>
#include <dxgmx/serialio.h>
#include <dxgmx/timekeep.h>
#include <dxgmx/todo.h>
#include <dxgmx/user.h>

#define KLOGF_PREFIX "ps2kbd: "
#define DRV_NAME "ps2kbd"

typedef struct LastReadInfo
{
    size_t idx;
    struct timespec time;
} LastReadInfo;

// scan code set 2 translation map
static u8 g_keycode_map[] = {
    [0x76] = KEY_ESC,
    [0x05] = KEY_F1,
    [0x06] = KEY_F2,
    [0x04] = KEY_F3,
    [0x0C] = KEY_F4,
    [0x03] = KEY_F5,
    [0x0B] = KEY_F6,
    [0x83] = KEY_F7,
    [0x0A] = KEY_F8,
    [0x01] = KEY_F9,
    [0x09] = KEY_F10,
    [0x78] = KEY_F11,
    [0x07] = KEY_F12,
    [0x0E] = KEY_BACKTICK,
    [0x16] = KEY_1,
    [0x1E] = KEY_2,
    [0x26] = KEY_3,
    [0x25] = KEY_4,
    [0x2E] = KEY_5,
    [0x36] = KEY_6,
    [0x3D] = KEY_7,
    [0x3E] = KEY_8,
    [0x46] = KEY_9,
    [0x45] = KEY_0,
    [0x4E] = KEY_MINUS,
    [0x55] = KEY_EQUAL,
    [0x66] = KEY_BACKSPACE,
    [0x0D] = KEY_TAB,
    [0x15] = KEY_Q,
    [0x1D] = KEY_W,
    [0x24] = KEY_E,
    [0x2D] = KEY_R,
    [0x2C] = KEY_T,
    [0x35] = KEY_Y,
    [0x3C] = KEY_U,
    [0x43] = KEY_I,
    [0x44] = KEY_O,
    [0x4D] = KEY_P,
    [0x54] = KEY_OPEN_SQ_BRACKET,
    [0x5B] = KEY_CLOSE_SQ_BRACKET,
    [0x5D] = KEY_BACKSLASH,
    [0x58] = KEY_CAPSLOCK,
    [0x1C] = KEY_A,
    [0x1B] = KEY_S,
    [0x23] = KEY_D,
    [0x2B] = KEY_F,
    [0x34] = KEY_G,
    [0x33] = KEY_H,
    [0x3B] = KEY_J,
    [0x42] = KEY_K,
    [0x4B] = KEY_L,
    [0x4C] = KEY_SEMI_COLON,
    [0x52] = KEY_SINGLE_QUOTE,
    [0x5A] = KEY_RETURN,
    [0x12] = KEY_LEFT_SHIFT,
    [0x1A] = KEY_Z,
    [0x22] = KEY_X,
    [0x21] = KEY_C,
    [0x2A] = KEY_V,
    [0x32] = KEY_B,
    [0x31] = KEY_N,
    [0x3A] = KEY_M,
    [0x41] = KEY_COMMA,
    [0x49] = KEY_DOT,
    [0x4A] = KEY_SLASH,
    [0x59] = KEY_RIGHT_SHIFT,
    [0x14] = KEY_LEFT_CTRL,
    [0x11] = KEY_LEFT_ALT,
    [0x29] = KEY_SPACE,
    [0x7E] = KEY_SCROLL_LOCK,
    [0x77] = KEY_NUM_LOCK,
    [0x7C] = KEY_NUM_ASTERISK,
    [0x7B] = KEY_NUM_MINUS,
    [0x6C] = KEY_NUM_7,
    [0x75] = KEY_NUM_8,
    [0x7D] = KEY_NUM_9,
    [0x79] = KEY_NUM_PLUS,
    [0x6B] = KEY_NUM_4,
    [0x73] = KEY_NUM_5,
    [0x74] = KEY_NUM_6,
    [0x69] = KEY_NUM_1,
    [0x72] = KEY_NUM_2,
    [0x7A] = KEY_NUM_3,
    [0x70] = KEY_NUM_0,
    [0x71] = KEY_NUM_DOT //
};

static u8 g_special_keycode_map[] = {
    [0x1F] = KEY_LEFT_COMMAND,
    [0x11] = KEY_RIGHT_ALT,
    [0x27] = KEY_RIGHT_COMMAND,
    [0x2F] = 0xFF, // menus key?
    [0x14] = KEY_RIGHT_CTRL,
    [0x70] = 0xFF, // insert
    [0x6C] = KEY_HOME,
    [0x7D] = KEY_PAGEUP,
    [0x71] = KEY_DEL,
    [0x69] = KEY_END,
    [0x7A] = KEY_PAGEDOWN,
    [0x75] = KEY_UP_ARROW,
    [0x6B] = KEY_LEFT_ARROW,
    [0x72] = KEY_DOWN_ARROW,
    [0x74] = KEY_RIGHT_ARROW,
    [0x4A] = KEY_NUM_SLASH,
    [0x5A] = KEY_NUM_RETURN //
};

static bool g_special_key;
static bool g_release_modif;

static bool g_ongoing_sequence;
static u8 g_sequence_count;

#define CACHED_EVENT_COUNT 64
static InputEvent g_cached_events[CACHED_EVENT_COUNT];
static size_t g_cached_event_idx;

static void ps2kbd_cache_event(const InputEvent* ev)
{
    if (g_cached_event_idx >= CACHED_EVENT_COUNT)
        g_cached_event_idx = 0;

    g_cached_events[g_cached_event_idx++] = *ev;
}

static InputEvent* ps2kbd_earliest_event(const struct timespec* ts, size_t* idx)
{
    for (size_t i = *idx; i < CACHED_EVENT_COUNT; ++i)
    {
        InputEvent* ev = &g_cached_events[i];
        if (TIMESPEC_IS_FIRST_BIGGER(&ev->time, ts))
        {
            *idx = i;
            return ev;
        }
    }

    for (size_t i = 0; i < *idx; ++i)
    {
        InputEvent* ev = &g_cached_events[i];
        if (TIMESPEC_IS_FIRST_BIGGER(&ev->time, ts))
        {
            *idx = i;
            return ev;
        }
    }

    return NULL;
}

static void kbd_isr()
{
    const u8 data = ps2io_read_data_byte_nochk();

    if (g_ongoing_sequence)
    {
        ++g_sequence_count;

        if (g_sequence_count == 8)
        {
            g_sequence_count = 0;
            g_ongoing_sequence = false;
            if (data == 0x77)
            {
                InputEvent ev = {
                    .time = timekeep_uptime(),
                    .action = KBD_KEY_PRESS,
                    .value = KEY_BREAK};
                ps2kbd_cache_event(&ev);
                ev.action = KBD_KEY_RELEASE;
                ps2kbd_cache_event(&ev);
            }
        }
        else if (g_sequence_count == 6 && data == 0x12)
        {
            g_sequence_count = 0;
            g_ongoing_sequence = false;
            g_special_key = false;
            g_release_modif = false;

            InputEvent ev = {
                .time = timekeep_uptime(),
                .action = KBD_KEY_RELEASE,
                .value = KEY_SS};
            ps2kbd_cache_event(&ev);
        }
        else if (g_sequence_count == 4 && data == 0x7C)
        {
            g_sequence_count = 0;
            g_ongoing_sequence = false;
            g_special_key = false;

            InputEvent ev = {
                .time = timekeep_uptime(),
                .action = KBD_KEY_PRESS,
                .value = KEY_SS};
            ps2kbd_cache_event(&ev);
        }

        interrupts_irq_done();
        return;
    }

    switch (data)
    {
    case 0xE1:
        g_ongoing_sequence = true; // pause/break
        g_sequence_count = 1;
        break;

    case 0xE0:
        g_special_key = true;
        break;

    case 0xF0:
        g_release_modif = true;
        break;

    case 0x12:
        if (g_special_key)
        {
            g_ongoing_sequence = true; // print screen press
            g_sequence_count = 2;
            break;
        }
        /* fallthrough */

    case 0x7C:
        if (g_special_key && g_release_modif)
        {
            g_ongoing_sequence = true; // print screen release
            g_sequence_count = 3;
            break;
        }
        /* fallthrough */

    default:
    {
        InputEvent ev = {
            .time = timekeep_uptime(),
            .action = g_release_modif ? KBD_KEY_RELEASE : KBD_KEY_PRESS,
            .value = g_special_key ? g_special_keycode_map[data]
                                   : g_keycode_map[data]};
        ps2kbd_cache_event(&ev);
        g_special_key = false;
        g_release_modif = false;
        break;
    }
    }

    interrupts_irq_done();
}

static int ps2kbd_vnode_open(VirtualNode* vnode, int flags)
{
    vnode->data = kmalloc(sizeof(LastReadInfo));
    if (!vnode->data)
        return -ENOMEM;

    LastReadInfo* info = vnode->data;
    info->time = timekeep_uptime();
    info->idx = 0;
    return 0;
}

static ssize_t
ps2kbd_vnode_read(const VirtualNode* vnode, void* buf, size_t n, off_t off)
{
    if (n % sizeof(InputEvent))
        return -EINVAL;

    size_t read = 0;
    const size_t entries = n / sizeof(InputEvent);
    LastReadInfo* info = vnode->data;

    while (read < entries)
    {
        size_t tmpidx = info->idx;
        InputEvent* ev = ps2kbd_earliest_event(&info->time, &tmpidx);
        if (!ev)
        {
            procm_sched_yield();
            continue;
        }

        info->time = ev->time;
        info->idx = tmpidx;
        user_copy_to(buf, ev, sizeof(InputEvent));
        ++read;
    }

    return n;
}

static ssize_t ps2kbd_vnode_write(
    VirtualNode* vnode, const void* _USERPTR buf, size_t n, off_t off)
{
    return -EINVAL;
}

static int ps2kbd_vnode_ioctl(VirtualNode* vnode, int req, void* data)
{
    return -EINVAL;
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

    int st = input_register(
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
