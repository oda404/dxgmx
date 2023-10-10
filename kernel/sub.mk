
KERNELOBJS += \
kernel/kabort.c.o \
kernel/klog.c.o \
kernel/kinit_stage2.c.o \
kernel/ksyms.c.o \
kernel/kimg.c.o \
kernel/module.c.o \
kernel/panic.c.o \
kernel/user.c.o \
kernel/kstdio.c.o \
kernel/kboot.c.o \

include kernel/mem/sub.mk
include kernel/utils/sub.mk
include kernel/crypto/sub.mk
include kernel/kstdlib/sub.mk
include kernel/storage/sub.mk
include kernel/fs/sub.mk
include kernel/elf/sub.mk
include kernel/proc/sub.mk
include kernel/syscalls/sub.mk
include kernel/sched/sub.mk
include kernel/time/sub.mk
