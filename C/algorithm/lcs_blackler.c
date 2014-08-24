#include <string.h>
#include <stdlib.h>
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
static void newNodePool(NodeSlotPool **activePool) {
  NodeSlotPool *newPool = malloc(sizeof(NodeSlotPool));
  // Pool size increases exponentially.
  if (*activePool)
    newPool->size = (*activePool)->size * 1.5;
  else
    newPool->size = 100;  // First pool size.
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

// Return the Longest Common Subsequence of the two supplied strings.
char *LCS_Blackler(const char *primary, const char *secondary) {
  size_t primaryLength = strlen(primary);
  size_t secondaryLength = strlen(secondary);

  // A maximum possible length of a common subsequence is identified.
  size_t upperLimit = primaryLength < secondaryLength ?
      primaryLength : secondaryLength;

  if (!upperLimit)
    return calloc(1, sizeof(char));  // Early out if either string is empty.

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
  size_t characterMapSize = sizeof(int) * (1 << sizeof(char) * 8);
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
  Node **sequences = calloc(upperLimit, sizeof(Node *));

  // A list of pools of slots is used for the nodes on the dynamically-generated
  // tree.
  NodeSlotPool *activePool = NULL;
  newNodePool(&activePool);

  // Slots recycled (available for reuse) are stored in a singly-linked list.
  Node *lastRecycledNode = NULL;

  // Where sequences could not possibly be extended to beat the longest known
  // sequence (because there are not enough characters left to scan) they are
  // deemed 'hopeless'; returned to the pool and no longer considered for
  // extension.
  int longestHopelessSequence = -1;

  // The first sequence that could possibly be extended.
  int minimumSequence = 0;
  int longestSequence = 0;  // The longest known sequence.

  // The outer loop considers characters from the primary string in order. If
  // pairs are only *appended* to subsequences, it is only necessary to ensure
  // that the appearance in the secondary string is in order.
  for (int primaryIndex = 0;
       primaryIndex < primaryLength && longestSequence < upperLimit;
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

    // Calculate the remaining characters in the primary string.
    int remainingCharacters = primaryLength - primaryIndex;

    // If fewer characters remain than exist in the longest known sequence, the
    // shorter sequences need no longer be considered as they cannot possibly be
    // extended to become the longest sequence.
    if (remainingCharacters < longestSequence) {
      if (minimumSequence < longestSequence - remainingCharacters)
        minimumSequence = longestSequence - remainingCharacters;
      // Mark the nodes from the 'hopeless' sequences for recycling. We start
      // one sequence before the baseSequence because this is still required.
      while (longestHopelessSequence < minimumSequence - 1 - 1) {
        longestHopelessSequence++;
        recycleSequence(sequences[longestHopelessSequence], &lastRecycledNode);
      }
    }

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
          secondaryIndex <= baseSequence->secondaryIndex) {
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
    while (secondaryIndex != -1) {
      // Step up through the sequence lengths until a record length is reached,
      // or a sequence is encountered that can be replaced by one with a smaller
      // index (in the secondary string) by extending the base sequence.
      while (sequenceLength < longestSequence &&
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
        if (lastRecycledNode) {
          // A recycled node can be used.
          newNode = lastRecycledNode;
          // Replace the head of the recycled node list to 'consume' the node.
          lastRecycledNode = lastRecycledNode->parent;
        } else {
          // Take a slot from the pool of slots never previously used.
          if (activePool->nextSlot - activePool->firstSlot == activePool->size)
            newNodePool(&activePool);
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

        // If this sequence cannot possibly be reduced again, update the length
        // of the minimum sequence under consideration.
        if (secondaryIndex == sequenceLength &&
            minimumSequence < secondaryIndex + 1)
          minimumSequence = secondaryIndex + 1;

        if (sequenceLength == longestSequence) {
          // We have created a record-length sequence. It won't be possible to
          // extend any more sequences in this |primaryIndex| loop.
          longestSequence++;
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

  // Character map no longer needed.
  free(characterMap);
  free(linkedChars);

  // Build the actual string from the longest sequence.
  Node *node = sequences[longestSequence - 1];

  // Make space for the string and the null terminator.
  char *result = malloc(longestSequence + 1);

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
