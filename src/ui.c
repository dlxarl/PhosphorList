#include "ui.h"
#include "commands.h"

#include <ncurses.h>
#include <locale.h>

void draw_ui(int selected, int offset) {
    clear();

    int h, w;
    getmaxyx(stdscr, h, w);

    for (int i = 0; i < h - 2; i++) {
        int idx = offset + i;
        if (idx >= cmd_count)
            break;

        if (idx == selected)
            attron(A_REVERSE);

        mvprintw(
            i,
            1,
            "%-20s %s",
            cmds[idx].name,
            cmds[idx].favorite ? "★" : " "
        );

        if (idx == selected)
            attroff(A_REVERSE);
    }

    mvprintw(
        h - 1,
        1,
        "↑↓ move   f fav   s sort   q quit"
    );

    refresh();
}

