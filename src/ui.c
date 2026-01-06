#include "ui.h"
#include "commands.h"

#include <ncurses.h>
#include <string.h>
#include <ctype.h>

/* color pairs */
#define CP_NORMAL    1
#define CP_SELECTED  2
#define CP_FAV       3
#define CP_DESC      4
#define CP_TAB       5
#define CP_TAB_ACTIVE 6

/* view mode comes from main via globals */
extern int current_view; /* 1 = ALL, 2 = FAV */

void ui_init_colors(void) {
    if (!has_colors()) return;

    start_color();
    use_default_colors();

    init_pair(CP_NORMAL,     COLOR_WHITE,  -1);
    init_pair(CP_SELECTED,   COLOR_BLACK,  COLOR_CYAN);
    init_pair(CP_FAV,        COLOR_YELLOW, -1);
    init_pair(CP_DESC,       COLOR_CYAN,   -1);
    init_pair(CP_TAB,        COLOR_WHITE,  -1);
    init_pair(CP_TAB_ACTIVE, COLOR_BLACK,  COLOR_GREEN);
}

/* =========================
 * DRAW TABS
 * ========================= */
static void draw_tabs(void) {
    int w = getmaxx(stdscr);

    attron(A_BOLD);
    mvhline(0, 0, ' ', w);

    if (current_view == 1)
        attron(COLOR_PAIR(CP_TAB_ACTIVE));
    else
        attron(COLOR_PAIR(CP_TAB));

    mvprintw(0, 2, "[1] ALL");

    attroff(COLOR_PAIR(CP_TAB_ACTIVE));
    attroff(COLOR_PAIR(CP_TAB));

    if (current_view == 2)
        attron(COLOR_PAIR(CP_TAB_ACTIVE));
    else
        attron(COLOR_PAIR(CP_TAB));

    mvprintw(0, 12, "[2] FAV");

    attroff(COLOR_PAIR(CP_TAB_ACTIVE));
    attroff(COLOR_PAIR(CP_TAB));
    attroff(A_BOLD);
}

/* =========================
 * MAIN UI
 * ========================= */
void draw_ui(int selected, int offset, const char *desc) {
    clear();

    int h, w;
    getmaxyx(stdscr, h, w);

    /* draw tabs */
    draw_tabs();

    int list_top = 2;
    int left_w = w / 3;
    int right_x = left_w + 1;

    /* vertical separator */
    for (int y = list_top; y < h - 1; y++)
        mvaddch(y, left_w, ACS_VLINE);

    /* left panel */
    for (int i = 0; i < h - list_top - 1; i++) {
        int v = offset + i;
        if (v >= visible_count) break;

        int idx = visible[v];
        int y = list_top + i;

        if (v == selected)
            attron(COLOR_PAIR(CP_SELECTED));
        else if (cmds[idx].favorite)
            attron(COLOR_PAIR(CP_FAV));
        else
            attron(COLOR_PAIR(CP_NORMAL));

        mvprintw(y, 1, "%-18s %s",
                 cmds[idx].name,
                 cmds[idx].favorite ? "★" : " ");

        attroff(COLOR_PAIR(CP_SELECTED));
        attroff(COLOR_PAIR(CP_FAV));
        attroff(COLOR_PAIR(CP_NORMAL));
    }

    /* right panel */
    attron(COLOR_PAIR(CP_DESC));
    mvprintw(list_top, right_x + 1, "Description");
    attroff(COLOR_PAIR(CP_DESC));

    int y = list_top + 2;
    if (desc) {
        const char *p = desc;
        while (*p && y < h - 1) {
            char line[256];
            int len = 0;

            while (*p && *p != '\n' && len < (w - right_x - 3))
                line[len++] = *p++;

            line[len] = 0;
            mvprintw(y++, right_x + 1, "%s", line);

            if (*p == '\n') p++;
        }
    }

    /* footer */
    attron(A_DIM);
    mvprintw(
        h - 1, 1,
        "[↑↓] move   [/] search   [f]av   [d]esc   [q]uit"
    );
    attroff(A_DIM);

    refresh();
}

/* =========================
 * SEARCH PROMPT
 * ========================= */
void draw_search(const char *query) {
    int h, w;
    getmaxyx(stdscr, h, w);

    attron(A_BOLD);
    mvprintw(h - 1, 1, "/%s", query);
    attroff(A_BOLD);

    clrtoeol();
    refresh();
}

/* =========================
 * EXECUTE DIALOG
 * ========================= */
int draw_execute_dialog(const char *cmd, char *args, int maxlen) {
    int h, w;
    getmaxyx(stdscr, h, w);

    int dh = 7;
    int dw = w / 2;
    int y = (h - dh) / 2;
    int x = (w - dw) / 2;

    WINDOW *win = newwin(dh, dw, y, x);
    keypad(win, TRUE);
    box(win, 0, 0);

    mvwprintw(win, 1, 2, "Execute command:");
    mvwprintw(win, 2, 2, "%s", cmd);
    mvwprintw(win, 4, 2, "Args:");
    mvwprintw(win, 5, 2, "[ Enter = Execute ]   [ Esc = Cancel ]");

    int len = strlen(args);
    wrefresh(win);

    int ch;
    while ((ch = wgetch(win))) {
        if (ch == 27) {
            delwin(win);
            return 0;
        }
        if (ch == '\n') {
            delwin(win);
            return 1;
        }
        if ((ch == KEY_BACKSPACE || ch == 127) && len > 0) {
            args[--len] = 0;
        }
        else if (isprint(ch) && len < maxlen - 1) {
            args[len++] = ch;
            args[len] = 0;
        }

        mvwprintw(win, 4, 8, "%-*s", dw - 10, args);
        wrefresh(win);
    }

    delwin(win);
    return 0;
}
