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

char *LCS_RogerZhang(const char *s, const char *t, size_t n, size_t m, size_t *lcs)
{
   char *result;
   int i, j, **a;

   if (!n || !m) { /* empty input string */
     *lcs = 0;
     return calloc(sizeof(char), 1);
   }

   a = (int**)calloc(n + 1, sizeof(int*));
   a[0] = (int*)calloc((n + 1) * (m + 1), sizeof(int));

   for (i = a[0][0] = 0; i <= n; i++) { /* find the length */
      if (!i || (a[i] = a[i - 1] + m + 1)) {
         for (j = 0; j <= m; j++) {
            if (!i || !j) { /* initialize the base row/column */
               a[i][j] = 0;
            } else if (s[i - 1] == t[j - 1]) { /* diagonal step */
               a[i][j] = a[i - 1][j - 1] + 1;
            } else { /* horizontal or vertical step */
               a[i][j] = a[i][j - 1] > a[i - 1][j] ? a[i][j - 1] : a[i - 1][j];
            }
         }
      } else {
         abort(); /* memory failure */
      }
   }

   if (!(i = a[n][m])) {
     free(a[0]);
     free(a);
     *lcs = 0;
     return calloc(sizeof(char), 1); /* no common sub-sequence */
   }

   *lcs = i;
   if (!(result = (char*)malloc(i)) || (result[i] = '\0')) {
      abort(); /* memory allocation failed */
   }

   while (n > 0 && m > 0) { /* back track to find the sequence */
      if (s[n - 1] == t[m - 1] && m--) {
         result[--i] = s[--n];
      } else {
         a[n][m - 1] >= a[n - 1][m] ? m-- : n--;
      }
   }

   free(a[0]);
   free(a);

   return result;
}