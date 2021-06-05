#include "stdafx.h"
#include "MorphologyTool.h"
#include "use_simd.h"
#ifndef UNIX_OS
#include "intsafe.h"
#endif

#define FILLHOLE_C_FAST 1 // can highly reduce the iteration of FillHole about 30%

#define FILLWATER_DEBUG  0
#define RISEUP_DEBUG 0

#if !defined(ANDROID_ARM) && !defined(IOS_ARM)
static inline void _mm_transpose16x16_epi8(__m128i &s0, __m128i &s1, __m128i &s2, __m128i &s3,
                                                  __m128i &s4, __m128i &s5, __m128i &s6, __m128i &s7,
                                                  __m128i &s8, __m128i &s9, __m128i &s10, __m128i &s11,
                                                  __m128i &s12, __m128i &s13, __m128i &s14, __m128i &s15,
                                                  __m128i &d0, __m128i &d1, __m128i &d2, __m128i &d3,
                                                  __m128i &d4, __m128i &d5, __m128i &d6, __m128i &d7,
                                                  __m128i &d8, __m128i &d9, __m128i &d10, __m128i &d11,
                                                  __m128i &d12, __m128i &d13, __m128i &d14, __m128i &d15)
{
    __m128i _t[16];

    _t[0] = _mm_unpacklo_epi8(s0, s1);
    _t[1] = _mm_unpacklo_epi8(s2, s3);
    _t[2] = _mm_unpacklo_epi8(s4, s5);
    _t[3] = _mm_unpacklo_epi8(s6, s7);
    _t[4] = _mm_unpacklo_epi8(s8, s9);
    _t[5] = _mm_unpacklo_epi8(s10, s11);
    _t[6] = _mm_unpacklo_epi8(s12, s13);
    _t[7] = _mm_unpacklo_epi8(s14, s15);

    _t[8] = _mm_unpacklo_epi16(_t[0], _t[1]);
    _t[9] = _mm_unpacklo_epi16(_t[2], _t[3]);
    _t[10] = _mm_unpacklo_epi16(_t[4], _t[5]);
    _t[11] = _mm_unpacklo_epi16(_t[6], _t[7]);

    _t[12] = _mm_unpacklo_epi32(_t[8], _t[9]);
    _t[13] = _mm_unpacklo_epi32(_t[10], _t[11]);
    d0 = _mm_unpacklo_epi64(_t[12],_t[13]);
    d1 = _mm_unpackhi_epi64(_t[12],_t[13]);

    _t[8] = _mm_unpackhi_epi32(_t[8], _t[9]);
    _t[10] = _mm_unpackhi_epi32(_t[10], _t[11]);
    d2 = _mm_unpacklo_epi64(_t[8],_t[10]);
    d3 = _mm_unpackhi_epi64(_t[8],_t[10]);

    _t[0] = _mm_unpackhi_epi16(_t[0], _t[1]);
    _t[2] = _mm_unpackhi_epi16(_t[2], _t[3]);
    _t[4] = _mm_unpackhi_epi16(_t[4], _t[5]);
    _t[6] = _mm_unpackhi_epi16(_t[6], _t[7]);

    _t[1] = _mm_unpacklo_epi32(_t[0], _t[2]);
    _t[3] = _mm_unpacklo_epi32(_t[4], _t[6]);
    d4 = _mm_unpacklo_epi64(_t[1],_t[3]);
    d5 = _mm_unpackhi_epi64(_t[1],_t[3]);

    _t[0] = _mm_unpackhi_epi32(_t[0], _t[2]);
    _t[4] = _mm_unpackhi_epi32(_t[4], _t[6]);
    d6 = _mm_unpacklo_epi64(_t[0],_t[4]);
    d7 = _mm_unpackhi_epi64(_t[0],_t[4]);

    _t[0] = _mm_unpackhi_epi8(s0, s1);
    _t[1] = _mm_unpackhi_epi8(s2, s3);
    _t[2] = _mm_unpackhi_epi8(s4, s5);
    _t[3] = _mm_unpackhi_epi8(s6, s7);
    _t[4] = _mm_unpackhi_epi8(s8, s9);
    _t[5] = _mm_unpackhi_epi8(s10, s11);
    _t[6] = _mm_unpackhi_epi8(s12, s13);
    _t[7] = _mm_unpackhi_epi8(s14, s15);

    _t[8] = _mm_unpacklo_epi16(_t[0], _t[1]);
    _t[9] = _mm_unpacklo_epi16(_t[2], _t[3]);
    _t[10] = _mm_unpacklo_epi16(_t[4], _t[5]);
    _t[11] = _mm_unpacklo_epi16(_t[6], _t[7]);

    _t[12] = _mm_unpacklo_epi32(_t[8], _t[9]);
    _t[13] = _mm_unpacklo_epi32(_t[10], _t[11]);
    d8 = _mm_unpacklo_epi64(_t[12],_t[13]);
    d9 = _mm_unpackhi_epi64(_t[12],_t[13]);

    _t[8] = _mm_unpackhi_epi32(_t[8], _t[9]);
    _t[10] = _mm_unpackhi_epi32(_t[10], _t[11]);
    d10 = _mm_unpacklo_epi64(_t[8],_t[10]);
    d11 = _mm_unpackhi_epi64(_t[8],_t[10]);

    _t[0] = _mm_unpackhi_epi16(_t[0], _t[1]);
    _t[2] = _mm_unpackhi_epi16(_t[2], _t[3]);
    _t[4] = _mm_unpackhi_epi16(_t[4], _t[5]);
    _t[6] = _mm_unpackhi_epi16(_t[6], _t[7]);

    _t[1] = _mm_unpacklo_epi32(_t[0], _t[2]);
    _t[3] = _mm_unpacklo_epi32(_t[4], _t[6]);
    d12 = _mm_unpacklo_epi64(_t[1],_t[3]);
    d13 = _mm_unpackhi_epi64(_t[1],_t[3]);

    _t[0] = _mm_unpackhi_epi32(_t[0], _t[2]);
    _t[4] = _mm_unpackhi_epi32(_t[4], _t[6]);
    d14 = _mm_unpacklo_epi64(_t[0],_t[4]);
    d15 = _mm_unpackhi_epi64(_t[0],_t[4]);
}

static inline void _mm_load16x16_si128(BYTE *p_src1, BYTE *p_src2, BYTE *p_src3, BYTE *p_src4,
                                              BYTE *p_src5, BYTE *p_src6, BYTE *p_src7, BYTE *p_src8,
                                              BYTE *p_src9, BYTE *p_src10, BYTE *p_src11, BYTE *p_src12,
                                              BYTE *p_src13, BYTE *p_src14, BYTE *p_src15, BYTE *p_src16,
                                              __m128i &v1, __m128i &v2, __m128i &v3, __m128i &v4,
                                              __m128i &v5, __m128i &v6, __m128i &v7, __m128i &v8,
                                              __m128i &v9, __m128i &v10, __m128i &v11, __m128i &v12,
                                              __m128i &v13, __m128i &v14, __m128i &v15, __m128i &v16)
{
    v1 = _mm_load_si128((__m128i *)(p_src1));
    v2 = _mm_load_si128((__m128i *)(p_src2));
    v3 = _mm_load_si128((__m128i *)(p_src3));
    v4 = _mm_load_si128((__m128i *)(p_src4));
    v5 = _mm_load_si128((__m128i *)(p_src5));
    v6 = _mm_load_si128((__m128i *)(p_src6));
    v7 = _mm_load_si128((__m128i *)(p_src7));
    v8 = _mm_load_si128((__m128i *)(p_src8));
    v9 = _mm_load_si128((__m128i *)(p_src9));
    v10 = _mm_load_si128((__m128i *)(p_src10));
    v11 = _mm_load_si128((__m128i *)(p_src11));
    v12 = _mm_load_si128((__m128i *)(p_src12));
    v13 = _mm_load_si128((__m128i *)(p_src13));
    v14 = _mm_load_si128((__m128i *)(p_src14));
    v15 = _mm_load_si128((__m128i *)(p_src15));
    v16 = _mm_load_si128((__m128i *)(p_src16));
}

static inline void _mm_store16x16_si128(BYTE *p_src1, BYTE *p_src2, BYTE *p_src3, BYTE *p_src4,
                                               BYTE *p_src5, BYTE *p_src6, BYTE *p_src7, BYTE *p_src8,
                                               BYTE *p_src9, BYTE *p_src10, BYTE *p_src11, BYTE *p_src12,
                                               BYTE *p_src13, BYTE *p_src14, BYTE *p_src15, BYTE *p_src16,
                                               __m128i &v1, __m128i &v2, __m128i &v3, __m128i &v4,
                                               __m128i &v5, __m128i &v6, __m128i &v7, __m128i &v8,
                                               __m128i &v9, __m128i &v10, __m128i &v11, __m128i &v12,
                                               __m128i &v13, __m128i &v14, __m128i &v15, __m128i &v16)
{
    _mm_store_si128((__m128i *)(p_src1), v1);
    _mm_store_si128((__m128i *)(p_src2), v2);
    _mm_store_si128((__m128i *)(p_src3), v3);
    _mm_store_si128((__m128i *)(p_src4), v4);
    _mm_store_si128((__m128i *)(p_src5), v5);
    _mm_store_si128((__m128i *)(p_src6), v6);
    _mm_store_si128((__m128i *)(p_src7), v7);
    _mm_store_si128((__m128i *)(p_src8), v8);
    _mm_store_si128((__m128i *)(p_src9), v9);
    _mm_store_si128((__m128i *)(p_src10), v10);
    _mm_store_si128((__m128i *)(p_src11), v11);
    _mm_store_si128((__m128i *)(p_src12), v12);
    _mm_store_si128((__m128i *)(p_src13), v13);
    _mm_store_si128((__m128i *)(p_src14), v14);
    _mm_store_si128((__m128i *)(p_src15), v15);
    _mm_store_si128((__m128i *)(p_src16), v16);
}
#else
static inline void transpose16x16_u8(uint8x16_t &src_0, uint8x16_t &src_1, uint8x16_t &src_2, uint8x16_t &src_3,
                                            uint8x16_t &src_4, uint8x16_t &src_5, uint8x16_t &src_6, uint8x16_t &src_7,
                                            uint8x16_t &src_8, uint8x16_t &src_9, uint8x16_t &src_a, uint8x16_t &src_b,
                                            uint8x16_t &src_c, uint8x16_t &src_d, uint8x16_t &src_e, uint8x16_t &src_f,
                                            uint8x16_t &dst_0, uint8x16_t &dst_1, uint8x16_t &dst_2, uint8x16_t &dst_3,
                                            uint8x16_t &dst_4, uint8x16_t &dst_5, uint8x16_t &dst_6, uint8x16_t &dst_7,
                                            uint8x16_t &dst_8, uint8x16_t &dst_9, uint8x16_t &dst_a, uint8x16_t &dst_b,
                                            uint8x16_t &dst_c, uint8x16_t &dst_d, uint8x16_t &dst_e, uint8x16_t &dst_f)
{
    uint8x16x2_t p01 = vtrnq_u8(src_0, src_1);
    uint8x16x2_t p23 = vtrnq_u8(src_2, src_3);
    uint8x16x2_t p45 = vtrnq_u8(src_4, src_5);
    uint8x16x2_t p67 = vtrnq_u8(src_6, src_7);
    uint8x16x2_t p89 = vtrnq_u8(src_8, src_9);
    uint8x16x2_t pab = vtrnq_u8(src_a, src_b);
    uint8x16x2_t pcd = vtrnq_u8(src_c, src_d);
    uint8x16x2_t pef = vtrnq_u8(src_e, src_f);

    uint16x8x2_t q02 = vtrnq_u16(vreinterpretq_u16_u8(p01.val[0]), vreinterpretq_u16_u8(p23.val[0]));
    uint16x8x2_t q13 = vtrnq_u16(vreinterpretq_u16_u8(p01.val[1]), vreinterpretq_u16_u8(p23.val[1]));
    uint16x8x2_t q46 = vtrnq_u16(vreinterpretq_u16_u8(p45.val[0]), vreinterpretq_u16_u8(p67.val[0]));
    uint16x8x2_t q57 = vtrnq_u16(vreinterpretq_u16_u8(p45.val[1]), vreinterpretq_u16_u8(p67.val[1]));
    uint16x8x2_t q8a = vtrnq_u16(vreinterpretq_u16_u8(p89.val[0]), vreinterpretq_u16_u8(pab.val[0]));
    uint16x8x2_t q9b = vtrnq_u16(vreinterpretq_u16_u8(p89.val[1]), vreinterpretq_u16_u8(pab.val[1]));
    uint16x8x2_t qce = vtrnq_u16(vreinterpretq_u16_u8(pcd.val[0]), vreinterpretq_u16_u8(pef.val[0]));
    uint16x8x2_t qdf = vtrnq_u16(vreinterpretq_u16_u8(pcd.val[1]), vreinterpretq_u16_u8(pef.val[1]));

    uint32x4x2_t r04 = vtrnq_u32(vreinterpretq_u32_u16(q02.val[0]), vreinterpretq_u32_u16(q46.val[0]));
    uint32x4x2_t r26 = vtrnq_u32(vreinterpretq_u32_u16(q02.val[1]), vreinterpretq_u32_u16(q46.val[1]));
    uint32x4x2_t r15 = vtrnq_u32(vreinterpretq_u32_u16(q13.val[0]), vreinterpretq_u32_u16(q57.val[0]));
    uint32x4x2_t r37 = vtrnq_u32(vreinterpretq_u32_u16(q13.val[1]), vreinterpretq_u32_u16(q57.val[1]));
    uint32x4x2_t r8c = vtrnq_u32(vreinterpretq_u32_u16(q8a.val[0]), vreinterpretq_u32_u16(qce.val[0]));
    uint32x4x2_t rae = vtrnq_u32(vreinterpretq_u32_u16(q8a.val[1]), vreinterpretq_u32_u16(qce.val[1]));
    uint32x4x2_t r9d = vtrnq_u32(vreinterpretq_u32_u16(q9b.val[0]), vreinterpretq_u32_u16(qdf.val[0]));
    uint32x4x2_t rbf = vtrnq_u32(vreinterpretq_u32_u16(q9b.val[1]), vreinterpretq_u32_u16(qdf.val[1]));

    dst_0 = vreinterpretq_u8_u32(vcombine_u32(vget_low_u32(r04.val[0]), vget_low_u32(r8c.val[0])));
    dst_1 = vreinterpretq_u8_u32(vcombine_u32(vget_low_u32(r15.val[0]), vget_low_u32(r9d.val[0])));
    dst_2 = vreinterpretq_u8_u32(vcombine_u32(vget_low_u32(r26.val[0]), vget_low_u32(rae.val[0])));
    dst_3 = vreinterpretq_u8_u32(vcombine_u32(vget_low_u32(r37.val[0]), vget_low_u32(rbf.val[0])));
    dst_4 = vreinterpretq_u8_u32(vcombine_u32(vget_low_u32(r04.val[1]), vget_low_u32(r8c.val[1])));
    dst_5 = vreinterpretq_u8_u32(vcombine_u32(vget_low_u32(r15.val[1]), vget_low_u32(r9d.val[1])));
    dst_6 = vreinterpretq_u8_u32(vcombine_u32(vget_low_u32(r26.val[1]), vget_low_u32(rae.val[1])));
    dst_7 = vreinterpretq_u8_u32(vcombine_u32(vget_low_u32(r37.val[1]), vget_low_u32(rbf.val[1])));
    dst_8 = vreinterpretq_u8_u32(vcombine_u32(vget_high_u32(r04.val[0]), vget_high_u32(r8c.val[0])));
    dst_9 = vreinterpretq_u8_u32(vcombine_u32(vget_high_u32(r15.val[0]), vget_high_u32(r9d.val[0])));
    dst_a = vreinterpretq_u8_u32(vcombine_u32(vget_high_u32(r26.val[0]), vget_high_u32(rae.val[0])));
    dst_b = vreinterpretq_u8_u32(vcombine_u32(vget_high_u32(r37.val[0]), vget_high_u32(rbf.val[0])));
    dst_c = vreinterpretq_u8_u32(vcombine_u32(vget_high_u32(r04.val[1]), vget_high_u32(r8c.val[1])));
    dst_d = vreinterpretq_u8_u32(vcombine_u32(vget_high_u32(r15.val[1]), vget_high_u32(r9d.val[1])));
    dst_e = vreinterpretq_u8_u32(vcombine_u32(vget_high_u32(r26.val[1]), vget_high_u32(rae.val[1])));
    dst_f = vreinterpretq_u8_u32(vcombine_u32(vget_high_u32(r37.val[1]), vget_high_u32(rbf.val[1])));
}

