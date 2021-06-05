#pragma once

#if defined(ANDROID_ARM) || defined(IOS_ARM)
struct libdivide_u32_t
{
     uint32_t magic;
     uint8_t more;
};

enum
{
    LIBDIVIDE_32_SHIFT_MASK = 0x1F,
    LIBDIVIDE_64_SHIFT_MASK = 0x3F,
    LIBDIVIDE_ADD_MARKER = 0x40,
    LIBDIVIDE_U32_SHIFT_PATH = 0x80,
    LIBDIVIDE_U64_SHIFT_PATH = 0x80,
    LIBDIVIDE_S32_SHIFT_PATH = 0x20,
    LIBDIVIDE_NEGATIVE_DIVISOR = 0x80
};

static inline int32_t libdivide__count_trailing_zeros32(uint32_t val)
{
    int32_t result = 0;
    val = (val ^ (val - 1)) >> 1;
    while (val) 
    {
        val >>= 1;
        result++;
    }
    return result;
}

static inline int32_t libdivide__count_leading_zeros32(uint32_t val)
{
    int32_t result = 0;
    while (! (val & (1U << 31)))
    {
        val <<= 1;
        result++;
    }
    return result;
}

static uint32_t libdivide_64_div_32_to_32(uint32_t u1, uint32_t u0, uint32_t v, uint32_t *r)
{
    uint64_t n = (((uint64_t)u1) << 32) | u0;
    uint32_t result = (uint32_t)(n / v);
    *r = (uint32_t)(n - result * (uint64_t)v);
    return result;
}

struct libdivide_u32_t libdivide_u32_gen(uint32_t d)
{
    struct libdivide_u32_t result;
    if ((d & (d - 1)) == 0)
    {
        result.magic = 0;
        result.more = libdivide__count_trailing_zeros32(d) | LIBDIVIDE_U32_SHIFT_PATH;
    }
    else
    {
        const uint32_t floor_log_2_d = 31 - libdivide__count_leading_zeros32(d);

        uint8_t more;
        uint32_t rem, proposed_m;
        proposed_m = libdivide_64_div_32_to_32(1U << floor_log_2_d, 0, d, &rem);

        const uint32_t e = d - rem;

        if (e < (1U << floor_log_2_d))
        {
            more = floor_log_2_d;
        }
        else
        {
            proposed_m += proposed_m;
            const uint32_t twice_rem = rem + rem;
            if (twice_rem >= d || twice_rem < rem) proposed_m += 1;
            more = floor_log_2_d | LIBDIVIDE_ADD_MARKER;
        }
        result.magic = 1 + proposed_m;
        result.more = more;
    }
    return result;
}

static inline uint32x4_t libdivide__mullhi_u32_flat_vector(uint32x4_t a, uint32_t b)
{
    uint32x2_t arm_a0a1 = vget_low_u32(a);
    uint32x2_t arm_a2a3 = vget_high_u32(a);

    uint64x2_t arm_a0a1_multiply_b = vmull_n_u32(arm_a0a1, b);
    uint64x2_t arm_a2a3_multiply_b = vmull_n_u32(arm_a2a3, b);

    uint32x2_t arm_hi_a0a1_multiply_b = vshrn_n_u64(arm_a0a1_multiply_b, 32);
    uint32x2_t arm_hi_a2a3_multiply_b = vshrn_n_u64(arm_a2a3_multiply_b, 32);

    return vcombine_u32(arm_hi_a0a1_multiply_b, arm_hi_a2a3_multiply_b);
}

inline uint32x4_t libdivide_u32_do_vector_general(uint32x4_t numerator, const struct libdivide_u32_t *p_denominator)
{
    uint8_t more = p_denominator->more;
    if (more & LIBDIVIDE_U32_SHIFT_PATH)
    {
        return vshlq_u32(numerator, vnegq_s32(vdupq_n_s32(more & LIBDIVIDE_32_SHIFT_MASK)));
    }
    else
    {
        uint32x4_t q = libdivide__mullhi_u32_flat_vector(numerator, p_denominator->magic);

        if (more & LIBDIVIDE_ADD_MARKER)
        {
            uint32x4_t t = vaddq_u32(vshrq_n_u32(vsubq_u32(numerator, q), 1), q);
            return vshlq_u32(t, vnegq_s32(vdupq_n_s32(more & LIBDIVIDE_32_SHIFT_MASK)));
        }
        else
        {
            return vshlq_u32(q, vnegq_s32(vdupq_n_s32(more)));
        }
    }
}

int libdivide_u32_get_case(const struct libdivide_u32_t *p_denominator)
{
    uint8_t more = p_denominator->more;
    if (more & LIBDIVIDE_U32_SHIFT_PATH)
        return 0;
    else if (more & LIBDIVIDE_ADD_MARKER)
        return 1;
    else
        return 2;
}

inline uint32x4_t libdivide_u32_do_vector_case0(uint32x4_t numerator, const struct libdivide_u32_t *p_denominator)
{
    uint8_t more = p_denominator->more;
    return vshlq_u32(numerator, vnegq_s32(vdupq_n_s32(more & LIBDIVIDE_32_SHIFT_MASK)));
}

inline uint32x4_t libdivide_u32_do_vector_case1(uint32x4_t numerator, const struct libdivide_u32_t *p_denominator)
{
    uint8_t more = p_denominator->more;
    uint32x4_t q = libdivide__mullhi_u32_flat_vector(numerator, p_denominator->magic);
    uint32x4_t t = vaddq_u32(vhsubq_u32(numerator, q), q);
    return vshlq_u32(t, vnegq_s32(vdupq_n_s32(more & LIBDIVIDE_32_SHIFT_MASK)));
}

inline uint32x4_t libdivide_u32_do_vector_case2(uint32x4_t numerator, const struct libdivide_u32_t *p_denominator)
{
    uint8_t more = p_denominator->more;
    uint32x4_t q = libdivide__mullhi_u32_flat_vector(numerator, p_denominator->magic);
    return vshlq_u32(q, vnegq_s32(vdupq_n_s32(more)));
}

#endif

