/* Omri Asudon 208853598 */
#include <emmintrin.h>   // SSE2
#include <smmintrin.h>   // SSE4.1
#include <string.h>

#define MAX_STR 256

int hamming_dist(char str1[MAX_STR], char str2[MAX_STR]) {
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    int min_len = len1 < len2 ? len1 : len2;
    int max_len = len1 > len2 ? len1 : len2;

    int diffs = 0;
    int i = 0;

    // Process 16 characters at a time using SSE
    for (; i + 15 < min_len; i += 16) {
        // Load 16 bytes from each string
        __m128i v1 = _mm_loadu_si128((__m128i*)&str1[i]);
        __m128i v2 = _mm_loadu_si128((__m128i*)&str2[i]);

        // Compare vectors byte-by-byte (0xFF if equal, 0x00 if not)
        __m128i eq = _mm_cmpeq_epi8(v1, v2);

        // Extract the most significant bit of each byte into a 16-bit mask
        int mask = _mm_movemask_epi8(eq);

        // mask has a '1' bit for every matching character.
        // We count the number of '1's (matches) using __builtin_popcount.
        // The number of differences is 16 minus the number of matches.
        diffs += 16 - __builtin_popcount(mask);
    }

    // Process any remaining characters (scalar tail loop)
    for (; i < min_len; i++) {
        if (str1[i] != str2[i]) {
            diffs++;
        }
    }

    // If strings are not the same length, add the difference in lengths
    diffs += (max_len - min_len);

    return diffs;
}