static inline void load16x16_u8(BYTE *p_src_0, BYTE *p_src_1, BYTE *p_src_2, BYTE *p_src_3,
                                       BYTE *p_src_4, BYTE *p_src_5, BYTE *p_src_6, BYTE *p_src_7,
                                       BYTE *p_src_8, BYTE *p_src_9, BYTE *p_src_a, BYTE *p_src_b,
                                       BYTE *p_src_c, BYTE *p_src_d, BYTE *p_src_e, BYTE *p_src_f,
                                       uint8x16_t &dst_0, uint8x16_t &dst_1, uint8x16_t &dst_2, uint8x16_t &dst_3,
                                       uint8x16_t &dst_4, uint8x16_t &dst_5, uint8x16_t &dst_6, uint8x16_t &dst_7,
                                       uint8x16_t &dst_8, uint8x16_t &dst_9, uint8x16_t &dst_a, uint8x16_t &dst_b,
                                       uint8x16_t &dst_c, uint8x16_t &dst_d, uint8x16_t &dst_e, uint8x16_t &dst_f)
{
    dst_0 = vld1q_u8(p_src_0);
    dst_1 = vld1q_u8(p_src_1);
    dst_2 = vld1q_u8(p_src_2);
    dst_3 = vld1q_u8(p_src_3);
    dst_4 = vld1q_u8(p_src_4);
    dst_5 = vld1q_u8(p_src_5);
    dst_6 = vld1q_u8(p_src_6);
    dst_7 = vld1q_u8(p_src_7);
    dst_8 = vld1q_u8(p_src_8);
    dst_9 = vld1q_u8(p_src_9);
    dst_a = vld1q_u8(p_src_a);
    dst_b = vld1q_u8(p_src_b);
    dst_c = vld1q_u8(p_src_c);
    dst_d = vld1q_u8(p_src_d);
    dst_e = vld1q_u8(p_src_e);
    dst_f = vld1q_u8(p_src_f);
}

static inline void store16x16_u8(uint8x16_t &src_0, uint8x16_t &src_1, uint8x16_t &src_2, uint8x16_t &src_3,
                                        uint8x16_t &src_4, uint8x16_t &src_5, uint8x16_t &src_6, uint8x16_t &src_7,
                                        uint8x16_t &src_8, uint8x16_t &src_9, uint8x16_t &src_a, uint8x16_t &src_b,
                                        uint8x16_t &src_c, uint8x16_t &src_d, uint8x16_t &src_e, uint8x16_t &src_f,
                                        BYTE *p_dst_0, BYTE *p_dst_1, BYTE *p_dst_2, BYTE *p_dst_3,
                                        BYTE *p_dst_4, BYTE *p_dst_5, BYTE *p_dst_6, BYTE *p_dst_7,
                                        BYTE *p_dst_8, BYTE *p_dst_9, BYTE *p_dst_a, BYTE *p_dst_b,
                                        BYTE *p_dst_c, BYTE *p_dst_d, BYTE *p_dst_e, BYTE *p_dst_f)
{
    vst1q_u8(p_dst_0, src_0);
    vst1q_u8(p_dst_1, src_1);
    vst1q_u8(p_dst_2, src_2);
    vst1q_u8(p_dst_3, src_3);
    vst1q_u8(p_dst_4, src_4);
    vst1q_u8(p_dst_5, src_5);
    vst1q_u8(p_dst_6, src_6);
    vst1q_u8(p_dst_7, src_7);
    vst1q_u8(p_dst_8, src_8);
    vst1q_u8(p_dst_9, src_9);
    vst1q_u8(p_dst_a, src_a);
    vst1q_u8(p_dst_b, src_b);
    vst1q_u8(p_dst_c, src_c);
    vst1q_u8(p_dst_d, src_d);
    vst1q_u8(p_dst_e, src_e);
    vst1q_u8(p_dst_f, src_f);
}
#endif

MorphologyTool::MorphologyTool()
    : m_width(0)
    , m_height(0)
    , m_align_width(0)
    , m_align_height(0)
    , mp_align_buffer(NULL)
    , mp_left_bound(NULL)
    , mp_right_bound(NULL)
    , mp_top_bound(NULL)
    , mp_bottom_bound(NULL)
    , m_stage(NONE)
    , m_num_thread(0)
    , mp_thread_control(NULL)
    , mp_thread_param(NULL)
    , mp_thread_pool(NULL)
    , mp_macroblock_event(NULL)
    , m_num_macroblock_i(0)
    , m_num_macroblock_j(0)
{
    InitialThread();
}

MorphologyTool::~MorphologyTool()
{
    _DELETE_PTRS(mp_thread_control);
    _DELETE_PTRS(mp_thread_param);

    ReleaseMacroBlockEvent();

    _ALIGNED_FREE_PTR(mp_left_bound);
    _ALIGNED_FREE_PTR(mp_right_bound);
    _ALIGNED_FREE_PTR(mp_top_bound);
    _ALIGNED_FREE_PTR(mp_bottom_bound);

    _ALIGNED_FREE_PTR(mp_align_buffer);
}

void MorphologyTool::Initialize(int width, int height)
{
    m_width = width;
    m_height = height;
    m_align_width = ALIGN(width, 16);
    m_align_height = ALIGN(height, 16);

    // temp buffer for fill hole
    _ALIGNED_MALLOC_PTR(mp_left_bound, BYTE, m_align_height);
    _ALIGNED_MALLOC_PTR(mp_right_bound, BYTE, m_align_height);
    _ALIGNED_MALLOC_PTR(mp_top_bound, BYTE, m_align_width);
    _ALIGNED_MALLOC_PTR(mp_bottom_bound, BYTE, m_align_width);
}

BYTE *MorphologyTool::GetAlignBuffer(BYTE *p_src, int src_stride)
{
    _MYASSERT(p_src);
    
    // "p_src == mp_align_buffer" means mp_align_buffer is created in the caller.
    // so it's not neccerary to create it again
    if (p_src == mp_align_buffer)
        return mp_align_buffer;

    if(src_stride != m_align_width || m_height != m_align_height || ((INT_PTR)p_src & 15))
    {
        _ALIGNED_MALLOC_PTR(mp_align_buffer, BYTE, m_align_width * m_align_height);
        _MYASSERT(mp_align_buffer);
        memset(mp_align_buffer, 0, m_align_width * m_align_height);

        // copy and pad src buffer
        int width = m_width;
        int dst_stride = m_align_width;
        BYTE *p_src_scan = p_src;
        BYTE *p_dst_scan = mp_align_buffer;
        int i = 0;
        for (; i < m_height; i++)
        {
            memcpy(p_dst_scan, p_src_scan, width);
            memset(p_dst_scan + width, p_dst_scan[width - 1], dst_stride - width);
            p_src_scan += src_stride;
            p_dst_scan += dst_stride;
        }
        for (; i < m_align_height; i++)
        {
            memcpy(p_dst_scan, p_dst_scan - dst_stride, dst_stride);
            p_dst_scan += dst_stride;
        }

        return mp_align_buffer;
    }
    else
        return p_src;
}

void MorphologyTool::FreeAndCopyResult(BYTE *p_dst, int dst_stride)
{
    _MYASSERT(p_dst);

    // "p_dst == mp_align_buffer" means mp_align_buffer is not created in the caller.
    // so it's not necessary to free and copy
    if (p_dst == mp_align_buffer)
        return;

    if(dst_stride != m_align_width || m_height != m_align_height || ((INT_PTR)p_dst & 15))
    {
        _MYASSERT(mp_align_buffer);
        // copy result to dst buffer
        int src_stride = m_align_width;
        BYTE *p_src_scan = mp_align_buffer;
        BYTE *p_dst_scan = p_dst;
        for (int i = 0; i < m_height; i++)
        {
            memcpy(p_dst_scan, p_src_scan, m_width);
            p_src_scan += src_stride;
            p_dst_scan += dst_stride;
        }

        _ALIGNED_FREE_PTR(mp_align_buffer);
    }
}

void MorphologyTool::Close(BYTE *p_src_dst, int src_stride, int close_parameter)
{
    _MYASSERT(p_src_dst);
    
    BYTE *p_align_buffer = GetAlignBuffer(p_src_dst, src_stride);

    BYTE *p_closing_temp_buffer = NULL;
    _ALIGNED_MALLOC_PTR(p_closing_temp_buffer, BYTE, m_align_width * m_align_height);

    //dilate
    int i = 0;
    for (; i < close_parameter >> 1; i++)
        LocalMaxFilter5x5(p_align_buffer, p_align_buffer, p_closing_temp_buffer,
                          m_height, m_width, m_align_width);
    if (close_parameter & 1)
        LocalMaxFilter3x3(p_align_buffer, p_align_buffer, p_closing_temp_buffer,
                          m_height, m_width, m_align_width);

    //erode
    i = 0;
    for (; i < close_parameter >> 1; i++)
        LocalMinFilter5x5(p_align_buffer, p_align_buffer, p_closing_temp_buffer,
                          m_height, m_width, m_align_width);
    if (close_parameter & 1)
        LocalMinFilter3x3(p_align_buffer, p_align_buffer, p_closing_temp_buffer,
                          m_height, m_width, m_align_width);

    _ALIGNED_FREE_PTR(p_closing_temp_buffer);

    FreeAndCopyResult(p_src_dst, src_stride);
}

void MorphologyTool::CloseFillHole(BYTE *p_src_dst, int src_stride, int close_parameter, int max_iteration, bool is_src_binary)
{
    _MYASSERT(p_src_dst);

    BYTE *p_align_buffer = GetAlignBuffer(p_src_dst, src_stride);

    BYTE *p_closing_temp_buffer = NULL;
    _ALIGNED_MALLOC_PTR(p_closing_temp_buffer, BYTE, m_align_width * m_align_height);

    //dilate
    int i = 0;
    for (; i < close_parameter >> 1; i++)
        LocalMaxFilter5x5(p_align_buffer, p_align_buffer, p_closing_temp_buffer,
                          m_height, m_width, m_align_width);
    if (close_parameter & 1)
        LocalMaxFilter3x3(p_align_buffer, p_align_buffer, p_closing_temp_buffer,
                          m_height, m_width, m_align_width);

    if (is_src_binary) // value should be only 0 and 255
        FillHoleBinary(p_align_buffer, m_align_width, max_iteration);
    else
        FillHole(p_align_buffer, m_align_width, max_iteration);

    //erode
    i = 0;
    for (; i < close_parameter >> 1; i++)
        LocalMinFilter5x5(p_align_buffer, p_align_buffer, p_closing_temp_buffer,
                          m_height, m_width, m_align_width);
    if (close_parameter & 1)
        LocalMinFilter3x3(p_align_buffer, p_align_buffer, p_closing_temp_buffer,
                          m_height, m_width, m_align_width);

    _ALIGNED_FREE_PTR(p_closing_temp_buffer);

    FreeAndCopyResult(p_src_dst, src_stride);
}

void MorphologyTool::LocalMinFilter5x5(BYTE *p_src_buffer,
                                       BYTE *p_min_buffer,
                                       BYTE *p_temp_buffer,
                                       int   height,
                                       int   width,
                                       int   stride)
{
    _MYASSERT(p_src_buffer);
    _MYASSERT(p_min_buffer);
    _MYASSERT(p_temp_buffer);

    if (p_src_buffer == p_min_buffer) //for in-place processing
    {
        LocalMinFilter3x3(p_src_buffer, p_min_buffer, p_temp_buffer, height, width, stride);
        memcpy(p_temp_buffer, p_min_buffer, sizeof(BYTE) * height * stride);
    }
    else
    {
        LocalMinFilter3x3(p_src_buffer, p_temp_buffer, p_min_buffer, height, width, stride);
    }

    if (width == 1 || height == 1)
        return;

    int j;

    BYTE *p_temp = p_temp_buffer;
    BYTE *p_min = p_min_buffer;

#if !defined(ANDROID_ARM) && !defined(IOS_ARM)
    if (g_is_support_SSE2)
    {
        //top row
        p_min[0] = p_temp[stride + 1];
        for (j = 1; j < width - 1; j++)
            p_min[j] = ch_Min(p_temp[stride + j - 1], p_temp[stride + j + 1]);
        p_min[j] = p_temp[stride + j - 1];

        //centre
        p_temp += stride;
        p_min += stride;
        for (int i = 1; i < height - 1; i++)
        {
            p_min[0] = ch_Min(p_temp[-stride + 1], p_temp[stride + 1]);
            for (j = 1; j < ((width - 1) & ~15); j += 16)
            {
                __m128i s0, s1;
                s0 = _mm_load_si128((__m128i *)(p_temp - stride + j - 1));
                s1 = _mm_loadu_si128((__m128i *)(p_temp - stride + j + 1));
                s0 = _mm_min_epu8(s0, s1);
                s1 = _mm_load_si128((__m128i *)(p_temp + stride + j - 1));
                s0 = _mm_min_epu8(s0, s1);
                s1 = _mm_loadu_si128((__m128i *)(p_temp + stride + j + 1));
                _mm_storeu_si128((__m128i *)(p_min + j), _mm_min_epu8(s0, s1));
            }
            for (; j < width - 1; j++)
            {
                p_min[j] = ch_Min(p_temp[-stride + j - 1], p_temp[-stride + j + 1]);
                p_min[j] = ch_Min(p_min[j], p_temp[stride + j - 1]);
                p_min[j] = ch_Min(p_min[j], p_temp[stride + j + 1]);
            }
            p_min[j] = ch_Min(p_temp[-stride + j - 1], p_temp[stride + j - 1]);

            p_temp += stride;
            p_min += stride;
        }

        //bottm row
        p_min[0] = p_temp[-stride + 1];
        for (j = 1; j < width - 1; j++)
            p_min[j] = ch_Min(p_temp[-stride + j - 1], p_temp[-stride + j + 1]);
        p_min[j] = p_temp[-stride + j - 1];
    }
    else
    {
        //top row
        p_min[0] = p_temp[stride + 1];
        for (j = 1; j < width - 1; j++)
            p_min[j] = ch_Min(p_temp[stride + j - 1], p_temp[stride + j + 1]);
        p_min[j] = p_temp[stride + j - 1];

        //centre
        p_temp += stride;
        p_min += stride;
        for (int i = 1; i < height - 1; i++)
        {
            p_min[0] = ch_Min(p_temp[-stride + 1], p_temp[stride + 1]);
            for (j = 1; j < width - 1; j++)
            {
                p_min[j] = ch_Min(p_temp[-stride + j - 1], p_temp[-stride + j + 1]);
                p_min[j] = ch_Min(p_min[j], p_temp[stride + j - 1]);
                p_min[j] = ch_Min(p_min[j], p_temp[stride + j + 1]);
            }
            p_min[j] = ch_Min(p_temp[-stride + j - 1], p_temp[stride + j - 1]);

            p_temp += stride;
            p_min += stride;
        }

        //bottm row
        p_min[0] = p_temp[-stride + 1];
        for (j = 1; j < width - 1; j++)
            p_min[j] = ch_Min(p_temp[-stride + j - 1], p_temp[-stride + j + 1]);
        p_min[j] = p_temp[-stride + j - 1];
    }
#else

    // top row
    p_min[0] = p_temp[stride + 1];
    uint8x16_t arm_result;
    const int simd_right = ((width - 1) & ~15);
    for (j = 1; j < width - 1; j++)
        p_min[j] = ch_Min(p_temp[stride + j - 1], p_temp[stride + j + 1]);
    p_min[j] = p_temp[stride + j - 1];

    // center
    p_temp += stride;
    p_min += stride;
    for (int i = 1; i < height - 1; i++)
    {
        p_min[0] = ch_Min(p_temp[-stride + 1], p_temp[stride + 1]);
        for (j = 1; j < simd_right; j += 16)
        {
            uint8x16_t arm_left_top = vld1q_u8(p_temp - stride + j - 1);
            uint8x16_t arm_right_top = vld1q_u8(p_temp - stride + j + 1);
            uint8x16_t arm_left_bottom = vld1q_u8(p_temp + stride + j - 1);
            uint8x16_t arm_right_bottom = vld1q_u8(p_temp + stride + j + 1);

            uint8x16_t arm_top_min = vminq_u8(arm_left_top, arm_right_top);
            uint8x16_t arm_bottom_max = vminq_u8(arm_left_bottom, arm_right_bottom);

            arm_result = vminq_u8(arm_top_min, arm_bottom_max);
            vst1q_u8(p_min + j, arm_result);
        }
        for (; j < width - 1; j++)
        {
            p_min[j] = ch_Min(p_temp[-stride + j - 1], p_temp[-stride + j + 1]);
            p_min[j] = ch_Min(p_min[j], p_temp[stride + j - 1]);
            p_min[j] = ch_Min(p_min[j], p_temp[stride + j + 1]);
        }
        p_min[j] = ch_Min(p_temp[-stride + j - 1], p_temp[stride + j - 1]);

        p_temp += stride;
        p_min += stride;
    }

    // bottom row
    p_min[0] = p_temp[-stride + 1];
    for (j = 1; j < width - 1; j++)
        p_min[j] = ch_Min(p_temp[-stride + j - 1], p_temp[-stride + j + 1]);
    p_min[j] = p_temp[-stride + j - 1];

#endif
}

