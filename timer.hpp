
#ifndef _TIMER_HPP
#define _TIMER_HPP

// #include <sys/time.h>
#include <chrono>

double getTimeAsDouble() {
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
  return seconds;
}
class Timer {
  // struct timeval start;
  // struct timeval startpause;
  double pausecompensate;

  double start;
  double startpause;

public:
  Timer() {}
  /// Start timing.
  void tick() {
    // gettimeofday(&start, 0);
    start = getTimeAsDouble();
    pausecompensate = 0;
  }
  double tuck(const char *msg) const {
    /*struct timeval end;*/
    double diff = get_time();

    if (msg != 0) {
      printf("%s: time=%fs\n", msg, diff);
    }
    return diff;
  }

  /// Return time in seconds (excluding paused time)
  double get_time() const {
    // struct timeval end;
    double diff;
    double diff_nopause;
    // gettimeofday(&end, 0);

    double end = getTimeAsDouble();

    diff_nopause = end - start;
    diff = diff_nopause - pausecompensate;

    return diff;
  }

  // Pause time tracking
  void pause() { startpause = getTimeAsDouble(); }

  // Continue time tracking
  void contin() {
    double endpause;
    double diff;
    endpause = getTimeAsDouble();

    diff = endpause - startpause;
    pausecompensate = pausecompensate + diff;
  }
};
Timer g_timer;

#endif
