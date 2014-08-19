#ifdef WIN32

#include <windows.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <malloc.h>

#endif  // WIN32

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "algorithm/lcs_blackler.h"
#include "thirdparty/lcs_neiljones.h"
#include "thirdparty/lcs_soarpenguin.h"
#include "util/loadfile.h"
#include "util/timing.h"
#import "benchmark.h"

static size_t process(char *name, char *(*function)(const char *, const char *),
    const char *a, const char *b) {
  long long before = getMicroseconds();
  char *lcs = function(a, b);
  long long after = getMicroseconds();
  if (strlen(lcs) < 200)
    printf("%s\n", lcs);
  printf("%s: %lld, %lu\n", name, after - before, strlen(lcs));
  size_t result = strlen(lcs);
  free(lcs);
  return result;
}

static char *randomString(int range, size_t size) {
  char *text = malloc(size + 1);
  char *ptr = text;
  while (size) {
    *ptr++ = (char) (' ' + rand() % range);
    size--;
  }
  *ptr = 0;
  return text;
}

static void testAll(const char *a, const char *b) {
  size_t result2 = process("Blackler", LCS_Blackler, a, b);
  size_t result = process("NeilJones", LCS_NeilJones, a, b);
  assert(result == result2);
  size_t result3 = process("SoarPenguin", LCS_SoarPenguin, a, b);
  assert(result == result3);
  printf("\n");
}

static void fileTest() {
  char *a = loadFile("data/bible1.txt");
  char *b = loadFile("data/bible2.txt");
  testAll(a, b);
  free(a);
  free(b);
}

static void phraseTest() {
  char *a = "The earth was without form, and void; and darkness was on the face of the deep. And the Spirit of God was hovering over the face of the waters.";
  char *b = "Now the earth was formless and empty, darkness was over the surface of the deep, and the Spirit of God was hovering over the waters.";
  testAll(a, b);
}

static void randomTest() {
  srand(1);
  int count;
  for (count = 0; count != 5; count++) {
    char *a = randomString(rand() % 96 + 1, (size_t) (rand() % 35000 + 1));
    char *b = randomString(rand() % 96 + 1, (size_t) (rand() % 35000 + 1));
    testAll(a, b);
    free(a);
    free(b);
  }
}

int main(int argc, const char *argv[]) {
  if (1) {
    benchmark();
  } else {
    phraseTest();
    fileTest();
    randomTest();
  }
#ifdef WIN32
  _CrtDumpMemoryLeaks();
#endif  // WIN32
  return 0;
}
