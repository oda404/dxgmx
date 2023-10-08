/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <cstring>
#include <dxgmx_libconf/core.h>
#include <iostream>
#include <ncurses.h>
#include <stack>

#define COLORS_ARROW 1
#define COLORS_DESCRIPTION 2

static std::stack<ConfigMenu*> g_menus;

static void help(const char* binname)
{
    std::cout << "\n" << binname << " [options]\n";
    std::cout << "\n";
    std::cout << "Configure the dxgmx kernel (ncurses)\n";
    std::cout << "\n";
    std::cout << "options:\n";
    std::cout << "  --config, -c             Config file\n";
    std::cout << "  --root-config-menu, -r   Root config menu json\n";
    std::cout << "  --help, -h               Print this message and exit\n";
    std::cout << "\n";
}

static void refresh_input_window(
    const std::string& opt_name, const std::string& value, WINDOW* input_win)
{
    wclear(input_win);

    wattron(input_win, COLOR_PAIR(COLORS_DESCRIPTION));
    box(input_win, 0, 0);
    mvwprintw(input_win, 0, 2, " %s ", opt_name.c_str());
    wattroff(input_win, COLOR_PAIR(COLORS_DESCRIPTION));

    mvwprintw(input_win, 2, 2, "%s", value.c_str());
    mvwvline(input_win, 2, value.size() + 2, 0, 1);
    wrefresh(input_win);
}

static std::string start_and_run_input_window(ConfigOption& opt)
{
    std::string res;

    int root_x;
    int root_y;
    getmaxyx(stdscr, root_y, root_x);

    int lines = 5;
    int cols = 40;

    WINDOW* input_win =
        newwin(lines, cols, root_y / 2 - lines / 2, root_x / 2 - cols / 2);
    keypad(input_win, true);

    refresh_input_window(opt.name, res, input_win);

    int ch;
    while ((ch = wgetch(input_win)) != '\n')
    {
        if (ch == KEY_BACKSPACE && res.size())
        {
            res.pop_back();
            refresh_input_window(opt.name, res, input_win);
        }
        else if (ch <= '~' && ch >= ' ')
        {
            res += ch;
            refresh_input_window(opt.name, res, input_win);
        }
    }

    delwin(input_win);
    wclear(stdscr);
    return res;
}

static int
draw_static_ui(WINDOW* ui_win, int w, int h, const std::string& config_path)
{
    const char* keyhelp = "[F2 Exit]";
    const char* title = " Dxgmx configuration utility ";

    box(ui_win, 0, 0);
    mvwprintw(ui_win, 0, 2, "%s", title);
    mvwprintw(ui_win, 0, w - 2 - strlen(keyhelp), "%s", keyhelp);
    mvwprintw(ui_win, 0, 2 + strlen(title) + 2, " %s ", config_path.c_str());
    wrefresh(ui_win);
    return 0;
}

