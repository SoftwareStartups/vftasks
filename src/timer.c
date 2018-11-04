#include "vftasks.h"
#include "platform.h"

#include <stdint.h>
#include <assert.h>

#ifdef _POSIX_SOURCE

#include <time.h>

void vftasks_timer_start(uint64_t *start)
{
  struct timespec tp;
  clock_gettime(CLOCK_REALTIME, &tp);
  *start = tp.tv_sec * ((uint64_t) 1000000000) + tp.tv_nsec;
}

uint64_t vftasks_timer_stop(uint64_t *start)
{
  struct timespec tp;
  uint64_t stop;

  clock_gettime(CLOCK_REALTIME, &tp);
  stop = tp.tv_sec * ((uint64_t) 1000000000)+ tp.tv_nsec;

  return stop - *start;
}

#elif defined (_WIN32)

#include <windows.h>

void vftasks_timer_start(uint64_t *start)
{
  int rc;
  LARGE_INTEGER val;

  rc = QueryPerformanceCounter(&val);
  assert(rc);
  *start = val.QuadPart;
}

uint64_t vftasks_timer_stop(uint64_t *start)
{
  LARGE_INTEGER val, freq;
  double resolution;
  int rc;

  rc = QueryPerformanceCounter(&val);
  assert(rc);
  rc = QueryPerformanceFrequency(&freq);
  assert(rc);
  resolution = (double)(1000000000L / freq.QuadPart);

  return (uint64_t)((val.QuadPart - *start) * resolution);
}

#else
#error("unsupported platform")
#endif /* _POSIX_SOURCE / _WIN32 */
