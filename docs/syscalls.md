# Syscalls in dxgmx
The first step of implementing a syscall is to first declare it in `kernel/syscalls_common.json`.<br>
You can scour this file and figure out how it works since it's pretty simple.<br> The only things worth mentioning are that no duplicate `n` fields should exists for any two or more syscalls and all syscalls should begin with `sys_`.<br>
Once declared in there the next step is to actually implement your syscall. Go to a suitable file and type out a function following the declaration in the json file. That's it, if you messed up the arguments, return type or even forgot to compile the function the compiler/linker will let you know.<br>
The sycall can now be called from userspace using it's `n` value.<br>
