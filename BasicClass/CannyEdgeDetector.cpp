#include "stdafx.h"
#include "CannyEdgeDetector.h"
#include "use_ipp.h"
#include "use_simd.h"

// default canny edge detection thresholds, set at initialized
#define CANNY_MOBILE_DEFAULT_LOW_THRESHOLD  100.0f
#define CANNY_MOBILE_DEFAULT_HIGH_THRESHOLD 200.0f

CannyEdgeDetector::CannyEdgeDetector()
{
    m_is_disable_ipp = false; // Force disable IPP if this is true.

    m_width = 0;
    m_height = 0;
    m_stride = 0;
    m_low_threshold = CANNY_MOBILE_DEFAULT_LOW_THRESHOLD;
    m_high_threshold = CANNY_MOBILE_DEFAULT_HIGH_THRESHOLD;

    mpp_stack = NULL;
    mpp_stack_top = NULL;

    mp_x_derivative = NULL;
    mp_y_derivative = NULL;

    mp_edge_image = NULL;
}

CannyEdgeDetector::~CannyEdgeDetector()
{
    FreeMemory();
}

inline void CannyEdgeDetector::StackPush(BYTE *p_label)
{
    _MYASSERT(p_label);
    _MYASSERT(mpp_stack_top);

    *p_label = 2;
    *mpp_stack_top = p_label;
    mpp_stack_top++;
}

inline BYTE *CannyEdgeDetector::StackPop()
{
    _MYASSERT(mpp_stack_top);

    mpp_stack_top--;
    return *mpp_stack_top;
}

inline void CannyEdgeDetector::CannyPushLabel(int &flag, BYTE *p_label, int label_step, bool is_pass_threshold)
{
    _MYASSERT(p_label);

    if (!flag && is_pass_threshold && p_label[-label_step] != 2)
    {
        StackPush(p_label);
        flag = 1;
    }
    else
    {
        *p_label = 0;
    }
}

bool CannyEdgeDetector::AllocateMemory(int width, int height, bool is_allocate_edge_image/* = true*/)
{
    // If is_allocate_edge_image = true, allocate the edge image at mp_edge_image.
    // If is_allocate_edge_image = false, let mp_edge_image be NULL (release it if allocated before),
    //                                    and the user should give the edge image when calling DetectEdge().

    ChAutoLock auto_lock(&m_data_lock);

    if (width <= 0 || height <= 0)
        return false;

    m_width = width;
    m_height = height;

    // Align the derivative buffers for SIMD in CannyEdgeDetect().
    int align_factor = 16 / sizeof(short);
    m_stride = ALIGN(width, align_factor);

    _ALIGNED_MALLOC_PTR(mp_x_derivative, short, m_stride * m_height);
    _ALIGNED_MALLOC_PTR(mp_y_derivative, short, m_stride * m_height);
    if (mp_x_derivative == NULL || mp_y_derivative == NULL)
        return false;

    hyReleaseImage(&mp_edge_image);

    if (is_allocate_edge_image)
    {
        mp_edge_image = hyCreateImage(hySize(width, height), HY_DEPTH_8U, 1);
        if (mp_edge_image == NULL)
            return false;
    }

    return true;
}

void CannyEdgeDetector::FreeMemory()
{
    ChAutoLock auto_lock(&m_data_lock);

    _ALIGNED_FREE_PTR(mp_x_derivative);
    _ALIGNED_FREE_PTR(mp_y_derivative);

    hyReleaseImage(&mp_edge_image);

    FreeStack();

    m_width = 0;
    m_height = 0;
    m_stride = 0;
}

void CannyEdgeDetector::FreeStack()
{
    _ALIGNED_FREE_PTR(mpp_stack);
    mpp_stack_top = NULL;
}

void CannyEdgeDetector::SetDisableIpp(bool is_disable_ipp)
{
    // Set true to force disable IPP functions.
    // ONLY call this for algorithm validation, or exclude IPP by some reasons.

    // This setting has no effect in ARM platforms. They just do not support IPP.

    m_is_disable_ipp = is_disable_ipp;
}

