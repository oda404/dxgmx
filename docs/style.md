# dxgmx coding style & structure
## clang-format
If you're not using clang format, install it and feed it the `.clang-format` in the root of the repo.
## Project structure
### Generic code
All platform agnostic code is to be put in `kernel/` and the headers in `include/`.

### Drivers/modules
All drivers/modules are to be put in `drivers/` under a suitable subdirectory. All modules should name their makefile `module.mk`. If a module has headers it wants to make public to other modules, it should create it's own `include/dxgmx/` dir in it's own directory and store them there. For example:<br>
The module in `drivers/video/fb/` has it's headers under `drivers/video/fb/include/dxgmx/`.<br>
Check out other modules on how they actually export their headers and compile their objects for more details.

### Architecture specific code
All architecture specific code is to be put under `arch/<name>` and built selectively using configurations.

## Naming conventions
The dxgmx naming convetions are as follows:
- Global, local or constant variables are lower snake case
- Functions are also lower snake case
- Macros of any kind are **ALWAYS** upper case with words separated by underscores
- No type information of any kind is to be used in the name of variables
- Structs are **defined** using PascalCase and always typedef'd. Instances still follow the regular variable rules
- Include guards always use the path of the file from the source root prefixed with an underscore. Example: `include/dxgmx/utils/hashtable.h` is `_DXGMX_UTILS_HASHTABLE_H`
- Variables that are global to a compilation unit are prefixed with `g_`
- Architecture specific core functions should end in `_arch`. Example: `mm_load_paging_struct_arch`. They should also not be declared in any header, the compilation unit that uses them will forward declare them as extern
- If trying to avoid name collisions, for example in a macro, a variable should be prefixed with a single underscore. 
- If a variable has external linkage (which should really be avoided at all costs) it should be prefixed with triple underscores. Example: `___kboot_info`
- Double underscores should not be used
