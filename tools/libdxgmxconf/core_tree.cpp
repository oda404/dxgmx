/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <algorithm>
#include <dxgmx_libconf/core.h>
#include <fstream>
#include <iostream>
#include <json/json.h>

static int conf_engine_parse_common(
    const Json::Value& option,
    ConfigInteractable* inter,
    ConfigInteractableType type)
{
    std::string type_str;
    if (type == CONFIG_OPTION)
        type_str = "option";
    else if (type == CONFIG_MENU)
        type_str = "menu";
    else
        std::cout << "Unkown type to str @conf_engine_parse_common()!";

    if (option["name"].isString())
    {
        inter->name = option["name"].asString();
    }
    else if (type == CONFIG_OPTION)
    {
        std::cout << "Bad or missing name for " << type_str << "!\n";
        return -1;
    }

    if (option["title"].isString())
    {
        inter->title = option["title"].asString();
    }
    else
    {
        std::cout << "Bad or missing title for " << type_str << " \""
                  << inter->name << "\"\n";
        return -1;
    }

    if (option["description"].isString())
    {
        inter->description = option["description"].asString();
    }
    else if (!option["description"].isNull())
    {
        std::cout << "Bad description for " << type_str << " \"" << inter->name
                  << "\"\n";
        return -1;
    }

    if (option["visible"].isString())
    {
        inter->visible_rule = option["visible"].asString();
    }
    else if (option["visible"].isBool())
    {
        inter->visible = option["visible"].asBool();
    }
    else if (!option["visible"].isNull())
    {
        std::cout << "Bad visible for " << type_str << " \"" << inter->name
                  << "\"\n";
        return -1;
    }

    if (option["type"].isString())
    {
        std::string type = option["type"].asString();
        if (type == "input")
        {
            inter->input_type = CONFIG_OPTION_INPUT;
        }
        else if (type == "bool")
        {
            inter->input_type = CONFIG_OPTION_BOOL;
        }
        else
        {
            std::cout << "Bad type value for " << type_str << " \""
                      << inter->name << "\"\n";
            return -1;
        }
    }
    else if (!option["type"].isNull())
    {
        std::cout << "Bad type for " << type_str << " \"" << inter->name
                  << "\"\n";
        return -1;
    }

    if (option["implies"].isArray())
    {
        size_t size = option["implies"].size();
        for (Json::Value::ArrayIndex i = 0; i < size; ++i)
        {
            const Json::Value& impl = option["implies"][i];
            if (!impl.isString())
            {
                std::cout << "Bad implied for " << type_str << " \""
                          << inter->name << "\"\n";
                return -1;
            }

            inter->implies.emplace_back(std::move(impl.asString()));
        }
    }
    else if (!option["implies"].isNull())
    {
        std::cout << "Bad implies for " << type_str << " \"" << inter->name
                  << "\"\n";
        return -1;
    }

    if (option["modules"].isArray())
    {
        size_t size = option["modules"].size();
        for (Json::Value::ArrayIndex i = 0; i < size; ++i)
        {
            const Json::Value& mod = option["modules"][i];
            if (!mod.isString())
            {
                std::cout << "Bad module for " << type_str << " \""
                          << inter->name << "\"\n";
                return -1;
            }

            inter->modules.emplace_back(std::move(mod.asString()));
        }
    }
    else if (!option["modules"].isNull())
    {
        std::cout << "Bad modules for " << type_str << " \"" << inter->name
                  << "\"\n";
        return -1;
    }

    return 0;
}

static int conf_engine_parse_option(
    const Json::Value& option,
    std::vector<std::unique_ptr<ConfigInteractable>>& interactables)
{
    std::unique_ptr<ConfigOption> config_option =
        std::make_unique<ConfigOption>();

    if (conf_engine_parse_common(option, config_option.get(), CONFIG_OPTION) <
        0)
        return -1;

    interactables.push_back(std::move(config_option));
    return 0;
}

static int conf_engine_parse_menu_from_file(
    const std::string& menu_path, ConfigMenu* menu);

static int
conf_engine_parse_menu(const Json::Value& menu, ConfigMenu* conf_menu)
{
    ConfigMenu tmp_menu;

    if (conf_engine_parse_common(menu, &tmp_menu, CONFIG_MENU) < 0)
        return -1;

    if (menu["options"].isArray())
    {
        size_t size = menu["options"].size();
        for (Json::Value::ArrayIndex i = 0; i < size; ++i)
        {
            int st = conf_engine_parse_option(
                menu["options"][i], tmp_menu.interactables);
            if (st < 0)
                return st;
        }
    }
    else if (!menu["options"].isNull())
    {
        std::cout << "Bad options for menu \"" << tmp_menu.title << "\"\n";
        return -1;
    }

    if (menu["menus"].isArray())
    {
        // i love recursion >:(
        size_t size = menu["menus"].size();
        for (Json::Value::ArrayIndex i = 0; i < size; ++i)
        {
            auto& json_menu = menu["menus"][i];

            std::unique_ptr<ConfigMenu> new_menu =
                std::make_unique<ConfigMenu>();

            if (json_menu.isString())
            {
                if (conf_engine_parse_menu_from_file(
                        json_menu.asString(), new_menu.get()) < 0)
                    return -1;
            }
            else if (json_menu.isObject())
            {
                if (conf_engine_parse_menu(json_menu, new_menu.get()) < 0)
                    return -1;
            }
            else
            {
                std::cout << "Bad menu entry for menu \"" << tmp_menu.title
                          << "\"\n";
                return -1;
            }

            tmp_menu.interactables.push_back(std::move(new_menu));
        }
    }
    else if (!menu["menus"].isNull())
    {
        std::cout << "Bad menus for menu \"" << tmp_menu.title << "\"\n";
        return -1;
    }

    *conf_menu = std::move(tmp_menu);
    return 0;
}

