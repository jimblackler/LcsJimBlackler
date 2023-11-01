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
                          int *lengthAtPrimary, int *lengthAtSecondary,
                          size_t primaryLength, size_t secondaryLength) {
  
  int cs = 0;
  const char *a = primary;
  const char *b = secondary;
  const char *a1 = a;
  const char *b1 = b;
  const char *primaryEnd = primary + primaryLength;
  const char *secondaryEnd = secondary + secondaryLength;
  
  while (a1 < primaryEnd && b1 < secondaryEnd) {
    if (*a == *b1) {
      while (b < b1) {
        lengthAtSecondary[b - secondary] = cs;
        b++;
      }
      
      cs++;
      lengthAtPrimary[a - primary] = cs;
      lengthAtSecondary[b - secondary] = cs;
      
      a++;
      b++;
      a1 = a;
      b1 = b;
      
    } else if (*b == *a1) {
      while (a < a1) {
        lengthAtPrimary[a - primary] = cs;
        a++;
      }
      
      cs++;
      lengthAtPrimary[a - primary] = cs;
      lengthAtSecondary[b - secondary] = cs;
      
      a++;
      b++;
      a1 = a;
      b1 = b;
    } else {
      a1++;
      b1++;
    }
    
  }
  
  while (a < primaryEnd) {
    lengthAtPrimary[a - primary] = cs;
    a++;
  }
  while (b < secondaryEnd) {
    lengthAtSecondary[b - secondary] = cs;
    b++;
  }
  
  return cs;
}


