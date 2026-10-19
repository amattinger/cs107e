/*
 * Using fatfs to access save/read high scores on the Pi SD card
 */

#include "printf.h"
#include "strings.h"
#include "uart.h"
#include "ff.h"
#include "ff_extra.h"
#include "leaderboard.h"

// from sb_shell
extern score_t leaderboard[];

#define LINE_LEN 80
#define NUM_SCORES 10

void read_highscores(const char *fname) {
    FRESULT res;
    printf("Attempting to open file %s.\n", fname);
    FIL fp;
    char line[100]; /* Line buffer */
    res = f_open(&fp, fname, FA_OPEN_EXISTING| FA_READ);

    // initialize all slots to 0
    for (int i = 0; i < NUM_SCORES; i++)
    {
        memset(leaderboard[i].name, '\0', LINE_LEN);
        leaderboard[i].score = 0;
    }

    // no file found -- no existing saved scores
    if (res != FR_OK) {
      return;
    }

    // read the entire file
    // each line should be stored as "name score"
    // store any exisitng high scores in leaderboard
    printf("Reading:\n");
    int i = 0;
    while (f_gets((TCHAR*) line, sizeof line, &fp)) {
        printf("%s", line);
        int j = 0;
        char name[LINE_LEN];
        int score = 0;
        while (line[j] != ' ') {
          name[j] = line[j];
          j++;
        }
        name[j] = 0;
        score = strtonum(line + j + 1, NULL);
        memcpy(leaderboard[i].name, (const char*) name, strlen(name));
        leaderboard[i].score = score;
        i++;
    }

    /* Close the file */
    f_close(&fp);

}


void clear_highscores(const char* fname) {
      // reset all leaderboard slots to 0
      for (int i = 0; i < NUM_SCORES; i++)
      {
          memset(leaderboard[i].name, '\0', LINE_LEN);
          leaderboard[i].score = 0;
      }

      // remove the file
      // see http://elm-chan.org/fsw/ff/doc/unlink.html
      // for information about the f_unlink function
      FRESULT res = f_unlink(fname);
      if (res != FR_OK) {
          printf("Could not remove file %s. Error: %s\n",fname, ff_err_msg(res));
          return;
      }
      printf("Removed file %s.\n",fname);
}


void write_highscores(const char *fname)
{

    FRESULT res;

    // create a new file in the root directory
    // see http://elm-chan.org/fsw/ff/doc/open.html
    // for information about the f_open function
    printf("Attempting to create file %s.\n", fname);
    FIL fp;
    res = f_open(&fp, fname, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        printf("Could not create file %s. Error: %s\n",fname, ff_err_msg(res));
        return;
    }

    // write all scores in leaderboard
    printf("Wrote:\n");
    int i = 0;
    while (i < NUM_SCORES && leaderboard[i].score != 0) {
      char line[LINE_LEN];
      snprintf(line, sizeof line, "%s %d\n", leaderboard[i].name, leaderboard[i].score);
      printf("%s", line);
      f_puts((const TCHAR*) line, &fp);
      i++;
    }

    f_close(&fp);   // close file (should commit to media)
}