bool CannyEdgeDetector::DoDetectEdge(const BYTE *const p_grey_image_data,
                                     const int grey_image_stride, HyImage *p_edge_image)
{
    if (p_grey_image_data == NULL || mp_x_derivative == NULL || mp_y_derivative == NULL || p_edge_image == NULL)
        return false;
    if (m_width != p_edge_image->width || m_height != p_edge_image->height)
        return false;

    HY_ZEROIMAGE(p_edge_image);

    bool is_use_ipp = false;
#if defined(ANDROID_ARM) || defined(IOS_ARM)
    is_use_ipp = false; // No IPP here
#else
    is_use_ipp = (m_is_disable_ipp == false);
#endif

    bool is_success = false;
    if (is_use_ipp)
        is_success = DetectEdgeByIpp(p_grey_image_data, grey_image_stride, p_edge_image);
    else
        is_success = DetectEdgeByC(p_grey_image_data, grey_image_stride, p_edge_image);

    return is_success;
}

bool CannyEdgeDetector::DetectEdge(const HyImage *const p_grey_image, HyImage *p_edge_image/* = NULL*/)
{
    ChAutoLock auto_lock(&m_data_lock);

    // If p_edge_image is NULL, the detection result is put into mp_edge_image.
    // Otherwise, the result is put into p_edge_image.

    if (p_grey_image == NULL)
        return false;
    if (p_grey_image->width != m_width || p_grey_image->height != m_height)
        return false;

    HyImage *p_result_image = NULL;

    if (p_edge_image == NULL)
        p_result_image = mp_edge_image;
    else
        p_result_image = p_edge_image;

    if (p_result_image == NULL)
        return false;
    if (p_result_image->width != m_width || p_result_image->height != m_height)
        return false;

    bool is_success = DoDetectEdge(p_grey_image->imageData, p_grey_image->widthStep, p_result_image);

    return is_success;
}

bool CannyEdgeDetector::DetectEdgeByIpp(const BYTE *const p_grey_image_data,
                                        const int grey_image_stride, HyImage *p_edge_image)
{
#if defined(ANDROID_ARM) || defined(IOS_ARM)
    return false; // should not use this function.
#else
    // Perform Canny edge detection by IPP functions.
    // They are not implements in C codes.

    _MYASSERT(p_edge_image);

    const IppiSize roi = ippiSize(m_width, m_height);

    // prepare computational buffers
    int buffer_size = 0;
    int max_buffer_size = 0;

    ippiFilterSobelNegVertGetBufferSize_8u16s_C1R(roi, ippMskSize3x3, &max_buffer_size);
    ippiFilterSobelHorizGetBufferSize_8u16s_C1R(roi, ippMskSize3x3, &buffer_size);
    max_buffer_size = ch_Max(max_buffer_size, buffer_size);
    ippiCannyGetSize(roi, &buffer_size);
    max_buffer_size = ch_Max(max_buffer_size, buffer_size);

    BYTE *p_buffer = NEW_NO_EXCEPTION BYTE[max_buffer_size];
    if (p_buffer == NULL)
        return false;

    const int derivative_step = sizeof(short) * m_stride;

    ippiFilterSobelNegVertBorder_8u16s_C1R(p_grey_image_data, grey_image_stride,
                                           mp_x_derivative, derivative_step,
                                           roi, ippMskSize3x3, ippBorderRepl, 0, p_buffer);
    ippiFilterSobelHorizBorder_8u16s_C1R(p_grey_image_data, grey_image_stride,
                                         mp_y_derivative, derivative_step,
                                         roi, ippMskSize3x3, ippBorderRepl, 0, p_buffer);
    ippiCanny_16s8u_C1R(mp_x_derivative, derivative_step,
                        mp_y_derivative, derivative_step,
                        p_edge_image->imageData, p_edge_image->widthStep,
                        roi, m_low_threshold, m_high_threshold, p_buffer);

    delete [] p_buffer;

    return true;
#endif
}

bool CannyEdgeDetector::DetectEdgeByC(const BYTE *const p_grey_image_data,
                                      const int grey_image_stride, HyImage *p_edge_image)
{
    // Perform Canny edge detection by C code with available SIMD optimization.

    _MYASSERT(p_edge_image);

    FilterSobelBorder(m_width, m_height,
                      p_grey_image_data, grey_image_stride, 
                      mp_x_derivative, m_stride, mp_y_derivative, m_stride);

    CannyEdgeDetect(mp_x_derivative, m_stride, mp_y_derivative, m_stride,
                    p_edge_image->imageData, p_edge_image->widthStep,
                    m_width, m_height, m_low_threshold, m_high_threshold);

    return true;
}

