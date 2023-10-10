/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx_libconf/core.h>
#include <iostream>

static ConfigOption*
conf_engine_find_option_by_name(ConfigMenu* root_menu, const std::string& name)
{
    for (size_t i = 0; i < root_menu->interactables.size(); ++i)
    {
        ConfigInteractable* inter = root_menu->interactables[i].get();
        switch (inter->type)
        {
        case CONFIG_OPTION:
        {
            ConfigOption* conf_opt = dynamic_cast<ConfigOption*>(inter);
            if (conf_opt->name == name)
                return conf_opt;

            break;
        }

        case CONFIG_MENU:
            return conf_engine_find_option_by_name(
                dynamic_cast<ConfigMenu*>(inter), name);

        default:
            std::cout << "Bad Interactable type " << inter->type << '\n';
            exit(1);
        }
    }

    return nullptr;
}

static int conf_engine_resolve_implies(
    ConfigMenu* menu, const std::vector<std::string>& implies)
{
    for (const std::string& rule : implies)
    {
        if (rule.find_first_of("=") != std::string::npos)
        {
            std::string key = rule.substr(0, rule.find_first_of(' '));
            std::string value = rule.substr(rule.find_last_of(' ') + 1);

            if (!key.size())
            {
                std::cout << "Found bad key!\n";
                return -1;
            }

            if (!value.size())
            {
                std::cout << "Found bad value for key " << key << '\n';
                return -1;
            }

            ConfigOption* found = conf_engine_find_option_by_name(menu, key);
            if (!found)
            {
                std::cout << "Couldn't find implied option \"" << key
                          << "\".\n";
                return -1;
            }

            found->set = true;
            found->value = value;
            conf_engine_resolve_implies(menu, found->implies);
        }
        else
        {
            ConfigOption* found = conf_engine_find_option_by_name(menu, rule);
            if (!found)
            {
                std::cout << "Couldn't find implied option \"" << rule << "\".";
                return -1;
            }

            found->set = true;
            conf_engine_resolve_implies(menu, found->implies);
        }
    }

    return 0;
}

int conf_engine_define_option(ConfigMenu* root_menu, ConfigOption& opt)
{
    opt.set = true;
    return conf_engine_resolve_implies(root_menu, opt.implies);
}

int conf_engine_undefine_option(ConfigMenu* root_menu, ConfigOption& opt)
{
    opt.set = false;
    /* FIXME: remove implied rules  */
    return 0;
}

int conf_engine_set_option(
    ConfigMenu* root_menu, const std::string& val, ConfigOption& opt)
{
    opt.set = true;
    opt.value = val;
    return conf_engine_resolve_implies(root_menu, opt.implies);
}

int conf_engine_unset_option(ConfigMenu* root_menu, ConfigOption& opt)
{
    opt.set = false;
    opt.value.clear();
    /* FIXME: remove implied rules  */
    return 0;
}
