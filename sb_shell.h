#ifndef SHELL_H
#define SHELL_H

#include <stddef.h>

typedef struct _command_struct {
    const char *name;
    const char *description;
} command_t;

// initializes leaderboard at start of game with previously saved scores [if any]
void leaderboard_init(void);

// helper function determines whether user made it to the leaderboard and places them accordingly
void sort_score(void);

// draws and displays graph/metrics of most recent strike
int cmd_graph(int a_samples[], int a_max, int v_samples[], int v_max, int size);

// gathers one user name and strike
int cmd_start(void);

// displays menu options
int cmd_menu(void);

// detailed instructions
int cmd_help(void);

// displays leaderboard
int cmd_leaderboard(void);

// clears any saved high score data from SD card
int cmd_clear(void);

// reboots pi
int cmd_quit(void);

// from assign5
typedef unsigned char (*input_fn_t)(void);
typedef int (*formatted_fn_t)(const char *format, ...) __attribute__((format(printf, 1, 2)));
void shell_init(input_fn_t read_fn, formatted_fn_t print_fn);
void shell_bell(void);
void shell_readline(char buf[], size_t bufsize);
int shell_evaluate(const char *line);
void shell_run(void);
#endif
