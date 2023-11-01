#include <stdio.h>
#include <stdlib.h>

char *loadFile(const char *filename, size_t *size) {
  char *buffer = 0;
  FILE *f = fopen(filename, "rb");
  if (f) {
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = malloc((size_t) length);
    if (buffer)
      fread(buffer, 1, (size_t) length, f);

    fclose(f);
    *size = (size_t) length;
  }
  return buffer;
}
