
#include<dxgmx/klog.h>
#include<dxgmx/string.h>
#include<dxgmx/kprintf.h>

static KLogConfig g_klogconfig;

int klog_init(const KLogConfig *kconfig)
{
    if(!kconfig)
    {
        g_klogconfig.loglevel = KLOG_INFO;
        return 1;
    }

    g_klogconfig = *kconfig;
    return 0;
}

int klog_set_max_level(uint8_t lvl)
{
    if(lvl > KLOG_QUIET)
        lvl = KLOG_QUIET;
    g_klogconfig.loglevel = lvl;
    return lvl;
}

size_t klog(uint8_t lvl, const char *fmt, ...)
{
    if(lvl > g_klogconfig.loglevel)
        return 0;

    size_t written = 0;

    switch(lvl)
    {
    case KLOG_INFO:
        written += kprintf("[INFO] ");
        break;

    case KLOG_WARN:
        written += kprintf("[WARN] ");
        break;

    case KLOG_ERR:
        written += kprintf("[ERR] ");
        break;

    case KLOG_FATAL:
        written += kprintf("[FATAL] ");
        break;

    default:
        return 0;
    }

    va_list valist;
    va_start(valist, fmt);
    written += vkprintf(fmt, valist);
    va_end(valist);

    return written;
}
