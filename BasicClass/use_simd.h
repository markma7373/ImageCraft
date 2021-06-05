#pragma once

extern bool g_is_support_SSE2;
extern bool g_is_support_SSSE3;
extern bool g_is_support_SSE41;
extern bool g_is_support_NEON;
extern bool g_is_houdini;

#if defined(ANDROID_ARM) || defined(IOS_ARM)

#include <arm_neon.h>

inline void v_horizontal_sum_f32(float *p_value, float32x4_t input)
{
    float32x2_t sum = vadd_f32(vget_high_f32(input), vget_low_f32(input));
    sum = vpadd_f32(sum, sum);
    vst1_lane_f32(p_value, sum, 0);
}

inline void v_horizontal_sum_u32(void *p_value, uint32x4_t input)
{
    uint32x2_t sum = vadd_u32(vget_high_u32(input), vget_low_u32(input));
    sum = vpadd_u32(sum, sum);
    vst1_lane_u32((unsigned int*)p_value, sum, 0);
}

inline void v_horizontal_sum_u8(unsigned int *p_value, uint8x16_t input)
{
    uint16x8_t sum_temp = vaddl_u8(vget_high_u8(input), vget_low_u8(input));
    uint16x4_t sum = vadd_u16(vget_high_u16(sum_temp), vget_low_u16(sum_temp));
    
    sum = vpadd_u16(sum, sum);
    sum = vpadd_u16(sum, sum);
    
    unsigned short short_temp;
    vst1_lane_u16(&short_temp, sum, 0);
    
    *p_value = short_temp;
}

inline float32x4_t vinvq_f32(float32x4_t q0)    // 1/q0
{
    float32x4_t q1, q2;

    // first estimation
    q1 = vrecpeq_f32(q0);

    // iteration 1
    q2 = vrecpsq_f32(q1, q0);
    q1 = vmulq_f32(q1, q2);

    // iteration 2
    q2 = vrecpsq_f32(q1, q0);
    q1 = vmulq_f32(q1, q2);

    return q1;
}

inline float32x4_t vinv_sqrtq_f32(float32x4_t q0)
{
    float32x4_t q1, q2, q3;

    // first estimation
    q1 = vrsqrteq_f32(q0);

    // iteration 1
    q2 = vmulq_f32(q0, q1);
    q3 = vrsqrtsq_f32(q2, q1);
    q1 = vmulq_f32(q1, q3);

    // iteration 2
    q2 = vmulq_f32(q0, q1);
    q3 = vrsqrtsq_f32(q2, q1);
    q1 = vmulq_f32(q1, q3);

    return q1;
}

#if !(defined(IOS_ARM) && defined(__arm64))
inline float32x4_t vdivq_f32(float32x4_t a, float32x4_t b)    // a/b
{
    float32x4_t inverse_b = vinvq_f32(b);
    float32x4_t result = vmulq_f32(a, inverse_b);

    return result;
}

inline float32x4_t vsqrtq_f32(float32x4_t q0)
{
    q0 = vinv_sqrtq_f32(q0);
    q0 = vinvq_f32(q0);
    return q0;
}
#endif

inline int32x4_t vsetq_s32(int s0, int s1, int s2, int s3)
{
    int32x4_t arm_int32x4;
    arm_int32x4 = vsetq_lane_s32(s0, arm_int32x4, 0);
    arm_int32x4 = vsetq_lane_s32(s1, arm_int32x4, 1);
    arm_int32x4 = vsetq_lane_s32(s2, arm_int32x4, 2);
    arm_int32x4 = vsetq_lane_s32(s3, arm_int32x4, 3);

    return arm_int32x4;
}

inline float32x4_t vsetq_f32(float s0, float s1, float s2, float s3)
{
    float32x4_t arm_float32x4;
    arm_float32x4 = vsetq_lane_f32(s0, arm_float32x4, 0);
    arm_float32x4 = vsetq_lane_f32(s1, arm_float32x4, 1);
    arm_float32x4 = vsetq_lane_f32(s2, arm_float32x4, 2);
    arm_float32x4 = vsetq_lane_f32(s3, arm_float32x4, 3);

    return arm_float32x4;
}