void MorphologyTool::LocalMaxFilter5x5(BYTE *p_src_buffer,
                                       BYTE *p_max_buffer,
                                       BYTE *p_temp_buffer,
                                       int   height,
                                       int   width,
                                       int   stride)
{
    _MYASSERT(p_src_buffer);
    _MYASSERT(p_max_buffer);
    _MYASSERT(p_temp_buffer);

    if (p_src_buffer == p_max_buffer) //for in-place processing
    {
        LocalMaxFilter3x3(p_src_buffer, p_max_buffer, p_temp_buffer, height, width, stride);
        memcpy(p_temp_buffer, p_max_buffer, sizeof(BYTE) * height * stride);
    }
    else
    {
        LocalMaxFilter3x3(p_src_buffer, p_temp_buffer, p_max_buffer, height, width, stride);
    }

    if (width == 1 || height == 1)
        return;

    int j;

    BYTE *p_temp = p_temp_buffer;
    BYTE *p_max = p_max_buffer;

#if !defined(ANDROID_ARM) && !defined(IOS_ARM)
    if (g_is_support_SSE2)
    {
        //top row
        p_max[0] = p_temp[stride + 1];
        for (j = 1; j < width - 1; j++)
            p_max[j] = ch_Max(p_temp[stride + j - 1], p_temp[stride + j + 1]);
        p_max[j] = p_temp[stride + j - 1];

        //centre
        p_temp += stride;
        p_max += stride;
        for (int i = 1; i < height - 1; i++)
        {
            p_max[0] = ch_Max(p_temp[-stride + 1], p_temp[stride + 1]);
            for (j = 1; j < ((width - 1) & ~15); j += 16)
            {
                __m128i s0, s1;
                s0 = _mm_load_si128((__m128i *)(p_temp - stride + j - 1));
                s1 = _mm_loadu_si128((__m128i *)(p_temp - stride + j + 1));
                s0 = _mm_max_epu8(s0, s1);
                s1 = _mm_load_si128((__m128i *)(p_temp + stride + j - 1));
                s0 = _mm_max_epu8(s0, s1);
                s1 = _mm_loadu_si128((__m128i *)(p_temp + stride + j + 1));
                _mm_storeu_si128((__m128i *)(p_max + j), _mm_max_epu8(s0, s1));
            }
            for (; j < width - 1; j++)
            {
                p_max[j] = ch_Max(p_temp[-stride + j - 1], p_temp[-stride + j + 1]);
                p_max[j] = ch_Max(p_max[j], p_temp[stride + j - 1]);
                p_max[j] = ch_Max(p_max[j], p_temp[stride + j + 1]);
            }
            p_max[j] = ch_Max(p_temp[-stride + j - 1], p_temp[stride + j - 1]);

            p_temp += stride;
            p_max += stride;
        }

        //bottm row
        p_max[0] = p_temp[-stride + 1];
        for (j = 1; j < width - 1; j++)
            p_max[j] = ch_Max(p_temp[-stride + j - 1], p_temp[-stride + j + 1]);
        p_max[j] = p_temp[-stride + j - 1];
    }
    else
    {
        //top row
        p_max[0] = p_temp[stride + 1];
        for (j = 1; j < width - 1; j++)
            p_max[j] = ch_Max(p_temp[stride + j - 1], p_temp[stride + j + 1]);
        p_max[j] = p_temp[stride + j - 1];

        //centre
        p_temp += stride;
        p_max += stride;
        for (int i = 1; i < height - 1; i++)
        {
            p_max[0] = ch_Max(p_temp[-stride + 1], p_temp[stride + 1]);
            for (j = 1; j < width - 1; j++)
            {
                p_max[j] = ch_Max(p_temp[-stride + j - 1], p_temp[-stride + j + 1]);
                p_max[j] = ch_Max(p_max[j], p_temp[stride + j - 1]);
                p_max[j] = ch_Max(p_max[j], p_temp[stride + j + 1]);
            }
            p_max[j] = ch_Max(p_temp[-stride + j - 1], p_temp[stride + j - 1]);

            p_temp += stride;
            p_max += stride;
        }

        //bottom row
        p_max[0] = p_temp[-stride + 1];
        for (j = 1; j < width - 1; j++)
            p_max[j] = ch_Max(p_temp[-stride + j - 1], p_temp[-stride + j + 1]);
        p_max[j] = p_temp[-stride + j - 1];
    }
#else

    // top row
    p_max[0] = p_temp[stride + 1];
    uint8x16_t arm_result;
    const int simd_right = ((width - 1) & ~15);
    for (j = 1; j < width - 1; j++)
        p_max[j] = ch_Max(p_temp[stride + j - 1], p_temp[stride + j + 1]);
    p_max[j] = p_temp[stride + j - 1];

    // center
    p_temp += stride;
    p_max += stride;
    for (int i = 1; i < height - 1; i++)
    {
        p_max[0] = ch_Max(p_temp[-stride + 1], p_temp[stride + 1]);
        for (j = 1; j < simd_right; j += 16)
        {
            uint8x16_t arm_left_top = vld1q_u8(p_temp - stride + j - 1);
            uint8x16_t arm_right_top = vld1q_u8(p_temp - stride + j + 1);
            uint8x16_t arm_left_bottom = vld1q_u8(p_temp + stride + j - 1);
            uint8x16_t arm_right_bottom = vld1q_u8(p_temp + stride + j + 1);

            uint8x16_t arm_top_max = vmaxq_u8(arm_left_top, arm_right_top);
            uint8x16_t arm_bottom_max = vmaxq_u8(arm_left_bottom, arm_right_bottom);

            arm_result = vmaxq_u8(arm_top_max, arm_bottom_max);
            vst1q_u8(p_max + j, arm_result);
        }
        for (; j < width - 1; j++)
        {
            p_max[j] = ch_Max(p_temp[-stride + j - 1], p_temp[-stride + j + 1]);
            p_max[j] = ch_Max(p_max[j], p_temp[stride + j - 1]);
            p_max[j] = ch_Max(p_max[j], p_temp[stride + j + 1]);
        }
        p_max[j] = ch_Max(p_temp[-stride + j - 1], p_temp[stride + j - 1]);

        p_temp += stride;
        p_max += stride;
    }

    // bottom row
    p_max[0] = p_temp[-stride + 1];
    for (j = 1; j < width - 1; j++)
        p_max[j] = ch_Max(p_temp[-stride + j - 1], p_temp[-stride + j + 1]);
    p_max[j] = p_temp[-stride + j - 1];

#endif
}

void MorphologyTool::LocalMinFilter3x3(BYTE *p_src_buffer,
                                       BYTE *p_min_buffer,
                                       BYTE *p_temp_buffer,
                                       int   height,
                                       int   width,
                                       int   stride)
{
    _MYASSERT(p_src_buffer);
    _MYASSERT(p_min_buffer);
    _MYASSERT(p_temp_buffer);

    int j;
    BYTE *p_src, *p_temp, *p_min;

#if !defined(ANDROID_ARM) && !defined(IOS_ARM)
    if (g_is_support_SSE2)
    {
        //Part 1: horizontal filtering

        p_src = p_src_buffer;
        p_temp = p_temp_buffer;
        //top row
        for (j = 1; j < width - 1; j++)
        {
            p_temp[j] = ch_Min(p_src[j - 1], p_src[j]);
            p_temp[j] = ch_Min(p_temp[j], p_src[j + 1]);
        }
        p_temp[0] = ch_Min(p_src[0], p_src[1]);
        p_temp[width - 1] = ch_Min(p_src[width - 2], p_src[width - 1]);
        //centre
        p_src += stride;
        p_temp += stride;
        for (int i = 1; i < height - 1; i++)
        {
            for (j = 0; j < (width & ~15); j += 16)
            {
                __m128i s0, s1;
                s0 = _mm_loadu_si128((__m128i *)(p_src + j - 1));
                s1 = _mm_load_si128((__m128i *)(p_src + j));
                s0 = _mm_min_epu8(s0, s1);
                s1 = _mm_loadu_si128((__m128i *)(p_src + j + 1));
                _mm_store_si128((__m128i *)(p_temp + j), _mm_min_epu8(s0, s1));
            }
            for (; j < width - 1; j++)
            {
                p_temp[j] = ch_Min(p_src[j - 1], p_src[j]);
                p_temp[j] = ch_Min(p_temp[j], p_src[j + 1]);
            }
            p_temp[0] = ch_Min(p_src[0], p_src[1]);
            p_temp[width - 1] = ch_Min(p_src[width - 2], p_src[width - 1]);

            //
            p_src += stride;
            p_temp += stride;
        }
        //bottom row
        for (j = 1; j < width - 1; j++)
        {
            p_temp[j] = ch_Min(p_src[j - 1], p_src[j]);
            p_temp[j] = ch_Min(p_temp[j], p_src[j + 1]);
        }
        p_temp[0] = ch_Min(p_src[0], p_src[1]);
        p_temp[width - 1] = ch_Min(p_src[width - 2], p_src[width - 1]);


        //Part 2: vertical filtering

        p_temp = p_temp_buffer;
        p_min = p_min_buffer;
        //top boundary
        for (j = 0; j < (width & ~15); j += 16)
        {
            __m128i s0, s1;
            s0 = _mm_load_si128((__m128i *)(p_temp + j));
            s1 = _mm_load_si128((__m128i *)(p_temp + stride + j));
            _mm_store_si128((__m128i *)(p_min + j), _mm_min_epu8(s0, s1));
        }
        for (; j < width; j++)
            p_min[j] = ch_Min(p_temp[j], p_temp[stride + j]);
        //centre
        p_temp += stride;
        p_min += stride;
        for (int i = 1; i < height - 1; i++)
        {
            for (j = 0; j < (width & ~15); j += 16)
            {
                __m128i s0, s1;
                s0 = _mm_load_si128((__m128i *)(p_temp - stride + j));
                s1 = _mm_load_si128((__m128i *)(p_temp + j));
                s0 = _mm_min_epu8(s0, s1);
                s1 = _mm_load_si128((__m128i *)(p_temp + stride + j));
                _mm_store_si128((__m128i *)(p_min + j), _mm_min_epu8(s0, s1));
            }
            for (; j < width; j++)
            {
                p_min[j] = ch_Min(p_temp[-stride + j], p_temp[j]);
                p_min[j] = ch_Min(p_min[j], p_temp[stride + j]);
            }
            p_temp += stride;
            p_min += stride;
        }
        //bottom boundary
        for (j = 0; j < (width & ~15); j += 16)
        {
            __m128i s0, s1;
            s0 = _mm_load_si128((__m128i *)(p_temp - stride + j));
            s1 = _mm_load_si128((__m128i *)(p_temp + j));
            _mm_store_si128((__m128i *)(p_min + j), _mm_min_epu8(s0, s1));
        }
        for (; j < width; j++)
            p_min[j] = ch_Min(p_temp[-stride + j], p_temp[j]);
    }
    else
    {
        //Part 1: horizontal filtering
        p_src = p_src_buffer;
        p_temp = p_temp_buffer;
        for (int i = 0; i < height; i++)
        {
            for (j = 1; j < width - 1; j++)
            {
                p_temp[j] = ch_Min(p_src[j - 1], p_src[j]);
                p_temp[j] = ch_Min(p_temp[j], p_src[j + 1]);
            }
            //left & right boundary
            p_temp[0] = ch_Min(p_src[0], p_src[1]);
            p_temp[width - 1] = ch_Min(p_src[width - 2], p_src[width - 1]);

            //
            p_src += stride;
            p_temp += stride;
        }


        //Part 2: vertical filtering
        p_temp = p_temp_buffer;
        p_min = p_min_buffer;

        //top boundary
        for (j = 0; j < width; j++)
            p_min[j] = ch_Min(p_temp[j], p_temp[stride + j]);
        //centre
        p_temp += stride;
        p_min += stride;
        for (int i = 1; i < height - 1; i++)
        {
            for (j = 0; j < width; j++)
            {
                p_min[j] = ch_Min(p_temp[-stride + j], p_temp[j]);
                p_min[j] = ch_Min(p_min[j], p_temp[stride + j]);
            }
            p_temp += stride;
            p_min += stride;
        }
        //bottom boundary
        for (j = 0; j < width; j++)
            p_min[j] = ch_Min(p_temp[-stride + j], p_temp[j]);
    }
#else

    const int align_right_16 = (width & ~15);
    const int align_right_32 = (width & ~31);

    // Part 1: horizontal filtering
    p_src = p_src_buffer;
    p_temp = p_temp_buffer;

    // top row
    for (j = 1; j < width - 1; j++)
    {
        p_temp[j] = ch_Min(p_src[j - 1], p_src[j]);
        p_temp[j] = ch_Min(p_temp[j], p_src[j + 1]);
    }
    p_temp[0] = ch_Min(p_src[0], p_src[1]);
    p_temp[width - 1] = ch_Min(p_src[width - 2], p_src[width - 1]);

    // center
    p_src += stride;
    p_temp += stride;
    for (int i = 1; i < height - 1; i++)
    {
        j = 0;

        for (; j < align_right_32; j += 32)
        {
            uint8x16_t s0 = vld1q_u8(p_src + j - 1);
            uint8x16_t s1 = vld1q_u8(p_src + j + 0);
            uint8x16_t s3 = vld1q_u8(p_src + j - 1 + 16);
            uint8x16_t s4 = vld1q_u8(p_src + j + 0 + 16);
            uint8x16_t s2 = vld1q_u8(p_src + j + 1);
            uint8x16_t s5 = vld1q_u8(p_src + j + 1 + 16);

            s0 = vminq_u8(s0, s1);
            s3 = vminq_u8(s3, s4);
            s0 = vminq_u8(s0, s2);
            s3 = vminq_u8(s3, s5);

            vst1q_u8(p_temp + j +  0, s0);
            vst1q_u8(p_temp + j + 16, s3);
        }

        for (; j < align_right_16; j += 16)
        {
            uint8x16_t s0 = vld1q_u8(p_src + j - 1);
            uint8x16_t s1 = vld1q_u8(p_src + j);
            uint8x16_t s2 = vld1q_u8(p_src + j + 1);

            s0 = vminq_u8(s0, s1);
            s0 = vminq_u8(s0, s2);

            vst1q_u8(p_temp + j, s0);
        }

        for (; j < width - 1; j++)
        {
            p_temp[j] = ch_Min(p_src[j - 1], p_src[j]);
            p_temp[j] = ch_Min(p_temp[j], p_src[j + 1]);
        }

        p_temp[0] = ch_Min(p_src[0], p_src[1]);
        p_temp[width - 1] = ch_Min(p_src[width - 2], p_src[width - 1]);

        //
        p_src += stride;
        p_temp += stride;
    }

    // bottom row
    for (j = 1; j < width - 1; j++)
    {
        p_temp[j] = ch_Min(p_src[j - 1], p_src[j]);
        p_temp[j] = ch_Min(p_temp[j], p_src[j + 1]);
    }
    p_temp[0] = ch_Min(p_src[0], p_src[1]);
    p_temp[width - 1] = ch_Min(p_src[width - 2], p_src[width - 1]);

    // Part 2: vertical filtering
    p_temp = p_temp_buffer;
    p_min = p_min_buffer;

    // top boundary
    for (j = 0; j < align_right_16; j += 16)
    {
        uint8x16_t s0 = vld1q_u8(p_temp + j);
        uint8x16_t s1 = vld1q_u8(p_temp + stride + j);
        s0 = vminq_u8(s0, s1);
        vst1q_u8(p_min + j, s0);
    }
    for (; j < width; j++)
    p_min[j] = ch_Min(p_temp[j], p_temp[stride + j]);

    // center
    p_temp += stride;
    p_min += stride;
    for (int i = 1; i < height - 1; i++)
    {
        j = 0;

        for (; j < align_right_32; j += 32)
        {
            uint8x16_t s0 = vld1q_u8(p_temp + j - stride);
            uint8x16_t s1 = vld1q_u8(p_temp + j);
            uint8x16_t s3 = vld1q_u8(p_temp + j + 16 - stride);
            uint8x16_t s4 = vld1q_u8(p_temp + j + 16);
            uint8x16_t s2 = vld1q_u8(p_temp + j + stride);
            uint8x16_t s5 = vld1q_u8(p_temp + j + 16 + stride);

            s0 = vminq_u8(s0, s1);
            s3 = vminq_u8(s3, s4);
            s0 = vminq_u8(s0, s2);
            s3 = vminq_u8(s3, s5);

            vst1q_u8(p_min + j, s0);
            vst1q_u8(p_min + j + 16, s3);
        }

        for (; j < align_right_16; j += 16)
        {
            uint8x16_t s0 = vld1q_u8(p_temp + j - stride);
            uint8x16_t s1 = vld1q_u8(p_temp + j);
            uint8x16_t s2 = vld1q_u8(p_temp + j + stride);

            s0 = vminq_u8(s0, s1);
            s0 = vminq_u8(s0, s2);

            vst1q_u8(p_min + j, s0);
        }

        for (; j < width; j++)
        {
            p_min[j] = ch_Min(p_temp[-stride + j], p_temp[j]);
            p_min[j] = ch_Min(p_min[j], p_temp[stride + j]);
        }
        p_temp += stride;
        p_min += stride;
    }

    // bottom boundary
    for (j = 0; j < align_right_16; j += 16)
    {
        uint8x16_t s0 = vld1q_u8(p_temp - stride + j);
        uint8x16_t s1 = vld1q_u8(p_temp + j);
        s0 = vminq_u8(s0, s1);
        vst1q_u8(p_min + j, s0);
    }
    for (; j < width; j++)
    p_min[j] = ch_Min(p_temp[-stride + j], p_temp[j]);

#endif
}

