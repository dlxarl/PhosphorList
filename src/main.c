#include <ncurses.h>
#include <stdlib.h>
#include "commands.h"
#include "favorites.h"
#include "sort.h"
#include "ui.h"

int main() {
    load_commands();

    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    int selected = 0;
    int sort_mode = 0;
    qsort(cmds, cmd_count, sizeof(Command), cmp_normal);

    draw_ui(selected);

    int ch;
    while ((ch = getch()) != 'q') {
        if (ch == KEY_UP && selected > 0) selected--;
        if (ch == KEY_DOWN && selected < cmd_count - 1) selected++;

        if (ch == 'f') {
            cmds[selected].favorite ^= 1;
            save_favorites();
        }

        if (ch == 's') {
            sort_mode ^= 1;
            qsort(cmds, cmd_count, sizeof(Command),
                  sort_mode ? cmp_fav : cmp_normal);
        }
    }

    endwin();
    return 0;
}

