#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

/**
* Longest Common Subsequence algorithm designed and written by Jim Blackler
* August 2014.  (c) jimblackler@gmail.com
*/

// A node representing a matching pair of characters across strings in the tree
// of subsequence characters (similar to a prefix tree or trie). Note that the
// primary index (index of the character pair in the primary string) is not
// stored, simply because it is never required.
typedef struct Node {
  int secondaryIndex;  // Index of the character in the secondary string.
  struct Node *parent;  // Previous pair in the subsequence.
  int outgoing;  // Number of outgoing nodes.
} Node;

// A list of pools of slots for nodes is used to allow memory allocation in
// batches.
typedef struct NodeSlotPool {
  struct NodeSlotPool *previousPool;
  int size;
  Node *firstSlot;
  Node *nextSlot;
} NodeSlotPool;

// Creates a new pool of slots for nodes, and sets it as the current
// |activePool|.
static void newNodePool(NodeSlotPool **activePool, int size) {
  NodeSlotPool *newPool = malloc(sizeof(NodeSlotPool));
  // Pool size increases exponentially.
  newPool->size = size;
  newPool->firstSlot = malloc(sizeof(Node) * newPool->size);
  newPool->nextSlot = newPool->firstSlot;
  newPool->previousPool = *activePool;  // Link the previous pool.
  *activePool = newPool;
}

// Marks the node slots used for the subsequence represented by the leaf |node|
// available for reuse (provided there are no other sequences using these
// nodes). Updates the |lastRecycledNode| pointer (the head of the list of
// recycled slots).
static void recycleSequence(Node *node, Node **lastRecycledNode) {
  // The nodes in the subsequence are walked and the outgoing counters
  // decremented.
  while (node) {
    // The walk is aborted when a node is encountered that is still in use by
    // another subsequence.
    if (node->outgoing)
      break;

    Node *parent = node->parent;
    // The outgoing counter for the parent node is decremented as this
    // subsequence no longer references this node.
    if (parent)
      parent->outgoing--;
    // The linked list is appropriated to connect recycled nodes in the pool.
    node->parent = *lastRecycledNode;
    *lastRecycledNode = node;

    node = parent;
  }
}

static int cleverGreedyCS(const char *primary, const char *secondary,
    size_t primaryLength, size_t secondaryLength) {

  int cs = 0;
  int _a = 0;
  int _b = 0;
  int _a1 = 0;
  int _b1 = 0;

  while (_a1 < primaryLength && _b1 < secondaryLength) {
    if (primary[_a] == secondary[_b1]) {
      _a++;
      _b = _b1 + 1;
      _a1 = _a;
      _b1 = _b;
      cs++;
    } else if (secondary[_b] == primary[_a1]) {
      _b++;
      _a = _a1 + 1;
      _a1 = _a;
      _b1 = _b;
      cs++;
    } else {
     _a1++;
     _b1++;
    }
    
  }
  return cs;
}

