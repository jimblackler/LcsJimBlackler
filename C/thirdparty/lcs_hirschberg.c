#include <stdlib.h>
#include <string.h>

// Based on
// https://code.google.com/p/algorithm800/source/browse/trunk/src/Hirschberg.java
// by Valentinos Georgiades and Minh Nguyen
// Converted from Java to C by Jim Blackler.

#define MAX(a, b) (((a)>(b))?(a):(b))

static int *algB(size_t m, size_t n, const char *a, const char *b) {

  // Step 1
  int *k0 = calloc(sizeof(int), n + 1);
  int *k1 = calloc(sizeof(int), n + 1);

  // Step 2
  for (int i = 1; i <= m; i++) {
    // Step 3
    for (int j = 0; j <= n; j++)
      k0[j] = k1[j];


    // Step 4
    for (int j = 1; j <= n; j++) {
      if (a[i - 1] == b[j - 1]) {
        k1[j] = k0[j - 1] + 1;
      } else {
        k1[j] = MAX(k1[j - 1], k0[j]);
      }
    }
  }

  free(k0);
  // Step 5
  return k1;
}

static int findK(int *l1, int *l2, int n) {
  int m = 0;
  int k = 0;

  for (int j = 0; j <= n; j++) {
    if (m < (l1[j] + l2[n - j])) {
      m = l1[j] + l2[n - j];
      k = j;
    }
  }

  return k;
}

char *reverseString(char const *a, size_t n) {
  char *c = malloc((n + 1) * sizeof(char));
  c += n;
  *c = 0;
  while (n--)
    *--c = *a++;
  return c;
}

char *algC(size_t m, size_t n, const char *a, const char *b) {

  // Step 1
  if (n == 0) {
    return calloc(sizeof(char), 1);
  } else if (m == 1) {
    for (int j = 0; j < n; j++) {
      if (a[0] == b[j]) {
        char *c = malloc(sizeof(char) * 2);
        c[0] = a[0];
        c[1] = 0;
        return c;
      }
    }
    return calloc(sizeof(char), 1);
    // Step 2
  } else {
    int i = m / 2;

    // Step 3
    int *l1 = algB(i, n, a, b);
    char *rev_a = reverseString(a + i, m - i);
    char *rev_b = reverseString(b, n);
    int *l2 = algB(m - i, n, rev_a, rev_b);

    // Step 4
    int k = findK(l1, l2, n);

    // Step 5
    char *c1 = algC(i, k, a, b);
    char *c2 = algC(m - i, n - k, a + i, b + k);

    char *c = malloc((strlen(c1) + strlen(c2) + 1) * sizeof(char));
    strcpy(c, c1);
    strcat(c, c2);
    
    free(c1);
    free(c2);
    free(rev_a);
    free(rev_b);
    free(l1);
    free(l2);
    
    return c;
  }
}

char *LCS_Hirschberg(const char *a, const char *b) {
  size_t m = strlen(a);
  size_t n = strlen(b);
  return algC(m, n, a, b);
}