void CannyEdgeDetector::StackResize(int new_size)
{
    // Resize the member stack to the new size and keep the old data.
    
    if (mpp_stack == NULL || mpp_stack_top == NULL)
        return;

    int stack_size = (int)(mpp_stack_top - mpp_stack);
    
    BYTE **pp_new_stack = NULL;
    _ALIGNED_MALLOC_PTR(pp_new_stack, BYTE *, new_size);
    memcpy(pp_new_stack, mpp_stack, sizeof(BYTE *) * stack_size);

    _ALIGNED_FREE_PTR(mpp_stack);
    mpp_stack = pp_new_stack;
    mpp_stack_top = mpp_stack + stack_size;
}

void CannyEdgeDetector::FilterSobelBorder(int width, int height,
                                          const BYTE *p_src_data, int src_step,
                                          short *p_dx_data, int dx_step, short *p_dy_data, int dy_step)
{
    // Perform 3x3 Sobel filter in x and y directions at the same time.

    _MYASSERT(width > 0);
    _MYASSERT(height > 0);
    _MYASSERT(p_src_data);
    _MYASSERT(p_dx_data);
    _MYASSERT(p_dy_data);

    // We need a 1D temp buffer in Step 2. Allocate it with error handling.
    short *p_result = NULL;
    _ALIGNED_MALLOC_PTR(p_result, short, width);
    if (p_result == NULL)
        return;

#if defined(ANDROID_ARM) || defined(IOS_ARM)
    const bool is_simd_optimization = g_is_support_NEON;
#else
    const bool is_simd_optimization = false; // not use SSE2 here
#endif

    // p_dx_data: Vertical kernel ("Negative" in IPP naming)
    //  1  0 -1      1
    //  2  0 -2  =   2  x  1  0 -1 
    //  1  0 -1      1

    // p_dy_data: Horizontal kernel
    //  1  2  1      1
    //  0  0  0  =   0  x  1  2  1 
    // -1 -2 -1     -1

    if (width == 1 || height == 1)
    {
        // Due to the padding, all result values will be 0.
        for (int y = 0; y < height; y++)
        {
            short *p_dx_scan = p_dx_data + y * dx_step;
            short *p_dy_scan = p_dy_data + y * dy_step;
            ZeroMemory(p_dx_scan, sizeof(short) * width);
            ZeroMemory(p_dy_scan, sizeof(short) * width);
        }

        return;
    }

    // 1. Perform vertical 1D kernel convolution.
    //    Put the results into the destination buffers (p_dx_data / p_dy_data).
    for (int y = 0; y < height; y++)
    {
        int prev_y = ch_Max(y - 1, 0);
        int next_y = ch_Min(y + 1, height - 1);

        const BYTE *p_prev_row = p_src_data + prev_y * src_step;
        const BYTE *p_curr_row = p_src_data + y * src_step;
        const BYTE *p_next_row = p_src_data + next_y * src_step;
        short *p_dx_scan = p_dx_data + y * dx_step;
        short *p_dy_scan = p_dy_data + y * dy_step;

        int x = 0;
        if (is_simd_optimization)
        {
#if defined(ANDROID_ARM) || defined(IOS_ARM)
            const int simd_width = (width / 16) * 16;
            for (; x < simd_width; x += 16)
            {
                uint8x16_t prev_row_pack = vld1q_u8(p_prev_row + x);
                uint8x16_t curr_row_pack = vld1q_u8(p_curr_row + x);
                uint8x16_t next_row_pack = vld1q_u8(p_next_row + x);

                int16x8_t v1_pack = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(prev_row_pack)));
                int16x8_t v2_pack = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(curr_row_pack)));
                int16x8_t v3_pack = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(next_row_pack)));

                int16x8_t dx_pack = vaddq_s16(vaddq_s16(v1_pack, v2_pack), vaddq_s16(v2_pack, v3_pack));
                int16x8_t dy_pack = vsubq_s16(v3_pack, v1_pack);
                vst1q_s16(p_dx_scan + x, dx_pack);
                vst1q_s16(p_dy_scan + x, dy_pack);

                v1_pack = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(prev_row_pack)));
                v2_pack = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(curr_row_pack)));
                v3_pack = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(next_row_pack)));

                dx_pack = vaddq_s16(vaddq_s16(v1_pack, v2_pack), vaddq_s16(v2_pack, v3_pack));
                dy_pack = vsubq_s16(v3_pack, v1_pack);
                vst1q_s16(p_dx_scan + x + 8, dx_pack);
                vst1q_s16(p_dy_scan + x + 8, dy_pack);
            }