static char *LCS(const char *primary, const char *secondary,
    size_t primaryLength, size_t secondaryLength, size_t *lcs) {

  size_t upperLimit = primaryLength < secondaryLength ?
      primaryLength : secondaryLength;

  int *lengthAtPrimary = malloc(primaryLength * sizeof(int));
  int *lengthAtSecondary = malloc(secondaryLength * sizeof(int));
  
  int lowerLimit = cleverGreedyCS(primary, secondary, lengthAtPrimary, lengthAtSecondary,
                                  primaryLength, secondaryLength);


  if (!upperLimit) {
    *lcs = 0;
    return calloc(1, sizeof(char));  // Early out if either string is empty.
  }

  if (lowerLimit == upperLimit) {
    char *result = malloc((upperLimit + 1) * sizeof(char));
    if (primaryLength == upperLimit)
      memcpy(result, primary, primaryLength * sizeof(char));
    else
      memcpy(result, secondary, secondaryLength * sizeof(char));
    result[upperLimit] = 0;
    *lcs = upperLimit;
    return result;
  }

  int *maxSecondaries = calloc(upperLimit + 1, sizeof(int));
  int *maxPrimaries = calloc(upperLimit + 1, sizeof(int));

  int *nextBlocks = calloc(upperLimit + 1, sizeof(int));
  int firstBlockIndex = upperLimit;

  int longestSequence = 0;  // The longest known sequence.
  maxSecondaries[longestSequence] = -1;

  size_t linkedCharSize = sizeof(int) * secondaryLength;
  int *linkedChars = malloc(linkedCharSize);
  memset(linkedChars, -1, linkedCharSize);  // -1 means no entry.

  size_t characterMapSize = sizeof(int) * (1 << sizeof(char) * 8);
  // An array is used to map each character code to the lowest index of
  // appearance of that character in the secondary string.
  int *characterMap = malloc(characterMapSize);
  // Reset to '-1' to mean no entry.
  memset(characterMap, -1, characterMapSize);

  for (int secondaryIndex = 0;
       secondaryIndex < secondaryLength; secondaryIndex++) {
    // Insert this character appearance in the singly linked list where the
    // |characterMap| stores the head of each list.
    unsigned char chr = secondary[secondaryIndex];
    linkedChars[secondaryIndex] = characterMap[chr];
    // Record the new head element.
    characterMap[chr] = secondaryIndex;
  }
  
  for (int primaryIndex = primaryLength - 1;
       primaryIndex >= 0 && longestSequence < upperLimit;
       primaryIndex--) {
    
    if (longestSequence > lowerLimit)
      lowerLimit = longestSequence;

    int remainA = lengthAtPrimary[primaryIndex];
    int sequenceLength;
    int basePosition;

    sequenceLength = nextBlocks[firstBlockIndex];

    // If fewer characters remain than exist in the longest known sequence, the
    // shorter sequences need no longer be considered as they cannot possibly be
    // extended to become the longest sequence.

    int remainingCharacters = primaryIndex + 1; // maxRemainPrimary;

    int endLength2 = lowerLimit - remainingCharacters - 1;
    while (sequenceLength < endLength2)
      sequenceLength = nextBlocks[sequenceLength];

    nextBlocks[firstBlockIndex] = sequenceLength;
    unsigned char chr = primary[primaryIndex];

    int previousBlock = firstBlockIndex;

    if (sequenceLength == 0)
      basePosition = secondaryLength;
    else
      basePosition = maxSecondaries[sequenceLength - 1];

    if (basePosition == 0)
      break;

    int secondaryIndex = characterMap[chr];
    // Pre-scan to the first viable appearance of the character in the
    // secondary string.
    while (secondaryIndex >= basePosition)
      secondaryIndex = linkedChars[secondaryIndex];
    // Update the first apeparance character map.
    characterMap[chr] = secondaryIndex;

    while (basePosition > 0) {

      while (secondaryIndex >= basePosition)
        secondaryIndex = linkedChars[secondaryIndex];

      if (secondaryIndex == -1)
        break;

      if (sequenceLength == longestSequence &&
          secondaryIndex < lowerLimit - longestSequence - 1)
        break;

      if (secondaryIndex <= maxSecondaries[sequenceLength]) {
        do {
          previousBlock = sequenceLength;
          sequenceLength = nextBlocks[previousBlock];
        } while (secondaryIndex <= maxSecondaries[sequenceLength]);
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

        int remainB = lengthAtSecondary[secondaryIndex];
        int minRemain = remainA < remainB ? remainA : remainB;
        int potentialMax = sequenceLength + minRemain;
        if (lowerLimit < potentialMax) {
          lowerLimit = potentialMax;
        }
        
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
    
    // Slight optimization.
    if (sequenceLength == longestSequence) {
      maxPrimaries[sequenceLength] = primaryIndex;
    }
  }

  free(lengthAtPrimary);
  free(lengthAtSecondary);

  *lcs = longestSequence;

  if (!longestSequence) {
    free(nextBlocks);
    free(maxSecondaries);
    free(maxPrimaries);
    return calloc(sizeof(char), 1);
  }

  memset(linkedChars, -1, linkedCharSize);  // -1 means no entry.
  // Reset to '-1' to mean no entry.
  memset(characterMap, -1, characterMapSize);

  for (int secondaryIndex = secondaryLength - 1;
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

  int longestHopelessSequence = -1;

  firstBlockIndex = upperLimit;
  nextBlocks[firstBlockIndex] = 0;

  int longestSequence2 = 0;  // The longest known sequence.

  // The outer loop considers characters from the primary string in order. If
  // pairs are only *appended* to subsequences, it is only necessary to ensure
  // that the appearance in the secondary string is in order.
  for (int primaryIndex = 0; longestSequence2 < longestSequence;
       primaryIndex++) {

    int sequenceLength = nextBlocks[firstBlockIndex];

    // blah2
    while (maxPrimaries[longestSequence - sequenceLength - 1] < primaryIndex)
      sequenceLength = nextBlocks[sequenceLength];

    nextBlocks[firstBlockIndex] = sequenceLength;
    unsigned char chr = primary[primaryIndex];

    int previousBlock = firstBlockIndex;


    int secondaryIndex = characterMap[chr];
    Node *baseSequence;
    if (sequenceLength == 0) {
      baseSequence = NULL;
    } else {
      baseSequence = sequences[sequenceLength - 1];
      if (baseSequence->secondaryIndex == secondaryLength - 1)
        break;
      
      // Pre-scan to the first viable appearance of the character in the
      // secondary string.
      while (secondaryIndex != -1 && secondaryIndex <= baseSequence->secondaryIndex)
        secondaryIndex = linkedChars[secondaryIndex];
      // Update the first apeparance character map.
      characterMap[chr] = secondaryIndex;
    }
    // Keep track of any sequences replaced in the table by extension. These are
    // marked for recycling as soon as it is known they will not themselves be
    // extended on the next iteration.
    Node *toRecycle = NULL;


    while (!baseSequence || baseSequence->secondaryIndex < secondaryLength - 1) {
      
      while (baseSequence && secondaryIndex != -1 &&
          secondaryIndex <= baseSequence->secondaryIndex)
        secondaryIndex = linkedChars[secondaryIndex];

      if (secondaryIndex == -1)
        break;

      if (sequenceLength == longestSequence2 &&
          secondaryIndex > maxSecondaries[longestSequence - longestSequence2 - 1])
        break;

      if (sequenceLength != longestSequence2 &&
          secondaryIndex >= sequences[sequenceLength]->secondaryIndex) {
        do {
          previousBlock = sequenceLength;
          sequenceLength = nextBlocks[previousBlock];
        } while (sequenceLength != longestSequence2 &&
            secondaryIndex >= sequences[sequenceLength]->secondaryIndex);
        baseSequence = sequences[sequenceLength - 1];

      } else {
      
        // Obtain a slot for the node that will form the leaf node for the
        // newly-extended sequence.
        Node *newNode;

        // If we need a new slot..
        // Mark the nodes from the 'hopeless' sequences for recycling. We start
        // one sequence before the baseSequence because this is still required.
        if (activePool->nextSlot - activePool->firstSlot == activePool->size) {
          while (!lastRecycledNode && longestHopelessSequence < nextBlocks[firstBlockIndex] - 1 - 1) {
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

        // Any subsequence awaiting recycling can now safely be recycled.
        recycleSequence(toRecycle, &lastRecycledNode);

        // The just-replaced sequence can be recycled; just not yet because it
        // might be about to be extended on the next loop.
        toRecycle = baseSequence;

        int previousIndex;
        if (sequenceLength == 0)
          previousIndex = secondaryLength;
        else
          previousIndex = sequences[sequenceLength - 1]->secondaryIndex;

        if (nextBlocks[sequenceLength] > sequenceLength + 1) {
          // Current sequence is start of a multiblock.

          // Set new (one shorter) start.
          nextBlocks[sequenceLength + 1] = nextBlocks[sequenceLength];
          if (secondaryIndex == previousIndex + 1) {  //ch
            // Piece ‘extends previous’
            nextBlocks[previousBlock] = sequenceLength + 1;
          } else {
            // Piece 'breaks off'.
            nextBlocks[sequenceLength] = sequenceLength + 1;
          }
          previousBlock = sequenceLength + 1;
          baseSequence = sequences[nextBlocks[previousBlock] - 1];
        } else {
          // Current sequence is a solo block (or end of the sequences).
          if (secondaryIndex == previousIndex + 1) { //ch
            // Piece ‘extends previous’
            nextBlocks[previousBlock] = sequenceLength + 1;
          } else {
            previousBlock = sequenceLength;
          }
          baseSequence = sequences[sequenceLength];
        }
        // The table is finally updated with a pointer to the leaf of the new
        // subsequence.
        sequences[sequenceLength] = newNode;

        if (sequenceLength == longestSequence2) {
          longestSequence2++;
          nextBlocks[previousBlock] = longestSequence2;
          break;
        }
        sequenceLength = nextBlocks[previousBlock];
      }
    }
  
  
  
    // Any subsequence awaiting recycling can now safely be recycled.
    recycleSequence(toRecycle, &lastRecycledNode);
  }

//printf("%d %d  %f\n", totalNodes, longestSequence, (float) totalNodes / longestSequence);
  // Free memory no longer needed.
  
  free(linkedChars);
  free(characterMap);
  free(nextBlocks);
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
//this = getMicroseconds();
//printf("3) %lld \n", this -prev);
//prev = this;


//printf("%d %d\n", extends0, totalNodes);
  return result;
}


// Return the Longest Common Subsequence of the two supplied strings.
char *LCS_Blackler34(const char *primary, const char *secondary, size_t primaryLength, size_t secondaryLength, size_t *length) {

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

