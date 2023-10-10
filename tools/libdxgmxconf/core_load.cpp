/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <algorithm>
#include <dxgmx_libconf/core.h>
#include <fstream>
#include <iostream>
#include <unordered_map>

static int conf_engine_traverse_menu_and_load(
    std::unordered_map<std::string, std::string>& conf_map, ConfigMenu* menu)
{
    for (size_t i = 0; i < menu->interactables.size(); ++i)
    {
        ConfigInteractable* inter = menu->interactables[i].get();
        switch (inter->type)
        {
        case CONFIG_OPTION:
        {
            ConfigOption* conf_opt = dynamic_cast<ConfigOption*>(inter);
            auto it = conf_map.find(conf_opt->name);
            if (it == conf_map.end())
                break;

            if (conf_opt->input_type == CONFIG_OPTION_BOOL)
            {
                conf_opt->set = it->second == "y";
            }
            else if (conf_opt->input_type == CONFIG_OPTION_INPUT)
            {
                conf_opt->set = true;
                conf_opt->value = it->second;
            }
            break;
        }

        case CONFIG_MENU:
            conf_engine_traverse_menu_and_load(
                conf_map, dynamic_cast<ConfigMenu*>(inter));
            break;

        default:
        case CONFIG_BASE:
            std::cout << "Bad Interactable type " << inter->type << '\n';
            exit(1);
        }
    }

    return 0;
}

int conf_engine_parse_conf_file(
    const std::string& config_path, ConfigMenu* menu)
{
    std::ifstream file(config_path);
    if (!file.is_open())
    {
        std::cout << "Couldn't open \"" << config_path << "\" for reading!\n";
        return -2;
    }

    std::unordered_map<std::string, std::string> conf_keys_map;
    std::string line;
    size_t line_num = 1;
    while (std::getline(file, line))
    {
        line.erase(
            std::remove_if(line.begin(), line.end(), isspace), line.end());

        if (!line.size())
            continue;

        if (line[0] == '#')
            continue;

        size_t eq = line.find('=');
        if (eq == line.npos)
        {
            std::cout << "Found bad option at line " << 4 + line_num << "!\n";
            return -1;
        }

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        if (!key.size())
        {
            std::cout << "Found bad key at line" << 4 + line_num << "!\n";
            return -1;
        }

        if (!value.size())
        {
            std::cout << "Found bad value for key " << key << '\n';
            return -1;
        }

        conf_keys_map.insert({key, value});
        ++line_num;
    }

    return conf_engine_traverse_menu_and_load(conf_keys_map, menu);
}
