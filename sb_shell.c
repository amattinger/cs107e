#include "sb_shell.h"
#include "uart.h"
#include "printf.h"
#include "keyboard.h"
#include "strings.h"
#include "malloc.h"
#include "backtrace.h"
#include "pi.h"
#include "armtimer.h"
#include "graph.h"
#include "sd_fatfs.h"
#include "leaderboard.h"

#include "timer.h"
#include "interrupts.h"

#include "accel.h"

#define LINE_LEN 80

static input_fn_t shell_read;
static formatted_fn_t shell_printf;


static const command_t commands[] = {
    {"menu", "          display options"},
    {"help", "          detailed instructions"},
    {"start", "         collect user strike"},
    {"scores", "        display leaderboard"},
    {"graph", "         display user performance"},
    {"clear", "         clear all high scores"},
    {"quit", "          end game"},
};


/*
 * COMMAND FUNCTIONS
 */
#define NUM_COMMANDS 7
#define LINE_LEN 80
#define MAX_SAMPLES 130
#define NUM_SCORES 10

// New user data
char new_name[LINE_LEN];
int new_score;
int a_max = 0;
int a_avg = 0;
int v_max = 0;
int v_avg = 0;
int straightness = 0;
int a_samples[MAX_SAMPLES];
int v_samples[MAX_SAMPLES];

// Leaderboard data
score_t leaderboard[NUM_SCORES];
int low_high_score = 0;
const char* file = "scores.txt"; // file to save high scores

// At start of game, initialize empty leaderboard (if no saved high scores)
// or populate leaderboard with existing saved high scores.
void leaderboard_init(void)
{
  read_highscores(file);
  low_high_score = leaderboard[NUM_SCORES - 1].score;
}

// collects one strike from the user
int cmd_start(void)
{
    // Get name, then commence countdown
    shell_printf("What's your name?\n");
    shell_readline(new_name, sizeof(new_name));
    shell_printf("Hi %s! Get ready in...\n", new_name);

    for (int i = 5; i > 0; i--)
    {
        shell_printf("%d\n", i);
        shell_bell();
        timer_delay(1);
    }
    shell_bell();
    shell_printf("GO!\n");

    // Gather strike data
    capture_one_punch(&a_max, &a_avg, &v_max, &v_avg, &straightness, a_samples, v_samples, MAX_SAMPLES);

    new_score = a_max;
 
    // Assess whether user made it to leaderboard
    shell_printf("Great punch!\n");
    sort_score();
    shell_printf("Your max acceleration was %d.%d g.\n", new_score/1000, new_score%1000);
    shell_printf("To view more metrics, type 'graph'.\n");
    shell_printf("To view the leaderboard, type 'scores'.\n");
    shell_printf("To quit the game, type 'quit'.\n");

    return 0;
}

// helper function to place user on leaderboard (if they got a high score) after their punch
void sort_score(void)
{
    int i = 0;

    // CASE: no high score for you :(
    if (new_score < low_high_score) { return; }

    // go through numscores until spot is found
    while (leaderboard[i].score > new_score && i < NUM_SCORES) { i++; }


    // handle different endings of places (e.g. 1st, 3rd, etc)
    if (i == 0) {
        shell_printf("You are in %dst place. Good job!\n", i+1); // 1st
    }
    else if (i == 1) {
        shell_printf("You are in %dnd place. Good job!\n", i+1); // 2nd
    }
    else if (i == 2) {
        shell_printf("You are in %drd place. Good job!\n", i+1); // 3rd
    }
    else {
      shell_printf("You are in %dth place. Good job!\n", i+1);
    }

    // shift everyone else down
    for (int j = NUM_SCORES - 1; j > i; j--)
    {
        leaderboard[j].score = leaderboard[j-1].score;
        memcpy(leaderboard[j].name, leaderboard[j-1].name, LINE_LEN);
    }

    // put new score in
    memcpy(leaderboard[i].name, new_name, sizeof(new_name));
    leaderboard[i].score = new_score;

    // set new low_high_score
    low_high_score = leaderboard[NUM_SCORES-1].score;
}

// leaderboard of high scores
int cmd_leaderboard(void)
{
    int i = 0;

    shell_printf("          Leaderboard\n");
    shell_printf("-----------------------------------\n");
    while (i < NUM_SCORES && leaderboard[i].score != 0)
    {
        shell_printf("%d:         %s      %d.%d g\n", i+1, leaderboard[i].name, leaderboard[i].score/1000, 
                leaderboard[i].score%1000);
        i++;
    }

    return 0;
}

// draws and displays performance graphs/metrics to user
int cmd_graph(int a_samples[], int a_max, int v_samples[], int v_max, int size)
{
    show_graph(a_samples, a_max, v_samples, v_max, size, new_name);

    return 0;
}

// shows all command options
int cmd_menu(void)
{
    for (int i = 0; i < NUM_COMMANDS; i++)
    {
        shell_printf("%s%s\n", commands[i].name, commands[i].description);
    }

    return 0;
}

// clears leaderboard of all saved entries
int cmd_clear(void)
{
  low_high_score = 0;
  clear_highscores(file);
  return 0;
}

