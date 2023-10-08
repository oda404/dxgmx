/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

enum ConfigOptionType
{
    CONFIG_OPTION_BOOL = 1,
    CONFIG_OPTION_INPUT = 2
};

enum ConfigInteractableType
{
    CONFIG_BASE = 0,
    CONFIG_MENU = 1,
    CONFIG_OPTION = 2,
};

/**
 * 'p' prefix - Parsed from the config files.
 * 'c' prefix - Computed based on some config rules.
 * 'e' prefix - Engine convenience stuff that may be useful for frontends.
 */

struct ConfigInteractable
{
    virtual ~ConfigInteractable() = default;

    std::string name;
    std::string title;
    std::string description;

    std::string view_only_rule;
    bool view_only = false;

    ConfigInteractableType type = CONFIG_BASE;
};

struct ConfigOption : ConfigInteractable
{
    ConfigOption()
    {
        type = CONFIG_OPTION;
    }

    ConfigOptionType input_type = CONFIG_OPTION_BOOL;
    std::vector<std::string> implies;

    bool set = false;
    std::string value;
};

struct ConfigMenu : ConfigInteractable
{
    ConfigMenu()
    {
        type = CONFIG_MENU;
    }

    ConfigInteractable* selected_inter()
    {
        if (sel_idx == -1)
            return nullptr;

        return interactables[sel_idx].get();
    }

    ssize_t first_interactable_idx()
    {
        for (size_t i = 0; i < interactables.size(); ++i)
        {
            if (!interactables[i]->view_only)
                return i;
        }

        return -1;
    }

    std::vector<std::unique_ptr<ConfigInteractable>> interactables;
    ssize_t sel_idx = -1;
};

int conf_engine_load_menus_recursive(
    const std::string& menu_path, ConfigMenu* root_menu);

int conf_engine_validate_implied(const ConfigMenu* root_menu);

int conf_engine_parse_conf_file(
    const std::string& config_path, ConfigMenu* menu);

int conf_engine_define_option(ConfigMenu* root_menu, ConfigOption& opt);
int conf_engine_undefine_option(ConfigMenu* root_menu, ConfigOption& opt);
int conf_engine_set_option(
    ConfigMenu* root_menu, const std::string& val, ConfigOption& opt);
int conf_engine_unset_option(ConfigMenu* root_menu, ConfigOption& opt);

int conf_engine_generate_gnumake_defs(
    const std::string& out_path, ConfigMenu* root);