static int draw_main_menu(WINDOW* menu_win, int w, int h)
{
    ConfigMenu* cur_menu = g_menus.top();

    if (cur_menu->sel_idx == -1 && cur_menu->interactables.size())
        cur_menu->sel_idx = 0;

    /* Draw menu title */
    mvwhline(menu_win, 0, 0, 0, w);
    wattron(menu_win, COLOR_PAIR(COLORS_DESCRIPTION));
    mvwprintw(
        menu_win,
        0,
        0,
        "%s%s ",
        g_menus.size() > 1 ? "← " : "",
        cur_menu->title.c_str());
    wattroff(menu_win, COLOR_PAIR(COLORS_DESCRIPTION));

    int max_w = cur_menu->title.size() + 2;

    ConfigInteractable* selected_inter = nullptr;

    for (size_t i = 0; i < cur_menu->interactables.size(); ++i)
    {
        ConfigInteractable* inter = cur_menu->interactables[i].get();
        if (inter->type == CONFIG_MENU)
        {
            ConfigMenu* config_menu = dynamic_cast<ConfigMenu*>(inter);
            if (config_menu->title.size() + 2 > max_w)
                max_w = config_menu->title.size() + 2;

            mvwprintw(menu_win, i + 2, 0, "  %s", config_menu->title.c_str());
            if (i == cur_menu->sel_idx)
            {
                wattron(menu_win, COLOR_PAIR(COLORS_ARROW));
                mvwprintw(menu_win, i + 2, 0, "→");
                wattroff(menu_win, COLOR_PAIR(COLORS_ARROW));
                selected_inter = inter;
            }
        }
        else if (inter->type == CONFIG_OPTION)
        {
            ConfigOption* config_opt = dynamic_cast<ConfigOption*>(inter);
            if (config_opt->title.size() + 2 > max_w)
                max_w = config_opt->title.size() + 2;

            if (config_opt->view_only)
                wattron(menu_win, COLOR_PAIR(COLORS_DESCRIPTION));
            else if (config_opt->set)
                wattron(menu_win, COLOR_PAIR(COLORS_ARROW));

            mvwprintw(menu_win, i + 2, 0, "  %s", config_opt->title.c_str());

            if (config_opt->view_only)
                wattroff(menu_win, COLOR_PAIR(COLORS_DESCRIPTION));
            else if (config_opt->set)
                wattroff(menu_win, COLOR_PAIR(COLORS_ARROW));

            if (i == cur_menu->sel_idx)
            {
                selected_inter = inter;
                wattron(menu_win, COLOR_PAIR(COLORS_ARROW));
                mvwprintw(menu_win, i + 2, 0, "◯");
                wattroff(menu_win, COLOR_PAIR(COLORS_ARROW));
            }
        }
    }

    if (selected_inter)
    {
        mvwvline(menu_win, 1, max_w + 1, 0, h);
        wrefresh(menu_win);

        WINDOW* preview_win = newwin(12, w - max_w - 2, 3, max_w + 4);
        wattron(preview_win, COLOR_PAIR(COLORS_DESCRIPTION));

        if (selected_inter->type == CONFIG_MENU)
        {
            mvwprintw(
                preview_win, 0, 0, "%s", selected_inter->description.c_str());
        }
        else if (selected_inter->type == CONFIG_OPTION)
        {
            ConfigOption* config_opt =
                dynamic_cast<ConfigOption*>(selected_inter);

            if (config_opt->set)
                wattron(preview_win, COLOR_PAIR(COLORS_ARROW));

            if (config_opt->input_type == CONFIG_OPTION_BOOL)
            {
                mvwprintw(
                    preview_win, 0, 0, "Defines %s", config_opt->name.c_str());
            }
            else if (config_opt->input_type == CONFIG_OPTION_INPUT)
            {
                if (config_opt->value.size())
                {
                    mvwprintw(
                        preview_win,
                        0,
                        0,
                        "Sets %s to (%s)",
                        config_opt->name.c_str(),
                        config_opt->value.c_str());
                }
                else
                {
                    mvwprintw(
                        preview_win, 0, 0, "Sets %s", config_opt->name.c_str());
                }
            }

            if (config_opt->set)
                wattroff(preview_win, COLOR_PAIR(COLORS_ARROW));

            if (config_opt->description.size())
                mvwprintw(
                    preview_win, 2, 0, "%s", config_opt->description.c_str());
            else
                mvwprintw(preview_win, 2, 0, "%s", "No description available");
        }

        wattroff(preview_win, COLOR_PAIR(COLORS_DESCRIPTION));
        wrefresh(preview_win);
        delwin(preview_win);
    }
    else
    {
        mvwprintw(
            menu_win, 1, 0, "This menu doesn't have any available options.");
        wrefresh(menu_win);
    }

    return 0;
}

