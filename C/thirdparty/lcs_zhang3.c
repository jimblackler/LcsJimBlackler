/*******************************************************
*     MYCPLUS Sample Code - http://www.mycplus.com     *
*                                                     *
*   This code is made available as a service to our   *
*      visitors and is provided strictly for the      *
*               purpose of illustration.              *
*                                                     *
* Please direct all inquiries to saqib at mycplus.com *
*******************************************************/

//******
// lcs.c
//** - finds the longest common sub-sequence between 2 strings
//** - implements the most famous dynamic programming algorithm
//** - output is a null-terminated string, so must be the input
//** Notes
//** - this package is provided as is with no warranty.
//** - the author is not responsible for any damage caused
//**   either directly or indirectly by using this package.
//** - anybody is free to do whatever he/she wants with this
//**   package as long as this header section is preserved.
//** Created on 2005-01-22 by
//** - Roger Zhang (rogerz@cs.dal.ca)
//** Modifications
//** -
//** Last compiled under Linux with gcc-3
//**************************

#include <string.h>
#include <stdlib.h>
#include <assert.h>s

#define a(i, j) mem[(i) * (m + 1) + (j)]

char *LCS_Zhang3(const char *s, const char *t) {

  size_t n = strlen(s);
  size_t m = strlen(t);

  // Size required will overflow size_t?
  if (m + 1 > SIZE_MAX / (n + 1))
    return NULL;

  int *mem = calloc((n + 1) * (m + 1), sizeof(int));
  if (!mem)
    return NULL;

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      if (s[i] == t[j]) {
        a(i + 1, j + 1) = a(i, j) + 1;
      } else {
        int v1 = a(i + 1, j);
        int v2 = a(i, j + 1);
        if (v1 > v2)
          a(i + 1, j + 1) = v1;
        else
          a(i + 1, j + 1) = v2;
      }
    }
  }

  int lcs = a(n, m);

  char *result = malloc(lcs + 1);
  if (!result) {
    free(mem);
    return NULL;
  }

  char *write = result + lcs;
  *write = 0;

  int n2 = n;
  int m2 = m;

  while (n2 > 0 && m2 > 0) {
    if (s[n2 - 1] == t[m2 - 1]) {
      n2--;
      m2--;
      *--write = s[n2];
    } else {
      if (a(n2, m2 - 1) >= a(n2 - 1, m2))
        m2--;
      else
        n2--;
    }
  }
  free(mem);
  return result;
}


