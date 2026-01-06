#include "commands.h"
#include "favorites.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

Command cmds[MAX_CMDS];
int cmd_count = 0;

int visible[MAX_CMDS];
int visible_count = 0;

static int match(const char *s, const char *q) {
    if (!*q) return 1;
    size_t qlen = strlen(q);
    for (; *s; s++) {
        if (strncasecmp(s, q, qlen) == 0)
            return 1;
    }
    return 0;
}

void load_commands(void) {
    char *path = strdup(getenv("PATH"));
    char *tok = strtok(path, ":");

    while (tok && cmd_count < MAX_CMDS) {
        DIR *d = opendir(tok);
        if (d) {
            struct dirent *e;
            while ((e = readdir(d)) && cmd_count < MAX_CMDS) {
                strncpy(cmds[cmd_count].name, e->d_name, NAME_LEN);
                cmds[cmd_count].favorite = is_favorite(e->d_name);
                cmd_count++;
            }
            closedir(d);
        }
        tok = strtok(NULL, ":");
    }
    free(path);

    clear_filter();
}

void clear_filter(void) {
    visible_count = cmd_count;
    for (int i = 0; i < cmd_count; i++)
        visible[i] = i;
}

void apply_filter(const char *query) {
    visible_count = 0;
    for (int i = 0; i < cmd_count; i++) {
        if (match(cmds[i].name, query))
            visible[visible_count++] = i;
    }
}