static int run(ConfigMenu* root_menu, const std::string& config_path)
{
    g_menus.push(root_menu);

    initscr();
    keypad(stdscr, true);
    noecho();
    curs_set(0);
    ESCDELAY = 0;

    start_color();
    use_default_colors();

    init_color(COLOR_WHITE, 750, 750, 750);
    init_pair(COLORS_ARROW, COLOR_GREEN, -1);
    init_pair(COLORS_DESCRIPTION, COLOR_WHITE, -1);

    int root_w, root_h;
    getmaxyx(stdscr, root_h, root_w);

    WINDOW* menu_win = newwin(root_h - 3, root_w - 4, 2, 2);
    int menu_w, menu_h;
    getmaxyx(menu_win, menu_h, menu_w);

    draw_static_ui(stdscr, root_w, root_h, config_path);
    draw_main_menu(menu_win, menu_w, menu_h);

    int ch;
    bool should_redraw = false;
    bool should_redraw_ui = false;
    while ((ch = getch()) != KEY_F(2))
    {
        ConfigMenu* cur_menu = g_menus.top();
        switch (ch)
        {
        case KEY_DOWN:
        case 's':
            if (cur_menu->sel_idx == -1 ||
                cur_menu->sel_idx >= cur_menu->interactables.size() - 1)
                break;

            ++cur_menu->sel_idx;
            should_redraw = true;
            break;

        case KEY_UP:
        case 'w':
            if (cur_menu->sel_idx <= 0)
                break;

            --cur_menu->sel_idx;
            should_redraw = true;
            break;

        case KEY_HOME:
            if (cur_menu->sel_idx <= 0)
                break;

            cur_menu->sel_idx = 0;
            should_redraw = true;
            break;

        case KEY_END:
            if (cur_menu->sel_idx == -1 ||
                cur_menu->sel_idx == cur_menu->interactables.size() - 1)
                break;

            cur_menu->sel_idx = cur_menu->interactables.size() - 1;
            should_redraw = true;
            break;

        case KEY_BACKSPACE:
        case 27:
            if (g_menus.size() == 1)
                break;

            g_menus.pop();
            wclear(menu_win);
            should_redraw = true;
            break;

        case '\n':
        {
            ConfigInteractable* inter = cur_menu->selected_inter();
            if (inter == nullptr || inter->view_only)
                break;

            if (inter->type == CONFIG_MENU)
            {
                g_menus.push(dynamic_cast<ConfigMenu*>(inter));
                wclear(menu_win);
                should_redraw = true;
            }
            else if (inter->type == CONFIG_OPTION)
            {
                ConfigOption* opt = dynamic_cast<ConfigOption*>(inter);
                if (opt->input_type == CONFIG_OPTION_BOOL)
                {
                    if (!opt->set)
                        conf_engine_define_option(root_menu, *opt);
                    else
                        conf_engine_undefine_option(root_menu, *opt);

                    should_redraw = true;
                }
                else if (opt->input_type == CONFIG_OPTION_INPUT)
                {
                    if (!opt->set)
                    {
                        std::string res = start_and_run_input_window(*opt);
                        conf_engine_set_option(root_menu, res, *opt);
                        should_redraw_ui = true;
                        should_redraw = true;
                    }
                    else
                    {
                        conf_engine_unset_option(root_menu, *opt);
                        should_redraw = true;
                    }
                }
            }
            break;
        }

        case KEY_RESIZE:
            getmaxyx(stdscr, root_h, root_w);
            wresize(menu_win, root_h - 3, root_w - 4);
            getmaxyx(menu_win, menu_h, menu_w);
            wclear(stdscr);
            wclear(menu_win);
            should_redraw_ui = true;
            should_redraw = true;
            break;
        }

        if (should_redraw_ui)
        {
            draw_static_ui(stdscr, root_w, root_h, config_path);
            should_redraw_ui = false;
        }

        if (should_redraw)
        {
            draw_main_menu(menu_win, menu_w, menu_h);
            should_redraw = false;
        }
    }

    endwin();
    return 0;
}

int main(int argc, const char** argv)
{
    if (argc == 1)
    {
        help(argv[0]);
        return 1;
    }

    setlocale(LC_CTYPE, "");

    std::string config_file;
    std::string root_menu_path;

    for (size_t i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "--config") || !strcmp(argv[i], "-c"))
        {
            if (i + 1 == argc)
            {
                help(argv[0]);
                std::cout << "--config, -c requires second argument.\n";
                return 1;
            }

            config_file = std::string(argv[i + 1]);
        }
        else if (
            !strcmp(argv[i], "--root-config-menu") || !strcmp(argv[i], "-r"))
        {
            if (i + 1 == argc)
            {
                help(argv[0]);
                std::cout
                    << "--root-config-menu, -r requires second argument.\n";
                return 1;
            }

            root_menu_path = std::string(argv[i + 1]);
        }
        else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
        {
            help(argv[0]);
            return 0;
        }
    }

    if (config_file == "")
    {
        help(argv[0]);
        std::cout << "--config, -c is required.\n";
        return 1;
    }

    if (root_menu_path == "")
    {
        help(argv[0]);
        std::cout << "--root-config-menu, -r is required.\n";
        return 1;
    }

    ConfigMenu root_menu;
    int st = conf_engine_load_menus_recursive(root_menu_path, &root_menu);
    if (st < 0)
        return st;

    st = conf_engine_validate_implied(&root_menu);
    if (st < 0)
        return st;

    st = conf_engine_parse_conf_file(config_file, &root_menu);
    if (st < 0 && st != -2)
        return st;

    st = run(&root_menu, config_file);
    if (st < 0)
        return st;

    st = conf_engine_generate_gnumake_defs(config_file, &root_menu);
    return st;
}
