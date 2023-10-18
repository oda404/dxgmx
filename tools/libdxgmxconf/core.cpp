/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <algorithm>
#include <dxgmx_libconf/core.h>
#include <iostream>

static int conf_engine_resolve_implies(
    const std::vector<std::string>& implies, ConfigTree& conf_tree)
{
    std::string key;
    std::string value;

    for (std::string rule : implies)
    {
        rule.erase(
            std::remove_if(rule.begin(), rule.end(), isspace), rule.end());

        size_t eq = rule.find_first_of("=");
        if (eq != std::string::npos)
        {
            key = rule.substr(0, eq);
            value = rule.substr(eq + 1);

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
        }
        else
        {
            key = rule;
            value.clear();
        }

        auto it = conf_tree.options_map.find(key);
        if (it == conf_tree.options_map.end())
        {
            // Should not happen
            std::cout << "Couldn't find implied option in tree!\n";
            abort();
        }

        it->second->set = true;
        it->second->value = value;
        conf_engine_resolve_implies(it->second->implies, conf_tree);
    }

    return 0;
}

// base
int ConfigInteractable::define(ConfigTree& tree)
{
    return 0;
}

int ConfigInteractable::define(const std::string& value, ConfigTree& tree)
{
    return 0;
}

int ConfigInteractable::undefine(ConfigTree& tree)
{
    return 0;
}

// option
ConfigOption::ConfigOption()
{
    type = CONFIG_OPTION;
}

int ConfigOption::define(ConfigTree& tree)
{
    if (conf_engine_resolve_implies(this->implies, tree) < 0)
        return -1;

    this->set = true;
    this->value.clear();
    return 0;
}

int ConfigOption::define(const std::string& value, ConfigTree& tree)
{
    if (conf_engine_resolve_implies(this->implies, tree) < 0)
        return -1;

    this->set = true;
    this->value = value;
    return 0;
}

int ConfigOption::undefine(ConfigTree& tree)
{
    this->set = false;
    this->value.clear();
    return 0;
}

// menu
ConfigMenu::ConfigMenu()
{
    type = CONFIG_MENU;
}

int ConfigMenu::define(ConfigTree& tree)
{
    if (conf_engine_resolve_implies(this->implies, tree) < 0)
        return -1;

    this->set = true;
    this->value.clear();
    return 0;
}

int ConfigMenu::define(const std::string& value, ConfigTree& tree)
{
    if (conf_engine_resolve_implies(this->implies, tree) < 0)
        return -1;

    this->set = true;
    this->value = value;
    return 0;
}

int ConfigMenu::undefine(ConfigTree& tree)
{
    this->set = false;
    this->value.clear();
    return 0;
}

ConfigInteractable* ConfigMenu::selected_inter()
{
    if (sel_idx == -1)
        return nullptr;

    return interactables[sel_idx].get();
}

int ConfigMenu::set_next_element()
{
    if (sel_idx == -1)
        return this->set_first_element();

    if (sel_idx >= interactables.size() - 1)
        return -1;

    for (size_t i = sel_idx + 1; i < this->interactables.size(); ++i)
    {
        if (this->interactables[i]->visible)
        {
            this->sel_idx = i;
            break;
        }
    }
    return 0;
}

int ConfigMenu::set_prev_element()
{
    if (sel_idx == -1)
        return this->set_last_element();

    if (sel_idx == 0)
        return -1;

    for (ssize_t i = sel_idx - 1; i >= 0; --i)
    {
        if (this->interactables[i]->visible)
        {
            this->sel_idx = i;
            break;
        }
    }
    return 0;
}

int ConfigMenu::set_first_element()
{
    if (sel_idx == 0)
        return 0;

    if (this->visible_elements_count() == 0)
        return -1;

    for (size_t i = 0; i < this->interactables.size(); ++i)
    {
        if (this->interactables[i]->visible)
        {
            this->sel_idx = i;
            break;
        }
    }
    return 0;
}

int ConfigMenu::set_last_element()
{
    if (sel_idx >= interactables.size() - 1)
        return 0;

    if (this->visible_elements_count() == 0)
        return -1;

    for (ssize_t i = this->interactables.size() - 1; i >= 0; --i)
    {
        if (this->interactables[i]->visible)
        {
            this->sel_idx = i;
            break;
        }
    }
    return 0;
}

bool ConfigMenu::needs_selecting()
{
    return this->name.size() > 0;
}

size_t ConfigMenu::visible_elements_count() const
{
    size_t cnt = 0;
    for (auto& elem : this->interactables)
    {
        if (!elem->visible)
            continue;

        ++cnt;
    }
    return cnt;
}
