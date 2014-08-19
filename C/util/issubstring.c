#include "issubstring.h"

bool isSubstring(const char *candidateSubstring, const char *largerString) {
  // Iterate over both strings in parallel.
  const char *a = candidateSubstring;
  const char *b = largerString;
  while (*b) {
    if (*a == *b)  // Match found, move to the next candidate substring char.
      a++;
    if (!*a)
      return true;  // Reached the end of the candidate substring.
    b++;
  }
  // Reached the end of the larger string without passing all of the characters
  // in the candidate substring.
  return false;
}