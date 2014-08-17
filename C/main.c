#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "algorithm/lcs_blackler.h"
#include "thirdParty_NeilJones/lcs.h"
#include "thirdParty_SoarPenguin/lcs.h"
#include "util/loadfile.h"

static long long int getMilliseconds() {
  struct timeval time;
  gettimeofday(&time, 0);
  long long before = 1000000 * time.tv_sec + time.tv_usec;
  return before;
}

static size_t process(char *name, char *(*function)(const char *, const char *),
    const char *a, const char *b) {
  long long before = getMilliseconds();
  char *lcs = function(a, b);
  long long after = getMilliseconds();
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
    *ptr++ = (char) (' ' + random() % range);
    size--;
  }
  *ptr = 0;
  return text;
}

static void testAll(const char *a, const char *b) {
  size_t result = process("NeilJones", LCS_NeilJones, a, b);
  size_t result2 = process("Blackler", LCS_Blackler, a, b);
  assert(result == result2);
  size_t result3 = process("SoarPenguin", LCS_SoarPenguin, a, b);
  assert(result == result3);
  printf("\n");
}

static void fileTest() {
  char *a = loadFile("bible1.txt");
  char *b = loadFile("bible2.txt");
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
  for (int count = 0; count != 500; count++) {
    char *a = randomString(random() % 96 + 1, (size_t) (random() % 50000 + 1));
    char *b = randomString(random() % 96 + 1, (size_t) (random() % 50000 + 1));
    testAll(a, b);
    free(a);
    free(b);
  }
}

int main(int argc, const char *argv[]) {
  phraseTest();
  fileTest();
  randomTest();
  return 0;
}
