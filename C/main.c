#ifdef WIN32
#include <windows.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
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
#include "benchmark.h"
#include "benchmark_files.h"

static size_t process(char *name, char *(*function)(const char *, const char *, size_t, size_t, size_t *),
    const char *a, const char *b,  size_t sizeA, size_t sizeB) {
  size_t length;
  long long before = getMicroseconds();
  char *lcs = function(a, b, sizeA, sizeB, &length);
  long long after = getMicroseconds();
  printf("%s: %lld, %lu\n", name, after - before, sizeA);
  free(lcs);
  return length;
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

static void testAll(const char *a, const char *b, size_t sizeA, size_t sizeB) {
  size_t result2 = process("Blackler", LCS_Blackler, a, b, sizeA, sizeB);
  size_t result = process("NeilJones", LCS_NeilJones, a, b, sizeA, sizeB);
  assert(result == result2);
  size_t result3 = process("SoarPenguin", LCS_SoarPenguin, a, b, sizeA, sizeB);
  assert(result == result3);
  printf("\n");
}

static void fileTest() {
  size_t sizeA;
  char *a = loadFile("data/bible1.txt", &sizeA);
  size_t sizeB;
  char *b = loadFile("data/bible2.txt", &sizeB);
  testAll(a, b, sizeA, sizeB);
  free(a);
  free(b);
}

static void phraseTest() {
  char *a = "The earth was without form, and void; and darkness was on the face of the deep. And the Spirit of God was hovering over the face of the waters.";
  char *b = "Now the earth was formless and empty, darkness was over the surface of the deep, and the Spirit of God was hovering over the waters.";
  testAll(a, b, strlen(a), strlen(b));
}

static void randomTest() {
  srand(1);
  int count;
  for (count = 0; count != 5; count++) {
    size_t sizeA = (size_t) (rand() % 35000 + 1);
    char *a = randomString(rand() % 96 + 1, sizeA);
    size_t sizeB = (size_t) (rand() % 35000 + 1);
    char *b = randomString(rand() % 96 + 1, sizeB);
    testAll(a, b, sizeA, sizeB);
    free(a);
    free(b);
  }
}

int main(int argc, const char *argv[]) {
  if (0) {
    benchmark();
  } else if (0) {
    phraseTest();
    fileTest();
    randomTest();
  } else {
    benchmarkFiles();
  }
#ifdef WIN32
  _CrtDumpMemoryLeaks();
#endif  // WIN32
  return 0;
}
