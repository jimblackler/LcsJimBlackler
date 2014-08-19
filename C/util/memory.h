#include <stddef.h>
#define MEMORY_PROFILE

#define malloc(a) profileMalloc(a)
#define calloc(a, b) profileCalloc(a, b)
extern void *profileMalloc(size_t size);
extern void *profileCalloc(size_t num_items, size_t size);

extern int memoryCount;