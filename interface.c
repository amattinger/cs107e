#include "timer.h"
#include "uart.h"
#include "i2c.h"
#include "printf.h"

#include "LSM6DS33.h"

#include "gpio.h"
#include "keyboard.h"


#include "sb_shell.h"
#include "console.h"
#include "interrupts.h"

#include "accel.h"
#include "math.h"

#include "ff.h"
#include "sd_fatfs.h"

#define NUM_ROWS 20
#define NUM_COLS 40

void main(void)
{
    timer_init();
    uart_init();

    FATFS fs;
    FRESULT res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        printf("Could not mount internal SD card. Error: %s\n", ff_err_msg(res));
        return;
    }

    i2c_init();
    lsm6ds33_init();

    gpio_init();
    keyboard_init(GPIO_PIN5, GPIO_PIN6);
    console_init(NUM_ROWS, NUM_COLS, GL_AMBER, GL_BLACK);

    shell_init(keyboard_read_next, console_printf);
    leaderboard_init();

    shell_run();
}
