#include "commands.h"
#include "favorites.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Command cmds[MAX_CMDS];
int cmd_count = 0;

void load_commands() {
    char *path = strdup(getenv("PATH"));
    char *token = strtok(path, ":");

    while (token) {
        DIR *d = opendir(token);
        if (d) {
            struct dirent *ent;
            while ((ent = readdir(d)) && cmd_count < MAX_CMDS) {
                strncpy(cmds[cmd_count].name, ent->d_name, NAME_LEN);
                cmds[cmd_count].favorite = is_favorite(ent->d_name);
                cmd_count++;
            }
            closedir(d);
        }
        token = strtok(NULL, ":");
    }
    free(path);
}

