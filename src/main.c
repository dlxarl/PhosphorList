#include <ncurses.h>
#include <stdlib.h>

#include "commands.h"
#include "favorites.h"
#include "sort.h"
#include "ui.h"

int main(void) {
    // ---------- load data ----------
    load_commands();

    // ---------- ncurses init ----------
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    set_escdelay(25);
    curs_set(0);

    // ---------- state ----------
    int selected = 0;
    int offset = 0;
    int sort_mode = 0;

    qsort(cmds, cmd_count, sizeof(Command), cmp_normal);

    // ---------- first draw ----------
    draw_ui(selected, offset);

    // ---------- main loop ----------
    int ch;
    while ((ch = getch()) != 'q') {

        if (ch == KEY_UP && selected > 0)
            selected--;

        else if (ch == KEY_DOWN && selected < cmd_count - 1)
            selected++;

        else if (ch == 'f') {
            cmds[selected].favorite ^= 1;
            save_favorites();
        }

        else if (ch == 's') {
            sort_mode ^= 1;
            qsort(
                cmds,
                cmd_count,
                sizeof(Command),
                sort_mode ? cmp_fav : cmp_normal
            );
        }

        // ---------- scrolling logic ----------
        int h, w;
        getmaxyx(stdscr, h, w);

        if (selected < offset)
            offset = selected;
        else if (selected >= offset + h - 2)
            offset = selected - (h - 3);

        // ---------- redraw ----------
        draw_ui(selected, offset);
    }

    // ---------- cleanup ----------
    endwin();
    return 0;
}

