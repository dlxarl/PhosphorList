#include <ncurses.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "commands.h"
#include "favorites.h"
#include "sort.h"
#include "ui.h"

typedef enum {
    MODE_NORMAL,
    MODE_SEARCH
} Mode;

int main(void) {
    setlocale(LC_ALL, "");

    load_commands();
    qsort(cmds, cmd_count, sizeof(Command), cmp_normal);

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    set_escdelay(25);
    curs_set(0);

    int selected = 0;
    int offset = 0;
    Mode mode = MODE_NORMAL;

    char search[64] = {0};
    char args[128] = {0};

    draw_ui(selected, offset);

    int ch;
    while ((ch = getch()) != 'q') {

        if (mode == MODE_NORMAL) {

            if (ch == '/') {
                mode = MODE_SEARCH;
                search[0] = 0;
            }
            else if (ch == KEY_UP && selected > 0) {
                selected--;
            }
            else if (ch == KEY_DOWN && selected < visible_count - 1) {
                selected++;
            }
            else if (ch == 'f') {
                int idx = visible[selected];
                cmds[idx].favorite ^= 1;
                save_favorites();
            }
            else if (ch == '\n' && visible_count > 0) {
                int idx = visible[selected];
                args[0] = 0;

                if (draw_execute_dialog(cmds[idx].name, args, sizeof(args))) {
                    endwin();
                    char cmdline[512];
                    snprintf(cmdline, sizeof(cmdline),
                             "%s %s", cmds[idx].name, args);
                    system(cmdline);

                    initscr();
                    raw();
                    noecho();
                    keypad(stdscr, TRUE);
                    curs_set(0);
                }
            }
        }
        else if (mode == MODE_SEARCH) {
            if (ch == '\n') {
                apply_filter(search);
                selected = offset = 0;
                mode = MODE_NORMAL;
            }
            else if (ch == 27) { // ESC
                clear_filter();
                mode = MODE_NORMAL;
            }
            else if (ch == KEY_BACKSPACE || ch == 127) {
                int l = strlen(search);
                if (l > 0) search[l - 1] = 0;
            }
            else if (isprint(ch)) {
                int l = strlen(search);
                if (l < (int)sizeof(search) - 1) {
                    search[l] = ch;
                    search[l + 1] = 0;
                }
            }
        }

        int h, w;
        getmaxyx(stdscr, h, w);

        if (selected < offset)
            offset = selected;
        else if (selected >= offset + h - 2)
            offset = selected - (h - 3);

        draw_ui(selected, offset);
        if (mode == MODE_SEARCH)
            draw_search(search);
    }

    endwin();
    return 0;
}
