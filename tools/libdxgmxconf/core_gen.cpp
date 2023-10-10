/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx_libconf/core.h>
#include <fstream>
#include <iostream>

static void conf_engine_generate_gnumake_defs_for_opt(
    std::ofstream& ofstream, ConfigOption& opt)
{
    if (opt.input_type == CONFIG_OPTION_BOOL)
        ofstream << opt.name << " = y\n";
    else if (opt.input_type == CONFIG_OPTION_INPUT)
        ofstream << opt.name << " = " << opt.value << '\n';
}

static void conf_engine_generate_gnumake_defs_for_menu(
    std::ofstream& ofstream, ConfigMenu* menu)
{
    for (size_t i = 0; i < menu->interactables.size(); ++i)
    {
        ConfigInteractable* inter = menu->interactables[i].get();
        switch (inter->type)
        {
        case CONFIG_OPTION:
        {
            ConfigOption* conf_opt = dynamic_cast<ConfigOption*>(inter);
            if (conf_opt->set)
                conf_engine_generate_gnumake_defs_for_opt(ofstream, *conf_opt);
            break;
        }

        case CONFIG_MENU:
            conf_engine_generate_gnumake_defs_for_menu(
                ofstream, dynamic_cast<ConfigMenu*>(inter));
            break;

        default:
        case CONFIG_BASE:
            std::cout << "Bad Interactable type " << inter->type << '\n';
            exit(1);
        }
    }
}

int conf_engine_generate_gnumake_defs(
    const std::string& out_path, ConfigMenu* root)
{
    std::ofstream file(out_path);
    if (!file.is_open())
    {
        std::cout << "Couldn't open " << out_path << " for writing!\n";
        return -1;
    }

    file << "# This file was auto-generated using the dxgmx configuration\n";
    file << "# engine, that made sure this is a valid configuration.\n";
    file << "# Edit at your own discretion.\n\n";

    conf_engine_generate_gnumake_defs_for_menu(file, root);
    return 0;
}
