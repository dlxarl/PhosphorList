#include "ui.h"
#include "commands.h"

#include <ncurses.h>
#include <ctype.h>
#include <string.h>

void draw_ui(int selected, int offset) {
    clear();

    int h, w;
    getmaxyx(stdscr, h, w);

    for (int i = 0; i < h - 2; i++) {
        int v = offset + i;
        if (v >= visible_count) break;

        int idx = visible[v];

        if (v == selected)
            attron(A_REVERSE);

        mvprintw(
            i, 1,
            "%-20s %s",
            cmds[idx].name,
            cmds[idx].favorite ? "★" : " "
        );

        if (v == selected)
            attroff(A_REVERSE);
    }

    mvprintw(h - 1, 1, "↑↓ move  / search  f fav  Enter run  q quit");
    refresh();
}

void draw_search(const char *query) {
    int h, w;
    getmaxyx(stdscr, h, w);
    mvprintw(h - 1, 1, "/%s", query);
    clrtoeol();
    refresh();
}

int draw_execute_dialog(const char *cmd, char *args, int maxlen) {
    int h, w;
    getmaxyx(stdscr, h, w);

    int dh = 7, dw = w / 2;
    int y = (h - dh) / 2;
    int x = (w - dw) / 2;

    WINDOW *win = newwin(dh, dw, y, x);
    box(win, 0, 0);

    mvwprintw(win, 1, 2, "Execute command:");
    mvwprintw(win, 2, 2, "%s", cmd);
    mvwprintw(win, 4, 2, "Args: ");
    mvwprintw(win, 5, 2, "[ Enter = Execute ]   [ Esc = Cancel ]");

    int len = strlen(args);
    wrefresh(win);

    int ch;
    while ((ch = wgetch(win))) {
        if (ch == 27) { // ESC
            delwin(win);
            return 0;
        }
        if (ch == '\n') {
            delwin(win);
            return 1;
        }
        if ((ch == KEY_BACKSPACE || ch == 127) && len > 0) {
            args[--len] = 0;
        } else if (isprint(ch) && len < maxlen - 1) {
            args[len++] = ch;
            args[len] = 0;
        }

        mvwprintw(win, 4, 8, "%-*s", dw - 10, args);
        wrefresh(win);
    }

    delwin(win);
    return 0;
}
