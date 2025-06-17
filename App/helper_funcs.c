#include <sys/time.h>

int _gettimeofday(struct timeval *tv, void *tzvp)
{
    // Set the Unix timestamp for 1st January 2025, 00:00:00
    tv->tv_sec = 1735680000;  // Seconds since Unix epoch for 1st January 2025, 00:00:00
    tv->tv_usec = 0;          // Set microseconds to 0 (exactly 00:00:00)

    return 0;  // Return 0 for success
}
