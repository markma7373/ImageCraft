#include "stdafx.h"
#include "SumImageBoxFilter.h"
#include "use_hylib.h"
#include "use_ipp.h"

#include "IntegerDivOpt.h"

void SumImageBoxFilter::FilterBox(const BYTE *p_src_data, int src_step, BYTE *p_dst_data, int dst_step,
                                  int width, int height, int radius, int *p_external_buffer/* = NULL*/)
{
    // Perform box filter by sum image. The source data MUST BE well padding.
    // The filter box is a square with length = (radius * 2 + 1), and the anchor point is at the center.
    // If p_external_buffer = NULL, the buffer of sum image will be allocated in this function.
    // If it is given, its size must be at least sizeof(int) * (width + length) * (height + length).

    _MYASSERT(p_src_data);
    _MYASSERT(p_dst_data);
    _MYASSERT(width > 0);
    _MYASSERT(height > 0);
    _MYASSERT(radius > 0);

    const int box_length = radius * 2 + 1;
    const int box_area = box_length * box_length;

    const int pad_width = width + radius * 2;
    const int pad_height = height + radius * 2;
    const int sum_width = pad_width + 1;
    const int sum_height = pad_height + 1;

    int *p_sum_buffer = p_external_buffer;
    bool is_allocate_buffer = false;
    if (p_sum_buffer == NULL)
    {
        p_sum_buffer = new int[sum_width * sum_height];
        is_allocate_buffer = true;
    }

    const BYTE *p_pad_src_start = p_src_data - radius * src_step - radius;
    ippiIntegral_8u32s_C1R(p_pad_src_start, src_step, p_sum_buffer,
                           sizeof(int) * sum_width, ippiSize(pad_width, pad_height), 0);

    const int half_box_area = box_area / 2;

#if defined(ANDROID_ARM) || defined(IOS_ARM)
    struct libdivide_u32_t denominator = libdivide_u32_gen((uint32_t)box_area);
    int libdivide_u32_case = libdivide_u32_get_case(&denominator);

    uint32x4_t (*p_libdivide_u32)(uint32x4_t, const struct libdivide_u32_t *);
    if (0 == libdivide_u32_case)
        p_libdivide_u32 = libdivide_u32_do_vector_case0;
    else if (1 == libdivide_u32_case)
        p_libdivide_u32 = libdivide_u32_do_vector_case1;
    else
        p_libdivide_u32 = libdivide_u32_do_vector_case2;
#endif

    for (int y = 0; y < height; y++)
    {
        BYTE *p_dst_scan = p_dst_data + y * dst_step;

        const int top_offset = y * sum_width;
        const int bottom_offset = (y + box_length) * sum_width;

        int x = 0;
#if defined(ANDROID_ARM) || defined(IOS_ARM)
        for (; x < (width & ~3); x += 4)
        {
            const int offset1 = top_offset + x;
            const int offset2 = offset1 + box_length;
            const int offset3 = bottom_offset + x;
            const int offset4 = offset3 + box_length;

            int32x4_t arm_sum_1 = vld1q_s32(p_sum_buffer + offset1);
            int32x4_t arm_sum_2 = vld1q_s32(p_sum_buffer + offset2);
            int32x4_t arm_sum_3 = vld1q_s32(p_sum_buffer + offset3);
            int32x4_t arm_sum_4 = vld1q_s32(p_sum_buffer + offset4);

            int32x4_t arm_box_sum = vdupq_n_s32(half_box_area);
            arm_box_sum = vaddq_s32(arm_box_sum, arm_sum_1);
            arm_box_sum = vaddq_s32(arm_box_sum, arm_sum_4);
            arm_box_sum = vsubq_s32(arm_box_sum, arm_sum_2);
            arm_box_sum = vsubq_s32(arm_box_sum, arm_sum_3);

            uint32x4_t arm_box_mean = p_libdivide_u32(vreinterpretq_u32_s32(arm_box_sum), &denominator);

            uint16x4_t arm_box_mean_u16x4 = vqmovn_u32(arm_box_mean);
            uint16x8_t arm_box_mean_u16x8 = vcombine_u16(arm_box_mean_u16x4, arm_box_mean_u16x4);
            uint8x8_t arm_box_mean_u8x8 = vmovn_u16(arm_box_mean_u16x8);

            int32x2_t arm_box_mean_s32x2 = vreinterpret_s32_u8(arm_box_mean_u8x8);
            int box_mean = vget_lane_s32(arm_box_mean_s32x2, 0);
            memcpy(p_dst_scan + x, &box_mean, 4);
        }
#endif
        for (; x < width; x++)
        {
            const int offset1 = top_offset + x;
            const int offset2 = offset1 + box_length;
            const int offset3 = bottom_offset + x;
            const int offset4 = offset3 + box_length;

            int box_sum = p_sum_buffer[offset1] - p_sum_buffer[offset2]
                        - p_sum_buffer[offset3] + p_sum_buffer[offset4];
            int box_mean = (box_sum + half_box_area) / box_area;

            p_dst_scan[x] = (BYTE)box_mean;
        }
    }


    if (is_allocate_buffer)
        delete [] p_sum_buffer;
}