void MorphologyTool::LocalMaxFilter3x3(BYTE *p_src_buffer,
                                       BYTE *p_max_buffer,
                                       BYTE *p_temp_buffer,
                                       int   height,
                                       int   width,
                                       int   stride)
{
    _MYASSERT(p_src_buffer);
    _MYASSERT(p_max_buffer);
    _MYASSERT(p_temp_buffer);

    int j;
    BYTE *p_src, *p_temp, *p_max;

#if !defined(ANDROID_ARM) && !defined(IOS_ARM)
    if (g_is_support_SSE2)
    {
        //Part 1: horizontal filtering

        p_src = p_src_buffer;
        p_temp = p_temp_buffer;
        //top row
        for (j = 1; j < width - 1; j++)
        {
            p_temp[j] = ch_Max(p_src[j - 1], p_src[j]);
            p_temp[j] = ch_Max(p_temp[j], p_src[j + 1]);
        }
        p_temp[0] = ch_Max(p_src[0], p_src[1]);
        p_temp[width - 1] = ch_Max(p_src[width - 2], p_src[width - 1]);
        //centre
        p_src += stride;
        p_temp += stride;
        for (int i = 1; i < height - 1; i++)
        {
            for (j = 0; j < (width & ~15); j += 16)
            {
                __m128i s0, s1;
                s0 = _mm_loadu_si128((__m128i *)(p_src + j - 1));
                s1 = _mm_load_si128((__m128i *)(p_src + j));
                s0 = _mm_max_epu8(s0, s1);
                s1 = _mm_loadu_si128((__m128i *)(p_src + j + 1));
                _mm_store_si128((__m128i *)(p_temp + j), _mm_max_epu8(s0, s1));
            }
            for (; j < width - 1; j++)
            {
                p_temp[j] = ch_Max(p_src[j - 1], p_src[j]);
                p_temp[j] = ch_Max(p_temp[j], p_src[j + 1]);
            }
            p_temp[0] = ch_Max(p_src[0], p_src[1]);
            p_temp[width - 1] = ch_Max(p_src[width - 2], p_src[width - 1]);

            //
            p_src += stride;
            p_temp += stride;
        }
        //bottom row
        for (j = 1; j < width - 1; j++)
        {
            p_temp[j] = ch_Max(p_src[j - 1], p_src[j]);
            p_temp[j] = ch_Max(p_temp[j], p_src[j + 1]);
        }
        p_temp[0] = ch_Max(p_src[0], p_src[1]);
        p_temp[width - 1] = ch_Max(p_src[width - 2], p_src[width - 1]);


        //Part 2: vertical filtering

        p_temp = p_temp_buffer;
        p_max = p_max_buffer;
        //top boundary
        for (j = 0; j < (width & ~15); j += 16)
        {
            __m128i s0, s1;
            s0 = _mm_load_si128((__m128i *)(p_temp + j));
            s1 = _mm_load_si128((__m128i *)(p_temp + stride + j));
            _mm_store_si128((__m128i *)(p_max + j), _mm_max_epu8(s0, s1));
        }
        for (; j < width; j++)
            p_max[j] = ch_Max(p_temp[j], p_temp[stride + j]);
        //centre
        p_temp += stride;
        p_max += stride;
        for (int i = 1; i < height - 1; i++)
        {
            for (j = 0; j < (width & ~15); j += 16)
            {
                __m128i s0, s1;
                s0 = _mm_load_si128((__m128i *)(p_temp - stride + j));
                s1 = _mm_load_si128((__m128i *)(p_temp + j));
                s0 = _mm_max_epu8(s0, s1);
                s1 = _mm_load_si128((__m128i *)(p_temp + stride + j));
                _mm_store_si128((__m128i *)(p_max + j), _mm_max_epu8(s0, s1));
            }
            for (; j < width; j++)
            {
                p_max[j] = ch_Max(p_temp[-stride + j], p_temp[j]);
                p_max[j] = ch_Max(p_max[j], p_temp[stride + j]);
            }
            p_temp += stride;
            p_max += stride;
        }
        //bottom boundary
        for (j = 0; j < (width & ~15); j += 16)
        {
            __m128i s0, s1;
            s0 = _mm_load_si128((__m128i *)(p_temp - stride + j));
            s1 = _mm_load_si128((__m128i *)(p_temp + j));
            _mm_store_si128((__m128i *)(p_max + j), _mm_max_epu8(s0, s1));
        }
        for (; j < width; j++)
            p_max[j] = ch_Max(p_temp[-stride + j], p_temp[j]);
    }
    else
    {
        //Part 1: horizontal filtering
        p_src = p_src_buffer;
        p_temp = p_temp_buffer;
        for (int i = 0; i < height; i++)
        {
            for (j = 1; j < width - 1; j++)
            {
                p_temp[j] = ch_Max(p_src[j - 1], p_src[j]);
                p_temp[j] = ch_Max(p_temp[j], p_src[j + 1]);
            }
            //left & right boundary
            p_temp[0] = ch_Max(p_src[0], p_src[1]);
            p_temp[width - 1] = ch_Max(p_src[width - 2], p_src[width - 1]);

            //
            p_src += stride;
            p_temp += stride;
        }


        //Part 2: vertical filtering
        p_temp = p_temp_buffer;
        p_max = p_max_buffer;

        //top boundary
        for (j = 0; j < width; j++)
            p_max[j] = ch_Max(p_temp[j], p_temp[stride + j]);
        //centre
        p_temp += stride;
        p_max += stride;
        for (int i = 1; i < height - 1; i++)
        {
            for (j = 0; j < width; j++)
            {
                p_max[j] = ch_Max(p_temp[-stride + j], p_temp[j]);
                p_max[j] = ch_Max(p_max[j], p_temp[stride + j]);
            }
            p_temp += stride;
            p_max += stride;
        }
        //bottom boundary
        for (j = 0; j < width; j++)
            p_max[j] = ch_Max(p_temp[-stride + j], p_temp[j]);
    }
#else

    const int align_right_16 = (width & ~15);
    const int align_right_32 = (width & ~31);

    // Part 1: horizontal filtering
    p_src = p_src_buffer;
    p_temp = p_temp_buffer;

    // top row
    for (j = 1; j < width - 1; j++)
    {
        p_temp[j] = ch_Max(p_src[j - 1], p_src[j]);
        p_temp[j] = ch_Max(p_temp[j], p_src[j + 1]);
    }
    p_temp[0] = ch_Max(p_src[0], p_src[1]);
    p_temp[width - 1] = ch_Max(p_src[width - 2], p_src[width - 1]);

    // center
    p_src += stride;
    p_temp += stride;
    for (int i = 1; i < height - 1; i++)
    {
        j = 0;

        for (; j < align_right_32; j += 32)
        {
            uint8x16_t s0 = vld1q_u8(p_src + j - 1);
            uint8x16_t s1 = vld1q_u8(p_src + j + 0);
            uint8x16_t s3 = vld1q_u8(p_src + j - 1 + 16);
            uint8x16_t s4 = vld1q_u8(p_src + j + 0 + 16);
            uint8x16_t s2 = vld1q_u8(p_src + j + 1);
            uint8x16_t s5 = vld1q_u8(p_src + j + 1 + 16);

            s0 = vmaxq_u8(s0, s1);
            s3 = vmaxq_u8(s3, s4);
            s0 = vmaxq_u8(s0, s2);
            s3 = vmaxq_u8(s3, s5);

            vst1q_u8(p_temp + j +  0, s0);
            vst1q_u8(p_temp + j + 16, s3);
        }

        for (; j < align_right_16; j += 16)
        {
            uint8x16_t s0 = vld1q_u8(p_src + j - 1);
            uint8x16_t s1 = vld1q_u8(p_src + j);
            uint8x16_t s2 = vld1q_u8(p_src + j + 1);

            s0 = vmaxq_u8(s0, s1);
            s0 = vmaxq_u8(s0, s2);

            vst1q_u8(p_temp + j, s0);
        }

        for (; j < width - 1; j++)
        {
            p_temp[j] = ch_Max(p_src[j - 1], p_src[j]);
            p_temp[j] = ch_Max(p_temp[j], p_src[j + 1]);
        }

        p_temp[0] = ch_Max(p_src[0], p_src[1]);
        p_temp[width - 1] = ch_Max(p_src[width - 2], p_src[width - 1]);

        //
        p_src += stride;
        p_temp += stride;
    }

    // bottom row
    for (j = 1; j < width - 1; j++)
    {
        p_temp[j] = ch_Max(p_src[j - 1], p_src[j]);
        p_temp[j] = ch_Max(p_temp[j], p_src[j + 1]);
    }
    p_temp[0] = ch_Max(p_src[0], p_src[1]);
    p_temp[width - 1] = ch_Max(p_src[width - 2], p_src[width - 1]);

    // Part 2: vertical filtering
    p_temp = p_temp_buffer;
    p_max = p_max_buffer;

    // top boundary
    for (j = 0; j < align_right_16; j += 16)
    {
        uint8x16_t s0 = vld1q_u8(p_temp + j);
        uint8x16_t s1 = vld1q_u8(p_temp + stride + j);
        s0 = vmaxq_u8(s0, s1);
        vst1q_u8(p_max + j, s0);
    }
    for (; j < width; j++)
    p_max[j] = ch_Max(p_temp[j], p_temp[stride + j]);

    // center
    p_temp += stride;
    p_max += stride;
    for (int i = 1; i < height - 1; i++)
    {
        j = 0;

        for (; j < align_right_32; j += 32)
        {
            uint8x16_t s0 = vld1q_u8(p_temp + j - stride);
            uint8x16_t s1 = vld1q_u8(p_temp + j);
            uint8x16_t s3 = vld1q_u8(p_temp + j + 16 - stride);
            uint8x16_t s4 = vld1q_u8(p_temp + j + 16);
            uint8x16_t s2 = vld1q_u8(p_temp + j + stride);
            uint8x16_t s5 = vld1q_u8(p_temp + j + 16 + stride);

            s0 = vmaxq_u8(s0, s1);
            s3 = vmaxq_u8(s3, s4);
            s0 = vmaxq_u8(s0, s2);
            s3 = vmaxq_u8(s3, s5);

            vst1q_u8(p_max + j, s0);
            vst1q_u8(p_max + j + 16, s3);
        }

        for (; j < align_right_16; j += 16)
        {
            uint8x16_t s0 = vld1q_u8(p_temp + j - stride);
            uint8x16_t s1 = vld1q_u8(p_temp + j);
            uint8x16_t s2 = vld1q_u8(p_temp + j + stride);

            s0 = vmaxq_u8(s0, s1);
            s0 = vmaxq_u8(s0, s2);

            vst1q_u8(p_max + j, s0);
        }

        for (; j < width; j++)
        {
            p_max[j] = ch_Max(p_temp[-stride + j], p_temp[j]);
            p_max[j] = ch_Max(p_max[j], p_temp[stride + j]);
        }
        p_temp += stride;
        p_max += stride;
    }

    // bottom boundary
    for (j = 0; j < align_right_16; j += 16)
    {
        uint8x16_t s0 = vld1q_u8(p_temp - stride + j);
        uint8x16_t s1 = vld1q_u8(p_temp + j);
        s0 = vmaxq_u8(s0, s1);
        vst1q_u8(p_max + j, s0);
    }
    for (; j < width; j++)
    p_max[j] = ch_Max(p_temp[-stride + j], p_temp[j]);

#endif
}

void MorphologyTool::FillHole(BYTE *p_src_dst, int src_stride, int max_iteration)
{
    _MYASSERT(p_src_dst);
    _MYASSERT(mp_left_bound);
    _MYASSERT(mp_right_bound);
    _MYASSERT(mp_top_bound);
    _MYASSERT(mp_bottom_bound);
    _MYASSERT((0 == (m_align_width & 15)) && (0 == (m_align_height & 15)));
    
    InitializeMacroBlock(m_align_width, m_align_height);

    BYTE *p_align_buffer = GetAlignBuffer(p_src_dst, src_stride);

    int width = m_width;
    int height = m_height;
    int align_width = m_align_width;
    int align_height = m_align_height;
    int stride = m_align_width;

    BYTE *p_water_valume = NULL;
    BYTE *p_prev_water_valume = NULL;
    _ALIGNED_MALLOC_PTR(p_water_valume, BYTE, align_width * align_height);
    _ALIGNED_MALLOC_PTR(p_prev_water_valume, BYTE, align_width * align_height);

#if FILLWATER_DEBUG == 1
    IplImage *src_image = cvCreateImageHeader(cvSize(width, height), 8, 1);
    IplImage *water_image = cvCreateImageHeader(cvSize(width, height), 8, 1);
    cvSetData(src_image, p_align_buffer, stride);
    cvSetData(water_image, p_water_valume, stride);
#endif

    // pad depth for block base
    // pad depth right
    BYTE *p_src_scan = p_align_buffer;
    for (int i = 0; i < height; i++)
    {
        memset(p_src_scan + width, p_src_scan[width - 1], align_width - width);
        p_src_scan += stride;
    }
    p_src_scan = p_align_buffer + height * stride;
    // pad depth bottom
    for (int i = height; i < align_height; i++)
    {
        memcpy(p_src_scan, p_src_scan - stride, align_width);
        p_src_scan += stride;
    }

    BYTE *p_water = p_water_valume;
    BYTE *p_prev_water = p_prev_water_valume;
    memset(p_water, 255, align_width * align_height);

    bool diff = true;
    int diff_line = 0;
    BYTE *p_water_scan;
    for (int iter = 0; iter < max_iteration && diff; iter++)
    {
        diff = false;
        memcpy(p_prev_water + diff_line * align_width, p_water + diff_line * align_width, align_width * (align_height - diff_line));

        const int BLOCK_SIZE = 16;

        // set bound value
        BYTE *p_left = mp_left_bound;
        BYTE *p_right = mp_right_bound;
        BYTE *p_top = mp_top_bound;
        BYTE *p_bottom = mp_bottom_bound;
        for (int i = 0; i < align_height; i++)
            p_left[i] = p_align_buffer[i * stride];
        for (int i = 0; i < align_height; i++)
            p_right[i] = p_align_buffer[i * stride + align_width - 1];
        memcpy(p_top, p_align_buffer, align_width);
        memcpy(p_bottom, p_align_buffer + (align_height - 1) * stride, align_width);

        // fill top and left
        ResetMacroBlockEvent();

        m_stage = FILL_HOLE_TOP_LEFT;
        for (int i = 0; i < m_num_thread; i++)
        {
            mp_thread_param[i].align_width = align_width;
            mp_thread_param[i].align_height = align_height;
            mp_thread_param[i].block_size = BLOCK_SIZE;
            mp_thread_param[i].p_align_buffer = p_align_buffer;
            mp_thread_param[i].p_water = p_water;
            mp_thread_param[i].p_left = p_left;
            mp_thread_param[i].p_top = p_top;
            mp_thread_param[i].p_block_event = mp_macroblock_event;
            mp_thread_control[i].SignalBegin();
        }
        for (int i = 0; i < m_num_thread; i++)
        {
            mp_thread_control[i].WaitComplete();
        }

        // fill bottom and right
        ResetMacroBlockEvent();

        m_stage = FILL_HOLE_BOTTOM_RIGHT;
        for (int i = 0; i < m_num_thread; i++)
        {
            mp_thread_param[i].align_width = align_width;
            mp_thread_param[i].align_height = align_height;
            mp_thread_param[i].block_size = BLOCK_SIZE;
            mp_thread_param[i].p_align_buffer = p_align_buffer;
            mp_thread_param[i].p_water = p_water;
            mp_thread_param[i].p_right = p_right;
            mp_thread_param[i].p_bottom = p_bottom;
            mp_thread_param[i].p_block_event = mp_macroblock_event;
            mp_thread_control[i].SignalBegin();
        }
        for (int i = 0; i < m_num_thread; i++)
        {
            mp_thread_control[i].WaitComplete();
        }

        p_water_scan = p_water;
        BYTE *p_prev_water_scan = p_prev_water;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (p_water_scan[j] != p_prev_water_scan[j])
                {
                    diff = true;
                    break;
                }
            }
            if (diff)
            {
                diff_line = i;
                break;
            }
            p_water_scan += stride;
            p_prev_water_scan += stride;
        }
    }
    memcpy(p_align_buffer, p_water, m_align_width * m_align_height);