static char *LCS(const char *primary, const char *secondary,
    size_t primaryLength, size_t secondaryLength, size_t *lcs) {

  int lowerLimit = cleverGreedyCS(primary, secondary, primaryLength, secondaryLength);

  // A maximum possible length of a common subsequence is identified.
  size_t upperLimit = primaryLength < secondaryLength ?
      primaryLength : secondaryLength;

  if (!upperLimit) {
  *lcs = 0;
    return calloc(1, sizeof(char));  // Early out if either string is empty.
  }

  int *maxSecondaries = calloc(upperLimit + 1, sizeof(int));
  int *maxPrimaries = calloc(upperLimit + 1, sizeof(int));

  int *nextBlocks = calloc(upperLimit + 1, sizeof(int));
  int firstBlockIndex = upperLimit;

  int numberCharacters = 1 << sizeof(char) * 8;
  size_t characterMapSize = sizeof(int) * numberCharacters;

  // Build index list.
  int *firstIndices = calloc(characterMapSize, sizeof(int));
  int *lastIndices = calloc(characterMapSize, sizeof(int));

  // Count appeaerances.
  for (int idx = 0; idx != secondaryLength; idx++) {
    unsigned char chr = secondary[idx];
    firstIndices[chr]++;
  }

  // Calculate start positions.
  int culm = 0;
  for (int idx = 0; idx != characterMapSize; idx++) {
    int value = firstIndices[idx];
    firstIndices[idx] = culm;
    culm += value;
    lastIndices[idx] = culm;
  }

  int *temp = malloc(characterMapSize * sizeof(int));
  memcpy(temp, firstIndices, characterMapSize);

  int *appearances = malloc(secondaryLength * sizeof(int));
  for (int idx = secondaryLength - 1; idx >= 0; idx--) {
    unsigned char chr = secondary[idx];
    appearances[temp[chr]++] = idx;
  }

  free(temp);

  int longestSequence = 0;  // The longest known sequence.
  maxSecondaries[longestSequence] = -1;

  for (int primaryIndex = primaryLength - 1;
       primaryIndex >= 0 && longestSequence < upperLimit;
       primaryIndex--) {

    int sequenceLength;
    int basePosition;

    sequenceLength = nextBlocks[firstBlockIndex];

    // If fewer characters remain than exist in the longest known sequence, the
    // shorter sequences need no longer be considered as they cannot possibly be
    // extended to become the longest sequence.
//if (maxPairsRemain != primaryIndex + 1)
//  printf("%d %d\n", maxPairsRemain, primaryIndex + 1);

    int remainingCharacters = primaryIndex + 1; // maxRemainPrimary;
    int endLength = longestSequence - remainingCharacters - 1;
    while (sequenceLength < endLength)
      sequenceLength = nextBlocks[sequenceLength];

    int endLength2 = lowerLimit - remainingCharacters - 1;
    while (sequenceLength < endLength2)
      sequenceLength = nextBlocks[sequenceLength];

    nextBlocks[firstBlockIndex] = sequenceLength;
    unsigned char chr = primary[primaryIndex];


    int appIdx = firstIndices[chr];
    int endIdx = lastIndices[chr];

    int previousBlock = firstBlockIndex;
    sequenceLength = nextBlocks[firstBlockIndex];
    if (sequenceLength == 0)
      basePosition = secondaryLength;
    else {
      basePosition = maxSecondaries[sequenceLength - 1];
    }


    while (appIdx < endIdx && appearances[appIdx] >= basePosition)
      appIdx++;

    firstIndices[chr] = appIdx;


    while (basePosition > 0 && appIdx < endIdx) {

      int secondaryIndex = -1;
      int end = maxSecondaries[sequenceLength] + 1;
      int prospect = lowerLimit - sequenceLength - 1;
      if (end < prospect) {
        end = prospect;
      }

//      for (int idx = basePosition - 1; idx >= end; idx--) {
//        unsigned char chr2 = secondary[idx];
//        if (chr2 == chr) {
//          secondaryIndex = idx;
//          break;
//        }
//      }

//      while (appIdx < endIdx && appearances[appIdx] >= end) {
//        if (appearances[appIdx] <= basePosition - 1) {
//          secondaryIndex = appearances[appIdx];
//          break;
//        }
//        appIdx++;
//      }

      int pos = appearances[appIdx];
      if (pos >= end) {

        if (pos < basePosition) {
          secondaryIndex = appearances[appIdx];
          appIdx++;
        } else {
          int endPoint = endIdx - 1;
          int s2 = appearances[endPoint];
          if (s2 <= basePosition - 1) {
            while (appIdx != endPoint) {
              float ratio = (float) (pos - (basePosition - 1)) / (pos - s2);
              int midPoint = appIdx + (endPoint - 1 - appIdx) * ratio;
              if (appearances[midPoint] < basePosition) {
                endPoint = midPoint;
              } else {
                appIdx = midPoint + 1;
                int pos = appearances[appIdx];
                if (pos < end) {
                  break;
                } else if (pos < basePosition) {
                  secondaryIndex = pos;
                  appIdx++;
                  break;
                }
              }
            }
          }
        }
      }
      if (secondaryIndex == -1) {
        if (sequenceLength == longestSequence)
          break;

        previousBlock = sequenceLength;
        sequenceLength = nextBlocks[previousBlock];
        basePosition = maxSecondaries[sequenceLength - 1];

      } else {
        int previousIndex;
        if (sequenceLength == 0)
          previousIndex = secondaryLength;
        else
          previousIndex = maxSecondaries[sequenceLength - 1];

        if (nextBlocks[sequenceLength] > sequenceLength + 1) {
          // Current sequence is start of a multiblock.

          // Set new (one shorter) start.
          nextBlocks[sequenceLength + 1] = nextBlocks[sequenceLength];
          if (secondaryIndex == previousIndex - 1) {
            // Piece ‘extends previous’
            nextBlocks[previousBlock] = sequenceLength + 1;
          } else {
            // Piece 'breaks off'.
            nextBlocks[sequenceLength] = sequenceLength + 1;
          }
          previousBlock = sequenceLength + 1;
          basePosition = maxSecondaries[nextBlocks[previousBlock] - 1];
        } else {
          // Current sequence is a solo block (or end of the sequences).
          if (secondaryIndex == previousIndex - 1) {
            // Piece ‘extends previous’
            nextBlocks[previousBlock] = sequenceLength + 1;
          } else {
            previousBlock = sequenceLength;
          }
          basePosition = maxSecondaries[sequenceLength];
        }

        maxSecondaries[sequenceLength] = secondaryIndex;

        if (sequenceLength == longestSequence) {
          maxPrimaries[sequenceLength] = primaryIndex;
          longestSequence++;
          nextBlocks[previousBlock] = longestSequence;
          maxSecondaries[longestSequence] = -1;
          break;
        }
        sequenceLength = nextBlocks[previousBlock];
      }
    }
  }

//for (int idx = 0; idx != longestSequence; idx++) {
//  printf("%d :\t%d\t%d\n", idx, maxSecondaries[longestSequence - 1 - idx], maxPrimaries[longestSequence - 1 - idx]);
//}

  free(appearances);
  free(firstIndices);
  free(lastIndices);
  free(nextBlocks);

  *lcs = longestSequence;
  if (!longestSequence)
    return calloc(sizeof(char), 1);

  // An array is used to form a series of singly-linked lists which connect
  // appearances of characters in the secondary string. This effectively forms a
  // map between characters and an ordered list of character appearance indices,
  // accelerating the process of identifying character pairs from a character in
  // the primary string.
  size_t linkedCharSize = sizeof(int) * secondaryLength;
  int *linkedChars = malloc(linkedCharSize);
  memset(linkedChars, -1, linkedCharSize);  // -1 means no entry.

  // An array is used to map each character code to the lowest index of
  // appearance of that character in the secondary string.
  int *characterMap = malloc(characterMapSize);
  // Reset to '-1' to mean no entry.
  memset(characterMap, -1, characterMapSize);

  // Build the singly-linked list of character appearances in the secondary
  // string. The string is traversed in reverse order so that at the end the
  // |characterMap| maps the first appearance of each character.
  for (int secondaryIndex = (int) secondaryLength - 1;
       secondaryIndex >= 0; secondaryIndex--) {
    // Insert this character appearance in the singly linked list where the
    // |characterMap| stores the head of each list.
    unsigned char chr = secondary[secondaryIndex];
    linkedChars[secondaryIndex] = characterMap[chr];
    // Record the new head element.
    characterMap[chr] = secondaryIndex;
  }

  // A subsequence is stored of every length 1.. n where s[n] is the sequence of
  // length n, from the set of all sequences of length n where final element
  // pair has the minimal index in the secondary string.
  // If this constraint can be maintained throughout a traversal of pairs in
  // order of appearance in the primary string (the outer loop) then at the end
  // of the loop then one of the set of Longest Common Subsequences will be the
  // longest sequence in the table.
  // The sequences themselves are stored as leaf elements in a tree where each
  // parent node represents the previous character appearance pair in the
  // subsequence. This allows sequences to be duplicated and extended at the
  // cost of a single extra node, not the entire length of the sequence.
  Node **sequences = calloc(longestSequence, sizeof(Node *));

  // A list of pools of slots is used for the nodes on the dynamically-generated
  // tree.
  NodeSlotPool *activePool = NULL;
  newNodePool(&activePool, longestSequence);

  // Slots recycled (available for reuse) are stored in a singly-linked list.
  Node *lastRecycledNode = NULL;

  // Where sequences could not possibly be extended to beat the longest known
  // sequence (because there are not enough characters left to scan) they are
  // deemed 'hopeless'; returned to the pool and no longer considered for
  // extension.
  int longestHopelessSequence = -1;

  // The first sequence that could possibly be extended.
  int minimumSequence = 0;
  int longestSequence2 = 0;  // The longest known sequence.

  // The outer loop considers characters from the primary string in order. If
  // pairs are only *appended* to subsequences, it is only necessary to ensure
  // that the appearance in the secondary string is in order.
  for (int primaryIndex = 0; longestSequence2 < longestSequence;
       primaryIndex++) {
    // Is possible to simultaneously iterate through the sequences and the
    // character appearances in the secondary string because the indices (in the
    // secondary string) of the final character pairs of the sequences are
    // always in ascending order.
    // Proof of this: if final_element(s[n+1]) < final_element(s[n]) then s[n]
    // could not be the sequence with the lowest final index, as any 'n'
    // characters from s[n+1] would have a lower final index.

    // The length of the sequence currently under consideration for replacement.
    // This is actually in index to the sequence table so 0 == a sequence of
    // length 1.
    int sequenceLength;

    // The base sequence is the sequence one character shorter than the sequence
    // length being considered (if one exists). If it is valid to do so and
    // results in a sequence with a final character of smaller index (in the
    // secondary string) this sequence will be extended by one character and any
    // current sequence of this length replaced.
    Node *baseSequence;

    // blah2
    while (maxPrimaries[longestSequence - minimumSequence - 1] < primaryIndex)
      minimumSequence++;

    // Find the first appearance of the character in the secondary string.
    unsigned char chr = primary[primaryIndex];
    int secondaryIndex = characterMap[chr];

    // Begin this scan at the shortest sequence that could potentially be
    // extended to exceed the longest sequence.
    sequenceLength = minimumSequence;
    if (sequenceLength == 0) {
      baseSequence = NULL;
    } else {
      baseSequence = sequences[sequenceLength - 1];

      // Pre-scan to the first viable appearance of the character in the
      // secondary string.
      while (secondaryIndex != -1 &&
          (secondaryIndex <= baseSequence->secondaryIndex)) {
        secondaryIndex = linkedChars[secondaryIndex];
      }
      // Update the first apeparance character map.
      characterMap[chr] = secondaryIndex;
    }
    // Keep track of any sequences replaced in the table by extension. These are
    // marked for recycling as soon as it is known they will not themselves be
    // extended on the next iteration.
    Node *toRecycle = NULL;

    // Pass over all characters in the secondary string that match the character
    // under consideration in the primary string.
    // -1 is used to indicate the end of the list.
    int c = maxSecondaries[longestSequence - longestSequence2 - 1];
    while (secondaryIndex != -1 && secondaryIndex <= c) {

      // Step up through the sequence lengths until a record length is reached,
      // or a sequence is encountered that can be replaced by one with a smaller
      // index (in the secondary string) by extending the base sequence.
      while (sequenceLength < longestSequence2 &&
          secondaryIndex >= sequences[sequenceLength]->secondaryIndex) {
        baseSequence = sequences[sequenceLength];
        sequenceLength++;
      }
      // Ensure that any base sequence can legitimately be extended
      // (|secondaryIndex| is greater than the existing final index).
      if (baseSequence == NULL ||
          secondaryIndex > baseSequence->secondaryIndex) {

        // Obtain a slot for the node that will form the leaf node for the
        // newly-extended sequence.
        Node *newNode;

        // If we need a new slot..
        // Mark the nodes from the 'hopeless' sequences for recycling. We start
        // one sequence before the baseSequence because this is still required.
        if (activePool->nextSlot - activePool->firstSlot == activePool->size) {
          while (!lastRecycledNode && longestHopelessSequence < minimumSequence - 1 - 1) {
            longestHopelessSequence++;
            recycleSequence(sequences[longestHopelessSequence], &lastRecycledNode);
          }
        }

        if (lastRecycledNode) {
          // A recycled node can be used.
          newNode = lastRecycledNode;
          // Replace the head of the recycled node list to 'consume' the node.
          lastRecycledNode = lastRecycledNode->parent;
        } else {
          // Take a slot from the pool of slots never previously used.
          if (activePool->nextSlot - activePool->firstSlot == activePool->size)
            newNodePool(&activePool, longestSequence / 4 + 1);
          newNode = activePool->nextSlot++;
        }

        // Set up the leaf node of the newly created or extended sequence.
        newNode->secondaryIndex = secondaryIndex;
        newNode->parent = baseSequence;  // Appended subsequence.
        newNode->outgoing = 0;

        // Update the number of outgoing links from any base sequence.
        if (baseSequence)
          baseSequence->outgoing++;

        // The replaced sequence becomes the base sequence for the next length
        // of sequence under consideration. It would not be valid to extend the
        // new sequence a second time in this |primaryIndex| loop because that
        // would result in multiple uses of the same primary character in the
        // subsequence.
        baseSequence = sequences[sequenceLength];

        // Any subsequence awaiting recycling can now safely be recycled.
        recycleSequence(toRecycle, &lastRecycledNode);

        // The just-replaced sequence can be recycled; just not yet because it
        // might be about to be extended on the next loop.
        toRecycle = baseSequence;

        // The table is finally updated with a pointer to the leaf of the new
        // subsequence.
        sequences[sequenceLength] = newNode;

        if (sequenceLength == longestSequence2) {
          // We have created a record-length sequence. It won't be possible to
          // extend any more sequences in this |primaryIndex| loop.
          longestSequence2++;
          break;
        }

        // The next sequence under consideration will be one longer.
        sequenceLength++;
      }
      // Move the secondary index to the next instance of a matching character.
      secondaryIndex = linkedChars[secondaryIndex];
    }
    // Any subsequence awaiting recycling can now safely be recycled.
    recycleSequence(toRecycle, &lastRecycledNode);
  }


  // Free memory no longer needed.
  free(characterMap);
  free(linkedChars);
  free(maxSecondaries);
  free(maxPrimaries);


  // Build the actual string from the longest sequence.
  Node *node = sequences[longestSequence - 1];

  // Make space for the string and the null terminator.
  char *result = malloc((longestSequence + 1) * sizeof(char));

  // As the sequence is represented by a leaf node in a tree representing the
  // final character, the string is built back-to-front.
  char *writePointer = result + longestSequence;
  *writePointer-- = 0;  // Write the null terminator.
  while (writePointer >= result) {
    // Copy the character from the secondary string.
    *writePointer-- = secondary[node->secondaryIndex];
    // Walk the tree to move to the previous character.
    node = node->parent;
  }

  // Free the subsequence table.
  free(sequences);

  // Free all the pools.
  while (activePool) {
    NodeSlotPool *previousPool = activePool->previousPool;
    free(activePool->firstSlot);
    free(activePool);
    activePool = previousPool;
  }


  return result;
}

