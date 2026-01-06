#include <ncurses.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>

#include "commands.h"
#include "favorites.h"
#include "sort.h"
#include "ui.h"

/* =========================
 * GLOBAL VIEW FOR UI (tabs)
 * ========================= */
int current_view = 1; /* 1 = ALL, 2 = FAV, 3 = RECENT */

typedef enum {
    MODE_NORMAL,
    MODE_SEARCH
} Mode;

typedef enum {
    VIEW_ALL    = 1,
    VIEW_FAV    = 2,
    VIEW_RECENT = 3
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
 * RECENT COMMANDS (PERSISTENT)
 * ========================= */

#define MAX_RECENT 32
#define RECENT_FILE ".config/cmdtui/recent.txt"

static int recent[MAX_RECENT];
static int recent_count = 0;

static void ensure_recent_dir(void) {
    char dir[512];
    snprintf(dir, sizeof(dir), "%s/.config/cmdtui", getenv("HOME"));
    mkdir(dir, 0755);
}

static void load_recent(void) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", getenv("HOME"), RECENT_FILE);

    FILE *f = fopen(path, "r");
    if (!f)
        return;

    char line[256];
    while (fgets(line, sizeof(line), f) && recent_count < MAX_RECENT) {
        line[strcspn(line, "\n")] = 0;

        for (int i = 0; i < cmd_count; i++) {
            if (strcmp(cmds[i].name, line) == 0) {
                recent[recent_count++] = i;
                break;
            }
        }
    }

    fclose(f);
}

static void save_recent(void) {
    ensure_recent_dir();

    char path[512];
    snprintf(path, sizeof(path), "%s/%s", getenv("HOME"), RECENT_FILE);

    FILE *f = fopen(path, "w");
    if (!f)
        return;

    for (int i = 0; i < recent_count; i++)
        fprintf(f, "%s\n", cmds[recent[i]].name);

    fclose(f);
}

static void add_recent(int idx) {
    for (int i = 0; i < recent_count; i++) {
        if (recent[i] == idx) {
            memmove(&recent[i], &recent[i + 1],
                    (recent_count - i - 1) * sizeof(int));
            recent_count--;
            break;
        }
    }

    if (recent_count < MAX_RECENT) {
        memmove(&recent[1], &recent[0],
                recent_count * sizeof(int));
        recent[0] = idx;
        recent_count++;
    }

    save_recent();
}

/* =========================
 * BUILD VISIBLE LIST
 * ========================= */

static void rebuild_visible(ViewMode view) {
    visible_count = 0;

    if (view == VIEW_RECENT) {
        for (int i = 0; i < recent_count; i++)
            visible[visible_count++] = recent[i];
        return;
    }

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

    load_commands();
    qsort(cmds, cmd_count, sizeof(Command), cmp_normal);

    load_recent();

    /* ---------- ncurses ---------- */
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
    draw_ui(selected, offset, desc);

    /* =========================
     * MAIN LOOP
     * ========================= */
    int ch;
    while (1) {
        ch = getch();

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
            else if (ch == '3') {
                view = VIEW_RECENT;
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

                    add_recent(idx);

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

                    initscr();
                    cbreak();
                    noecho();
                    keypad(stdscr, TRUE);
                    curs_set(0);
                    ui_init_colors();

                    rebuild_visible(view);
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
            else if (ch == 27) {
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

        if (dirty) {
            draw_ui(selected, offset, desc);
            if (mode == MODE_SEARCH)
                draw_search(search);
            dirty = 0;
        }
    }

    save_recent();
    endwin();
    return 0;
}
