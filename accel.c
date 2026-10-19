#include "timer.h"
#include "uart.h"
#include "i2c.h"
#include "printf.h"
#include "malloc.h"

#include "LSM6DS33.h"
#include "math.h"

#define DELAY 150 // in ms
#define DATA_DELAY 10 // in ms
#define PUNCHTIME 1000000 // in us

void capture_one_punch(int* a_max, int* a_avg, int* v_max, int* v_avg, int* straightness, int* a_samples, int* v_samples, int max_samples) {

    // initialize max acceleration and average acceleration
    *a_max = 0;
    *a_avg = 0;
    int num_samples = 0;

    timer_delay_ms(DELAY);

    // get initial acceleration
    int ax_last, ay_last, az_last;
    lsm6ds33_read_accelerometer(&ax_last, &ay_last, &az_last);

    // store initial x-acceleration for velocity calculations
    // also keep track of when the last reading was
    int last_reading = timer_get_ticks();
    ax_last = ax_last / 2;
    ay_last = ay_last / 2;
    az_last = az_last / 2;


    // we assume the hand starts stationary
    // initialize max velocity and average velocity
    *v_max = 0;
    *v_avg = 0;
    int vx = 0;
    int vy = 0;
    int vz = 0;

  int start_time = timer_get_ticks();

    // a fast punch is < 100 us, we assume any punch will be < 1 s
	while((timer_get_ticks() - start_time) < PUNCHTIME && num_samples < max_samples) {

        int ax, ay, az;
        lsm6ds33_read_accelerometer(&ax, &ay, &az);

        // handle negative accelerations (this is necessary bc negative values
        // are stored in two's complement in the register)
        if (ax > (1 << 15)) {
          ax = ax - (1 << 16);
        }
        if (ay > (1 << 15)) {
          ay = ay - (1 << 16);
        }
        if (az > (1 << 15)) {
          az = az - (1 << 16);
        }

        // collect time between readings
        int t = timer_get_ticks();
        int dt = t - last_reading;
        last_reading = t;

        // 2048 is 1g (1g == 1000mg)
        // to calculate this: we have 16 bits, i.e. 2^16 to store +/- 16g
        // so divide 2^16 / 32 = 2048 for 1g, meaning that we want to divide
        // by ~2 to get units of mg
        ax = ax / 2;
        ay = ay / 2;
        az = az / 2;
        printf("x: %d, y: %d, z: %d\n", ax, ay, az);
        int ax_mag = abs(ax);
        int ay_mag = abs(ay);
        int az_mag = abs(az);

        // The ideal punch will be 'straight' along the x-axis. We can get an
        // approximate measurement of this by comparing the magnitude of the
        // linear x-acceleration to the magnitude of the total linear acceleration
        // (minus the effects of gravity).
        // This is because in a perfect punch, gravity will only be in the
        // y and z directions for the duration of the punch.
        int a_mag = sqrt(ax * ax + (ay) * (ay) + (az) * (az));
        *straightness += abs(a_mag - ax_mag);

        // calculate average velocity for this time interval
        // units should be mg * us
        // We can get this into cm/s by dividing by 10^7
        vx = vx + (((ax + ax_last) / 2) * dt);
        vy = vy + (((ay + ay_last) / 2) * dt);
        vz = vz + (((az + az_last) / 2) * dt);
        int vx_mag = abs(vx) / 10000000;
        int vy_mag = abs(vy) / 10000000;
        int vz_mag = abs(vz) / 10000000;
        ax_last = ax;
        ay_last = ay;
        az_last = az;

        // acceleration stored in mg
        // velocity stored in cm / s.
        a_samples[num_samples] = ax_mag;
        v_samples[num_samples] = vx_mag;

        if (ax_mag > *a_max) {
          *a_max = ax_mag;
        }

        if (vx_mag > *v_max) {
          *v_max = vx_mag;
        }

        *a_avg += ax_mag;
        *v_avg += vx_mag;
        num_samples += 1;

        timer_delay_us(DATA_DELAY);
	}

  // divide by total number of samples collected to get the overall magnitude
  // of acceleration and velocity
  *a_avg = (*a_avg) / num_samples;
  *v_avg = (*v_avg) / num_samples;

  // we only store the average "straightness" of the punch
  *straightness = (*straightness) / num_samples;
}