// Return the Longest Common Subsequence of the two supplied strings.
char *LCS_Blackler10(const char *primary, const char *secondary, size_t primaryLength, size_t secondaryLength, size_t *length) {

  const char *primary1 = primary;
  const char *secondary1 = secondary;

  while (secondary1 < secondary + secondaryLength && *secondary1 == *primary1) {
    secondary1++;
    primary1++;
  }

  const char *primary2 = primary1;
  while (primary2 < primary + primaryLength) {
    primary2++;
  }

  const char *secondary2 = secondary1;
  while (secondary2 < secondary + secondaryLength) {
    secondary2++;
  }

  int tailTrimmed = 0;
  while (secondary2 > secondary1 && primary2 > primary1 &&
      *(secondary2 - 1) == *(primary2 - 1)) {
    primary2--;
    secondary2--;
    tailTrimmed++;
  }

  size_t primaryLength0 = primary2 - primary1;
  size_t secondaryLength0 = secondary2 - secondary1;

  size_t lcs;
  char *originalResult =
      LCS(primary1, secondary1, primaryLength0, secondaryLength0, &lcs);

  int headTrimmed = primary1 - primary;
  char *newResult =
      malloc((lcs + headTrimmed + tailTrimmed + 1) * sizeof(char));

  memcpy(newResult, primary, headTrimmed * sizeof(char));
  memcpy(newResult + headTrimmed * sizeof(char),
      originalResult, lcs * sizeof(char));
  memcpy(newResult + (headTrimmed + lcs) * sizeof(char),
      primary2, (tailTrimmed + 1) * sizeof(char));
  *length = headTrimmed + lcs + tailTrimmed;

  free(originalResult);
  return newResult;
}
