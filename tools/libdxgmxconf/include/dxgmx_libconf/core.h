/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
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

struct ConfigTree;

struct ConfigInteractable
{
    virtual ~ConfigInteractable() = default;

    /* If this is set the Interactable will define this macro once it's
     * selected. In case of menus/radios, they need to be selected first before
     * they can be expanded */
    std::string name;

    std::string title;
    std::string description;

    std::string visible_rule;
    bool visible = true;

    bool set = false;
    std::string value;

    ConfigOptionType input_type = CONFIG_OPTION_BOOL;
    std::vector<std::string> implies;
    std::vector<std::string> modules;

    ConfigInteractableType type = CONFIG_BASE;

    virtual int define(ConfigTree& tree);
    virtual int define(const std::string& value, ConfigTree& tree);
    virtual int undefine(ConfigTree& tree);
};

struct ConfigOption : ConfigInteractable
{
    ConfigOption();

    int define(ConfigTree& tree) override;
    int define(const std::string& value, ConfigTree& tree) override;
    int undefine(ConfigTree& tree) override;
};

struct ConfigMenu : ConfigInteractable
{
    ConfigMenu();

    ConfigInteractable* selected_inter();

    int set_next_element();
    int set_prev_element();

    int set_first_element();
    int set_last_element();

    bool needs_selecting();

    int define(ConfigTree& tree) override;
    int define(const std::string& value, ConfigTree& tree) override;
    int undefine(ConfigTree& tree) override;

    size_t visible_elements_count() const;

    std::vector<std::unique_ptr<ConfigInteractable>> interactables;
    ssize_t sel_idx = -1;
};

struct ConfigTree
{
    ConfigMenu root_menu;
    std::unordered_map<std::string, ConfigInteractable*> options_map;

    int build_from_path(const std::string& path);
    int load_options_from_conf_file(const std::string& conf_path);
    int save_to_gnumake_file(const std::string& conf_path);
};

int conf_engine_generate_gnumake_defs(
    const std::string& out_path, ConfigMenu* root);
