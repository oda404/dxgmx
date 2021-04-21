
# libkc
This library implements various cstdlib functions.
Functions that are kernel specific and can't be used in user space are prefixed with 'k'. eg: kmalloc, kfree, kabort
Functions that can be used in userspace are prefixed with double underscores. eg: __strlen, __abs, __isdigit
