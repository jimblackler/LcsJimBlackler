#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif  // WIN32

long long int getMicroseconds() {
#ifdef WIN32
  LARGE_INTEGER time;
  QueryPerformanceCounter(&time);
  return time.QuadPart;
#else
  struct timeval time;
  gettimeofday(&time, 0);
  long long before = 1000000 * time.tv_sec + time.tv_usec;
  return before;
#endif
}