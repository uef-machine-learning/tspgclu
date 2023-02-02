
#ifndef _TIMER_HPP
#define _TIMER_HPP

#include <sys/time.h>
class Timer
{
    struct  timeval start;
    struct  timeval startpause;
    double pausecompensate;

    public:
    Timer () {}
    /// Start timing.
    void tick ()
    {
        gettimeofday(&start, 0);
        pausecompensate=0;
    }
    double tuck (const char *msg) const
    {
        /*struct timeval end;*/
        double   diff = get_time();

        if (msg != 0) {
            printf("%s: time=%fs\n",msg,diff);
        }
        return diff;
    }


    /// Return time in seconds (excluding paused time)
    double get_time () const
    {
        struct timeval end;
        double   diff;
        double   diff_nopause;
        gettimeofday(&end, 0);

        diff_nopause = (end.tv_sec - start.tv_sec)
                + (end.tv_usec - start.tv_usec) * 0.000001;
        diff = diff_nopause - pausecompensate;

        return diff;
    }



    // Pause time tracking
    void pause() {
        gettimeofday(&startpause, 0);
    }

    // Continue time tracking
    void contin() {
        struct timeval endpause;
        double   diff;
        gettimeofday(&endpause, 0);

        diff = (endpause.tv_sec - startpause.tv_sec)
                + (endpause.tv_usec - startpause.tv_usec) * 0.000001;
        pausecompensate = pausecompensate + diff;

    }


};
Timer g_timer;


#endif
