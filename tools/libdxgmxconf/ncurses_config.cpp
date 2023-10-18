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
#define COLORS_SAFE 3
#define COLORS_DANGER 4
#define COLORS_HIGHLIGHT 5

static ConfigTree g_config_tree;
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

static void refresh_exit_window(bool yes_sel, int w, int h, WINDOW* exit_win)
{
    wclear(exit_win);

    wattron(exit_win, COLOR_PAIR(COLORS_DESCRIPTION));
    box(exit_win, 0, 0);
    mvwprintw(exit_win, 0, 2, " %s ", "Save configuration?");
    wattroff(exit_win, COLOR_PAIR(COLORS_DESCRIPTION));

    if (yes_sel)
        wattron(exit_win, COLOR_PAIR(COLORS_SAFE));

    mvwprintw(exit_win, h / 2, w / 2 - strlen("Yes") - 1, " Yes ");

    if (yes_sel)
        wattroff(exit_win, COLOR_PAIR(COLORS_SAFE));
    else
        wattron(exit_win, COLOR_PAIR(COLORS_DANGER));

    mvwprintw(exit_win, h / 2, w / 2 - strlen("No") + 4, " No ");

    if (!yes_sel)
        wattroff(exit_win, COLOR_PAIR(COLORS_DANGER));

    wrefresh(exit_win);
}

#define SHOULD_SAVE 0
#define SHOULD_NOT_SAVE -1
#define SHOULD_NOT_EXIT -2

