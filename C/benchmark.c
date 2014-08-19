#include "benchmark.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lcs_rogerzhang.h"
#include "lcs_blackler.h"
#include "lcs_neiljones.h"
#include "lcs_soarpenguin.h"
#include "util/issubstring.h"
#include "util/timing.h"

// Information about each sample (individual benchmark run).
typedef struct Sample {
  int methodNumber;  // Index of the method.
  size_t size;  // Input data size in bytes.

  long long duration;  // Time duration.
  size_t strlen;  // Length of the LCS returned.
  bool valid;  // If the LCS was actually a substring of the inputs.

  struct Sample *next;  // Next sample for this method.
} Sample;

// Generate a random string of the specified size.
static char *randomString(size_t size) {
  char *text = malloc(size + 1);
  char *ptr = text;
  while (size) {
    *ptr++ = (char) (' ' + rand() % (256 - ' '));
    size--;
  }
  *ptr = 0;
  return text;
}

// Perform a sample of the specified |size|, and specified method specs.
static Sample *doSample(size_t size,
    int methodNumber, char *(*method)(char const *, char const *),
    const char *methodName) {
  // The input is seeded to ensure consistent data across the methods.
  srand(size);
  // Generate the two input strings.
  const char *a = randomString(size);
  const char *b = randomString(size);

  // Allocate the sample structure on the heap.
  Sample *sample = malloc(sizeof(Sample));
  sample->methodNumber = methodNumber;
  sample->size = size;
  sample->next = NULL;

  printf("%s (%lu) .. ", methodName, size);  // Log to console.

  long long before = getMicroseconds();  // Get time before.
  char *result = method(a, b);  // Perform the method.
  long long after = getMicroseconds();  // Get time afterwards.

  // Record the timing data.
  sample->duration = after - before;
  // Check the result was valid.
  sample->valid = isSubstring(result, a) && isSubstring(result, b);
  sample->strlen = strlen(result);  // Record the length of the result.
  free(result);  // Free the memory.

  // Log to the console.
  if (!sample->valid)
    printf("*INVALID* ");
  printf("%qi\n", sample->duration);

  return sample;
}

// Perform a benchmark test of increasing data size on all LCS methods
// and output the results to a CSV file.
void benchmark() {

  // Data about the methods and names.
  size_t numberMethods = 4;
  const char *methodNames[] =
      {"Blackler", "NeilJones", "SoarPenguin", "RogerZhang"};
  char *(*methods[])(const char *, const char *) =
      {LCS_Blackler, LCS_NeilJones, LCS_SoarPenguin, LCS_RogerZhang};

  // Tables of first samples for each method.
  Sample **firstSample = calloc(sizeof(Sample *), numberMethods);

  // Perform the first sample of every method to bootstrap the system.
  for (int methodNumber = 0; methodNumber != numberMethods;
       methodNumber++) {
    // First sample is one single byte.
    Sample *sample = doSample(1, methodNumber, methods[methodNumber],
        methodNames[methodNumber]);

    // Record the data.
    firstSample[methodNumber] = sample;
  }

  // Progressively sample methods; the method selected is the one that completed
  // in the shortest time on the previous occasion it was run (whatever the
  // sample size). This gives faster feedback during experiments because
  // slow-running algorithms do not dominate the testing time.
  while (true) {

    // Find the method with the fastest most recent sample.
    // O(n^2) time but has negligible impact on program running time.
    Sample *fastestMostRecentSample = NULL;
    for (int methodNumber = 0; methodNumber != numberMethods; methodNumber++) {
      // Find most recent sample.
      Sample *sample = firstSample[methodNumber];
      while (sample->next)
        sample = sample->next;
      if (!fastestMostRecentSample ||
          sample->duration < fastestMostRecentSample->duration) {
        fastestMostRecentSample = sample;
      }
    }

    // If the fastest method was over the cap time, stop.
    if (fastestMostRecentSample->duration >= 1000000 * 0.5)
      break;

    // Make a new sample with a size of a percentage increase from the previous.
    Sample *sample =
        doSample((size_t) (fastestMostRecentSample->size * 1.04 + 1),
            fastestMostRecentSample->methodNumber,
            methods[fastestMostRecentSample->methodNumber],
            methodNames[fastestMostRecentSample->methodNumber]);

    // Link the new sample to the previous.
    fastestMostRecentSample->next = sample;
  }

  // Write the results to a CSV file for import into a chart package.
  FILE *outFile = fopen("lcs.csv", "w");

  // Write the header (method names).
  for (int methodNumber = 0; methodNumber != numberMethods; methodNumber++) {
    fprintf(outFile, ",%s", methodNames[methodNumber]);
  }
  fprintf(outFile, "\n");

  // Write the data (sorted in size order on the fly).
  size_t minimum = 0;
  while (true) {
    // Find the smallest size not yet output.
    Sample *smallest = NULL;
    for (int methodNumber = 0; methodNumber != numberMethods; methodNumber++) {
      Sample *sample = firstSample[methodNumber];
      while (sample && sample->size <= minimum)
        sample = sample->next;
      if (!sample)
        continue;
      if (!smallest || sample->size < smallest->size)
        smallest = sample;
    }
    if (!smallest)
      break;

    // Find the longest valid LCS for this sample size.
    size_t bestLCS = 0;
    for (int methodNumber = 0; methodNumber != numberMethods; methodNumber++) {
      Sample *sample = firstSample[methodNumber];
      while (sample && sample->size < smallest->size)
        sample = sample->next;
      if (sample && sample->size == smallest->size &&
          sample->valid && sample->strlen > bestLCS)
        bestLCS = sample->strlen;
    }

    // Write the size column.
    fprintf(outFile, "%lu", smallest->size);

    // Write the results for this row.
    for (int methodNumber = 0; methodNumber != numberMethods; methodNumber++) {
      Sample *sample = firstSample[methodNumber];
      while (sample && sample->size < smallest->size)
        sample = sample->next;
      fprintf(outFile, ",");
      if (sample && sample->size == smallest->size) {
        // Write the results cell (including error cases if results were invalid
        // or sub-optimal).
        if (!sample->valid)
          fprintf(outFile, "NOT SS");
        else if (sample->strlen < bestLCS)
          fprintf(outFile, "NOT LCS");
        else fprintf(outFile, "%f", (float) sample->duration / 1000000);
      }
    }

    fprintf(outFile, "\n");  // Complete the row.
    minimum = smallest->size;  // On to the next size.
  }
  fclose(outFile);  // Write the file.

  // Free the data.
  for (int methodNumber = 0; methodNumber != numberMethods; methodNumber++) {
    Sample *sample = firstSample[methodNumber];
    while (sample) {
      Sample *next = sample->next;
      free(sample);
      sample = next;
    }
  }
  free(firstSample);
}