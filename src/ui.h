#pragma once

/* init */
void ui_init_colors(void);

/* main split ui */
void draw_ui(int selected, int offset, const char *desc);

/* search prompt */
void draw_search(const char *query);

/* execute dialog
 * return 1 = execute, 0 = cancel
 */
int draw_execute_dialog(const char *cmd, char *args, int maxlen);
