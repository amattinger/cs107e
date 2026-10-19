#include "console.h"
#include "gl.h"
#include "fb.h"
#include "timer.h"
#include "keyboard.h"
#include "printf.h"
#include "strings.h"

#define ESC_SCANCODE 0x76
#define LINE_LEN 80

extern char new_name[];
extern int straightness;

int signed_to_base(char *buf, size_t bufsize, unsigned int val, int base, size_t min_width);

void show_graph(int a_samples[], int a_max, int v_samples[], int v_max, int size, char* name)
{
    gl_swap_buffer();
    gl_clear(GL_BLACK);
    int height = gl_get_height(); // should be 420
    int width = gl_get_width(); // should be 560

    char page_title[LINE_LEN] = "Statistics for ";
    strlcat(page_title, new_name, LINE_LEN);
    strlcat(page_title, "'s last punch:", LINE_LEN);

    gl_draw_string(10, 10, page_title, GL_WHITE);

    // Exit page on escape
    gl_draw_string(10, 35, "(Press ESC to exit this page).", GL_WHITE);

    // Max acceleration and velocity
    char acc[LINE_LEN] = "Your max accleration was ";
    char a_max_str[LINE_LEN];
    char a_max_rem[LINE_LEN];
    signed_to_base(a_max_str, LINE_LEN, a_max/1000, 10, 0);
    strlcat(acc, a_max_str, LINE_LEN);
    strlcat(acc, ".", LINE_LEN);
    signed_to_base(a_max_rem, LINE_LEN, a_max%1000, 10, 0);
    strlcat(acc, a_max_rem, LINE_LEN);
    strlcat(acc, " g.", LINE_LEN);

    gl_draw_string(10, 60, acc, GL_WHITE);

    char vel[LINE_LEN] = "Your max velocity was ";
    char v_max_str[LINE_LEN];
    signed_to_base(v_max_str, LINE_LEN, v_max, 10, 0);
    strlcat(vel, v_max_str, LINE_LEN);
    strlcat(vel, " cm/s.", LINE_LEN);

    gl_draw_string(10, 85, vel, GL_WHITE);

    // straightness
    char straight[LINE_LEN] = "Your straightness was ";
    int percent_straightness = 100 - (straightness * 100) / a_max;
    char str[LINE_LEN];
    signed_to_base(str, LINE_LEN, percent_straightness, 10, 0);
    strlcat(straight, str, LINE_LEN);
    strlcat(straight, " \%.", LINE_LEN);

    gl_draw_string(10, 110, straight, GL_WHITE);
    gl_draw_string(10, 135, "(Ideal straightness would be 100%).", GL_WHITE);

    // velocity
    gl_draw_string(10, height/2 - 15, "velocity vs time", GL_BLUE);
    gl_draw_rect(10, height/2 + 10, width/2 - 20, height/2 - 20, GL_WHITE);

    // fit 130 2x2 dots into width of graph
    for (int i = 0; i < size; i++)
    {
        // the max sample should be ~200 units above the bottom
        // other samples are scaled proportionally
        int v_y = height - 10 - ((v_samples[i] * 180) / v_max);

        // single dots look kind of small, so making 2x2 "dots"
        gl_draw_pixel(2*i+10, v_y, GL_BLUE);
        gl_draw_pixel(2*i+11, v_y, GL_BLUE);
        gl_draw_pixel(2*i+10, v_y-1, GL_BLUE);
        gl_draw_pixel(2*i+11, v_y-1, GL_BLUE);

    }

    // acceleration
    gl_draw_string(width/2+20, height/2 - 15, "accel vs time", GL_RED);
    gl_draw_rect(width/2+10, height/2 + 10, width/2-20, height/2 - 20, GL_WHITE);

    for (int i = 0; i < size; i++)
    {
        // scale vals
        int a_x = 2*i + width/2 + 10;
        int a_y = height - 10 - ((a_samples[i] * 180) / a_max);

        // single dots look kind of small, so making 2x2 "dots"
        gl_draw_pixel(a_x, a_y, GL_RED);
        gl_draw_pixel(a_x+1, a_y, GL_RED);
        gl_draw_pixel(a_x, a_y-1, GL_RED);
        gl_draw_pixel(a_x+1, a_y-1, GL_RED);

    }

    // show drawn graphs
    gl_swap_buffer();

    // "ESC" key to return to main console screen
    while (keyboard_read_next() != ps2_keys[ESC_SCANCODE].ch) {}
}