// ends game and reboots pi
int cmd_quit(void)
{
    shell_printf("Quitting game. Goodbye!\n");
    write_highscores(file);
    uart_putchar(EOT);
    pi_reboot();
    return 0;
}

// detailed instructions for gameplay
int cmd_help(void)
{
    shell_printf("To play:\n\n");
    shell_printf("1. Gear up, then enter 'start'.\n\n");
    shell_printf("2. Enter your name to begin countdown.\n\n");
    shell_printf("3. At 'GO!' immediately throw a straight   punch.\n\n");
    shell_printf("4. Interpreting 'graph':\n");
    shell_printf("   Ideally, your retraction should be no   slower than your extension, and there   should be almost no delay between the   two.\n\n");
    shell_printf("5. Enter 'start' to go again, or 'menu'    for all options.\n\n");
    return 0;
}

/*
 * SHELL
 */

void shell_init(input_fn_t read_fn, formatted_fn_t print_fn)
{
    shell_read = read_fn;
    shell_printf = print_fn;
}

// Shell bell used for countdown
void shell_bell(void)
{
    uart_putchar('\a');
}

#define CTRL_A 0x9A
#define CTRL_E 0x9B
#define CTRL_U 0x9C

// Shell readline from assign5
void shell_readline(char buf[], size_t bufsize)
{
    // initialize buffer to fix truncation by previous line issue
    for (int b = 0; b < bufsize; b++) { buf[b] = '\0'; }

    int i = 0;          // represents place in buffer
    int endline = 0;    // represents end of buffer; usually same as i


    while (buf[i] != '\n' && endline < bufsize)
    {
        char dummy = buf[i];
        buf[i] = shell_read();

        // CASE: exceeds allowable length
        if (endline >= bufsize-2) { shell_bell(); }

        // CASE: backspace
        else if (buf[i] == '\b')
        {
            // can't backspace past beginning of line
            if (i < 1) { shell_bell(); }

            else
            {
                shell_printf("\b \b");
                buf[endline] = ' ';

                if (i != endline)
                {
                    for (int j = i; j < endline; j++)
                    {
                        buf[j] = buf[j+1];
                        shell_printf("%c", buf[j]);
                    }

                    buf[endline] = ' ';
                    shell_printf(" \b"); // delete from end

                    for (int j = endline; j > i; j--)
                    {
                        shell_printf("\b");
                    }

                }
                endline--;
                i--;
            }
        }

        // CASE: left arrow
        else if (buf[i] == PS2_KEY_ARROW_LEFT)
        {
            if (i < 1) { shell_bell(); }

            else {
                buf[i] = dummy;
                shell_printf("\b");
                i--;
            }
        }

        // CASE: right arrow
        else if (buf[i] == PS2_KEY_ARROW_RIGHT)
        {
            // check that not and end of typed chars
            if (i == endline) { shell_bell(); }
            else {
                // to "move" right, rewrite char to right of cursor
                shell_printf("%c", dummy);
                buf[i] = dummy;     // store actual char, not arrow key press
                i++;
            }
        }

        // CASE: Ctrl-a
        else if (buf[i] == CTRL_A)
        {
            buf[i] = dummy;
            shell_printf("\rPi> ");
            i = 0;
        }

        // CASE: Ctrl-e
        else if (buf[i] == CTRL_E)
        {
            buf[i] = dummy;
            for (int j = i; j < endline; j++)
            {
                shell_printf("%c", buf[j]);
            }
            i = endline;
        }

        // Barring above cases or newline, print and keep going
        else if (buf[i] != '\n')
        {
            shell_printf("%c", buf[i]);
            endline++;
            i++;
            if (i < endline)
            {
                for (int j = endline; j > i; j--)
                {
                    buf[j] = buf[j-1];
                }
                for (int j = i; j < endline; j++)
                {
                    shell_printf("%c", buf[j]);
                }
                for (int j = endline; j > i; j--)
                {
                    shell_printf("\b");
                }
            }
        }
    }

    shell_printf("\n");
    buf[endline] = '\0';
}

// Evaluate which command has been entered
int shell_evaluate(const char *line)
{
    int result = -1;

    // Evaluate
    if (strcmp(line, "start") == 0) { result = cmd_start(); }
    else if (strcmp(line, "scores") == 0) { result = cmd_leaderboard(); }
    else if (strcmp(line, "help") == 0) { result = cmd_help(); }
    else if (strcmp(line, "menu") == 0) { result = cmd_menu(); }
    else if (strcmp(line, "quit") == 0) { result = cmd_quit(); }
    else if (strcmp(line, "graph") == 0) { result = cmd_graph(a_samples, a_max, v_samples, v_max, MAX_SAMPLES); }
    else if (strcmp(line, "clear") == 0) { result = cmd_clear(); }
    else { shell_printf("error: no such command '%s'.\n", line); }

    return result;
}

// Shell run with welcome message
void shell_run(void)
{
    shell_printf("Do you like...\n");
    shell_printf("...arbitrary perfomance metrics?\n");
    shell_printf("...aggressively displacing air?\n");
    shell_printf("Good! You've come to the right place.\n\n");
    shell_printf("Type 'menu' for your options.\n");

    while (1)
    {
        char line[LINE_LEN];

        shell_printf("Pi> ");
        shell_readline(line, sizeof(line));
        shell_evaluate(line);
    }
}
