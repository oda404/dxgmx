/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <algorithm>
#include <dxgmx_libconf/core.h>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <unordered_map>

static int conf_engine_parse_option(
    const Json::Value& option,
    std::vector<std::unique_ptr<ConfigInteractable>>& interactables)
{
    std::unique_ptr<ConfigOption> config_option =
        std::make_unique<ConfigOption>();

    if (option["name"].isString())
    {
        config_option->name = option["name"].asString();
    }
    else
    {
        std::cout << "Bad or missing name for option!\n";
        return -1;
    }

    if (option["title"].isString())
    {
        config_option->title = option["title"].asString();
    }
    else
    {
        std::cout << "Bad or missing title for option \"" << config_option->name
                  << "\"\n";
        return -1;
    }

    if (option["description"].isString())
    {
        config_option->description = option["description"].asString();
    }
    else if (!option["description"].isNull())
    {
        std::cout << "Bad description for option \"" << config_option->name
                  << "\"\n";
        return -1;
    }

    if (option["view_only"].isString())
    {
        config_option->view_only_rule = option["view_only"].asString();
    }
    else if (option["view_only"].isBool())
    {
        config_option->view_only = option["view_only"].asBool();
    }
    else if (!option["view_only"].isNull())
    {
        std::cout << "Bad view_only for option \"" << config_option->name
                  << "\"\n";
        return -1;
    }

    if (option["type"].isString())
    {
        std::string type = option["type"].asString();
        if (type == "input")
        {
            config_option->input_type = CONFIG_OPTION_INPUT;
        }
        else if (type == "bool")
        {
            config_option->input_type = CONFIG_OPTION_BOOL;
        }
        else
        {
            std::cout << "Bad type value for option \"" << config_option->name
                      << "\"\n";
            return -1;
        }
    }
    else if (!option["type"].isNull())
    {
        std::cout << "Bad type for option \"" << config_option->name << "\"\n";
        return -1;
    }

    if (option["implies"].isArray())
    {
        size_t size = option["implies"].size();
        for (Json::Value::ArrayIndex i = 0; i < size; ++i)
        {
            if (option["implies"][i].isString())
            {
                config_option->implies.emplace_back(
                    std::move(option["implies"][i].asString()));
            }
            else
            {
                std::cout << "Bad implied for option \"" << config_option->name
                          << "\"\n";
                return -1;
            }
        }
    }
    else if (!option["implies"].isNull())
    {
        std::cout << "Bad implies for option \"" << config_option->name
                  << "\"\n";
        return -1;
    }

    interactables.push_back(std::move(config_option));
    return 0;
}

int conf_engine_load_menus_recursive(
    const std::string& menu_path, ConfigMenu* menu)
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

    ConfigMenu tmp_menu;
    if (root["title"].isString())
    {
        tmp_menu.title = root["title"].asString();
    }
    else
    {
        std::cout << "Bad or missing title for a menu in \"" << menu_path
                  << "\"\n";
        return -1;
    }

    if (root["description"].isString())
    {
        tmp_menu.description = root["description"].asString();
    }
    else if (!root["description"].isNull())
    {
        std::cout << "Bad description for menu \"" << tmp_menu.title
                  << "\" in \"" << menu_path << "\".\n";
        return -1;
    }

    if (root["view_only"].isString())
    {
        tmp_menu.view_only_rule = root["view_only"].asString();
    }
    else if (root["view_only"].isBool())
    {
        tmp_menu.view_only = root["view_only"].asBool();
    }
    else if (!root["view_only"].isNull())
    {
        std::cout << "Bad view_only for menu \"" << tmp_menu.title << "\" in \""
                  << menu_path << "\".\n";
        return -1;
    }

    if (root["options"].isArray())
    {
        size_t size = root["options"].size();
        for (Json::Value::ArrayIndex i = 0; i < size; ++i)
        {
            int st = conf_engine_parse_option(
                root["options"][i], tmp_menu.interactables);
            if (st < 0)
                return st;
        }
    }
    else if (!root["options"].isNull())
    {
        std::cout << "Bad options for menu \"" << tmp_menu.title << "\" in \""
                  << menu_path << "\".\n";
        return -1;
    }

    if (root["menus"].isArray())
    {
        // i love recursion >:(
        size_t size = root["menus"].size();
        for (Json::Value::ArrayIndex i = 0; i < size; ++i)
        {
            std::unique_ptr<ConfigMenu> new_menu =
                std::make_unique<ConfigMenu>();

            if (conf_engine_load_menus_recursive(
                    root["menus"][i].asString(), new_menu.get()) < 0)
                return -1;

            tmp_menu.interactables.push_back(std::move(new_menu));
        }
    }
    else if (!root["menus"].isNull())
    {
        std::cout << "Bad menus for menu \"" << tmp_menu.title << "\" in \""
                  << menu_path << "\".\n";
        return -1;
    }

    *menu = std::move(tmp_menu);
    return 0;
}

static int conf_engine_populate_options_and_implieds(
    std::vector<std::string>& implieds,
    std::unordered_map<std::string, bool>& options,
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
            options.insert({conf_opt->name, true});

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
                    implieds.push_back(tmp_rule);
                    continue;
                }

                implieds.push_back(rule);
            }
            break;
        }

        case CONFIG_MENU:
            conf_engine_populate_options_and_implieds(
                implieds, options, dynamic_cast<ConfigMenu*>(inter));
            break;

        default:
        case CONFIG_BASE:
            std::cout << "Bad Interactable type " << inter->type << '\n';
            exit(1);
        }
    }

    return 0;
}

int conf_engine_validate_implied(const ConfigMenu* root_menu)
{
    std::vector<std::string> all_implieds;
    std::unordered_map<std::string, bool> all_options;

    conf_engine_populate_options_and_implieds(
        all_implieds, all_options, root_menu);

    for (const std::string& rule : all_implieds)
    {
        auto it = all_options.find(rule);
        if (it == all_options.end())
        {
            std::cout << "Implied option \"" << rule
                      << "\" wasn't previously defined as an option!\n";
            return -1;
        }
    }

    return 0;
}
