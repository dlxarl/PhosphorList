#include "favorites.h"
#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void fav_path(char *buf, size_t n) {
    snprintf(buf, n, "%s/.config/cmdtui", getenv("HOME"));
    mkdir(buf, 0755);
    snprintf(buf, n, "%s/.config/cmdtui/favorites.txt", getenv("HOME"));
}

int is_favorite(const char *name) {
    char path[512];
    fav_path(path, sizeof(path));

    FILE *f = fopen(path, "r");
    if (!f) return 0;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;
        if (!strcmp(line, name)) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void save_favorites() {
    char path[512];
    fav_path(path, sizeof(path));

    FILE *f = fopen(path, "w");
    if (!f) return;

    for (int i = 0; i < cmd_count; i++)
        if (cmds[i].favorite)
            fprintf(f, "%s\n", cmds[i].name);

    fclose(f);
}

