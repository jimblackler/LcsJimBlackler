#include "benchmark.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#import <math.h>


#include "algorithm/lcs_blackler.h"
#include "algorithm/lcs_blackler2.h"
#include "algorithm/lcs_blackler3.h"
#include "algorithm/lcs_blackler4.h"
#include "algorithm/lcs_blackler5.h"
#include "algorithm/lcs_blackler6.h"
#include "algorithm/lcs_blackler7.h"
#include "algorithm/lcs_blackler8.h"
#include "algorithm/lcs_blackler9.h"
#include "algorithm/lcs_blackler10.h"
#include "algorithm/lcs_blackler11.h"
#include "algorithm/lcs_blackler12.h"
#include "algorithm/lcs_blackler13.h"
#include "algorithm/lcs_blackler14.h"
#include "algorithm/lcs_blackler15.h"
#include "algorithm/lcs_blackler16.h"
#include "algorithm/lcs_blackler17.h"
#include "algorithm/lcs_blackler18.h"
#include "algorithm/lcs_blackler19.h"
#include "algorithm/lcs_blackler20.h"
#include "algorithm/lcs_blackler21.h"
#include "algorithm/lcs_blackler22.h"
#include "algorithm/lcs_blackler23.h"
#include "algorithm/lcs_blackler24.h"
#include "algorithm/lcs_blackler25.h"
#include "algorithm/lcs_blackler26.h"
#include "algorithm/lcs_blackler27.h"
#include "algorithm/lcs_blackler28.h"
#include "algorithm/lcs_blackler29.h"
#include "algorithm/lcs_blackler30.h"
#include "algorithm/lcs_blackler31.h"
#include "algorithm/lcs_blackler32.h"
#include "algorithm/lcs_blackler33.h"
#include "algorithm/lcs_blackler34.h"
#include "algorithm/lcs_blackler35.h"
#include "algorithm/lcs_blackler36.h"
#include "algorithm/lcs_blackler37.h"
#include "algorithm/lcs_blackler38.h"
#include "algorithm/lcs_blackler39.h"
#include "algorithm/lcs_blackler40.h"
#include "thirdparty/lcs_hirschberg.h"
#include "thirdparty/lcs_neiljones.h"
#include "thirdparty/lcs_rogerzhang.h"
#include "thirdparty/lcs_soarpenguin.h"
#include "thirdparty/lcs_zhang2.h"
#include "thirdparty/lcs_zhang3.h"
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
    double r = pow((float)rand() / RAND_MAX, 25);
    int x = r * (256 - ' '); //1.2; // 100
    *ptr++ = (char) (' ' + x);
    size--;
  }
  *ptr = 0;
  return text;
}

// Perform a sample of the specified |size|, and specified method specs.
static Sample *doSample(size_t size,
    int methodNumber, char *(*method)(char const *, char const *, size_t, size_t, size_t *),
    const char *methodName, int units) {
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

  size_t lcsSize;
  char *result = method(a, b, size, size, &lcsSize);  // Perform the method.

#ifdef MEMORY_PROFILE
  long long after = memoryCount;  // Get memory afterwards.
#else
  long long after = getMicroseconds();  // Get time afterwards.
#endif  // MEMORY_PROFILE

  // Record the timing/memory data.
  // Check the result was valid.
  if (result) {
    sample->measure = after - before;
    sample->valid = isSubstring(result, a, lcsSize, size) && isSubstring(result, b, lcsSize, size);
    sample->strlen = lcsSize;
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
  printf("%f  [%lu]\n", (float) sample->measure / units, sample->strlen);

  return sample;
}

// Perform a benchmark test of increasing data size on all LCS methods
// and output the results to a CSV file.
void benchmark() {


#ifdef MEMORY_PROFILE
  int units = 1024 * 1024;
  int maxMeasure = units * 50;
#else
  int units = 1000000;
  int maxMeasure = units * 6;
#endif  // MEMORY_PROFILE
  int startSample = 1000;
  double growthMultiply = 1.12;
  int growthAdd = 1000;

  // Data about the methods and names.
  size_t numberMethods = 11;
  const char *methodNames[] = {  "Blackler40", "Blackler39", "Blackler38", "Blackler37", "Blackler36", "Blackler35", "Blackler34", "Blackler29", "Blackler33", "Blackler32",  "Blackler31", "Blackler30", "Blackler25",  "Blackler28", "Blackler27",
    "Blackler26", "Blackler20", "Blackler18", "Blackler24",
    "Blackler23", "Blackler17", "Blackler22", "Blackler21",  "Blackler5",
    "Blackler2", "Blackler3", "Blackler13", "Blackler19",
    "Blackler4","Blackler6", "Blackler7", "Blackler8", "Blackler9",
    "Blackler10", "Blackler11", "Blackler12", "Blackler14", "Blackler15",
    "Blackler16", "Blackler", "Zhang3",  "NeilJones", "SoarPenguin",
    "RogerZhang", "Hirschberg"};
  char *(*methods[])(const char *, const char *, size_t, size_t, size_t *) = {LCS_Blackler40,
    LCS_Blackler39, LCS_Blackler38, LCS_Blackler37, LCS_Blackler36, LCS_Blackler35, LCS_Blackler34,
    LCS_Blackler29, LCS_Blackler33, LCS_Blackler32, LCS_Blackler31, LCS_Blackler30,
    LCS_Blackler25, LCS_Blackler28, LCS_Blackler27, LCS_Blackler26, LCS_Blackler20,
    LCS_Blackler18,  LCS_Blackler24,
    LCS_Blackler23, LCS_Blackler17, LCS_Blackler22, LCS_Blackler21, LCS_Blackler5,
    LCS_Blackler2, LCS_Blackler3, LCS_Blackler13, LCS_Blackler19,
    LCS_Blackler4, LCS_Blackler6, LCS_Blackler7, LCS_Blackler8, LCS_Blackler9,
    LCS_Blackler10, LCS_Blackler11, LCS_Blackler12, LCS_Blackler14, LCS_Blackler15,
    LCS_Blackler16, LCS_Blackler,  LCS_Zhang3, LCS_NeilJones, LCS_SoarPenguin, LCS_RogerZhang,
    LCS_Hirschberg};

  //  size_t numberMethods = 1;
  //  const char *methodNames[] = {"Blackler9"};
  //  char *(*methods[])(const char *, const char *) = {LCS_Blackler9};


  // Tables of first samples for each method.
  Sample **firstSample = calloc(sizeof(Sample *), numberMethods);

  // Perform the first sample of every method to bootstrap the system.
  for (int methodNumber = 0; methodNumber != numberMethods;
       methodNumber++) {
    // First sample is one single byte.
    Sample *sample = doSample(startSample, methodNumber, methods[methodNumber],
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

    // Find the method with the fastest/smallest most recent valid sample.
    // O(n^2) time but has negligible impact on program running time.
    Sample *smallestMostRecentSample = NULL;
    for (int methodNumber = 0; methodNumber != numberMethods; methodNumber++) {
      // Find most recent sample.
      Sample *sample = firstSample[methodNumber];
      while (sample->next)
        sample = sample->next;
      if (!smallestMostRecentSample ||
          (sample->measure < smallestMostRecentSample->measure &&
           sample->valid)) {
            smallestMostRecentSample = sample;
          }
    }

    // If the fastest method was over the cap time, stop.
    if (smallestMostRecentSample->measure >= maxMeasure)
      break;

    // Make a new sample with a size of a percentage increase from the previous.
    int previous = smallestMostRecentSample->size;
    int targetNext = previous * growthMultiply + growthAdd;

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