#if FILLWATER_DEBUG == 1
    cvShowImage("src", src_image);
    cvShowImage("water", water_image);
    cvWaitKey(1);
    _DPRINTF((_T("iter %d"), iter));
    cvReleaseImageHeader(&src_image);
    cvReleaseImageHeader(&water_image);
#endif

    _ALIGNED_FREE_PTR(p_prev_water_valume);
    _ALIGNED_FREE_PTR(p_water_valume);

    FreeAndCopyResult(p_src_dst, src_stride);
}

void MorphologyTool::FillHole16x16TopLeft(BYTE *p_src, BYTE *p_water, BYTE *p_top, BYTE *p_left)
{
    _MYASSERT(p_src);
    _MYASSERT(p_water);
    _MYASSERT(p_top);
    _MYASSERT(p_left);

    int stride = m_align_width;

#if !defined(ANDROID_ARM) && !defined(IOS_ARM)
    if (g_is_support_SSE2)
    {
        BYTE *pp_src[16];
        BYTE *pp_water[16];

        for (int i = 0; i < 16; i++)
        {
            pp_src[i] = p_src + i * stride;
            pp_water[i] = p_water + i * stride;
        }

        __m128i v[16], d[16], s[16], t[16];
        __m128i top, left;

        top = _mm_load_si128((__m128i *)p_top);

        _mm_load16x16_si128(pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                            pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                            pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                            pp_water[12], pp_water[13], pp_water[14], pp_water[15],
                            v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                            v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
        _mm_load16x16_si128(pp_src[0], pp_src[1], pp_src[2], pp_src[3],
                            pp_src[4], pp_src[5], pp_src[6], pp_src[7],
                            pp_src[8], pp_src[9], pp_src[10], pp_src[11],
                            pp_src[12], pp_src[13], pp_src[14], pp_src[15],
                            s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                            s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);

        __m128i *pre_v = &top;
        for (int i = 0; i < 16; i++)
        {
            v[i] = _mm_max_epu8(_mm_min_epu8(v[i], pre_v[0]), s[i]);
            pre_v = v + i;
        }
        _mm_store_si128((__m128i *)(p_top), pre_v[0]);
        _mm_store16x16_si128(pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                            pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                            pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                            pp_water[12], pp_water[13], pp_water[14], pp_water[15],
                            v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                            v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);

        _mm_transpose16x16_epi8(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                            v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15],
                            d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7],
                            d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
        _mm_transpose16x16_epi8(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                            s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15],
                            t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
                            t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15]);

        left = _mm_load_si128((__m128i *)p_left);

        __m128i *pre_d = &left;
        for (int i = 0; i < 16; i++)
        {
            d[i] = _mm_max_epu8(_mm_min_epu8(d[i], pre_d[0]), t[i]);
            pre_d = d + i;
        }
        _mm_store_si128((__m128i *)(p_left), pre_d[0]);

        _mm_transpose16x16_epi8(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7],
                                d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15],
                                v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                                v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);

        _mm_store16x16_si128(pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                             pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                             pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                             pp_water[12], pp_water[13], pp_water[14], pp_water[15],
                             v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                             v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
    }
    else
#endif
    {
#if (FILLHOLE_C_FAST == 1)

        BYTE *p_src_scan = p_src;
        BYTE *p_water_scan = p_water;

        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                p_water_scan[j] = ch_Max(ch_Min(p_water_scan[j], ch_Min(p_top[j], p_left[i])), p_src_scan[j]);
                p_top[j] = p_left[i] = p_water_scan[j];
            }
            p_src_scan += stride;
            p_water_scan += stride;
        }

#else

        BYTE *p_src_scan = p_src;
        BYTE *p_water_scan = p_water;
        BYTE *p_pre_water_scan = p_top;

        // top
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                p_water_scan[j] = ch_Max(ch_Min(p_water_scan[j], p_pre_water_scan[j]), p_src_scan[j]);
            }
            p_pre_water_scan = p_water_scan;
            p_src_scan += stride;
            p_water_scan += stride;
        }
        memcpy(p_top, p_pre_water_scan, 16);

        // left
        p_src_scan = p_src;
        p_water_scan = p_water;
        for (int i = 0; i < 16; i++)
        {
            p_pre_water_scan = p_left + i;
            for (int j = 0; j < 16; j++)
            {
                p_water_scan[j] = ch_Max(ch_Min(p_water_scan[j], p_pre_water_scan[0]), p_src_scan[j]);
                p_pre_water_scan = p_water_scan + j;
            }
            p_left[i] = p_pre_water_scan[0];
            p_src_scan += stride;
            p_water_scan += stride;
        }
#endif
    }
}

void MorphologyTool::FillHole16x16BottomRight(BYTE *p_src, BYTE *p_water, BYTE *p_bottom, BYTE *p_right)
{
    _MYASSERT(p_src);
    _MYASSERT(p_water);
    _MYASSERT(p_bottom);
    _MYASSERT(p_right);

    int stride = m_align_width;

#if !defined(ANDROID_ARM) && !defined(IOS_ARM)
    if (g_is_support_SSE2)
    {
        BYTE *pp_src[16];
        BYTE *pp_water[16];

        for (int i = 0; i < 16; i++)
        {
            pp_src[i] = p_src + i * stride;
            pp_water[i] = p_water + i * stride;
        }

        __m128i v[16], d[16], s[16], t[16];
        __m128i bottom, right;

        bottom = _mm_load_si128((__m128i *)p_bottom);

        _mm_load16x16_si128(pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                            pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                            pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                            pp_water[12], pp_water[13], pp_water[14], pp_water[15],
                            v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                            v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
        _mm_load16x16_si128(pp_src[0], pp_src[1], pp_src[2], pp_src[3],
                            pp_src[4], pp_src[5], pp_src[6], pp_src[7],
                            pp_src[8], pp_src[9], pp_src[10], pp_src[11],
                            pp_src[12], pp_src[13], pp_src[14], pp_src[15],
                            s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                            s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);

        __m128i *pre_v = &bottom;
        for (int i = 15; i >= 0; i--)
        {
            v[i] = _mm_max_epu8(_mm_min_epu8(v[i], pre_v[0]), s[i]);
            pre_v = v + i;
        }
        _mm_store_si128((__m128i *)(p_bottom), pre_v[0]);
        _mm_store16x16_si128(pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                             pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                             pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                             pp_water[12], pp_water[13], pp_water[14], pp_water[15],
                             v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                             v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);

        _mm_transpose16x16_epi8(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                                v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15],
                                d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7],
                                d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
        _mm_transpose16x16_epi8(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                                s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15],
                                t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
                                t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15]);

        right = _mm_load_si128((__m128i *)p_right);

        __m128i *pre_d = &right;
        for (int i = 15; i >= 0; i--)
        {
            d[i] = _mm_max_epu8(_mm_min_epu8(d[i], pre_d[0]), t[i]);
            pre_d = d + i;
        }
        _mm_store_si128((__m128i *)(p_right), pre_d[0]);

        _mm_transpose16x16_epi8(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7],
                                d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15],
                                v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                                v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);

        _mm_store16x16_si128(pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                             pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                             pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                             pp_water[12], pp_water[13], pp_water[14], pp_water[15],
                             v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                             v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
    }
    else
#endif
    {
#if (FILLHOLE_C_FAST == 1)

        BYTE *p_src_scan = p_src + 15 * stride;
        BYTE *p_water_scan = p_water + 15 * stride;

        for (int i = 15; i >= 0; i--)
        {
            for (int j = 15; j >= 0; j--)
            {
                p_water_scan[j] = ch_Max(ch_Min(p_water_scan[j], ch_Min(p_bottom[j], p_right[i])), p_src_scan[j]);
                p_bottom[j] = p_right[i] = p_water_scan[j];
            }
            p_src_scan -= stride;
            p_water_scan -= stride;
        }

#else

        BYTE *p_src_scan = p_src + 15 * stride;
        BYTE *p_water_scan = p_water + 15 * stride;
        BYTE *p_pre_water_scan = p_bottom;
        // bottom
        for (int i = 15; i >= 0; i--)
        {
            for (int j = 0; j < 16; j++)
            {
                p_water_scan[j] = ch_Max(ch_Min(p_water_scan[j], p_pre_water_scan[j]), p_src_scan[j]);
            }
            p_pre_water_scan = p_water_scan;
            p_src_scan -= stride;
            p_water_scan -= stride;
        }
        memcpy(p_bottom, p_pre_water_scan, 16);

        // right
        p_src_scan = p_src;
        p_water_scan = p_water;
        for (int i = 0; i < 16; i++)
        {
            p_pre_water_scan = p_right + i;
            for (int j = 15; j >= 0; j--)
            {
                p_water_scan[j] = ch_Max(ch_Min(p_water_scan[j], p_pre_water_scan[0]), p_src_scan[j]);
                p_pre_water_scan = p_water_scan + j;
            }
            p_right[i] = p_pre_water_scan[0];
            p_src_scan += stride;
            p_water_scan += stride;
        }
#endif
    }
}

void MorphologyTool::FillHoleBinary(BYTE *p_src_dst, int src_stride, int max_iteration)
{
    _MYASSERT(p_src_dst);
    _MYASSERT(mp_left_bound);
    _MYASSERT(mp_right_bound);
    _MYASSERT(mp_top_bound);
    _MYASSERT(mp_bottom_bound);
    _MYASSERT((0 == (m_align_width & 15)) && (0 == (m_align_height & 15)));

    BYTE *p_align_buffer = GetAlignBuffer(p_src_dst, src_stride);

    int width = m_width;
    int height = m_height;
    int align_width = m_align_width;
    int align_height = m_align_height;
    int stride = m_align_width;

    BYTE *p_water_valume = NULL;
    BYTE *p_prev_water_valume = NULL;
    _ALIGNED_MALLOC_PTR(p_water_valume, BYTE, align_width * align_height);
    _ALIGNED_MALLOC_PTR(p_prev_water_valume, BYTE, align_width * align_height);

#if FILLWATER_DEBUG == 1
    IplImage *src_image = cvCreateImageHeader(cvSize(width, height), 8, 1);
    IplImage *water_image = cvCreateImageHeader(cvSize(width, height), 8, 1);
    cvSetData(src_image, p_align_buffer, stride);
    cvSetData(water_image, p_water_valume, stride);
#endif

    // pad depth for block base
    // pad depth right
    BYTE *p_src_scan = p_align_buffer;
    for (int i = 0; i < height; i++)
    {
        memset(p_src_scan + width, p_src_scan[width - 1], align_width - width);
        p_src_scan += stride;
    }
    p_src_scan = p_align_buffer + height * stride;
    // pad depth bottom
    for (int i = height; i < align_height; i++)
    {
        memcpy(p_src_scan, p_src_scan - stride, align_width);
        p_src_scan += stride;
    }

    BYTE *p_water = p_water_valume;
    BYTE *p_prev_water = p_prev_water_valume;
    memset(p_water, 255, align_width * align_height);

    bool diff = true;
    BYTE *p_water_scan;
    for (int iter = 0; iter < max_iteration && diff; iter++)
    {
        diff = false;
        memcpy(p_prev_water, p_water, align_width * align_height);

        const int BLOCK_SIZE = 16;
        const int block_stride = BLOCK_SIZE * stride;

        // set bound value
        BYTE *p_left = mp_left_bound;
        BYTE *p_right = mp_right_bound;
        BYTE *p_top = mp_top_bound;
        BYTE *p_bottom = mp_bottom_bound;
        for (int i = 0; i < align_height; i++)
            p_left[i] = p_align_buffer[i * stride];
        for (int i = 0; i < align_height; i++)
            p_right[i] = p_align_buffer[i * stride + align_width - 1];
        memcpy(p_top, p_align_buffer, align_width);
        memcpy(p_bottom, p_align_buffer + (align_height - 1) * stride, align_width);

        // fill top and left
        p_water_scan = p_water;
        p_src_scan = p_align_buffer;
        for (int i = 0; i < align_height; i += BLOCK_SIZE)
        {
            for (int j = 0; j < align_width; j += BLOCK_SIZE)
            {
                FillHoleBinary16x16TopLeft(p_src_scan + j, p_water_scan + j, p_top + j, p_left + i);
            }
            p_water_scan += block_stride;
            p_src_scan += block_stride;
        }

        // fill bottom and right
        p_water_scan = p_water + (align_height - BLOCK_SIZE) * stride;
        p_src_scan = p_align_buffer + (align_height - BLOCK_SIZE) * stride;

        for (int i = align_height - BLOCK_SIZE; i >= 0 ; i -= BLOCK_SIZE)
        {
            for (int j = align_width - BLOCK_SIZE; j >= 0 ; j -= BLOCK_SIZE)
            {
                FillHoleBinary16x16BottomRight(p_src_scan + j, p_water_scan + j, p_bottom + j, p_right + i);
            }
            p_water_scan -= block_stride;
            p_src_scan -= block_stride;
        }

        p_water_scan = p_water;
        BYTE *p_prev_water_scan = p_prev_water;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (p_water_scan[j] != p_prev_water_scan[j])
                {
                    diff = true;
                    break;
                }
            }
            if (diff) break;
            p_water_scan += stride;
            p_prev_water_scan += stride;
        }
    }
    memcpy(p_align_buffer, p_water, m_align_width * m_align_height);

#if FILLWATER_DEBUG == 1
    cvShowImage("src", src_image);
    cvShowImage("water", water_image);
    cvWaitKey(1);
    _DPRINTF((_T("iter %d"), iter));
    cvReleaseImageHeader(&src_image);
    cvReleaseImageHeader(&water_image);
#endif

    _ALIGNED_FREE_PTR(p_prev_water_valume);
    _ALIGNED_FREE_PTR(p_water_valume);

    FreeAndCopyResult(p_src_dst, src_stride);
}

void MorphologyTool::FillHoleBinary16x16TopLeft(BYTE *p_src, BYTE *p_water, BYTE *p_top, BYTE *p_left)
{
    _MYASSERT(p_src);
    _MYASSERT(p_water);
    _MYASSERT(p_top);
    _MYASSERT(p_left);

    int stride = m_align_width;

#if !defined(ANDROID_ARM) && !defined(IOS_ARM)
    if (g_is_support_SSE2)
    {
        BYTE *pp_src[16];
        BYTE *pp_water[16];

        for (int i = 0; i < 16; i++)
        {
            pp_src[i] = p_src + i * stride;
            pp_water[i] = p_water + i * stride;
        }

        __m128i v[16], d[16], s[16], t[16];
        __m128i top, left;

        top = _mm_load_si128((__m128i *)p_top);

        _mm_load16x16_si128(pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                            pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                            pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                            pp_water[12], pp_water[13], pp_water[14], pp_water[15],
                            v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                            v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
        _mm_load16x16_si128(pp_src[0], pp_src[1], pp_src[2], pp_src[3],
                            pp_src[4], pp_src[5], pp_src[6], pp_src[7],
                            pp_src[8], pp_src[9], pp_src[10], pp_src[11],
                            pp_src[12], pp_src[13], pp_src[14], pp_src[15],
                            s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                            s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);

        __m128i *pre_v = &top;
        for (int i = 0; i < 16; i++)
        {
            v[i] = _mm_or_si128(_mm_and_si128(v[i], pre_v[0]), s[i]);
            pre_v = v + i;
        }
        _mm_store_si128((__m128i *)(p_top), pre_v[0]);
        _mm_store16x16_si128(pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                             pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                             pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                             pp_water[12], pp_water[13], pp_water[14], pp_water[15],
                             v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                             v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);

        _mm_transpose16x16_epi8(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                                v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15],
                                d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7],
                                d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
        _mm_transpose16x16_epi8(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                                s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15],
                                t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
                                t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15]);

        left = _mm_load_si128((__m128i *)p_left);

        __m128i *pre_d = &left;
        for (int i = 0; i < 16; i++)
        {
            d[i] = _mm_or_si128(_mm_and_si128(d[i], pre_d[0]), t[i]);
            pre_d = d + i;
        }
        _mm_store_si128((__m128i *)(p_left), pre_d[0]);

        _mm_transpose16x16_epi8(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7],
                                d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15],
                                v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                                v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);

        _mm_store16x16_si128(pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                             pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                             pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                             pp_water[12], pp_water[13], pp_water[14], pp_water[15],
                             v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                             v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
    }
    else
