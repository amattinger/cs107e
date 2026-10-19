#include "ff.h"

// Read from a file (if one exists) to see if we have any
// saved high scores before we start the game.
// If so, the leaderboard is pre-populated with the existing high scores.
void read_highscores(const char *fname);

// Clear all existing high scores, and reset the leaderboard to all zeroes.
void clear_highscores(const char* fname);

// Write all high scores from this session to a file to be saved on the SD card.
void write_highscores(const char *fname);
