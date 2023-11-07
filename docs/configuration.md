# dxgmx configuration
If you've ever configured linux or better, know how the linux configuration works internally you'll probably find the dxgmx configuration pretty familiar feeling (with some twists of course).<br>

## Configuring the kernel
An ncurses app can be used to configure the kernel. After exporting `HOSTCC` and `HOSTCXX` to a C and C++ compiler, respectively, run:
```
$ make conf
```
Look around the options and follow the instructions.

## Cofiguration internals
### Defining & tweaking config options
Configuration options are defined in json files called `config_menu.json`, the only exception is the `root_config_menu.json`. If you want to know how they work, just dig through them a little bit, they're not that complex.

### Config stages
The configuration process has two *stages*:
- The GNU make stage
- The C stage

Configuring the kernel using libconfig will spit out a file called `config.mk`. This file contains first, a list of variables and second, a list of module includes. This way, any included modules have access to any defined variables. The root makefile includes all the specified modules using their `module.mk` and cobbles together the final list of objects to build. The root makefile may also use the defined variables for more *generic* configuration like the optimization level, or retaining source level debug info.

Before starting to compile all of the objects, a file called `kconfig.h` will get generated from the aforementioned `config.mk`. This file only contains the *variable definitions* part of the config.mk, translated to C and can be used in C code in order to check for anything of interest.

### Config engine problems
- The UI fucking sucks
- No radio-input-like support
- No config option dependency relations are being kept. For example: if CONFIG_X86_64 is selected, it's also going to automatically select CONFIG_64BIT. But if we deselect CONFIG_X86_64, CONFIG_64BIT is going to stay. Now, in this case CONFIG_64BIT could probably get automatically deselected but in other more nuanced case like when CONFIG_64BIT could be an option that the user may select on it's own independent of CONFIG_X86_64, we can't just deselect it.
- The ncurses utility always asks if you would like to save even if no changes were made.
- A lot of logical operations could be added to config menus.
- We depend on jsoncpp for json parsing and while it's a great library it kind of upsets me.