#pragma once

void draw_ui(int selected, int offset);
void draw_search(const char *query);

/* returns 1 = execute, 0 = cancel */
int draw_execute_dialog(const char *cmd, char *args, int maxlen);