static int
conf_engine_parse_menu_from_file(const std::string& menu_path, ConfigMenu* menu)
{
    std::ifstream file(menu_path);
    if (!file.is_open())
    {
        std::cout << "Couldn't open \"" << menu_path << "\" for reading!\n";
        return -1;
    }

    Json::Value root;
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING errs;

    if (!Json::parseFromStream(builder, file, &root, &errs))
    {
        std::cout << errs << '\n';
        return -1;
    }

    return conf_engine_parse_menu(root, menu);
}

static int conf_engine_build_option_map_and_implies_vector(
    std::vector<std::string>& implies,
    std::unordered_map<std::string, ConfigInteractable*>& options,
    const ConfigMenu* menu)
{
    for (size_t i = 0; i < menu->interactables.size(); ++i)
    {
        ConfigInteractable* inter = menu->interactables[i].get();
        switch (inter->type)
        {
        case CONFIG_OPTION:
        {
            ConfigOption* conf_opt = dynamic_cast<ConfigOption*>(inter);
            options.insert({conf_opt->name, inter});

            for (const std::string& rule : conf_opt->implies)
            {
                size_t eq = rule.find('=');
                if (eq != rule.npos)
                {
                    std::string tmp_rule = rule.substr(0, eq);
                    tmp_rule.erase(
                        std::remove_if(
                            tmp_rule.begin(), tmp_rule.end(), isspace),
                        tmp_rule.end());
                    implies.push_back(tmp_rule);
                }
                else
                {
                    implies.push_back(rule);
                }
            }
            break;
        }

        case CONFIG_MENU:
        {
            ConfigMenu* conf_menu = dynamic_cast<ConfigMenu*>(inter);
            if (conf_menu->needs_selecting())
                options.insert({conf_menu->name, inter});

            conf_engine_build_option_map_and_implies_vector(
                implies, options, dynamic_cast<ConfigMenu*>(inter));
            break;
        }

        default:
        case CONFIG_BASE:
            std::cout << "Bad Interactable type " << inter->type << '\n';
            exit(1);
        }
    }

    /* Remove dups since 2 options might imply the same option twice. */
    std::sort(implies.begin(), implies.end());
    implies.erase(std::unique(implies.begin(), implies.end()), implies.end());
    return 0;
}

int ConfigTree::build_from_path(const std::string& path)
{
    if (conf_engine_parse_menu_from_file(path, &this->root_menu) < 0)
        return -1;

    std::vector<std::string> all_implies;
    if (conf_engine_build_option_map_and_implies_vector(
            all_implies, this->options_map, &this->root_menu) < 0)
    {
        return -1;
    }

    for (const std::string& impl : all_implies)
    {
        auto it = this->options_map.find(impl);
        if (it == this->options_map.end())
        {
            std::cout << "Implied option \"" << impl
                      << "\" wasn't previously defined as an option!\n";
            return -1;
        }
    }

    return 0;
}

int ConfigTree::load_options_from_conf_file(const std::string& conf_path)
{
    std::ifstream file(conf_path);
    if (!file.is_open())
    {
        std::cout << "Couldn't open \"" << conf_path << "\" for reading!\n";
        return -2;
    }

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

        if (line.find("include") == 0)
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

        auto it = this->options_map.find(key);
        if (it == this->options_map.end())
        {
            std::cout
                << "Config file specifies " << key << " = " << value
                << " but the option was not defined when building the tree.\n";
            return -1;
        }

        ConfigInteractable* inter = it->second;
        inter->set = true;
        inter->value = value;
        ++line_num;
    }

    return 0;
}

int ConfigTree::save_to_gnumake_file(const std::string& conf_path)
{
    std::ofstream file(conf_path);
    if (!file.is_open())
    {
        std::cout << "Couldn't open \"" << conf_path << "\" for writing!\n";
        return -1;
    }

    file
        << "# This file was auto-generated using the dxgmx configuration engine.\n";
    file << "# Manually edit at your own discretion.\n\n";

    for (auto& it : this->options_map)
    {
        const ConfigInteractable* elem = it.second;
        if (!elem->set)
            continue;

        if (elem->input_type == CONFIG_OPTION_INPUT)
            file << elem->name << " = " << elem->value << '\n';
        else
            file << elem->name << " = y\n";
    }

    file << '\n';

    for (auto& it : this->options_map)
    {
        const ConfigInteractable* elem = it.second;
        if (!elem->set)
            continue;

        for (const std::string& mod : elem->modules)
            file << "include " << mod << "/module.mk\n";
    }

    return 0;
}
