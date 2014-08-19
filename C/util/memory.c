#ifdef MEMORY_PROFILE
#undef malloc
#undef calloc
#include <stdlib.h>

int memoryCount = 0;

void *profileMalloc(size_t size) {
  memoryCount += size;
  return malloc(size);
}

void *profileCalloc(size_t num_items, size_t size) {
  memoryCount += num_items * size;
  return calloc(num_items, size);
}
#endif