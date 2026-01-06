#include <ncurses.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "commands.h"
#include "favorites.h"
#include "sort.h"
#include "ui.h"

/* =========================
 * GLOBAL VIEW FOR UI (tabs)
 * ========================= */
int current_view = 1; /* 1 = ALL, 2 = FAV */

typedef enum {
    MODE_NORMAL,
    MODE_SEARCH
} Mode;

typedef enum {
    VIEW_ALL = 1,
    VIEW_FAV = 2
} ViewMode;

/* =========================
 * DESCRIPTION CACHE
 * ========================= */

#define MAX_DESC 512

static char desc_cache[MAX_CMDS][MAX_DESC];
static char desc_loaded[MAX_CMDS];

static void load_description_cached(int idx, char *out, size_t n) {
    if (!desc_loaded[idx]) {
        char buf[512];
        snprintf(buf, sizeof(buf), "whatis %s 2>/dev/null", cmds[idx].name);

        FILE *p = popen(buf, "r");
        if (p) {
            if (!fgets(desc_cache[idx], MAX_DESC, p))
                desc_cache[idx][0] = 0;
            pclose(p);
        } else {
            desc_cache[idx][0] = 0;
        }
        desc_loaded[idx] = 1;
    }

    strncpy(out, desc_cache[idx], n);
    out[n - 1] = 0;
}

/* =========================
 * BUILD VISIBLE LIST
 * ========================= */
static void rebuild_visible(ViewMode view) {
    visible_count = 0;

    for (int i = 0; i < cmd_count; i++) {
        if (view == VIEW_FAV && !cmds[i].favorite)
            continue;

        visible[visible_count++] = i;
    }
}

/* =========================
 * MAIN
 * ========================= */
int main(void) {
    setlocale(LC_ALL, "");

    /* ---------- load commands ---------- */
    load_commands();
    qsort(cmds, cmd_count, sizeof(Command), cmp_normal);

    /* ---------- ncurses init ---------- */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    set_escdelay(25);
    curs_set(0);

    ui_init_colors();

    /* ---------- state ---------- */
    int selected = 0;
    int offset = 0;
    int dirty = 1;

    Mode mode = MODE_NORMAL;
    ViewMode view = VIEW_ALL;
    current_view = view;

    char search[64] = {0};
    char args[128] = {0};
    char desc[MAX_DESC] = "Press 'd' to load description";

    rebuild_visible(view);

    /* ---------- first draw ---------- */
    draw_ui(selected, offset, desc);

    /* =========================
     * MAIN LOOP
     * ========================= */
    int ch;
    while (1) {
        ch = getch();

        /* global exit */
        if (ch == 'q')
            break;

        if (mode == MODE_NORMAL) {

            /* ----- tabs ----- */
            if (ch == '1') {
                view = VIEW_ALL;
                current_view = view;
                rebuild_visible(view);
                selected = offset = 0;
                dirty = 1;
            }
            else if (ch == '2') {
                view = VIEW_FAV;
                current_view = view;
                rebuild_visible(view);
                selected = offset = 0;
                dirty = 1;
            }

            else if (ch == '/') {
                mode = MODE_SEARCH;
                search[0] = 0;
                dirty = 1;
            }
            else if (ch == KEY_UP && selected > 0) {
                selected--;
                dirty = 1;
            }
            else if (ch == KEY_DOWN && selected < visible_count - 1) {
                selected++;
                dirty = 1;
            }
            else if (ch == 'f' && visible_count > 0) {
                int idx = visible[selected];
                cmds[idx].favorite ^= 1;
                save_favorites();
                rebuild_visible(view);
                if (selected >= visible_count)
                    selected = visible_count - 1;
                dirty = 1;
            }
            else if (ch == 'd' && visible_count > 0) {
                load_description_cached(
                    visible[selected],
                    desc,
                    sizeof(desc)
                );
                dirty = 1;
            }
            else if (ch == '\n' && visible_count > 0) {
                int idx = visible[selected];
                args[0] = 0;

                if (draw_execute_dialog(
                        cmds[idx].name,
                        args,
                        sizeof(args)
                    )) {

                    endwin();

                    char cmdline[512];
                    snprintf(
                        cmdline,
                        sizeof(cmdline),
                        "%s %s",
                        cmds[idx].name,
                        args
                    );

                    system(cmdline);

                    /* restore ncurses */
                    initscr();
                    cbreak();
                    noecho();
                    keypad(stdscr, TRUE);
                    curs_set(0);
                    ui_init_colors();

                    dirty = 1;
                }
            }
        }
        else if (mode == MODE_SEARCH) {

            if (ch == '\n') {
                apply_filter(search);
                selected = offset = 0;
                mode = MODE_NORMAL;
                dirty = 1;
            }
            else if (ch == 27) { /* ESC */
                rebuild_visible(view);
                mode = MODE_NORMAL;
                dirty = 1;
            }
            else if (ch == KEY_BACKSPACE || ch == 127) {
                int l = strlen(search);
                if (l > 0) {
                    search[l - 1] = 0;
                    dirty = 1;
                }
            }
            else if (isprint(ch)) {
                int l = strlen(search);
                if (l < (int)sizeof(search) - 1) {
                    search[l] = ch;
                    search[l + 1] = 0;
                    dirty = 1;
                }
            }
        }

        /* ---------- scrolling ---------- */
        int h, w;
        getmaxyx(stdscr, h, w);

        if (selected < offset) {
            offset = selected;
            dirty = 1;
        }
        else if (selected >= offset + h - 3) {
            offset = selected - (h - 4);
            dirty = 1;
        }

        /* ---------- redraw ---------- */
        if (dirty) {
            draw_ui(selected, offset, desc);
            if (mode == MODE_SEARCH)
                draw_search(search);
            dirty = 0;
        }
    }

    endwin();
    return 0;
}
