#pragma once

#define MAX_CMDS 4096
#define NAME_LEN 256

typedef struct {
    char name[NAME_LEN];
    int favorite;
} Command;

extern Command cmds[MAX_CMDS];
extern int cmd_count;

void load_commands(void);