#else
    if (g_is_support_NEON)
    {
        BYTE *pp_src[16];
        BYTE *pp_water[16];

        for (int i = 0; i < 16; i++)
        {
            pp_src[i] = p_src + i * stride;
            pp_water[i] = p_water + i * stride;
        }

        uint8x16_t v[16], d[16], s[16], t[16];
        uint8x16_t top, left;

        top = vld1q_u8(p_top);

        load16x16_u8(pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                     pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                     pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                     pp_water[12], pp_water[13], pp_water[14], pp_water[15],
                     v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                     v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
        load16x16_u8(pp_src[0], pp_src[1], pp_src[2], pp_src[3],
                     pp_src[4], pp_src[5], pp_src[6], pp_src[7],
                     pp_src[8], pp_src[9], pp_src[10], pp_src[11],
                     pp_src[12], pp_src[13], pp_src[14], pp_src[15],
                     s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                     s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);

        uint8x16_t *pre_v = &top;
        for (int i = 0; i < 16; i++)
        {
            v[i] = vorrq_u8(vandq_u8(v[i], pre_v[0]), s[i]);
            pre_v = v + i;
        }
        vst1q_u8(p_top, pre_v[0]);
        store16x16_u8(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                      v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15],
                      pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                      pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                      pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                      pp_water[12], pp_water[13], pp_water[14], pp_water[15]);

        transpose16x16_u8(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                          v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15],
                          d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7],
                          d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
        transpose16x16_u8(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                          s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15],
                          t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
                          t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15]);

        left = vld1q_u8(p_left);

        uint8x16_t *pre_d = &left;
        for (int i = 0; i < 16; i++)
        {
            d[i] = vorrq_u8(vandq_u8(d[i], pre_d[0]), t[i]);
            pre_d = d + i;
        }
        vst1q_u8(p_left, pre_d[0]);

        transpose16x16_u8(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7],
                          d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15],
                          v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                          v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);

        store16x16_u8(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                      v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15],
                      pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                      pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                      pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                      pp_water[12], pp_water[13], pp_water[14], pp_water[15]);
    }
    else
#endif
    {
        BYTE *p_src_scan = p_src;
        BYTE *p_water_scan = p_water;
        BYTE *p_pre_water_scan = p_top;

        // top
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                p_water_scan[j] = (p_water_scan[j] & p_pre_water_scan[j]) | p_src_scan[j];
            }
            p_pre_water_scan = p_water_scan;
            p_src_scan += stride;
            p_water_scan += stride;
        }
        memcpy(p_top, p_pre_water_scan, 16);

        // left
        p_src_scan = p_src;
        p_water_scan = p_water;
        for (int i = 0; i < 16; i++)
        {
            p_pre_water_scan = p_left + i;
            for (int j = 0; j < 16; j++)
            {
                p_water_scan[j] = (p_water_scan[j] & p_pre_water_scan[0]) | p_src_scan[j];
                p_pre_water_scan = p_water_scan + j;
            }
            p_left[i] = p_pre_water_scan[0];
            p_src_scan += stride;
            p_water_scan += stride;
        }
    }
}

void MorphologyTool::FillHoleBinary16x16BottomRight(BYTE *p_src, BYTE *p_water, BYTE *p_bottom, BYTE *p_right)
{
    _MYASSERT(p_src);
    _MYASSERT(p_water);
    _MYASSERT(p_bottom);
    _MYASSERT(p_right);

    int stride = m_align_width;

#if !defined(ANDROID_ARM) && !defined(IOS_ARM)
    if (g_is_support_SSE2)
    {
        BYTE *pp_src[16];
        BYTE *pp_water[16];

        for (int i = 0; i < 16; i++)
        {
            pp_src[i] = p_src + i * stride;
            pp_water[i] = p_water + i * stride;
        }

        __m128i v[16], d[16], s[16], t[16];
        __m128i bottom, right;

        bottom = _mm_load_si128((__m128i *)p_bottom);

        _mm_load16x16_si128(pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                            pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                            pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                            pp_water[12], pp_water[13], pp_water[14], pp_water[15],
                            v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                            v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
        _mm_load16x16_si128(pp_src[0], pp_src[1], pp_src[2], pp_src[3],
                            pp_src[4], pp_src[5], pp_src[6], pp_src[7],
                            pp_src[8], pp_src[9], pp_src[10], pp_src[11],
                            pp_src[12], pp_src[13], pp_src[14], pp_src[15],
                            s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                            s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);

        __m128i *pre_v = &bottom;
        for (int i = 15; i >= 0; i--)
        {
            v[i] = _mm_or_si128(_mm_and_si128(v[i], pre_v[0]), s[i]);
            pre_v = v + i;
        }
        _mm_store_si128((__m128i *)(p_bottom), pre_v[0]);
        _mm_store16x16_si128(pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                             pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                             pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                             pp_water[12], pp_water[13], pp_water[14], pp_water[15],
                             v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                             v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);

        _mm_transpose16x16_epi8(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                                v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15],
                                d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7],
                                d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
        _mm_transpose16x16_epi8(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                                s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15],
                                t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
                                t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15]);

        right = _mm_load_si128((__m128i *)p_right);

        __m128i *pre_d = &right;
        for (int i = 15; i >= 0; i--)
        {
            d[i] = _mm_or_si128(_mm_and_si128(d[i], pre_d[0]), t[i]);
            pre_d = d + i;
        }
        _mm_store_si128((__m128i *)(p_right), pre_d[0]);

        _mm_transpose16x16_epi8(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7],
                                d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15],
                                v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                                v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);

        _mm_store16x16_si128(pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                             pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                             pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                             pp_water[12], pp_water[13], pp_water[14], pp_water[15],
                             v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                             v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
    }
    else
#else
    if (g_is_support_NEON)
    {
        BYTE *pp_src[16];
        BYTE *pp_water[16];

        for (int i = 0; i < 16; i++)
        {
            pp_src[i] = p_src + i * stride;
            pp_water[i] = p_water + i * stride;
        }

        uint8x16_t v[16], d[16], s[16], t[16];
        uint8x16_t bottom, right;

        bottom = vld1q_u8(p_bottom);

        load16x16_u8(pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                     pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                     pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                     pp_water[12], pp_water[13], pp_water[14], pp_water[15],
                     v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                     v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
        load16x16_u8(pp_src[0], pp_src[1], pp_src[2], pp_src[3],
                     pp_src[4], pp_src[5], pp_src[6], pp_src[7],
                     pp_src[8], pp_src[9], pp_src[10], pp_src[11],
                     pp_src[12], pp_src[13], pp_src[14], pp_src[15],
                     s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                     s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);

        uint8x16_t *pre_v = &bottom;
        for (int i = 15; i >= 0; i--)
        {
            v[i] = vorrq_u8(vandq_u8(v[i], pre_v[0]), s[i]);
            pre_v = v + i;
        }
        vst1q_u8(p_bottom, pre_v[0]);
        store16x16_u8(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                      v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15],
                      pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                      pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                      pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                      pp_water[12], pp_water[13], pp_water[14], pp_water[15]);

        transpose16x16_u8(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                          v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15],
                          d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7],
                          d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
        transpose16x16_u8(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                          s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15],
                          t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
                          t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15]);

        right = vld1q_u8(p_right);

        uint8x16_t *pre_d = &right;
        for (int i = 15; i >= 0; i--)
        {
            d[i] = vorrq_u8(vandq_u8(d[i], pre_d[0]), t[i]);
            pre_d = d + i;
        }
        vst1q_u8(p_right, pre_d[0]);

        transpose16x16_u8(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7],
                          d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15],
                          v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                          v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);

        store16x16_u8(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                      v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15],
                      pp_water[0], pp_water[1], pp_water[2], pp_water[3],
                      pp_water[4], pp_water[5], pp_water[6], pp_water[7],
                      pp_water[8], pp_water[9], pp_water[10], pp_water[11],
                      pp_water[12], pp_water[13], pp_water[14], pp_water[15]);
    }
    else
#endif
    {
        BYTE *p_src_scan = p_src + 15 * stride;
        BYTE *p_water_scan = p_water + 15 * stride;
        BYTE *p_pre_water_scan = p_bottom;
        // bottom
        for (int i = 15; i >= 0; i--)
        {
            for (int j = 0; j < 16; j++)
            {
                p_water_scan[j] = (p_water_scan[j] & p_pre_water_scan[j]) | p_src_scan[j];
            }
            p_pre_water_scan = p_water_scan;
            p_src_scan -= stride;
            p_water_scan -= stride;
        }
        memcpy(p_bottom, p_pre_water_scan, 16);

        // right
        p_src_scan = p_src;
        p_water_scan = p_water;
        for (int i = 0; i < 16; i++)
        {
            p_pre_water_scan = p_right + i;
            for (int j = 15; j >= 0; j--)
            {
                p_water_scan[j] = (p_water_scan[j] & p_pre_water_scan[0]) | p_src_scan[j];
                p_pre_water_scan = p_water_scan + j;
            }
            p_right[i] = p_pre_water_scan[0];
            p_src_scan += stride;
            p_water_scan += stride;
        }
    }
}

void MorphologyTool::BoundGradient(BYTE *p_src_dst, int src_stride, int bound_value)
{
    _MYASSERT(p_src_dst);
    _MYASSERT(mp_left_bound);
    _MYASSERT(mp_right_bound);
    _MYASSERT(mp_top_bound);
    _MYASSERT(mp_bottom_bound);
    _MYASSERT((0 == (m_align_width & 15)) && (0 == (m_align_height & 15)));
    
    InitializeMacroBlock(m_align_width, m_align_height);

    BYTE *p_align_buffer = GetAlignBuffer(p_src_dst, src_stride);

    int width = m_width;
    int height = m_height;
    int align_width = m_align_width;
    int align_height = m_align_height;
    int stride = m_align_width;

#if RISEUP_DEBUG == 1
    IplImage *src_image = cvCreateImageHeader(cvSize(width, height), 8, 1);
    cvSetData(src_image, p_align_buffer, stride);
    cvShowImage("source", src_image);

    int iter = 0;
#endif

    // pad for block base
    // pad right
    BYTE *p_buffer_scan = p_align_buffer;
    for (int i = 0; i < height; i++)
    {
        memset(p_buffer_scan + width, p_buffer_scan[width - 1], align_width - width);
        p_buffer_scan += stride;
    }
    p_buffer_scan = p_align_buffer + height * stride;
    // pad bottom
    for (int i = height; i < align_height; i++)
    {
        memcpy(p_buffer_scan, p_buffer_scan - stride, align_width);
        p_buffer_scan += stride;
    }

    const int ITERATIONS = 2; // convergence at 2 iter
    const int BLOCK_SIZE = 16;

    for (int k = 0; k < ITERATIONS; k++)
    {
        // set bound value
        BYTE *p_left = mp_left_bound;
        BYTE *p_right = mp_right_bound;
        BYTE *p_top = mp_top_bound;
        BYTE *p_bottom = mp_bottom_bound;
        for (int i = 0; i < align_height; i++)
            p_left[i] = p_align_buffer[i * stride];
        for (int i = 0; i < align_height; i++)
            p_right[i] = p_align_buffer[i * stride + align_width - 1];
        memcpy(p_top, p_align_buffer, align_width);
        memcpy(p_bottom, p_align_buffer + (align_height - 1) * stride, align_width);

        // fill top and left
        ResetMacroBlockEvent();

        m_stage = BOUND_GRADIENT_TOP_LEFT;
        for (int i = 0; i < m_num_thread; i++)
        {
            mp_thread_param[i].align_width = align_width;
            mp_thread_param[i].align_height = align_height;
            mp_thread_param[i].block_size = BLOCK_SIZE;
            mp_thread_param[i].bound_value = bound_value;
            mp_thread_param[i].p_align_buffer = p_align_buffer;
            mp_thread_param[i].p_left = p_left;
            mp_thread_param[i].p_top = p_top;
            mp_thread_param[i].p_block_event = mp_macroblock_event;
            mp_thread_control[i].SignalBegin();
        }
        for (int i = 0; i < m_num_thread; i++)
        {
            mp_thread_control[i].WaitComplete();
        }

        // fill bottom and right
        ResetMacroBlockEvent();

        m_stage = BOUND_GRADIENT_BOTTOM_RIGHT;
        for (int i = 0; i < m_num_thread; i++)
        {
            mp_thread_param[i].align_width = align_width;
            mp_thread_param[i].align_height = align_height;
            mp_thread_param[i].block_size = BLOCK_SIZE;
            mp_thread_param[i].bound_value = bound_value;
            mp_thread_param[i].p_align_buffer = p_align_buffer;
            mp_thread_param[i].p_right = p_right;
            mp_thread_param[i].p_bottom = p_bottom;
            mp_thread_param[i].p_block_event = mp_macroblock_event;
            mp_thread_control[i].SignalBegin();
        }
        for (int i = 0; i < m_num_thread; i++)
        {
            mp_thread_control[i].WaitComplete();
        }
    }

#if RISEUP_DEBUG == 1
    cvShowImage("result", src_image);
    cvWaitKey(1);
    _DPRINTF((_T("iter %d"), iter));
    cvReleaseImageHeader(&src_image);
#endif

    FreeAndCopyResult(p_src_dst, src_stride);
}

void MorphologyTool::BoundGradient16x16TopLeft(BYTE *p_src, BYTE *p_top, BYTE *p_left, int bound_value)
{
    _MYASSERT(p_src);
    _MYASSERT(p_top);
    _MYASSERT(p_left);
    _MYASSERT(bound_value >= 0);
    _MYASSERT(bound_value <= BYTE_MAX);

    int stride = m_align_width;

#if defined(ANDROID_ARM) || defined(IOS_ARM)

    BYTE *pp_src[16];

    for (int i = 0; i < 16; i++)
    {
        pp_src[i] = p_src + i * stride;
    }

    uint8x16_t s[16], t[16];
    uint8x16_t top, left;

    top = vld1q_u8(p_top);

    load16x16_u8(pp_src[0], pp_src[1], pp_src[2], pp_src[3],
        pp_src[4], pp_src[5], pp_src[6], pp_src[7],
        pp_src[8], pp_src[9], pp_src[10], pp_src[11],
        pp_src[12], pp_src[13], pp_src[14], pp_src[15],
        s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
        s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);

    uint8x16_t *pre_s = &top;
    uint8x16_t value = vdupq_n_u8((BYTE)bound_value);
    for (int i = 0; i < 16; i++)
    {
        s[i] = vmaxq_u8(s[i], vqsubq_u8(pre_s[0], value));
        pre_s = s + i;
    }
    vst1q_u8(p_top, pre_s[0]);

    transpose16x16_u8(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
        s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15],
        t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
        t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15]);

    left = vld1q_u8(p_left);

    uint8x16_t *pre_t = &left;
    for (int i = 0; i < 16; i++)
    {
        t[i] = vmaxq_u8(t[i], vqsubq_u8(pre_t[0], value));
        pre_t = t + i;
    }
    vst1q_u8(p_left, pre_t[0]);

    transpose16x16_u8(t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
        t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15],
        s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
        s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);

    store16x16_u8(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
        s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15],
        pp_src[0], pp_src[1], pp_src[2], pp_src[3],
        pp_src[4], pp_src[5], pp_src[6], pp_src[7],
        pp_src[8], pp_src[9], pp_src[10], pp_src[11],
        pp_src[12], pp_src[13], pp_src[14], pp_src[15]);

#else
    if (g_is_support_SSE2)
    {
        BYTE *pp_src[16];

        for (int i = 0; i < 16; i++)
        {
            pp_src[i] = p_src + i * stride;
        }

        __m128i s[16], t[16];
        __m128i top, left;

        top = _mm_load_si128((__m128i *)p_top);

        _mm_load16x16_si128(pp_src[0], pp_src[1], pp_src[2], pp_src[3],
                            pp_src[4], pp_src[5], pp_src[6], pp_src[7],
                            pp_src[8], pp_src[9], pp_src[10], pp_src[11],
                            pp_src[12], pp_src[13], pp_src[14], pp_src[15],
                            s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                            s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);

        __m128i *pre_s = &top;
        __m128i value = _mm_set1_epi8((BYTE)bound_value);
        for (int i = 0; i < 16; i++)
        {
            s[i] = _mm_max_epu8(s[i], _mm_subs_epu8(pre_s[0], value));
            pre_s = s + i;
        }
        _mm_store_si128((__m128i *)(p_top), pre_s[0]);

        _mm_transpose16x16_epi8(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                                s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15],
                                t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
                                t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15]);

        left = _mm_load_si128((__m128i *)p_left);

        __m128i *pre_t = &left;
        for (int i = 0; i < 16; i++)
        {
            t[i] = _mm_max_epu8(t[i], _mm_subs_epu8(pre_t[0], value));
            pre_t = t + i;
        }
        _mm_store_si128((__m128i *)(p_left), pre_t[0]);

        _mm_transpose16x16_epi8(t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
                                t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15],
                                s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                                s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);

        _mm_store16x16_si128(pp_src[0], pp_src[1], pp_src[2], pp_src[3],
                             pp_src[4], pp_src[5], pp_src[6], pp_src[7],
                             pp_src[8], pp_src[9], pp_src[10], pp_src[11],
                             pp_src[12], pp_src[13], pp_src[14], pp_src[15],
                             s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                             s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);
    }
    else
    {
        BYTE *p_src_scan = p_src;
        BYTE *p_pre_src_scan = p_top;

        // top
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                p_src_scan[j] = (BYTE)ch_Max((int)p_src_scan[j], p_pre_src_scan[j] - bound_value);
            }
            p_pre_src_scan = p_src_scan;
            p_src_scan += stride;
        }
        memcpy(p_top, p_pre_src_scan, 16);

        // left
        p_src_scan = p_src;
        for (int i = 0; i < 16; i++)
        {
            p_pre_src_scan = p_left + i;
            for (int j = 0; j < 16; j++)
            {
                p_src_scan[j] = (BYTE)ch_Max((int)p_src_scan[j], p_pre_src_scan[0] - bound_value);
                p_pre_src_scan = p_src_scan + j;
            }
            p_left[i] = p_pre_src_scan[0];
            p_src_scan += stride;
        }
    }
