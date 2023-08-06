/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

struct SyscallDef
{
    size_t idx;
    std::string ret;
    std::string name;
    std::vector<std::string> args;
};

static std::string str_to_upper(const std::string& str)
{
    std::string ret = "";
    for (char c : str)
        ret += toupper(c);
    return ret;
}

static void help()
{
    std::cout << "syscalls_generate [options]\n\n";
    std::cout << "Generate dxgmx syscalls based on the given .defs\n\n";
    std::cout << "options: \n";
    std::cout << "  --common-defs <path>    Path to common .defs file.\n";
    std::cout
        << "  --output      <path>    Path where to output the generated syscalls.\n";
    std::cout << "  -h,--help               Print this message and exit.\n";
}

static bool is_line_empty(const std::string& line)
{
    bool line_empty = true;
    for (size_t i = 0; i < line.size(); ++i)
    {
        if (isalnum(line[i]))
        {
            line_empty = false;
            break;
        }
    }
    return line_empty;
}

static void
parse_defs_file(const std::string& line, std::vector<SyscallDef>& defs)
{
    if (!line.size())
        return;

    if (is_line_empty(line))
        return;

    if (line[0] == '#')
        return;

    SyscallDef def = {.idx = defs.size()};
    size_t section = 0;

    // FIXME: do validation
    for (char c : line)
    {
        if (c == '\t')
        {
            ++section;
            if (section > 1)
                def.args.push_back("");
            continue;
        }

        switch (section)
        {
        case 0:
            def.ret += c;
            break;

        case 1:
            def.name += c;
            break;

        default:
            def.args[def.args.size() - 1] += c;
            break;
        }
    }
    defs.emplace_back(def);
}

static void print_output_header(std::ofstream& of)
{
    of << "/**\n * Copyright dxgmx team. \n * Distributed under the MIT license. \n */\n\n";
    of << "// This file was auto-generated using the dxgmx syscall generator tool. EDIT AT YOUR OWN DISCRETION.\n\n";
    of << "#ifndef _DXMGX_SYSCALL_DEFS_H\n";
    of << "#define _DXMGX_SYSCALL_DEFS_H\n\n";
}

static std::string generate_syscall_def_macro(const SyscallDef& def)
{
    std::string def_macro = "";

    if (def.ret == "void")
        def_macro += "SYSCALL_VOID_";
    else
        def_macro += "SYSCALL_RETV_";

    def_macro += std::to_string(def.args.size()) + '(';
    if (def.ret != "void")
        def_macro += def.ret + ", ";

    def_macro += def.name + (def.args.size() > 0 ? ", " : "");

    for (size_t i = 0; i < def.args.size(); ++i)
        def_macro += def.args[i] + (i < def.args.size() - 1 ? ", " : "");

    def_macro += ");";

    return def_macro;
}

static void
print_output_content(std::vector<SyscallDef>& syscall_defs, std::ofstream& of)
{
    for (const SyscallDef& def : syscall_defs)
    {
        std::string defname = str_to_upper(def.name);
        of << "#define " << defname << ' ' << def.idx << '\n';
    }

    of << "\n#ifdef _KERNEL\n\n";
    of << "#include <dxgmx/compiler_attrs.h>\n";
    of << "#include <dxgmx/syscall_types.h>\n\n";
    // Include notice
    of << "/* This file is included once in the kernel in kernel/syscalls.c. DO NOT\n * include this file anywhere else in the kernel. Userspace is free to do\n * whatever. */\n\n";

    for (const SyscallDef& def : syscall_defs)
        of << generate_syscall_def_macro(def) << '\n';

    of << "\n#endif // _KERNEL\n";
}

static void print_output_footer(std::ofstream& of)
{
    of << "\n#endif // !_DXMGX_SYSCALL_DEFS_H\n";
}

int main(int argc, char** argv)
{
    std::string common_defs = "";
    std::string output = "";

    for (size_t i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--common-defs") == 0 && i < argc - 1)
        {
            common_defs = std::string(argv[++i]);
        }
        else if (strcmp(argv[i], "--output") == 0 && i < argc - 1)
        {
            output = std::string(argv[++i]);
        }
        else if (strcmp(argv[i], "--h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            help();
            return 0;
        }
        else
        {
            help();
            std::cout << "\n";
            std::cout << "Unknown argument '" << argv[i] << "'.\n";
            return 1;
        }
    }

    if (common_defs == "")
    {
        help();
        std::cout << "\n --common-defs cannot be empty.\n";
        return 1;
    }

    if (output == "")
    {
        help();
        std::cout << "\n --output cannot be empty.\n";
        return 1;
    }

    std::vector<SyscallDef> syscall_defs;
    std::ifstream common_defs_file(common_defs);
    if (!common_defs_file.is_open())
    {
        std::cout << "Could not open '" << common_defs << "'.\n";
        return 1;
    }

    std::string line;
    while (std::getline(common_defs_file, line))
        parse_defs_file(line, syscall_defs);

    common_defs_file.close();

    std::ofstream output_file(output);
    if (!output_file.is_open())
    {
        std::cout << "Could not open '" << output << "'.\n";
        return 1;
    }

    print_output_header(output_file);
    print_output_content(syscall_defs, output_file);
    print_output_footer(output_file);
    output_file.close();
    return 0;
}
