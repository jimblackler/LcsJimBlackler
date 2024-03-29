#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LCS_NONE	0
#define LCS_UP		1
#define LCS_LEFT	2
#define LCS_UP_LEFT	3

struct LCS {
  int **b;
  int **c;
  int m;
  int n;
  const char *x;
  const char *y;
  char *z;
};

static struct LCS *lcs = NULL;

int
initLCS(const char *X, const char *Y, int n, int m)
{
  int i, j;

  if(NULL == X || NULL == Y)
    return 0;

  lcs = (struct LCS *)malloc(sizeof(*lcs));

  if(NULL == lcs)
    return -1;

  lcs->m = m;
  lcs->n = n;
  lcs->x = X;
  lcs->y = Y;

  lcs->b = (int **)malloc((lcs->m + 1) * sizeof(int *));
  lcs->c = (int **)malloc((lcs->m + 1) * sizeof(int *));

  if(NULL == lcs->b || NULL == lcs->c){
    free(lcs->b);
    free(lcs->c);
    free(lcs);
    return -1;
  }

  for(i = 0; i <= lcs->m; ++i) {
    lcs->b[i] = (int *)malloc((lcs->n + 1) * sizeof(int));
    lcs->c[i] = (int *)malloc((lcs->n + 1) * sizeof(int));
  }

  for(i = 0; i <= lcs->m; ++i) {
    lcs->c[i][0] = 0;
    lcs->b[i][0] = LCS_UP;
  }

  for(j = 0; j <= lcs->n; ++j) {
    lcs->c[0][j] = 0;
    lcs->b[0][j] = LCS_LEFT;
  }
  lcs->z = NULL;

  return 1;
}

static size_t
LCSDP()
{
  int i, j, p;

  for(i = 1; i <= lcs->m; ++i) {
    for(j = 1; j <= lcs->n; ++j) {
      if(lcs->x[i-1] == lcs->y[j-1]) {
        lcs->c[i][j] = lcs->c[i - 1][j - 1] + 1;
        lcs->b[i][j] = LCS_UP_LEFT;
      } else if(lcs->c[i - 1][j] >= lcs->c[i][j - 1]) {
        lcs->c[i][j] = lcs->c[i - 1][j];
        lcs->b[i][j] = LCS_UP;
      } else {
        lcs->c[i][j] = lcs->c[i][j - 1];
        lcs->b[i][j] = LCS_LEFT;
      }
    }
  }

  i = lcs->m;
  j = lcs->n;
  p = lcs->c[i][j];

  lcs->z = (char *)malloc(sizeof(char) * p);
  size_t size = p;
  p--; //lcs->z[p--] = '\0';
  while(i > 0 || j > 0) {
    if(lcs->b[i][j] == LCS_UP_LEFT) {
      i--; j--;
      lcs->z[p--] = lcs->x[i];
    } else if(lcs->b[i][j] == LCS_UP) {
      i--;
    } else {
      j--;
    }
  }
  return size;
}

void
destroyLCS()
{
  int i = 0;

  if(lcs != NULL) {
    for(i = 0; i <= lcs->m; ++i) {
      free(lcs->c[i]);
      free(lcs->b[i]);
    }

    //free(lcs->z);
    free(lcs->b);
    free(lcs->c);
    free(lcs);
    lcs = NULL;
  }
}

char*
LCS_SoarPenguin(const char *X, const char *Y, size_t n, size_t m, size_t *size)
{
  initLCS(X, Y, m, n);
  *size = LCSDP();
  char* result = lcs->z;
  destroyLCS();
  return result;
}