static int spawn_and_run_exit_window()
{
    int res = SHOULD_SAVE;

    int root_x;
    int root_y;
    getmaxyx(stdscr, root_y, root_x);

    int lines = 5;
    int cols = 25;

    WINDOW* exit_win =
        newwin(lines, cols, root_y / 2 - lines / 2, root_x / 2 - cols / 2);
    keypad(exit_win, true);

    bool yes_sel = true;
    bool running = true;
    refresh_exit_window(yes_sel, cols, lines, exit_win);

    while (running)
    {
        switch (wgetch(exit_win))
        {
        case KEY_BACKSPACE:
        case 27:
            res = SHOULD_NOT_EXIT;
            running = false;
            break;

        case '\n':
            if (yes_sel)
            {
                res = SHOULD_SAVE;
                running = false;
            }
            else
            {
                res = SHOULD_NOT_SAVE;
                running = false;
            }
            break;

        case KEY_LEFT:
            if (!yes_sel)
                yes_sel = true;
            break;

        case KEY_RIGHT:
            if (yes_sel)
                yes_sel = false;
            break;
        }

        refresh_exit_window(yes_sel, cols, lines, exit_win);
    }

    delwin(exit_win);
    wclear(stdscr);
    return res;
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

static std::string spawn_and_run_input_window(ConfigInteractable& inter)
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

    refresh_input_window(inter.name, res, input_win);

    bool running = true;
    while (running)
    {
        int ch = wgetch(input_win);

        if (ch == '\n')
        {
            running = false;
            break;
        }
        else if (ch == 27)
        {
            res = "";
            running = false;
            break;
        }
        else if (ch == KEY_BACKSPACE && res.size())
        {
            res.pop_back();
            refresh_input_window(inter.name, res, input_win);
        }
        else if (ch <= '~' && ch >= ' ')
        {
            res += ch;
            refresh_input_window(inter.name, res, input_win);
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
    mvwprintw(
        ui_win,
        h - 1,
        w - 12 - config_path.size(),
        " Config: %s ",
        config_path.c_str());
    wrefresh(ui_win);
    return 0;
}

static void draw_element_description(WINDOW* win, ConfigInteractable* inter)
{
    if (inter->set)
        wattron(win, COLOR_PAIR(COLORS_ARROW));

    int h = 0;

    // Might be false for menus
    if (inter->name.size())
    {
        if (inter->input_type == CONFIG_OPTION_INPUT && inter->value.size())
        {
            mvwprintw(
                win,
                0,
                0,
                "Defines %s as (%s)",
                inter->name.c_str(),
                inter->value.c_str());
        }
        else
        {
            mvwprintw(win, 0, 0, "Defines %s", inter->name.c_str());
        }
        h = 2;
    }

    if (inter->set)
        wattroff(win, COLOR_PAIR(COLORS_ARROW));

    if (inter->description.size())
        mvwprintw(win, h, 0, "%s", inter->description.c_str());
    else
        mvwprintw(win, h, 0, "%s", "No description available");
}

static int draw_main_menu(WINDOW* menu_win, int w, int h)
{
    ConfigMenu* cur_menu = g_menus.top();

    if (cur_menu->selected_inter() == nullptr)
        cur_menu->set_first_element();

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

    int max_w = w / 4;
    if (cur_menu->title.size() > max_w)
        max_w = cur_menu->title.size() + 2;

    size_t shown_element = 0;
    for (size_t i = 0; i < cur_menu->interactables.size(); ++i)
    {
        ConfigInteractable* inter = cur_menu->interactables[i].get();
        if (!inter->visible)
            continue;

        if (inter->title.size() + 2 > max_w)
            max_w = inter->title.size() + 2;

        attr_t set_color = 0;

        if (inter != cur_menu->selected_inter())
        {
            if (inter->set)
            {
                set_color = COLORS_ARROW;
                wattron(menu_win, COLOR_PAIR(set_color));
            }
        }
        else
        {
            set_color = COLORS_HIGHLIGHT;
            wattron(menu_win, COLOR_PAIR(set_color));
        }

        mvwprintw(menu_win, shown_element + 2, 0, "  %s", inter->title.c_str());

        if (set_color)
        {
            wattroff(menu_win, COLOR_PAIR(set_color));
            set_color = 0;
        }

        set_color = COLORS_ARROW;
        wattron(menu_win, COLOR_PAIR(set_color));

        if (inter->type == CONFIG_OPTION)
        {
            mvwprintw(menu_win, shown_element + 2, 0, "◯");
        }
        else if (inter->type == CONFIG_MENU)
        {
            ConfigMenu* conf_menu = dynamic_cast<ConfigMenu*>(inter);
            if (conf_menu->needs_selecting())
            {
                if (conf_menu->set)
                    mvwprintw(menu_win, shown_element + 2, 0, "→");
                else
                    mvwprintw(menu_win, shown_element + 2, 0, "☐");
            }
            else
            {
                mvwprintw(menu_win, shown_element + 2, 0, "→");
            }
        }
        else
        {
            std::cout << "Unkown element type " << inter->type << " \n";
            abort();
        }

        if (set_color)
        {
            wattroff(menu_win, COLOR_PAIR(set_color));
            set_color = 0;
        }

        ++shown_element;
    }

    if (cur_menu->selected_inter())
    {
        mvwvline(menu_win, 1, max_w + 1, 0, h);
        wrefresh(menu_win);

        WINDOW* preview_win = newwin(12, w - max_w - 2, 3, max_w + 4);
        wattron(preview_win, COLOR_PAIR(COLORS_DESCRIPTION));

        draw_element_description(preview_win, cur_menu->selected_inter());

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

static int define_element(ConfigInteractable* inter)
{
    if (inter->input_type == CONFIG_OPTION_BOOL)
    {
        if (!inter->set)
            inter->define(g_config_tree);
        else
            inter->undefine(g_config_tree);
    }
    else if (inter->input_type == CONFIG_OPTION_INPUT)
    {
        std::string res = spawn_and_run_input_window(*inter);
        if (res.size() > 0)
            inter->define(res, g_config_tree);
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

    init_color(COLOR_WHITE, 650, 650, 650);
    init_color(COLOR_MAGENTA, 1000, 0, 1000);
    init_pair(COLORS_ARROW, COLOR_GREEN, -1);
    init_pair(COLORS_DESCRIPTION, COLOR_WHITE, -1);
    init_pair(COLORS_SAFE, COLOR_BLACK, COLOR_GREEN);
    init_pair(COLORS_DANGER, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(COLORS_HIGHLIGHT, COLOR_MAGENTA, -1);

    int root_w, root_h;
    getmaxyx(stdscr, root_h, root_w);

    WINDOW* menu_win = newwin(root_h - 3, root_w - 4, 2, 2);
    int menu_w, menu_h;
    getmaxyx(menu_win, menu_h, menu_w);

    draw_static_ui(stdscr, root_w, root_h, config_path);
    draw_main_menu(menu_win, menu_w, menu_h);

    bool should_save = true;
    bool running = true;

    bool should_redraw = false;
    bool should_redraw_ui = false;
    while (running)
    {
        int ch = wgetch(stdscr);

        ConfigMenu* cur_menu = g_menus.top();
        switch (ch)
        {
        case KEY_DOWN:
        case 's':
            if (cur_menu->set_next_element() == 0)
                should_redraw = true;
            break;

        case KEY_UP:
        case 'w':
            if (cur_menu->set_prev_element() == 0)
                should_redraw = true;
            break;

        case KEY_HOME:
            if (cur_menu->set_first_element() == 0)
                should_redraw = true;
            break;

        case KEY_END:
            if (cur_menu->set_last_element() == 0)
                should_redraw = true;
            break;

        case KEY_F(2):
        {
            int st = spawn_and_run_exit_window();
            if (st == SHOULD_SAVE)
            {
                should_save = true;
                running = false;
            }
            else if (st == SHOULD_NOT_SAVE)
            {
                should_save = false;
                running = false;
            }
            else
            {
                /* We keeps going */
                should_redraw = true;
                should_redraw_ui = true;
            }
            break;
        }

        case KEY_BACKSPACE:
        case 27:
            if (g_menus.size() == 1)
            {
                int st = spawn_and_run_exit_window();
                if (st == SHOULD_SAVE)
                {
                    should_save = true;
                    running = false;
                }
                else if (st == SHOULD_NOT_SAVE)
                {
                    should_save = false;
                    running = false;
                }
                else
                {
                    /* We keeps going */
                    should_redraw = true;
                    should_redraw_ui = true;
                }
                break;
            }

            g_menus.pop();
            wclear(menu_win);
            should_redraw = true;
            break;

        case '\n':
        {
            ConfigInteractable* inter = cur_menu->selected_inter();
            if (inter == nullptr)
                break;

            if (inter->type == CONFIG_MENU)
            {
                ConfigMenu* conf_menu = dynamic_cast<ConfigMenu*>(inter);
                if (conf_menu->needs_selecting() && !conf_menu->set)
                {
                    define_element(inter);
                }
                else
                {
                    g_menus.push(conf_menu);
                    wclear(menu_win);
                }

                should_redraw = true;
                should_redraw_ui = true; // may not be needed
            }
            else if (inter->type == CONFIG_OPTION)
            {
                define_element(inter);
                should_redraw = true;
                should_redraw_ui = true; // may not be needed
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
    return should_save ? SHOULD_SAVE : SHOULD_NOT_SAVE;
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

    if (g_config_tree.build_from_path(root_menu_path) < 0)
        return 1;

    if (g_config_tree.load_options_from_conf_file(config_file) == -1)
        return 1;

    if (run(&g_config_tree.root_menu, config_file) == SHOULD_SAVE)
        return g_config_tree.save_to_gnumake_file(config_file);

    return 0;
}