void SumImageBoxFilter::FilterBox_C4R(const BYTE *p_src_data, int src_step,
                                      BYTE *p_dst_data, int dst_step,
                                      int width, int height, int radius,
                                      int *p_external_buffer/* = NULL*/)
{
    // Perform box filter by sum image. The source data MUST BE well padding.
    // The filter box is a square with length = (radius * 2 + 1), and the anchor point is at the center.
    // If p_external_buffer = NULL, the buffer of sum image will be allocated in this function.
    // If it is given, its size must be at least sizeof(int) * (width + length) * (height + length).

    _MYASSERT(p_src_data);
    _MYASSERT(p_dst_data);
    _MYASSERT(width > 0);
    _MYASSERT(height > 0);
    _MYASSERT(radius > 0);

    const int box_length = radius * 2 + 1;
    const int box_area = box_length * box_length;

    const int pad_width = width + radius * 2;
    const int pad_height = height + radius * 2;
    const int sum_width = pad_width + 1;
    const int sum_height = pad_height + 1;

    int *p_sum_buffer = p_external_buffer;
    bool is_allocate_buffer = false;
    if (p_sum_buffer == NULL)
    {
        p_sum_buffer = new int[sum_width * sum_height];
        is_allocate_buffer = true;
    }

    const BYTE *p_pad_src_start = p_src_data - radius * src_step - radius * 4;
    for (int channels = 0; channels < 4; channels++)
    {
        GetSumImage(p_pad_src_start + channels, src_step, 4, p_sum_buffer, sum_width,
                    pad_width, pad_height);

        const int half_box_area = box_area / 2;
        for (int y = 0; y < height; y++)
        {
            BYTE *p_dst_scan = p_dst_data + y * dst_step;

            const int top_offset = y * sum_width;
            const int bottom_offset = (y + box_length) * sum_width;

            for (int x = 0; x < width; x++)
            {
                const int offset1 = top_offset + x;
                const int offset2 = offset1 + box_length;
                const int offset3 = bottom_offset + x;
                const int offset4 = offset3 + box_length;

                int box_sum = p_sum_buffer[offset1] - p_sum_buffer[offset2]
                - p_sum_buffer[offset3] + p_sum_buffer[offset4];
                int box_mean = (box_sum + half_box_area) / box_area;

                p_dst_scan[x * 4 + channels] = (BYTE)box_mean;
            }
        }
    }

    if (is_allocate_buffer)
        delete [] p_sum_buffer;
}

void SumImageBoxFilter::GetSumImage(const BYTE *p_src_data, int src_stride, int src_step,
                                    int *p_dst_data, int dst_stride,
                                    int width, int height)
{
    _MYASSERT(p_src_data);
    _MYASSERT(p_dst_data);
    _MYASSERT(width > 0);
    _MYASSERT(height > 0);

    // init first row and column
    memset(p_dst_data, 0, sizeof(int) * (width + 1));
    int *p_dst_col = p_dst_data;
    for (int j = 0; j < height + 1; j++)
    {
        p_dst_col[0] = 0;
        p_dst_col += dst_stride;
    }

    const BYTE *p_src_row = p_src_data;
    int *p_dst_row = p_dst_data + 1 + dst_stride;

    // horizontal
    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src_row;
        int *p_dst_scan = p_dst_row;

        for (int x = 0; x < width; x++)
        {
            p_dst_scan[0] = p_dst_scan[-1] + p_src_scan[0];
            p_src_scan += src_step;
            p_dst_scan += 1;
        }

        p_src_row += src_stride;
        p_dst_row += dst_stride;
    }

    // vertical
    p_dst_col = p_dst_data + 1 + dst_stride;
    for (int x = 0; x < width; x++)
    {
        int *p_dst_scan = p_dst_col + dst_stride;

        for (int y = 1; y < height; y++)
        {
            p_dst_scan[0] += p_dst_scan[-dst_stride];
            p_dst_scan += dst_stride;
        }
        p_dst_col += 1;
    }
}