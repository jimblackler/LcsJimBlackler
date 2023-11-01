#include <stdbool.h>
#include <stddef.h>

bool isSubstring(const char *a0, const char *b0, size_t sizeA, size_t sizeB) {
  // Iterate over both strings in parallel.
  const char *a = a0;
  if (a == a0 + sizeA)
    return true;  // Empty substring.
  const char *b = b0;
  while (b < b0 + sizeB) {
    if (*a == *b)
      a++;
    if (a == a0 + sizeA)
      return true;
    b++;
  }
  // Reached the end of the larger string without passing all of the characters
  // in the candidate substring.
  return false;
}