#endif
}

void MorphologyTool::BoundGradient16x16BottomRight(BYTE *p_src, BYTE *p_bottom, BYTE *p_right, int bound_value)
{
    _MYASSERT(p_src);
    _MYASSERT(p_bottom);
    _MYASSERT(p_right);
    _MYASSERT(bound_value >= 0);
    _MYASSERT(bound_value <= BYTE_MAX);

    int stride = m_align_width;

#if defined(ANDROID_ARM) || defined(IOS_ARM)

    BYTE *pp_src[16];

    for (int i = 0; i < 16; i++)
    {
        pp_src[i] = p_src + i * stride;
    }

    uint8x16_t s[16], t[16];
    uint8x16_t bottom, right;

    bottom = vld1q_u8(p_bottom);

    load16x16_u8(pp_src[0], pp_src[1], pp_src[2], pp_src[3],
        pp_src[4], pp_src[5], pp_src[6], pp_src[7],
        pp_src[8], pp_src[9], pp_src[10], pp_src[11],
        pp_src[12], pp_src[13], pp_src[14], pp_src[15],
        s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
        s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);

    uint8x16_t *pre_s = &bottom;
    uint8x16_t value = vdupq_n_u8((BYTE)bound_value);
    for (int i = 15; i >= 0; i--)
    {
        s[i] = vmaxq_u8(s[i], vqsubq_u8(pre_s[0], value));
        pre_s = s + i;
    }
    vst1q_u8(p_bottom, pre_s[0]);

    transpose16x16_u8(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
        s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15],
        t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
        t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15]);

    right = vld1q_u8(p_right);

    uint8x16_t *pre_t = &right;
    for (int i = 15; i >= 0; i--)
    {
        t[i] = vmaxq_u8(t[i], vqsubq_u8(pre_t[0], value));
        pre_t = t + i;
    }
    vst1q_u8(p_right, pre_t[0]);

    transpose16x16_u8(t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
        t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15],
        s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
        s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);

    store16x16_u8(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
        s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15],
        pp_src[0], pp_src[1], pp_src[2], pp_src[3],
        pp_src[4], pp_src[5], pp_src[6], pp_src[7],
        pp_src[8], pp_src[9], pp_src[10], pp_src[11],
        pp_src[12], pp_src[13], pp_src[14], pp_src[15]);

#else
    if (g_is_support_SSE2)
    {
        BYTE *pp_src[16];

        for (int i = 0; i < 16; i++)
        {
            pp_src[i] = p_src + i * stride;
        }

        __m128i s[16], t[16];
        __m128i bottom, right;

        bottom = _mm_load_si128((__m128i *)p_bottom);

        _mm_load16x16_si128(pp_src[0], pp_src[1], pp_src[2], pp_src[3],
                            pp_src[4], pp_src[5], pp_src[6], pp_src[7],
                            pp_src[8], pp_src[9], pp_src[10], pp_src[11],
                            pp_src[12], pp_src[13], pp_src[14], pp_src[15],
                            s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                            s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);

        __m128i *pre_s = &bottom;
        __m128i value = _mm_set1_epi8((BYTE)bound_value);
        for (int i = 15; i >= 0; i--)
        {
            s[i] = _mm_max_epu8(s[i], _mm_subs_epu8(pre_s[0], value));
            pre_s = s + i;
        }
        _mm_store_si128((__m128i *)(p_bottom), pre_s[0]);

        _mm_transpose16x16_epi8(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                                s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15],
                                t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
                                t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15]);

        right = _mm_load_si128((__m128i *)p_right);

        __m128i *pre_t = &right;
        for (int i = 15; i >= 0; i--)
        {
            t[i] = _mm_max_epu8(t[i], _mm_subs_epu8(pre_t[0], value));
            pre_t = t + i;
        }
        _mm_store_si128((__m128i *)(p_right), pre_t[0]);

        _mm_transpose16x16_epi8(t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
                                t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15],
                                s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                                s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);

        _mm_store16x16_si128(pp_src[0], pp_src[1], pp_src[2], pp_src[3],
                             pp_src[4], pp_src[5], pp_src[6], pp_src[7],
                             pp_src[8], pp_src[9], pp_src[10], pp_src[11],
                             pp_src[12], pp_src[13], pp_src[14], pp_src[15],
                             s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                             s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15]);
    }
    else
    {
        BYTE *p_src_scan = p_src + 15 * stride;
        BYTE *p_pre_src_scan = p_bottom;
        // bottom
        for (int i = 15; i >= 0; i--)
        {
            for (int j = 0; j < 16; j++)
            {
                p_src_scan[j] = (BYTE)ch_Max((int)p_src_scan[j], p_pre_src_scan[j] - bound_value);
            }
            p_pre_src_scan = p_src_scan;
            p_src_scan -= stride;
        }
        memcpy(p_bottom, p_pre_src_scan, 16);

        // right
        p_src_scan = p_src;
        for (int i = 0; i < 16; i++)
        {
            p_pre_src_scan = p_right + i;
            for (int j = 15; j >= 0; j--)
            {
                p_src_scan[j] = (BYTE)ch_Max((int)p_src_scan[j], p_pre_src_scan[0] - bound_value);
                p_pre_src_scan = p_src_scan + j;
            }
            p_right[i] = p_pre_src_scan[0];
            p_src_scan += stride;
        }
    }
#endif
}

void MorphologyTool::Dilate(BYTE *p_src_dst, int stride, int dilate_parameter)
{
    _MYASSERT(p_src_dst);

    BYTE *p_align_buffer = GetAlignBuffer(p_src_dst, stride);

    BYTE *p_closing_temp_buffer = NULL;
    _ALIGNED_MALLOC_PTR(p_closing_temp_buffer, BYTE, m_align_width * m_align_height);

    int i = 0;
    for (; i < dilate_parameter >> 1; i++)
        LocalMaxFilter5x5(p_align_buffer, p_align_buffer, p_closing_temp_buffer,
                          m_height, m_width, m_align_width);
    if (dilate_parameter & 1)
        LocalMaxFilter3x3(p_align_buffer, p_align_buffer, p_closing_temp_buffer,
                          m_height, m_width, m_align_width);

    _ALIGNED_FREE_PTR(p_closing_temp_buffer);

    FreeAndCopyResult(p_src_dst, stride);
}

void MorphologyTool::Erode(BYTE *p_src_dst, int stride, int erode_parameter)
{
    _MYASSERT(p_src_dst);

    BYTE *p_align_buffer = GetAlignBuffer(p_src_dst, stride);

    BYTE *p_closing_temp_buffer = NULL;
    _ALIGNED_MALLOC_PTR(p_closing_temp_buffer, BYTE, m_align_width * m_align_height);

    int i = 0;
    for (; i < erode_parameter >> 1; i++)
        LocalMinFilter5x5(p_align_buffer, p_align_buffer, p_closing_temp_buffer,
                          m_height, m_width, m_align_width);
    if (erode_parameter & 1)
        LocalMinFilter3x3(p_align_buffer, p_align_buffer, p_closing_temp_buffer,
                          m_height, m_width, m_align_width);
    
    _ALIGNED_FREE_PTR(p_closing_temp_buffer);
    
    FreeAndCopyResult(p_src_dst, stride);
}


void MorphologyTool::GradientMap(BYTE *p_src, int src_stride, short *p_dst, int dst_stride, int &max_gradient, int &min_gradient)
{
    _MYASSERT(p_src);
    _MYASSERT(p_dst);
    
    //*****************************************************************************//
    // This function will calculate the gradient map in masked region(src == 255).
    // example:
    //                src:   0   0   0 255 255 255 255 255 255 255 255   0   0
    //         result dst: 255 255 255 254 253 252 251 251 252 253 254 255 255
    //       max_gradient: 255
    //       min_gradient: 251
    //*****************************************************************************//

    const int width = m_width;
    const int height = m_height;

    BYTE *p_gradient = NULL;
    _ALIGNED_MALLOC_PTR(p_gradient, BYTE, height * dst_stride);

    BYTE *p_scan_src = p_src;
    short *p_scan_dst = p_dst;
    BYTE *p_scan_gradient = p_gradient;

#if defined(ANDROID_ARM) || defined(IOS_ARM)
    const int neon_step = 16;
    const int neon_right = width / neon_step * neon_step;
    uint8x16_t arm_255 = vdupq_n_u8(255);
    int16x8_t arm_1 = vdupq_n_s16(1);
    uint8x16_t arm_zero = vdupq_n_u8(0);
#endif

    for (int y = 0; y < height; y++)
    {
        int x = 0;

#if defined(ANDROID_ARM) || defined(IOS_ARM)

        for (; x < neon_right; x += neon_step)
        {
            uint8x16_t arm_src = vld1q_u8(p_scan_src + x);
            uint8x16_t arm_compare_mask = vceqq_u8(arm_src, arm_zero);
            uint8x16x2_t arm_compare_mask_zip = vzipq_u8(arm_compare_mask, arm_compare_mask);
            uint8x16_t arm_gradient = vandq_u8(arm_compare_mask, arm_255);
            int16x8_t arm_dst_low = vandq_s16(vreinterpretq_s16_u8(arm_compare_mask_zip.val[0]), arm_1);
            int16x8_t arm_dst_high = vandq_s16(vreinterpretq_s16_u8(arm_compare_mask_zip.val[1]), arm_1);
            vst1q_u8(p_scan_gradient + x, arm_gradient);
            vst1q_s16(p_scan_dst + x, arm_dst_low);
            vst1q_s16(p_scan_dst + x + 8, arm_dst_high);
        }

#endif

        for (; x < width; x++)
        {
            if (p_scan_src[x] != 0)
            {
                p_scan_gradient[x] = 0;
                p_scan_dst[x] = 0;
            }
            else
            {
                p_scan_gradient[x] = 255;
                p_scan_dst[x] = 1;
            }
        }

        p_scan_src += src_stride;
        p_scan_dst += dst_stride;
        p_scan_gradient += dst_stride;
    }

    max_gradient = 0;
    min_gradient = INT_MAX;

    while (true)
    {
        BoundGradient(p_gradient, dst_stride, 1);

        if (max_gradient + 254 >= SHRT_MAX)
            break;

        max_gradient += 254;
        min_gradient = 255;

        BYTE *p_scan_gradient = p_gradient;
        short *p_scan_dst = p_dst;

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int gradient = ch_Min((int)p_scan_gradient[x], 254);
                min_gradient = ch_Min(min_gradient, gradient);

                p_scan_gradient[x] = (gradient > 0) ? 255 : 0;
                p_scan_dst[x] = (BYTE)CLAMP(p_scan_dst[x] + gradient, 255, 0);
            }

            p_scan_gradient += dst_stride;
            p_scan_dst += dst_stride;
        }

        if (min_gradient != 0)
            break;
    }

    _ALIGNED_FREE_PTR(p_gradient);
}

void MorphologyTool::SetThreadPool(ThreadPool *p_thread_pool)
{
    _MYASSERT(p_thread_pool);
    mp_thread_pool = p_thread_pool;
    for(int i = 0; i < m_num_thread; i++)
    {
        mp_thread_control[i].SetThreadPool(mp_thread_pool);
    }
}

void MorphologyTool::InitialThread()
{
    if (0 != m_num_thread)
        return;
    m_num_thread = ch_Max(1, GetLogicalCPUCount());
    _NEW_PTRS(mp_thread_param, ThreadParameter, m_num_thread);
    _NEW_PTRS(mp_thread_control, ThreadControlShell, m_num_thread);

    for(int i = 0; i < m_num_thread; i ++)
    {
        mp_thread_param[i].thread_id = i;
        mp_thread_param[i].p_context = this;
        mp_thread_control[i].CreateThread_Run(MorphologyTool::MorphologyToolMultiCore, &mp_thread_param[i]);
    }
}

