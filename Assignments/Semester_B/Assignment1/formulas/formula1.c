/* Omri Asudon 208853598 */
#include <xmmintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pmmintrin.h>  // SSE3 for _mm_hadd_ps
#include <smmintrin.h>  // SSE4.1

#define MAX_STR 255

float formula1(float *x, unsigned int length) {
    __m128 v_sum = _mm_setzero_ps();
    __m128 v_prod = _mm_set1_ps(1.0f);
    __m128 v_one = _mm_set1_ps(1.0f);

    unsigned int i = 0;
    
    // Process 4 floats at a time using SSE
    for (; i + 3 < length; i += 4) {
        // Load 4 floats
        __m128 x_vec = _mm_loadu_ps(&x[i]);
        
        // sum += sqrt(x)
        __m128 x_sqrt = _mm_sqrt_ps(x_vec);
        v_sum = _mm_add_ps(v_sum, x_sqrt);
        
        // product *= (x * x + 1.0f)
        __m128 x_sq = _mm_mul_ps(x_vec, x_vec);
        __m128 term = _mm_add_ps(x_sq, v_one);
        v_prod = _mm_mul_ps(v_prod, term);
    }
    
    // Horizontal sum for v_sum using SSE3 hadd
    __m128 sum_half = _mm_hadd_ps(v_sum, v_sum);
    __m128 sum_full = _mm_hadd_ps(sum_half, sum_half);
    float final_sum = _mm_cvtss_f32(sum_full);
    
    // Horizontal product for v_prod
    float p[4];
    _mm_storeu_ps(p, v_prod);
    float final_prod = p[0] * p[1] * p[2] * p[3];
    
    // Process remaining elements (scalar tail loop)
    for (; i < length; i++) {
        final_sum += sqrtf(x[i]);
        final_prod *= (x[i] * x[i] + 1.0f);
    }
    
    // Final calculation: sqrtf(1.0f + cbrtf(sum) / product)
    return sqrtf(1.0f + cbrtf(final_sum) / final_prod);
}