inline int32x2_t vround_to_positive_int_f32(float32x2_t d1)
{
    float32x2_t const_05 = vdup_n_f32(0.5f);
    d1 = vadd_f32(d1, const_05);
    return vcvt_s32_f32(d1);
}

inline int32x4_t vround_to_positive_intq_f32(float32x4_t q1)
{
    float32x4_t const_05 = vdupq_n_f32(0.5f);
    q1 = vaddq_f32(q1, const_05);
    return vcvtq_s32_f32(q1);
}

/////////////////////////////////////////////////////////////////////
// C code of vround_to_int_f32 (for reference)
//inline int round_to_int(float flt)
//{
//    const int const_get_signed = 0x80000000;
//    const int const_05 = 0x3f000000;
//
//    int tmp_flt = *reinterpret_cast<int *>(&flt);
//    int tmp_signed_05 = ((tmp_flt & const_get_signed) | const_05);
//    float signed_05 = *reinterpret_cast<float *>&tmp_signed_05;
//    return (int)(flt + signed_05);
//}

inline int32x2_t vround_to_int_f32(float32x2_t d1)
{
    const int32x2_t const_get_signed = vdup_n_s32(0x80000000);
    const int32x2_t const_05 = vdup_n_s32(0x3f000000);

    int32x2_t int_d1 = vreinterpret_s32_f32(d1);
    int_d1 = vorr_s32(vand_s32(int_d1, const_get_signed), const_05);
    float32x2_t signed_05 = vreinterpret_f32_s32(int_d1);
    d1 = vadd_f32(d1, signed_05);
    return vcvt_s32_f32(d1);
}

inline int32x4_t vround_to_intq_f32(float32x4_t q1)
{
    const int32x4_t const_get_signed = vdupq_n_s32(0x80000000);
    const int32x4_t const_05 = vdupq_n_s32(0x3f000000);

    int32x4_t int_q1 = vreinterpretq_s32_f32(q1);
    int_q1 = vorrq_s32(vandq_s32(int_q1, const_get_signed), const_05);
    float32x4_t signed_05 = vreinterpretq_f32_s32(int_q1);
    q1 = vaddq_f32(q1, signed_05);
    return vcvtq_s32_f32(q1);
}

#else

#include <emmintrin.h>

static inline __m128i _mm_sub_abs_epu8(__m128i &m1, __m128i &m2)
{
    __m128i sub1 = _mm_subs_epu8(m1, m2);
    __m128i sub2 = _mm_subs_epu8(m2, m1);
    return _mm_add_epi8(sub1, sub2);
}

static inline __m128i _mm_sub_abs_epu16(__m128i m1, __m128i m2)
{
    __m128i x1 = _mm_subs_epu16(m1, m2);
    __m128i x2 = _mm_subs_epu16(m2, m1);
    return _mm_add_epi16(x1, x2);
}

static inline __m128i _mm_abs_epi32_sse2(__m128i m1)
{
    __m128i sign = _mm_setzero_si128();
    sign = _mm_cmpgt_epi32(sign, m1);
    m1 = _mm_xor_si128(m1, sign);
    return _mm_sub_epi32(m1, sign);
}

static inline __m128i _mm_abs_epi16_sse2(__m128i m1)
{
    __m128i sign = _mm_setzero_si128();
    sign = _mm_cmpgt_epi16(sign, m1);
    m1 = _mm_xor_si128(m1, sign);
    return _mm_sub_epi16(m1, sign);
}

static inline __m128 _mm_abs_ps(__m128 x)
{
    static const __m128 zero = _mm_setzero_ps();
    return _mm_max_ps(_mm_sub_ps(zero, x), x);
}

#endif
