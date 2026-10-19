## CS107E Punchthingy (better title indefinitely TBD): a bare-metal haptic shadowboxing game

A Raspberry Pi game that measures how fast and how straight you punch. Built with Emma Cruz as the final project for Stanford's CS107E (Computer Systems from the Ground Up), Fall 2022, in ~10 days during finals, on a $25 parts budget.

No operating system and no standard library. Everything runs bare-metal on the Pi: the console, keyboard driver, graphics library, printf, malloc, interrupts and backtrace are all things we wrote earlier in the quarter as assignments. That system layer is not in this repo, per course honor policy on assignment code; this repo is the project built on top of it.

Hardware: a Pololu LSM6DS33 IMU (accelerometer, ±16g) over I2C, mounted on a borrowed sparring glove; a PS/2 keyboard; HDMI out; the SD card for persistence.

Demo (2.5 minutes): https://lnkd.in/p/gkC_hDPg

## Description (how to play)

Put on the glove, type start, enter your name, and throw a straight right at the end of the countdown. The game reports your peak acceleration, tells you whether you made the leaderboard, and can show velocity-vs-time and acceleration-vs-time plots plus a "straightness" score. The leaderboard persists across reboots on the Pi's SD card.

Commands: menu, help, start, scores, graph, clear, quit.

## The interesting parts
- Integration without a library. Velocity is accumulated by the trapezoidal rule: each step adds the mean of the last two acceleration samples times the elapsed microseconds, using the Pi's hardware timer for dt. Units are kept in integers throughout (milli-g, microseconds) and scaled down at the end, because there is no floating-point library and we wanted to avoid soft-float anyway.
- Straightness. For an ideal straight punch, the acceleration vector lies along the glove's x-axis. We score straightness as how much the total acceleration magnitude exceeds the x-component, averaged over the punch, then report it as a percentage of peak acceleration.
- Raw IMU plumbing. The sensor returns 16-bit two's-complement values that arrive as unsigned; sign recovery and the 2048-counts-per-g scaling are done by hand in accel.c.
- Persistence on bare metal. The leaderboard is a text file parsed line by line with our own tokenizer, since there is no scanf.

## References
- We used the existing i2c, LSM6D33, and ff_extra files (Emma altered the LSM6D33 file to suit our project).
- Anna based sb_shell largely on her assign5 shell code.
- Emma based sd_fatfs on SD card example code.

## Known issues and limitations (what we'd have fixed with another week)
- Gravity is not separated out. As the glove rotates during a punch, gravity leaks into the x-axis and velocity drifts. A sensor with onboard fusion, or using the gyro we couldn't get working, would fix it.
- The accelerometer saturates. ±16g is not enough for a fast punch.
- sqrt overflows. The first Newton guess is stored in a short, so inputs above 32767 wrap. The squared-magnitude sum can exceed that on a hard punch.
- A countdown instead of a threshold. Detecting punch onset from acceleration would be better than asking the user to wait for a timer.
- Straight punches only. Hooks and uppercuts were the original plan.
