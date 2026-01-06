#include "ui.h"
#include "commands.h"
#include <ncurses.h>

void draw_ui(int selected) {
    clear();
    int h, w;
    getmaxyx(stdscr, h, w);

    for (int i = 0; i < h - 2 && i < cmd_count; i++) {
        if (i == selected) attron(A_REVERSE);
        mvprintw(i, 1, "%-20s %s",
                 cmds[i].name,
                 cmds[i].favorite ? "★" : " ");
        if (i == selected) attroff(A_REVERSE);
    }

    mvprintw(h - 1, 1, "↑↓ move  f fav  s sort  q quit");
    refresh();
}
