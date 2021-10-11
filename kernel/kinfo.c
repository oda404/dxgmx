
#include<dxgmx/kinfo.h>

extern const ptr _sym_kbase;
extern const ptr _sym_kend;

ptr kinfo_get_kbase()
{
    return (ptr)&_sym_kbase;
}

ptr kinfo_get_kend()
{
    return (ptr)&_sym_kend;
}

ptr kinfo_get_kstack_top()
{
    return kstack_get_top();
}

ptr kinfo_get_kstack_bot()
{
    return kstack_get_bot();
}
