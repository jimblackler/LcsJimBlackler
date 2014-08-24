#include "benchmark.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "algorithm/lcs_blackler.h"
#include "thirdparty/lcs_hirschberg.h"
#include "thirdparty/lcs_neiljones.h"
#include "thirdparty/lcs_rogerzhang.h"
#include "thirdparty/lcs_soarpenguin.h"
#include "util/issubstring.h"
#include "util/timing.h"

// Information about each sample (individual benchmark run).
typedef struct Sample {
  int methodNumber;  // Index of the method.
  size_t size;  // Input data size in bytes.

  long long measure;  // Measure, either time duration or memory used.
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

#ifdef MEMORY_PROFILE
  long long before = memoryCount;  // Get memory before.
#else
  long long before = getMicroseconds();  // Get time before.
#endif  // MEMORY_PROFILE

  char *result = method(a, b);  // Perform the method.

#ifdef MEMORY_PROFILE
  long long after = memoryCount;  // Get memory afterwards.
#else
  long long after = getMicroseconds();  // Get time afterwards.
#endif  // MEMORY_PROFILE

  // Record the timing/memory data.
  // Check the result was valid.
  if (result) {
    sample->measure = after - before;
    sample->valid = isSubstring(result, a) && isSubstring(result, b);
    sample->strlen = strlen(result);  // Record the length of the result.
    free(result);  // Free the memory.
  } else {
    sample->measure = 0;
    sample->valid = 0;
    sample->strlen = 0;
  }

  free(a);
  free(b);

  // Log to the console.
  if (!sample->valid)
    printf("*INVALID* ");
  printf("%lld\n", sample->measure);

  return sample;
}

// Perform a benchmark test of increasing data size on all LCS methods
// and output the results to a CSV file.
void benchmark() {

#ifdef MEMORY_PROFILE
  int units = 1;
  int maxMeasure = 1024 * 1024 * 5;
#else
  int units = 1000000;
  int maxMeasure = units * 5;
#endif  // MEMORY_PROFILE

  // Data about the methods and names.
  size_t numberMethods = 5;
  const char *methodNames[] =
      {"Blackler", "NeilJones", "SoarPenguin", "RogerZhang", "Hirschberg"};
  char *(*methods[])(const char *, const char *) =
      {LCS_Blackler, LCS_NeilJones, LCS_SoarPenguin, LCS_RogerZhang, LCS_Hirschberg};

  // Tables of first samples for each method.
  Sample **firstSample = calloc(sizeof(Sample *), numberMethods);

  // Perform the first sample of every method to bootstrap the system.
  for (int methodNumber = 0; methodNumber != numberMethods;
       methodNumber++) {
    // First sample is one single byte.
    Sample *sample = doSample(0, methodNumber, methods[methodNumber],
        methodNames[methodNumber], units);

    // Record the data.
    firstSample[methodNumber] = sample;
  }

  // Progressively sample methods; the method selected is the one that completed
  // in the shortest time on the previous occasion it was run (whatever the
  // sample size), or in the case of memory profile, least memory used. This
  // gives faster feedback during experiments because heavy algorithms do not
  // dominate the testing time.
  while (true) {

    // Find the method with the fastest/smallest most recent sample.
    // O(n^2) time but has negligible impact on program running time.
    Sample *smallestMostRecentSample = NULL;
    for (int methodNumber = 0; methodNumber != numberMethods; methodNumber++) {
      // Find most recent sample.
      Sample *sample = firstSample[methodNumber];
      while (sample->next)
        sample = sample->next;
      if (!smallestMostRecentSample ||
          sample->measure < smallestMostRecentSample->measure) {
        smallestMostRecentSample = sample;
      }
    }

    // If the fastest method was over the cap time, stop.
    if (smallestMostRecentSample->measure >= maxMeasure)
      break;

    // Make a new sample with a size of a percentage increase from the previous.
    int previous = smallestMostRecentSample->size;
    int targetNext = previous * 1.10 + 10;

    // Round down to the largest-rounded milestone figure that won't take the
    // figure below the previous result.
    int roundUnit = 5;  // Lowest milestone rounding is 5.
    int nextMultiplier = 2;
    int next = targetNext;
    while (true) {
      int prospect = (targetNext / roundUnit) * roundUnit;  // Round down.
      if (prospect > previous)  // Not less than previous figure?
        next = prospect;  // Store as potential result.
      else
        break;  // Break out and use the last potential result.
      // Units go 5, 10, 50, 100, 500... etc.
      roundUnit *= nextMultiplier;
      if (nextMultiplier == 2)
        nextMultiplier = 5;
      else
        nextMultiplier = 2;
    }
    
    Sample *sample =
        doSample((size_t) (next),
            smallestMostRecentSample->methodNumber,
            methods[smallestMostRecentSample->methodNumber],
            methodNames[smallestMostRecentSample->methodNumber], units);

    // Link the new sample to the previous.
    smallestMostRecentSample->next = sample;
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
        if (!sample->valid)  // Test failed, not a subsequence.
          fprintf(outFile, "NOT SS");
        else if (sample->strlen < bestLCS)
          fprintf(outFile, "NOT LCS");  // A better valid alternative is known.
        else fprintf(outFile, "%f", (float) sample->measure / units);
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