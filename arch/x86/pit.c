
#include<dxgmx/x86/pit.h>
#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/interrupts.h>
#include<dxgmx/x86/portio.h>
#include<dxgmx/attrs.h>
#include<dxgmx/klog.h>
#include<dxgmx/assert.h>
#include<dxgmx/string.h>

#define KLOGF(lvl, fmt, ...) klog(lvl, "pit: " fmt, ##__VA_ARGS__)

#define PIT_BASE_FREQ_HZ 1193181

#define PIT_CHANNEL0_PORT 0x40
#define PIT_CHANNEL1_PORT 0x41
#define PIT_CHANNEL2_PORT 0x42
#define PIT_COMMAND_PORT  0x43

#define PIT_CHANNEL0 (0b0 << 6)
#define PIT_CHANNEL1 (0b1 << 6)
#define PIT_CHANNEL2 (0b10 << 6)
#define PIT_READBACK (0b11 << 6)

#define PIT_LATCH_COUNT (0b0 << 4)
#define PIT_ACCESS_LO   (0b1 << 4)
#define PIT_ACCESS_HI   (0b10 << 4)
#define PIT_ACCESS_LOHI (0b11 << 4)

#define PIT_OPMODE_INT_ON_TERM_CNT (0b000 << 1)
#define PIT_OPMODE_ONE_SHOT        (0b001 << 1)
#define PIT_OPMODE_RATE_GEN        (0b010 << 1)
#define PIT_OPMODE_SQUARE_WAVE_GEN (0b011 << 1)
#define PIT_OPMODE_SOFTW_STROBE    (0b100 << 1)
#define PIT_OPMODE_HARDW_STROBE    (0b101 << 1)

#define PIT_MODE_BIN 0
#define PIT_MODE_BCD 1

static volatile struct timespec 
g_periodic_int_timespec; 
static bool g_periodic_ints_enabled = false;
static float g_freq = 0;

static void pit_isr(
    const InterruptFrame _ATTR_UNUSED *frame,
    const void _ATTR_UNUSED *data
)
{
    ASSERT(!data);

    g_periodic_int_timespec.tv_nsec += 1.f / g_freq * 1000000000;
    while(g_periodic_int_timespec.tv_nsec >= 1000000000)
    {
        ++g_periodic_int_timespec.tv_sec;
        g_periodic_int_timespec.tv_nsec -= 1000000000;
    }
}

int pit_init()
{
    return 0;
}

void pit_enable_periodic_int()
{
    const u16 freqdiv = 2;

    port_outb(
        (PIT_MODE_BIN | PIT_OPMODE_SQUARE_WAVE_GEN | PIT_ACCESS_LOHI | PIT_CHANNEL0),
        PIT_COMMAND_PORT
    );
    /* lo */
    port_outb((u8)freqdiv, PIT_CHANNEL0_PORT);
    /* hi */
    port_outb(freqdiv >> 8, PIT_CHANNEL0_PORT);

    g_freq = PIT_BASE_FREQ_HZ / freqdiv;
    idt_register_isr(IRQ0, pit_isr);

    g_periodic_ints_enabled = true;
}

void pit_disable_periodic_int()
{
    g_periodic_ints_enabled = false;
}

bool pit_periodic_ints_enabled()
{
    return g_periodic_ints_enabled;
}

struct timespec pit_now()
{
    return g_periodic_int_timespec;
}
