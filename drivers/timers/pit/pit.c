/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/compiler_attrs.h>
#include <dxgmx/interrupts.h>
#include <dxgmx/klog.h>
#include <dxgmx/module.h>
#include <dxgmx/string.h>
#include <dxgmx/timekeep.h>
#include <dxgmx/x86/idt.h>
#include <dxgmx/x86/pic.h>
#include <dxgmx/x86/portio.h>

#define KLOGF_PREFIX "pit: "

#define PIT_BASE_FREQ_HZ 1193181

#define PIT_CHANNEL0_PORT 0x40
#define PIT_CHANNEL1_PORT 0x41
#define PIT_CHANNEL2_PORT 0x42
#define PIT_COMMAND_PORT 0x43

#define PIT_CHANNEL0 (0b0 << 6)
#define PIT_CHANNEL1 (0b1 << 6)
#define PIT_CHANNEL2 (0b10 << 6)
#define PIT_READBACK (0b11 << 6)

#define PIT_LATCH_COUNT (0b0 << 4)
#define PIT_ACCESS_LO (0b1 << 4)
#define PIT_ACCESS_HI (0b10 << 4)
#define PIT_ACCESS_LOHI (0b11 << 4)

#define PIT_OPMODE_INT_ON_TERM_CNT (0b000 << 1)
#define PIT_OPMODE_ONE_SHOT (0b001 << 1)
#define PIT_OPMODE_RATE_GEN (0b010 << 1)
#define PIT_OPMODE_SQUARE_WAVE_GEN (0b011 << 1)
#define PIT_OPMODE_SOFTW_STROBE (0b100 << 1)
#define PIT_OPMODE_HARDW_STROBE (0b101 << 1)

#define PIT_MODE_BIN 0
#define PIT_MODE_BCD 1

static volatile struct timespec g_periodic_int_timespec;
static size_t g_freq = 0;

static void pit_isr()
{
    g_periodic_int_timespec.tv_nsec += 1.f / g_freq * 1000000000;
    while (g_periodic_int_timespec.tv_nsec >= 1000000000)
    {
        ++g_periodic_int_timespec.tv_sec;
        g_periodic_int_timespec.tv_nsec -= 1000000000;
    }

    interrupts_irq_done();
}

static void pit_enable_periodic_int()
{
    g_freq = 4096;
    const u16 freqdiv = PIT_BASE_FREQ_HZ / g_freq;

    interrupts_disable_irqs();
    interrupts_reqister_irq_isr(IRQ_PIT, pit_isr);

    port_outb(
        (PIT_MODE_BIN | PIT_OPMODE_SQUARE_WAVE_GEN | PIT_ACCESS_LOHI |
         PIT_CHANNEL0),
        PIT_COMMAND_PORT);
    /* lo */
    port_outb((u8)freqdiv, PIT_CHANNEL0_PORT);
    /* hi */
    port_outb(freqdiv >> 8, PIT_CHANNEL0_PORT);

    interrupts_enable_irqs();
}

int pit_init(TimeSource*)
{
    pit_enable_periodic_int();
    return 0;
}

static int pit_destroy(TimeSource*)
{
    return 0;
}

struct timespec pit_now()
{
    return g_periodic_int_timespec;
}

static TimeSource g_pit_timesource = {
    .name = "pit",
    .init = pit_init,
    .destroy = pit_destroy,
    .priority = 100,
    .now = pit_now};

static int pit_main()
{
    return timekeep_register_timesource(&g_pit_timesource);
}

static int pit_exit()
{
    return timekeep_unregister_timesource(&g_pit_timesource);
}

MODULE g_pit_module = {
    .name = "pit", .main = pit_main, .exit = pit_exit, .stage = MODULE_STAGE2};