#endif
        }

        for (; x < width; x++)
        {
            short v1 = (short)p_prev_row[x];
            short v2 = (short)p_curr_row[x];
            short v3 = (short)p_next_row[x];

            p_dx_scan[x] = v1 + v2 * 2 + v3;
            p_dy_scan[x] = v3 - v1;
        }
    }

    // 2. Perform horizontal 1D kernel convolution.
    // At here, width >= 2 and height >= 2.

    // p_dx_data
    for (int y = 0; y < height; y++)
    {
        short *p_dx_scan = p_dx_data + y * dx_step;

        // x = 0
        p_result[0] = p_dx_scan[1] - p_dx_scan[0];

        // 0 < x < width - 1
        int x = 1;
        if (is_simd_optimization)
        {
#if defined(ANDROID_ARM) || defined(IOS_ARM)
            const int arm_simd_width = 1 + ((width - 2) / 8) * 8;
            for (; x < arm_simd_width; x += 8)
            {
                int16x8_t v1_pack = vld1q_s16(p_dx_scan + x - 1);
                int16x8_t v3_pack = vld1q_s16(p_dx_scan + x + 1);

                vst1q_s16(p_result + x, vsubq_s16(v3_pack, v1_pack));
            }
#endif
        }
        for (; x < width - 1; x++)
            p_result[x] = p_dx_scan[x + 1] - p_dx_scan[x - 1];

        // x = width - 1
        p_result[width - 1] = p_dx_scan[width - 1] - p_dx_scan[width - 2];

        memcpy(p_dx_scan, p_result, sizeof(short) * width);
    }

    // p_dy_data
    for (int y = 0; y < height; y++)
    {
        short *p_dy_scan = p_dy_data + y * dy_step;

        // x = 0
        p_result[0] = p_dy_scan[0] * 3 + p_dy_scan[1];

        // 0 < x < width - 1
        int x = 1;
        if (is_simd_optimization)
        {
#if defined(ANDROID_ARM) || defined(IOS_ARM)
            const int arm_simd_width = 1 + ((width - 2) / 8) * 8;
            for (; x < arm_simd_width; x += 8)
            {
                int16x8_t v1_pack = vld1q_s16(p_dy_scan + x - 1);
                int16x8_t v2_pack = vld1q_s16(p_dy_scan + x);
                int16x8_t v3_pack = vld1q_s16(p_dy_scan + x + 1);

                vst1q_s16(p_result + x, vaddq_s16(vaddq_s16(v1_pack, v2_pack), vaddq_s16(v2_pack, v3_pack)));
            }
#endif
        }
        for (; x < width - 1; x++)
            p_result[x] = p_dy_scan[x - 1] + p_dy_scan[x] * 2 + p_dy_scan[x + 1];

        // x = width - 1
        p_result[width - 1] = p_dy_scan[width - 2] + p_dy_scan[width - 1] * 3;

        memcpy(p_dy_scan, p_result, sizeof(short) * width);
    }

    _ALIGNED_FREE_PTR(p_result);
}

