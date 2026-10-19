
int abs(int x) {
  if (x > 0) {
    return x;
  }
  else {
    return -x;
  }
}

// This uses the concept from this website: https://www.geeksforgeeks.org/square-root-of-a-number-without-using-sqrt-function/
// Essentially we are solving the equation
// y^2 – 2y√x + x = 0
// where y = sqrt(x).

int sqrt(int x) {

  if (x < 2) {
        return x;
  }

  short y = x;
    double z = (y + (x / y)) / 2;

    // this will give the square root within 1 unit
    // (which is all that we need)
    while (abs(y - z) > 1) {
        y = z;
        z = (y + (x / y)) / 2;
    }

    return abs(z);

}
