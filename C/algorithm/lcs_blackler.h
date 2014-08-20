/**
* Longest Common Subsequence algorithm designed and written by Jim Blackler
* August 2014.  (c) jimblackler@gmail.com
*
* The LCS is built by passing over one of the strings while collecting
* subsequences of paired character appearances in both strings.
*
* One subsequence is stored of every length from one to the maximum length
* found. The subsequence selected for any given length is the one which from
* all the possibilities the final paired character appearance has the earliest
* appearance in the second string (thus maximizing the chance of being extended
* again).
*
* At the end of the outer loop the longest subsequence will be the LCS.
*
* Subsequences are created by extending subsequences of one shorter length
* wherever this would create a valid subsequence, and if there is an existing
* subsequence of that same length, this new subsequence's final character has
* the earlier appearance in the second subsequence.
*
* Subsequences are stored in the form of leaf nodes from a set of trees where
* any parent node represents the previous character pair in the subsequence.
* This highly-compact format allows sequences to be duplicated for extension in
* O(1) time and space. In non-garbage collected languages a housekeeping
* algorithm recycles tree nodes that are no longer in use.
*
* The final character appearance pairs of all the subsequences 1 .. n will be
* in ascending order of appearance in the second string. (If this were not the
* case e.g. final_char(s[i - 1]) >= final_char(s[i]) then an s[i] of lower
* final char would exist in in the form of any i-1 pairs from s[i-1] which is a
* contradiction.) This allows the sequences and the second string to be
* iterated in parallel in an inner loop. Furthermore, this loop can be aborted
* in the event of a record-length subsequence being encountered since that
* could not be further extended in that loop as each character can only be used
* once. An additional performance improvement can be obtained by precalculating
* a map of lists of character appearances in the second string rather than
* iterating over every character looking for pairs.
*
* The final time and space improvement is obtained by discarding and no longer
* considering for extension any subsequence that could never be extended to
* form the longest-known subsequence because there are not enough characters
* remaining to be processed in the primary string.
*/

// Return the Longest Common Subsequence of the two supplied strings.
extern char *LCS_Blackler(const char *primary, const char *secondary);