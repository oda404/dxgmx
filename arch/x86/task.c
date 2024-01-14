/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/compiler_attrs.h>
#include <dxgmx/task/task.h>
#include <dxgmx/x86/gdt.h>

/* Switch between two tasks, this code was written with help from
 * https://wiki.osdev.org/Kernel_Multitasking, and also digging
 * through the linux sources (arch/x86/entry/entry_32.S
 * __switch_to_asm) */

/* This function only works if/because the following 2 conditins are
 * met: 1 - A task is running in kernel-space. The only way a
 * process is able to yield or be preempted is for it to switch to
 * kernel mode first, where it has it's own kernel stack. This way,
 * it's context is saved on it's stack that no other process uses.
 * When it is to run again, we just need to switch back to the stack
 * pointer it was using previously and pop everything that is just
 * sitting there.
 *
 *  2 - The task we are switching to should have already ran this
 * code before. Switching to another task pops that task's info off
 * of it's stack. If the task never pushed it's info on the stack,
 * running this exact code would not only not work, but it would
 * also be dangerous because we're doing ret with a random address.
 * While there are hoops we could jump through to make this work for
 * such cases, I think it's way nicer to just have a separate
 * function for this, proc_enter_initial().
 */
#define I686_TASK_SWITCH(_esp0_off)                                            \
    __asm__ volatile("push %ebx                        \n"                     \
                     "push %esi                        \n"                     \
                     "push %edi                        \n"                     \
                     "push %ebp                        \n"                     \
                     "pushfl                           \n"                     \
                     "                                 \n"                     \
                     "mov 0x18(%esp), %ebx            \n"                      \
                     "mov %esp, " #_esp0_off "(%ebx)   \n"                     \
                     "                                 \n"                     \
                     "mov 0x1C(%esp), %ebx            \n"                      \
                     "mov " #_esp0_off "(%ebx), %esp   \n"                     \
                     "                                 \n"                     \
                     "popfl                            \n"                     \
                     "pop %ebp                         \n"                     \
                     "pop %edi                         \n"                     \
                     "pop %esi                         \n"                     \
                     "pop %ebx                         \n"                     \
                     "ret                              \n")

#define X86_64_TASK_SWITCH(_rsp0_off)                                          \
    __asm__ volatile("push %rbx                        \n"                     \
                     "push %rbp                        \n"                     \
                     "push %r12                        \n"                     \
                     "push %r13                        \n"                     \
                     "push %r14                        \n"                     \
                     "push %r15                        \n"                     \
                     "pushfq                           \n"                     \
                     "                                 \n"                     \
                     "mov %rsp, " #_rsp0_off "(%rdi)   \n"                     \
                     "                                 \n"                     \
                     "mov " #_rsp0_off "(%rsi), %rsp   \n"                     \
                     "                                 \n"                     \
                     "popfq                            \n"                     \
                     "pop %r15                         \n"                     \
                     "pop %r14                         \n"                     \
                     "pop %r13                         \n"                     \
                     "pop %r12                         \n"                     \
                     "pop %rbp                         \n"                     \
                     "pop %rbx                         \n"                     \
                     "ret                              \n")

int task_set_impending_stack_top(ptr sp)
{
    tss_set_esp0(sp);
    return 0;
}

/* God I love the C pre-processor >:( */
STATIC_ASSERT(OFFSETOF(TaskContext, stack_ptr) == 0, "Bad TaskContext offset");

_ATTR_NAKED _CDECL void task_switch(TaskContext* prevctx, TaskContext* nextctx)
{
#ifdef CONFIG_64BIT
    X86_64_TASK_SWITCH(0);
#else
    I686_TASK_SWITCH(0);
#endif
}