void CannyEdgeDetector::CannyEdgeDetect(const short *p_dx_data, int dx_step,
                                        const short *p_dy_data, int dy_step,
                                        BYTE *p_edge_data, int edge_step,
                                        int width, int height, float low_threshold, float high_threshold)
{
    _MYASSERT(p_dx_data);
    _MYASSERT(p_dy_data);
    _MYASSERT(p_edge_data);
    _MYASSERT(width > 0);
    _MYASSERT(height > 0);

    const int low_value = ch_Round(floorf(low_threshold));
    const int high_value = ch_Round(floorf(ch_Max(low_threshold, high_threshold)));

    const int buffer_width = width + 2;
    const int buffer_height = height + 2;

    // The edge label map:
    //   0 - the pixel might belong to an edge
    //   1 - the pixel can not belong to an edge
    //   2 - the pixel does belong to an edge
    BYTE *p_label_map = new BYTE[buffer_width * buffer_height];
    memset(p_label_map, 1, buffer_width);
    memset(p_label_map + (height + 1) * buffer_width, 1, buffer_width);

    // magnitude buffers (3 rows)
    // We need to pad 1 pixel at both ends of each row,
    // but we add additional padding to align the memory for SIMD.
    const int padded_width = ALIGN(buffer_width + 3, 4); // 3 more pixels at left, pad to 4x at right

    int *p_magnitude_buffer = NULL;
    _ALIGNED_MALLOC_PTR(p_magnitude_buffer, int, padded_width * 3);
    int *pp_mag_buffers[3];
    pp_mag_buffers[0] = p_magnitude_buffer + 3;
    pp_mag_buffers[1] = pp_mag_buffers[0] + padded_width;
    pp_mag_buffers[2] = pp_mag_buffers[1] + padded_width;
    memset(pp_mag_buffers[0], 0, sizeof(int) * buffer_width);

    int max_stack_size = ch_Max(1 << 10, width * height / 10);
    _ALIGNED_MALLOC_PTR(mpp_stack, BYTE *, max_stack_size);
    mpp_stack_top = mpp_stack;

    /* sector numbers
       (Top-Left Origin)

        1   2   3
         *  *  *
          * * *
        0*******0
          * * *
         *  *  *
        3   2   1
    */

    const int CANNY_SHIFT = 15;
    const int tan225_factor = (int)(0.4142135623730950488016887242097 * (1 << CANNY_SHIFT) + 0.5);

#if defined(ANDROID_ARM) || defined(IOS_ARM)
    const bool is_simd_optimization = g_is_support_NEON;
#else
    const bool is_simd_optimization = g_is_support_SSE2;
#endif

    // calculate magnitude and angle of gradient, perform non-maxima suppression.
    for (int y = 0; y <= height; y++)
    {
        int *p_norm = pp_mag_buffers[(y > 0) + 1] + 1;
        if (y < height)
        {
            const short *p_dx = p_dx_data + y * dx_step;
            const short *p_dy = p_dy_data + y * dy_step;

            int x = 0;

            if (is_simd_optimization)
            {
#if defined(ANDROID_ARM) || defined(IOS_ARM)
                for (; x <= width - 8; x += 8)
                {
                    int16x8_t dx_pack = vld1q_s16(p_dx + x);
                    int16x8_t dy_pack = vld1q_s16(p_dy + x);
                    vst1q_s32(p_norm + x, vaddq_s32(vabsq_s32(vmovl_s16(vget_low_s16(dx_pack))),
                                                    vabsq_s32(vmovl_s16(vget_low_s16(dy_pack)))));
                    vst1q_s32(p_norm + x + 4, vaddq_s32(vabsq_s32(vmovl_s16(vget_high_s16(dx_pack))),
                                                        vabsq_s32(vmovl_s16(vget_high_s16(dy_pack)))));
                }
#else
                __m128i zero_pack = _mm_setzero_si128();
                for (; x <= width - 8; x += 8)
                {
                    __m128i dx_pack = _mm_load_si128((const __m128i *)(p_dx + x));
                    __m128i dy_pack = _mm_load_si128((const __m128i *)(p_dy + x));
                    dx_pack = _mm_max_epi16(dx_pack, _mm_sub_epi16(zero_pack, dx_pack));
                    dy_pack = _mm_max_epi16(dy_pack, _mm_sub_epi16(zero_pack, dy_pack));

                    __m128i norm_pack = _mm_add_epi32(_mm_unpacklo_epi16(dx_pack, zero_pack), _mm_unpacklo_epi16(dy_pack, zero_pack));
                    _mm_store_si128((__m128i *)(p_norm + x), norm_pack);

                    norm_pack = _mm_add_epi32(_mm_unpackhi_epi16(dx_pack, zero_pack), _mm_unpackhi_epi16(dy_pack, zero_pack));
                    _mm_store_si128((__m128i *)(p_norm + x + 4), norm_pack);
                }
#endif
            }

            for (; x < width; x++)
                p_norm[x] = ch_Abs((int)p_dx[x]) + ch_Abs((int)p_dy[x]);

            p_norm[-1] = 0;
            p_norm[width] = 0;
        }
        else
        {
            memset(p_norm - 1, 0, sizeof(int) * buffer_width);
        }

        // at the very beginning we do not have a complete ring
        // buffer of 3 magnitude rows for non-maxima suppression
        if (y == 0)
            continue;

        BYTE *p_label_scan = p_label_map + y * buffer_width + 1;
        p_label_scan[-1] = 1;
        p_label_scan[width] = 1;

        int *p_magnitude = pp_mag_buffers[1] + 1; // take the central row

        const short *p_x = p_dx_data + (y - 1) * dx_step;
        const short *p_y = p_dy_data + (y - 1) * dy_step;

        int stack_size = (int)(mpp_stack_top - mpp_stack);
        if (stack_size + width > max_stack_size)
        {
            max_stack_size = ch_Max(max_stack_size * 3 / 2, stack_size + width);
            StackResize(max_stack_size);
        }

        const int prev_magnitude_offset = (int)(pp_mag_buffers[0] - pp_mag_buffers[1]);
        const int next_magnitude_offset = (int)(pp_mag_buffers[2] - pp_mag_buffers[1]);

        int push_flag = 0;
        for (int x = 0; x < width; x++)
        {
            int m = p_magnitude[x];

            if (m > low_value)
            {
                int x_value = p_x[x];
                int y_value = p_y[x];
                int abs_x_value = ch_Abs(x_value);
                int abs_y_value = ch_Abs(y_value) << CANNY_SHIFT;

                int tan225_x = abs_x_value * tan225_factor;

                if (abs_y_value < tan225_x)
                {
                    if (m > p_magnitude[x - 1] && m >= p_magnitude[x + 1])
                    {
                        CannyPushLabel(push_flag, p_label_scan + x, buffer_width, m > high_value);
                        continue;
                    }
                }
                else
                {
                    int tan675_x = tan225_x + (abs_x_value << (CANNY_SHIFT + 1));
                    if (abs_y_value > tan675_x)
                    {
                        if (m > p_magnitude[x + prev_magnitude_offset] && m >= p_magnitude[x + next_magnitude_offset])
                        {
                            CannyPushLabel(push_flag, p_label_scan + x, buffer_width, m > high_value);
                            continue;
                        }
                    }
                    else
                    {
                        int sign = (x_value ^ y_value) < 0 ? -1 : 1;
                        if (m > p_magnitude[x + prev_magnitude_offset - sign] && m > p_magnitude[x + next_magnitude_offset + sign])
                        {
                            CannyPushLabel(push_flag, p_label_scan + x, buffer_width, m > high_value);
                            continue;
                        }
                    }
                }
            }

            push_flag = 0;
            p_label_scan[x] = 1;
        }

        // scroll the ring buffer
        p_magnitude = pp_mag_buffers[0];
        pp_mag_buffers[0] = pp_mag_buffers[1];
        pp_mag_buffers[1] = pp_mag_buffers[2];
        pp_mag_buffers[2] = p_magnitude;
    }

    _ALIGNED_FREE_PTR(p_magnitude_buffer);

    // now track the edges (hysteresis thresholding)
    const int label_neighbor_offset[8] = 
    {-1, 1, -buffer_width - 1, -buffer_width, -buffer_width + 1, buffer_width - 1, buffer_width, buffer_width + 1};

    while (mpp_stack_top > mpp_stack)
    {
        int stack_size = (int)(mpp_stack_top - mpp_stack);
        if (stack_size + 8 > max_stack_size)
        {
            max_stack_size = ch_Max(max_stack_size * 3 / 2, stack_size + 8);
            StackResize(max_stack_size);
        }

        BYTE *p_label = StackPop();

        for (int i = 0; i < 8; i++)
        {
            BYTE *p_neighbor = p_label + label_neighbor_offset[i];
            if (!p_neighbor[0])
                StackPush(p_neighbor);
        }
    }

    // Free the stack memory when its work is done.
    FreeStack();

    // the final pass, form the final image
    for (int y = 0; y < height; y++)
    {
        const BYTE *p_label_scan = p_label_map + (y + 1) * buffer_width + 1;
        BYTE *p_edge_scan = p_edge_data + y * edge_step;

        for (int x = 0; x < width; x++)
        {
            if (p_label_scan[x] == 2)
                p_edge_scan[x] = 255;
            else
                p_edge_scan[x] = 0;
        }
    }

    delete [] p_label_map;
}