ThreadFunctionReturnType WINAPI MorphologyTool::MorphologyToolMultiCore(LPVOID parameter)
{
    if (parameter == NULL)
        return 0;

    ThreadParameter *p_parameter = (ThreadParameter *) parameter;
    int thread_id = p_parameter->thread_id;
    MorphologyTool *p_context = p_parameter->p_context;
    const int num_thread = p_context->m_num_thread;

    if (p_context->m_stage == BOUND_GRADIENT_TOP_LEFT)
    {
        const int align_width = p_parameter->align_width;
        const int align_height = p_parameter->align_height;
        const int src_stride = align_width;
        const int macroblock_stride = src_stride * MULTITHREAD_MACROBLOCK_SIZE;
        const int block_size = p_parameter->block_size;
        const int bound_value = p_parameter->bound_value;
        BYTE *p_align_buffer = p_parameter->p_align_buffer;
        BYTE *p_top = p_parameter->p_top;
        BYTE *p_left = p_parameter->p_left;

        const int num_block_per_macroblock = MULTITHREAD_MACROBLOCK_SIZE / block_size;
        const int num_macroblock_i = p_context->m_num_macroblock_i;
        const int num_macroblock_j = p_context->m_num_macroblock_j;

        HANDLE *p_block_event = p_parameter->p_block_event;

        // need to handle boundary case
        const int remain_height = align_height - (num_macroblock_i - 1) * MULTITHREAD_MACROBLOCK_SIZE;
        const int remain_width = align_width - (num_macroblock_j - 1) * MULTITHREAD_MACROBLOCK_SIZE;

        BYTE *p_buffer_scan = p_align_buffer + macroblock_stride * thread_id;
        int i = thread_id;
        for (; i < num_macroblock_i - 1; i += num_thread)
        {
            for (int j = 0; j < num_macroblock_j - 1; j++)
            {
                if (i != 0)
                {
                    int dependence_block_index_top = (i - 1) * num_macroblock_j + j;
                    WaitForSingleObject(p_block_event[dependence_block_index_top], INFINITE);
                }

                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;
                p_context->BoundGradient16nx16mTopLeft(p_buffer_scan + pixel_index_j, p_top + pixel_index_j,
                                                       p_left + pixel_index_i, src_stride, bound_value,
                                                       num_block_per_macroblock, num_block_per_macroblock);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);
            }

            // last macroblock
            {
                const int j = num_macroblock_j - 1;

                if (i != 0)
                {
                    int dependence_block_index_top = (i - 1) * num_macroblock_j + j;
                    WaitForSingleObject(p_block_event[dependence_block_index_top], INFINITE);
                }

                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;
                const int remain_num_macroblock_j = remain_width / block_size;
                p_context->BoundGradient16nx16mTopLeft(p_buffer_scan + pixel_index_j, p_top + pixel_index_j,
                                                       p_left + pixel_index_i, src_stride, bound_value,
                                                       num_block_per_macroblock, remain_num_macroblock_j);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);
            }
            p_buffer_scan += macroblock_stride * num_thread;
        }
        // last macroblock line
        if (i == (num_macroblock_i - 1))
        {
            const int remain_num_macroblock_i = remain_height / block_size;
            for (int j = 0; j < num_macroblock_j - 1; j++)
            {
                if (i != 0)
                {
                    int dependence_block_index_top = (i - 1) * num_macroblock_j + j;
                    WaitForSingleObject(p_block_event[dependence_block_index_top], INFINITE);
                }

                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;

                p_context->BoundGradient16nx16mTopLeft(p_buffer_scan + pixel_index_j, p_top + pixel_index_j,
                                                       p_left + pixel_index_i, src_stride, bound_value,
                                                       remain_num_macroblock_i, num_block_per_macroblock);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);
            }

            {
                const int j = num_macroblock_j - 1;

                if (i != 0)
                {
                    int dependence_block_index_top = (i - 1) * num_macroblock_j + j;
                    WaitForSingleObject(p_block_event[dependence_block_index_top], INFINITE);
                }

                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;
                const int remain_num_macroblock_j = remain_width / block_size;
                p_context->BoundGradient16nx16mTopLeft(p_buffer_scan + pixel_index_j, p_top + pixel_index_j,
                                                       p_left + pixel_index_i, align_width, bound_value,
                                                       remain_num_macroblock_i, remain_num_macroblock_j);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);
            }
        }
    }
    else if (p_context->m_stage == BOUND_GRADIENT_BOTTOM_RIGHT)
    {
        const int align_width = p_parameter->align_width;
        const int align_height = p_parameter->align_height;
        const int src_stride = align_width;
        const int macroblock_stride = src_stride * MULTITHREAD_MACROBLOCK_SIZE;
        const int block_size = p_parameter->block_size;
        const int bound_value = p_parameter->bound_value;
        BYTE *p_align_buffer = p_parameter->p_align_buffer;
        BYTE *p_bottom = p_parameter->p_bottom;
        BYTE *p_right = p_parameter->p_right;

        const int num_block_per_macroblock = MULTITHREAD_MACROBLOCK_SIZE / block_size;
        const int num_macroblock_i = p_context->m_num_macroblock_i;
        const int num_macroblock_j = p_context->m_num_macroblock_j;

        HANDLE *p_block_event = p_parameter->p_block_event;

        // need to handle boundary case
        const int remain_height = align_height - (num_macroblock_i - 1) * MULTITHREAD_MACROBLOCK_SIZE;
        const int remain_width = align_width - (num_macroblock_j - 1) * MULTITHREAD_MACROBLOCK_SIZE;

        BYTE *p_buffer_scan = p_align_buffer + (num_macroblock_i - 1 - thread_id) * macroblock_stride;
        int i = num_macroblock_i - 1 - thread_id;
        if (i == (num_macroblock_i - 1))
        {
            const int remain_num_macroblock_i = remain_height / block_size;
            {
                int j = num_macroblock_j - 1;

                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;
                const int remain_num_macroblock_j = remain_width / block_size;
                p_context->BoundGradient16nx16mBottomRight(p_buffer_scan + pixel_index_j, p_bottom + pixel_index_j,
                                                           p_right + pixel_index_i, src_stride, bound_value,
                                                           remain_num_macroblock_i, remain_num_macroblock_j);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);
            }
            for (int j = num_macroblock_j - 2; j >= 0; j--)
            {
                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;
                p_context->BoundGradient16nx16mBottomRight(p_buffer_scan + pixel_index_j, p_bottom + pixel_index_j,
                                                           p_right + pixel_index_i, src_stride, bound_value,
                                                           remain_num_macroblock_i, num_block_per_macroblock);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);
            }
            i -= num_thread;
            p_buffer_scan -= macroblock_stride * num_thread;
        }

        for (; i >= 0; i -= num_thread)
        {
            {
                int j = num_macroblock_j - 1;

                if (i != (num_macroblock_i - 1))
                {
                    int dependence_block_index_bottom = (i + 1) * num_macroblock_j + j;
                    WaitForSingleObject(p_block_event[dependence_block_index_bottom], INFINITE);
                }

                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;
                const int remain_num_macroblock_j = remain_width / block_size;
                p_context->BoundGradient16nx16mBottomRight(p_buffer_scan + pixel_index_j, p_bottom + pixel_index_j,
                                                           p_right + pixel_index_i, src_stride, bound_value,
                                                           num_block_per_macroblock, remain_num_macroblock_j);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);

            }
            for (int j = num_macroblock_j - 2; j >= 0; j--)
            {
                if (i != (num_macroblock_i - 1))
                {
                    int dependence_block_index_bottom = (i + 1) * num_macroblock_j + j;
                    WaitForSingleObject(p_block_event[dependence_block_index_bottom], INFINITE);
                }

                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;
                p_context->BoundGradient16nx16mBottomRight(p_buffer_scan + pixel_index_j, p_bottom + pixel_index_j,
                                                           p_right + pixel_index_i, src_stride, bound_value,
                                                           num_block_per_macroblock, num_block_per_macroblock);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);
            }

            p_buffer_scan -= macroblock_stride * num_thread;
        }
    }
    else if (p_context->m_stage == FILL_HOLE_TOP_LEFT)
    {
        const int align_width = p_parameter->align_width;
        const int align_height = p_parameter->align_height;
        const int src_stride = align_width;
        const int macroblock_stride = src_stride * MULTITHREAD_MACROBLOCK_SIZE;
        const int block_size = p_parameter->block_size;
        BYTE *p_align_buffer = p_parameter->p_align_buffer;
        BYTE *p_water = p_parameter->p_water;
        BYTE *p_top = p_parameter->p_top;
        BYTE *p_left = p_parameter->p_left;

        const int num_block_per_macroblock = MULTITHREAD_MACROBLOCK_SIZE / block_size;
        const int num_macroblock_i = p_context->m_num_macroblock_i;
        const int num_macroblock_j = p_context->m_num_macroblock_j;

        HANDLE *p_block_event = p_parameter->p_block_event;

        // need to handle boundary case
        const int remain_height = align_height - (num_macroblock_i - 1) * MULTITHREAD_MACROBLOCK_SIZE;
        const int remain_width = align_width - (num_macroblock_j - 1) * MULTITHREAD_MACROBLOCK_SIZE;

        BYTE *p_buffer_scan = p_align_buffer + macroblock_stride * thread_id;
        BYTE *p_water_scan = p_water + macroblock_stride * thread_id;
        int i = thread_id;
        for (; i < num_macroblock_i - 1; i += num_thread)
        {
            for (int j = 0; j < num_macroblock_j - 1; j++)
            {
                if (i != 0)
                {
                    int dependence_block_index_top = (i - 1) * num_macroblock_j + j;
                    WaitForSingleObject(p_block_event[dependence_block_index_top], INFINITE);
                }

                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;
                p_context->FillHole16nx16mTopLeft(p_buffer_scan + pixel_index_j, p_water_scan + pixel_index_j, 
                                                  p_top + pixel_index_j, p_left + pixel_index_i, src_stride,
                                                  num_block_per_macroblock, num_block_per_macroblock);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);
            }

            // last macroblock
            {
                const int j = num_macroblock_j - 1;

                if (i != 0)
                {
                    int dependence_block_index_top = (i - 1) * num_macroblock_j + j;
                    WaitForSingleObject(p_block_event[dependence_block_index_top], INFINITE);
                }

                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;
                const int remain_num_macroblock_j = remain_width / block_size;
                p_context->FillHole16nx16mTopLeft(p_buffer_scan + pixel_index_j, p_water_scan + pixel_index_j, 
                                                  p_top + pixel_index_j, p_left + pixel_index_i, src_stride,
                                                  num_block_per_macroblock, remain_num_macroblock_j);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);
            }
            p_buffer_scan += macroblock_stride * num_thread;
            p_water_scan += macroblock_stride * num_thread;
        }
        // last macroblock line
        if (i == (num_macroblock_i - 1))
        {
            const int remain_num_macroblock_i = remain_height / block_size;
            for (int j = 0; j < num_macroblock_j - 1; j++)
            {
                if (i != 0)
                {
                    int dependence_block_index_top = (i - 1) * num_macroblock_j + j;
                    WaitForSingleObject(p_block_event[dependence_block_index_top], INFINITE);
                }

                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;

                p_context->FillHole16nx16mTopLeft(p_buffer_scan + pixel_index_j, p_water_scan + pixel_index_j,
                                                  p_top + pixel_index_j, p_left + pixel_index_i, src_stride,
                                                  remain_num_macroblock_i, num_block_per_macroblock);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);
            }

            {
                const int j = num_macroblock_j - 1;

                if (i != 0)
                {
                    int dependence_block_index_top = (i - 1) * num_macroblock_j + j;
                    WaitForSingleObject(p_block_event[dependence_block_index_top], INFINITE);
                }

                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;
                const int remain_num_macroblock_j = remain_width / block_size;
                p_context->FillHole16nx16mTopLeft(p_buffer_scan + pixel_index_j, p_water_scan + pixel_index_j,
                                                  p_top + pixel_index_j, p_left + pixel_index_i, align_width,
                                                  remain_num_macroblock_i, remain_num_macroblock_j);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);
            }
        }
    }
    else if (p_context->m_stage == FILL_HOLE_BOTTOM_RIGHT)
    {
        const int align_width = p_parameter->align_width;
        const int align_height = p_parameter->align_height;
        const int src_stride = align_width;
        const int macroblock_stride = src_stride * MULTITHREAD_MACROBLOCK_SIZE;
        const int block_size = p_parameter->block_size;
        BYTE *p_align_buffer = p_parameter->p_align_buffer;
        BYTE *p_water = p_parameter->p_water;
        BYTE *p_bottom = p_parameter->p_bottom;
        BYTE *p_right = p_parameter->p_right;

        const int num_block_per_macroblock = MULTITHREAD_MACROBLOCK_SIZE / block_size;
        const int num_macroblock_i = p_context->m_num_macroblock_i;
        const int num_macroblock_j = p_context->m_num_macroblock_j;

        HANDLE *p_block_event = p_parameter->p_block_event;

        // need to handle boundary case
        const int remain_height = align_height - (num_macroblock_i - 1) * MULTITHREAD_MACROBLOCK_SIZE;
        const int remain_width = align_width - (num_macroblock_j - 1) * MULTITHREAD_MACROBLOCK_SIZE;

        BYTE *p_buffer_scan = p_align_buffer + (num_macroblock_i - 1 - thread_id) * macroblock_stride;
        BYTE *p_water_scan = p_water + (num_macroblock_i - 1 - thread_id) * macroblock_stride;
        int i = num_macroblock_i - 1 - thread_id;
        if (i == (num_macroblock_i - 1))
        {
            const int remain_num_macroblock_i = remain_height / block_size;
            {
                int j = num_macroblock_j - 1;

                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;
                const int remain_num_macroblock_j = remain_width / block_size;
                p_context->FillHole16nx16mBottomRight(p_buffer_scan + pixel_index_j, p_water_scan + pixel_index_j,
                                                      p_bottom + pixel_index_j, p_right + pixel_index_i, src_stride,
                                                      remain_num_macroblock_i, remain_num_macroblock_j);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);
            }
            for (int j = num_macroblock_j - 2; j >= 0; j--)
            {
                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;
                p_context->FillHole16nx16mBottomRight(p_buffer_scan + pixel_index_j, p_water_scan + pixel_index_j,
                                                      p_bottom + pixel_index_j, p_right + pixel_index_i, src_stride,
                                                      remain_num_macroblock_i, num_block_per_macroblock);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);
            }
            i -= num_thread;
            p_buffer_scan -= macroblock_stride * num_thread;
            p_water_scan -= macroblock_stride * num_thread;
        }

        for (; i >= 0; i -= num_thread)
        {
            {
                int j = num_macroblock_j - 1;

                if (i != (num_macroblock_i - 1))
                {
                    int dependence_block_index_bottom = (i + 1) * num_macroblock_j + j;
                    WaitForSingleObject(p_block_event[dependence_block_index_bottom], INFINITE);
                }

                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;
                const int remain_num_macroblock_j = remain_width / block_size;
                p_context->FillHole16nx16mBottomRight(p_buffer_scan + pixel_index_j, p_water_scan + pixel_index_j,
                                                      p_bottom + pixel_index_j, p_right + pixel_index_i, src_stride,
                                                      num_block_per_macroblock, remain_num_macroblock_j);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);

            }
            for (int j = num_macroblock_j - 2; j >= 0; j--)
            {
                if (i != (num_macroblock_i - 1))
                {
                    int dependence_block_index_bottom = (i + 1) * num_macroblock_j + j;
                    WaitForSingleObject(p_block_event[dependence_block_index_bottom], INFINITE);
                }

                const int pixel_index_i = i * MULTITHREAD_MACROBLOCK_SIZE;
                const int pixel_index_j = j * MULTITHREAD_MACROBLOCK_SIZE;
                p_context->FillHole16nx16mBottomRight(p_buffer_scan + pixel_index_j, p_water_scan + pixel_index_j,
                                                      p_bottom + pixel_index_j, p_right + pixel_index_i, src_stride,
                                                      num_block_per_macroblock, num_block_per_macroblock);

                int dependence_block_index = i * num_macroblock_j + j;
                SetEvent(p_block_event[dependence_block_index]);
            }

            p_buffer_scan -= macroblock_stride * num_thread;
            p_water_scan -= macroblock_stride * num_thread;
        }
    }
    else
    {
        _MYASSERT(FORCE_FALSE);
    }
    return 0;
}

void MorphologyTool::BoundGradient16nx16mTopLeft( BYTE *p_src, BYTE *p_top, BYTE *p_left, int src_stride, int bound_value, int num_block_i, int num_block_j )
{
    _MYASSERT(p_src);
    _MYASSERT(p_top);
    _MYASSERT(p_left);
    _MYASSERT(num_block_i > 0);
    _MYASSERT(num_block_j > 0);

    const int BLOCK_SIZE = 16;
    BYTE *p_src_scan = p_src;
    for (int i = 0; i < num_block_i; i++)
    {
        for (int j = 0; j < num_block_j; j++)
        {
            BoundGradient16x16TopLeft(p_src_scan + j * BLOCK_SIZE, p_top + j * BLOCK_SIZE, p_left + i * BLOCK_SIZE, bound_value);
        }
        p_src_scan += src_stride * BLOCK_SIZE;
    }
}

void MorphologyTool::BoundGradient16nx16mBottomRight( BYTE *p_src, BYTE *p_bottom, BYTE *p_right, int src_stride, int bound_value, int num_block_i, int num_block_j )
{
    _MYASSERT(p_src);
    _MYASSERT(p_bottom);
    _MYASSERT(p_right);
    _MYASSERT(num_block_i > 0);
    _MYASSERT(num_block_j > 0);

    const int BLOCK_SIZE = 16;
    BYTE *p_src_scan = p_src + src_stride * BLOCK_SIZE * (num_block_i - 1);
    for (int i = num_block_i - 1; i >= 0; i--)
    {
        for (int j = num_block_j - 1; j >= 0; j--)
        {
            BoundGradient16x16BottomRight(p_src_scan + j * BLOCK_SIZE, p_bottom + j * BLOCK_SIZE, p_right + i * BLOCK_SIZE, bound_value);
        }
        p_src_scan -= src_stride * BLOCK_SIZE;
    }
}

void MorphologyTool::FillHole16nx16mTopLeft(BYTE *p_src, BYTE *p_water, BYTE *p_top, BYTE *p_left, int stride, int num_block_i, int num_block_j)
{
    _MYASSERT(p_src);
    _MYASSERT(p_water);
    _MYASSERT(p_top);
    _MYASSERT(p_left);
    _MYASSERT(num_block_i > 0);
    _MYASSERT(num_block_j > 0);

    const int BLOCK_SIZE = 16;
    BYTE *p_src_scan = p_src;
    BYTE *p_water_scan = p_water;
    for (int i = 0; i < num_block_i; i++)
    {
        for (int j = 0; j < num_block_j; j++)
        {
            FillHole16x16TopLeft(p_src_scan + j * BLOCK_SIZE, p_water_scan + j * BLOCK_SIZE, p_top + j * BLOCK_SIZE, p_left + i * BLOCK_SIZE);
        }
        p_src_scan += stride * BLOCK_SIZE;
        p_water_scan += stride * BLOCK_SIZE;
    }
}

void MorphologyTool::FillHole16nx16mBottomRight(BYTE *p_src, BYTE *p_water, BYTE *p_bottom, BYTE *p_right, int stride, int num_block_i, int num_block_j)
{
    _MYASSERT(p_src);
    _MYASSERT(p_water);
    _MYASSERT(p_bottom);
    _MYASSERT(p_right);
    _MYASSERT(num_block_i > 0);
    _MYASSERT(num_block_j > 0);

    const int BLOCK_SIZE = 16;
    BYTE *p_src_scan = p_src + stride * BLOCK_SIZE * (num_block_i - 1);
    BYTE *p_water_scan = p_water + stride * BLOCK_SIZE * (num_block_i - 1);
    for (int i = num_block_i - 1; i >= 0; i--)
    {
        for (int j = num_block_j - 1; j >= 0; j--)
        {
            FillHole16x16BottomRight(p_src_scan + j * BLOCK_SIZE, p_water_scan + j * BLOCK_SIZE, p_bottom + j * BLOCK_SIZE, p_right + i * BLOCK_SIZE);
        }
        p_src_scan -= stride * BLOCK_SIZE;
        p_water_scan -= stride * BLOCK_SIZE;
    }
}

void MorphologyTool::InitializeMacroBlock(int align_width, int align_height)
{
    _MYASSERT((MULTITHREAD_MACROBLOCK_SIZE & 15) == 0);
    _MYASSERT((align_width & 15) == 0);
    _MYASSERT((align_height & 15) == 0);

    ReleaseMacroBlockEvent();

    m_num_macroblock_i = (align_height + MULTITHREAD_MACROBLOCK_SIZE - 1) / MULTITHREAD_MACROBLOCK_SIZE;
    m_num_macroblock_j = (align_width + MULTITHREAD_MACROBLOCK_SIZE - 1) / MULTITHREAD_MACROBLOCK_SIZE;

    _NEW_PTRS(mp_macroblock_event, HANDLE, m_num_macroblock_i * m_num_macroblock_j);

    for (int i = 0; i < m_num_macroblock_i * m_num_macroblock_j; i++)
    {
        mp_macroblock_event[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
    }
}

void MorphologyTool::ResetMacroBlockEvent()
{
    _MYASSERT(mp_macroblock_event);

    for (int i = 0; i < m_num_macroblock_i * m_num_macroblock_j; i++)
        ResetEvent(mp_macroblock_event[i]);
}

void MorphologyTool::ReleaseMacroBlockEvent()
{
    if (mp_macroblock_event != NULL)
    {
        for (int i = 0; i < m_num_macroblock_i * m_num_macroblock_j; i++)
            CloseHandle(mp_macroblock_event[i]);
        _DELETE_PTRS(mp_macroblock_event);
    }
}
