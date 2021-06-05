#include "stdafx.h"
#include "ipp_arm.h"
#include <float.h>

#define MACRO_CLAMP(val, uBound, lBound)  (val > uBound ? uBound : (val < lBound ? lBound : val))
#define UPSCALE_SHIFT               14
#define _PREC_SHIFT    10

const int SUPER_UPSCALE_SHIFT_BIT = 16;
const int SUPER_UPSCALE = (1 << SUPER_UPSCALE_SHIFT_BIT) - 1;
 
const float inverse_255 = 1.0f / 255.0f;

inline short shortfastRound(float flt)
{
    return (flt < 0.0f) ? (short) (flt - 0.5f) : (short) (flt + 0.5f);
}

inline unsigned short ushortfastRound(float flt)
{
    return (flt < 0.0f) ? (unsigned short) (flt - 0.5f) : (unsigned short) (flt + 0.5f);
}

// NOTE:
// 1. IPP malloc function allocate memory with 32-byte aligned.
//    We also implement the function to satisfy this memory alignment.
// 2. The allocate macro does not automatically free the old pointer,
//    and the free macro does not automatically set the pointer to NULL.
//    These behaviors are the same as IPP functions.
#ifdef UNIX_OS
#if IS_ANDROID
#define _IPP_ALIGNED_MALLOC_PTR(ptr, type, size)    { ptr = (type *)memalign(32, sizeof(type) * size); }
#else
#define _IPP_ALIGNED_MALLOC_PTR(ptr, type, size)    { void *_ptr = NULL; posix_memalign(&_ptr, 32, sizeof(type) * (size)); ptr = (type *)_ptr; }
#endif
#define _IPP_ALIGNED_FREE_PTR(ptr)                  { if (ptr) { free(ptr); } }
#else
#define _IPP_ALIGNED_MALLOC_PTR(ptr, type, size)    { ptr = (type *) _aligned_malloc(sizeof(type) * (size), 32); }
#define _IPP_ALIGNED_FREE_PTR(ptr)                  { if (ptr) { _aligned_free(ptr); } }
#endif

bool FloodFillTool::Initialize(int width, int height)
{
    if (m_is_initialized)
        UnInitialize();

    if (width <= 0 || height <= 0)
        return false;

    m_width = width;
    m_height = height;

    m_is_initialized = true;

    return true;
}

void FloodFillTool::UnInitialize()
{
    m_width = 0;
    m_height = 0;

    m_top_search_queue.Clear();
    m_bottom_search_queue.Clear();

    m_is_initialized = false;
}

bool FloodFillTool::FloodFill_4Connect(BYTE *p_image_data, int image_step, const HyPoint &seed,
                                       BYTE new_value, ConnectedComponent &region)
{
    return FloodFill_Kernel(p_image_data, image_step, seed, new_value, region, Connection_4Neighbor);
}

bool FloodFillTool::FloodFill_8Connect(BYTE *p_image_data, int image_step, const HyPoint &seed,
                                       BYTE new_value, ConnectedComponent &region)
{
    return FloodFill_Kernel(p_image_data, image_step, seed, new_value, region, Connection_8Neighbor);
}

inline int FloodFillTool::GetSeedPoints(const BYTE *p_data_list, BYTE seed_value, int *p_seed_list, int length)
{
    _MYASSERT(p_data_list);
    _MYASSERT(p_seed_list);

    // Given a list of pixel data, 
    // Find every connected parts of the data with value = seed_value,
    // and give one seed index for each connected part.
    // Return the number of seed indexs found this way.

    int seed_count = 0;

    int start_index = 0;

    // Find the start index of the first connected part in the data list.
    // If there are no data with value = seed_value, return 0.
    for (start_index = 0; start_index < length; start_index++)
        if (p_data_list[start_index] == seed_value)
            break;

    if (start_index == length)
        return 0;

    while (true)
    {
        // 1. Record the start index as the seed point of current part.
        p_seed_list[seed_count] = start_index;
        seed_count++;

        // 2. Move start_index to the position next to the last index of current part.
        for (; start_index < length; start_index++)
            if (p_data_list[start_index] != seed_value)
                break;

        // 3. Find the start index of the next connected part in the data list.
        for (; start_index < length; start_index++)
            if (p_data_list[start_index] == seed_value)
                break;

        // 4. If we reach the end of the list (either in 2. or 3.), return.
        if (start_index == length)
            return seed_count;
    }
}

bool FloodFillTool::FloodFill_Kernel(BYTE *p_image_data, int image_step, const HyPoint &seed,
                                     BYTE new_value, ConnectedComponent &region, FloodFillConnection connection)
{
    if (m_is_initialized == false)
        return false;

    if (p_image_data == NULL)
        return false;

    const int width = m_width;
    const int height = m_height;

    if (width <= 0 || height <= 0 || image_step < width)
        return false;

    if (seed.x < 0 || seed.y < 0 || seed.x >= width || seed.y >= height)
        return false;

    if (seed.x >= 65536 || seed.y >= 65536 || width > 65536 || height > 65536)
        return false; // Max ROI size: 65536 x 65536.

    _MYASSERT(connection == Connection_4Neighbor || connection == Connection_8Neighbor);

    // Initialize the values in region.
    region.area = 0;
    region.rect = hyRect(seed.x, seed.y, 0, 0);
    region.value = new_value;

    // Get the value of the seed pixel.
    // If the value = new_value, return true immediately.
    // (We do not find the region from the seed point in this case.)
    const BYTE seed_value = p_image_data[seed.y * image_step + seed.x];
    if (seed_value == new_value)
        return true;

    m_top_search_queue.Clear();
    m_bottom_search_queue.Clear();

    ShortPoint seed_point(seed.x, seed.y);
    m_top_search_queue.Push(seed_point);

    region.rect = hyRect(seed.x, seed.y, 1, 1);

    // Alternatively search in the two queue, until both of them are empty.
    while (m_top_search_queue.IsEmpty() == false || m_bottom_search_queue.IsEmpty() == false)
    {
        FloodFill_Kernel_Search(m_top_search_queue, p_image_data, image_step,
                                seed_value, new_value, region, connection);
        FloodFill_Kernel_Search(m_bottom_search_queue, p_image_data, image_step,
                                seed_value, new_value, region, connection);
    }

    return true;
}

void FloodFillTool::FloodFill_Kernel_Search(CLQueue<ShortPoint> &active_queue,
                                            BYTE *p_image_data, int image_step, BYTE seed_value, BYTE new_value, 
                                            ConnectedComponent &region, FloodFillConnection connection)
{
    _MYASSERT(p_image_data);

    // Search the region with only the points in active_queue.
    // active_queue may be m_top_search_queue or m_bottom_search_queue.
    // We still need to check the vertical neighbors in both side (top/bottom),
    // and push the neighbor points in either m_top_search_queue or m_bottom_search_queue.

    const int width = m_width;
    const int height = m_height;

    const int last_x = (width - 1);
    const int last_y = (height - 1);

    int min_x = region.rect.x;
    int max_x = region.rect.x + region.rect.width - 1;
    int min_y = region.rect.y;
    int max_y = region.rect.y + region.rect.height - 1;

    // Use scan line algorithm for the search of region.

    ChAutoPtr<int> seed_buffer(width);

    while (active_queue.IsEmpty() == false)
    {
        ShortPoint current_point = active_queue.Front();
        active_queue.Pop();

        int current_x = current_point.x;
        int current_y = current_point.y;
        BYTE *p_current_pixel = p_image_data + current_y * image_step + current_x;

        if (p_current_pixel[0] != seed_value)
        {
            // The pixel has been visited (and set to new_value).
            continue;
        }

        // Find the left / right bound of the scan line from current pixel.
        BYTE *p_current_scan_line = p_image_data + current_y * image_step;

        int left_x = current_x - 1;
        for (left_x = current_x - 1; left_x >= 0; left_x--)
            if (p_current_scan_line[left_x] != seed_value)
                break;

        left_x++;

        int right_x = current_x + 1;
        for (right_x = current_x + 1; right_x <= last_x; right_x++)
            if (p_current_scan_line[right_x] != seed_value)
                break;

        right_x--;

        int valid_length = right_x - left_x + 1;

        // Set all pixels with value = seed_value in the scan line to new_value.
        memset(p_current_scan_line + left_x, new_value, valid_length);

        // Record the area of the bounding rectangle of the region.
        region.area += valid_length;
        min_x = ch_Min(min_x, left_x);
        max_x = ch_Max(max_x, right_x);
        min_y = ch_Min(min_y, current_y);
        max_y = ch_Max(max_y, current_y);

        // Search for the top/bottom neighbor pixels with value = seed_value.
        // For 4-Connection, the search range is [left_x, right_x].
        // For 8-Connection, the search range is extended by 1 pixel at both sides.
        if (connection == Connection_8Neighbor)
        {
            left_x = ch_Max(left_x - 1, 0);
            right_x = ch_Min(right_x + 1, last_x);
            valid_length = right_x - left_x + 1;
        }

        // Find the neighboring pixels with value = seed_value.
        // We do not push all of those pixels into the stack.
        // Just insert the minimal pixels that are enough for line search.
        if (current_y > 0)
        {
            const BYTE *p_previous_scan_line = p_current_scan_line - image_step;
            const int previous_y = current_y - 1;

            int seed_count = GetSeedPoints(p_previous_scan_line + left_x, seed_value, seed_buffer, valid_length);

            for (int i = 0; i < seed_count; i++)
                m_top_search_queue.Push(ShortPoint(left_x + seed_buffer[i], previous_y));
        }
        if (current_y < last_y)
        {
            const BYTE *p_next_scan_line = p_current_scan_line + image_step;
            const int next_y = current_y + 1;

            int seed_count = GetSeedPoints(p_next_scan_line + left_x, seed_value, seed_buffer, valid_length);

            for (int i = 0; i < seed_count; i++)
                m_bottom_search_queue.Push(ShortPoint(left_x + seed_buffer[i], next_y));
        }
    }

    region.rect = hyRect(min_x, min_y, max_x - min_x + 1, max_y - min_y + 1);
}

///////////////////////////////////////////////////////////////////////////////////
// Implementation of IPP functions

IppStatus ippInit()
{
    // For now we don't have anything needed to do at initialization.
    // Just return ippStsNoErr.

    return ippStsNoErr;
}

Ipp8u *ippsMalloc_8u(int size)
{
    if (size <= 0)
        return NULL;

    BYTE *pointer = NULL;
    _IPP_ALIGNED_MALLOC_PTR(pointer, BYTE, size);

    return pointer;
}

void ippsFree(void *pointer)
{
    _IPP_ALIGNED_FREE_PTR(pointer);
}

IppStatus ippiCopy_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = p_src;
    BYTE *p_dst_scan = p_dst;

    for (int y = 0; y < height; y++)
    {
        memcpy(p_dst_scan, p_src_scan, width);

        p_src_scan += src_step;
        p_dst_scan += dst_step;
    }

    return ippStsNoErr;
}

IppStatus ippiCopy_8u_C3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = p_src;
    BYTE *p_dst_scan = p_dst;

    for (int y = 0; y < height; y++)
    {
        memcpy(p_dst_scan, p_src_scan, width * 3);

        p_src_scan += src_step;
        p_dst_scan += dst_step;
    }

    return ippStsNoErr;
}

IppStatus ippiCopy_8u_C4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = p_src;
    BYTE *p_dst_scan = p_dst;

    for (int y = 0; y < height; y++)
    {
        memcpy(p_dst_scan, p_src_scan, width * 4);

        p_src_scan += src_step;
        p_dst_scan += dst_step;
    }

    return ippStsNoErr;
}

IppStatus ippiCopy_8u_AC4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    // Copy only the color channels of 4-channel image. The 4th channel is unchanged.

    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = p_src;
    BYTE *p_dst_scan = p_dst;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
            memcpy(p_dst_scan + x * 4, p_src_scan + x * 4, 3);

        p_src_scan += src_step;
        p_dst_scan += dst_step;
    }

    return ippStsNoErr;
}

IppStatus ippiCopy_8u_C3AC4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    // Copy only the color channels from 3-channel to 4-channel image. The 4th channel is unchanged.

    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
	
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = p_src;
    BYTE *p_dst_scan = p_dst;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
            memcpy(p_dst_scan + x * 4, p_src_scan + x * 3, 3);

        p_src_scan += src_step;
        p_dst_scan += dst_step;
    }

    return ippStsNoErr;
}

IppStatus ippiCopy_8u_C3P3R(const Ipp8u *p_src, int src_step, Ipp8u *const p_dst[3], int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    if (p_dst[0] == NULL || p_dst[1] == NULL || p_dst[2] == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = p_src;
    BYTE *p_dst_scan_0 = p_dst[0];
    BYTE *p_dst_scan_1 = p_dst[1];
    BYTE *p_dst_scan_2 = p_dst[2];

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            p_dst_scan_0[x] = p_src_scan[x * 3];
            p_dst_scan_1[x] = p_src_scan[x * 3 + 1];
            p_dst_scan_2[x] = p_src_scan[x * 3 + 2];
        }

        p_src_scan += src_step;
        p_dst_scan_0 += dst_step;
        p_dst_scan_1 += dst_step;
        p_dst_scan_2 += dst_step;
    }

    return ippStsNoErr;
}

IppStatus ippiCopy_8u_P3C3R(const Ipp8u *const p_src[3], int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    if (p_src[0] == NULL || p_src[1] == NULL || p_src[2] == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan_0 = p_src[0];
    const BYTE *p_src_scan_1 = p_src[1];
    const BYTE *p_src_scan_2 = p_src[2];
    BYTE *p_dst_scan = p_dst;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            p_dst_scan[x * 3] = p_src_scan_0[x];
            p_dst_scan[x * 3 + 1] = p_src_scan_1[x];
            p_dst_scan[x * 3 + 2] = p_src_scan_2[x];
        }

        p_src_scan_0 += src_step;
        p_src_scan_1 += src_step;
        p_src_scan_2 += src_step;
        p_dst_scan += dst_step;
    }

    return ippStsNoErr;
}

IppStatus ippiCopy_8u_C1MR(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                           IppiSize roi_size, const Ipp8u *p_mask, int mask_step)
{
    if (p_src == NULL || p_dst == NULL || p_mask == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = p_src;
    BYTE *p_dst_scan = p_dst;
    const BYTE *p_mask_scan = p_mask;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
            if (p_mask_scan[x] > 0)
                p_dst_scan[x] = p_src_scan[x];

        p_src_scan += src_step;
        p_dst_scan += dst_step;
        p_mask_scan += mask_step;
    }

    return ippStsNoErr;
}

IppStatus ippiCopy_8u_C3MR(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                           IppiSize roi_size, const Ipp8u *p_mask, int mask_step)
{
    if (p_src == NULL || p_dst == NULL || p_mask == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = p_src;
    BYTE *p_dst_scan = p_dst;
    const BYTE *p_mask_scan = p_mask;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
            if (p_mask_scan[x] > 0)
                memcpy(p_dst_scan + x * 3, p_src_scan + x * 3, 3);

        p_src_scan += src_step;
        p_dst_scan += dst_step;
        p_mask_scan += mask_step;
    }

    return ippStsNoErr;
}

IppStatus ippiCopy_8u_C4MR(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                           IppiSize roi_size, const Ipp8u *p_mask, int mask_step)
{
    if (p_src == NULL || p_dst == NULL || p_mask == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = p_src;
    BYTE *p_dst_scan = p_dst;
    const BYTE *p_mask_scan = p_mask;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
            if (p_mask_scan[x] > 0)
                memcpy(p_dst_scan + x * 4, p_src_scan + x * 4, 4);

        p_src_scan += src_step;
        p_dst_scan += dst_step;
        p_mask_scan += mask_step;
    }

    return ippStsNoErr;
}

IppStatus ippiCopy_8u_AC4MR(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                            IppiSize roi_size, const Ipp8u *p_mask, int mask_step)
{
    // Copy only the color channels of 4-channel image. The 4th channel is unchanged.

    if (p_src == NULL || p_dst == NULL || p_mask == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = p_src;
    BYTE *p_dst_scan = p_dst;
    const BYTE *p_mask_scan = p_mask;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
            if (p_mask_scan[x] > 0)
                memcpy(p_dst_scan + x * 4, p_src_scan + x * 4, 3);

        p_src_scan += src_step;
        p_dst_scan += dst_step;
        p_mask_scan += mask_step;
    }

    return ippStsNoErr;
}

IppStatus ippiCopy_16s_C1R(const Ipp16s *p_src, int src_step, Ipp16s *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = (const BYTE *)p_src;
    BYTE *p_dst_scan = (BYTE *)p_dst;

    for (int y = 0; y < height; y++)
    {
        memcpy(p_dst_scan, p_src_scan, width * sizeof(Ipp16s));

        p_src_scan += src_step;
        p_dst_scan += dst_step;
    }

    return ippStsNoErr;
}

IppStatus ippiColorToGray_8u_C3C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size, const Ipp32f coeffs[3])
{
    if (p_src == NULL || p_dst == NULL || coeffs == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = p_src;
    BYTE *p_dst_scan = p_dst;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float b = (float)p_src_scan[3 * x];
            float g = (float)p_src_scan[3 * x + 1];
            float r = (float)p_src_scan[3 * x + 2];

            float result = b * coeffs[0] + g * coeffs[1] + r * coeffs[2];
            p_dst_scan[x] = (BYTE)FitInRange(ch_Round(result), 0, 255);
        }

        p_src_scan += src_step;
        p_dst_scan += dst_step;
    }

    return ippStsNoErr;
}

IppStatus ippiColorToGray_8u_AC4C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size, const Ipp32f coeffs[3])
{
    if (p_src == NULL || p_dst == NULL || coeffs == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = p_src;
    BYTE *p_dst_scan = p_dst;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float b = (float)p_src_scan[4 * x];
            float g = (float)p_src_scan[4 * x + 1];
            float r = (float)p_src_scan[4 * x + 2];

            float result = b * coeffs[0] + g * coeffs[1] + r * coeffs[2];
            p_dst_scan[x] = (BYTE)FitInRange(ch_Round(result), 0, 255);
        }

        p_src_scan += src_step;
        p_dst_scan += dst_step;
    }

    return ippStsNoErr;
}

IppStatus ippiDup_8u_C1C3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = p_src;
    BYTE *p_dst_scan = p_dst;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int pixel = p_src_scan[x];
            p_dst_scan[3 * x    ] = pixel;
            p_dst_scan[3 * x + 1] = pixel;
            p_dst_scan[3 * x + 2] = pixel;
        }

        p_src_scan += src_step;
        p_dst_scan += dst_step;
    }

    return ippStsNoErr;
}

IppStatus ippiDup_8u_C1C4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = p_src;
    BYTE *p_dst_scan = p_dst;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int pixel = p_src_scan[x];
            p_dst_scan[4 * x    ] = pixel;
            p_dst_scan[4 * x + 1] = pixel;
            p_dst_scan[4 * x + 2] = pixel;
            p_dst_scan[4 * x + 3] = pixel;
        }

        p_src_scan += src_step;
        p_dst_scan += dst_step;
    }

    return ippStsNoErr;
}

IppStatus ippiSwapChannels_8u_C3C4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                                    IppiSize roi_size, const int dst_order[4], Ipp8u value)
{
    // Swap the channel of a 3-channel image to a 4-channel image.
    // The fourth channel may be set by one of the input channel, a specified value, or keep its original values.

    if (p_src == NULL || p_dst == NULL || dst_order == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            for (int i = 0; i < 4; i++)
            {
                int order = dst_order[i];
                if (order == 0)
                    p_dst_scan[i] = p_src_scan[0];
                else if (order == 1)
                    p_dst_scan[i] = p_src_scan[1];
                else if (order == 2)
                    p_dst_scan[i] = p_src_scan[2];
                else if (order == 3)
                    p_dst_scan[i] = value;

                // For other values, p_dst_scan[i] is not set.
            }

            p_src_scan += 3;
            p_dst_scan += 4;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiCopy_8u_AC4C3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    // Convert 4-channel image to 3-channel image. Drop the alpha channel.

    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const BYTE *p_src_scan = p_src;
    BYTE *p_dst_scan = p_dst;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
            memcpy(p_dst_scan + x * 3, p_src_scan + x * 4, 3);

        p_src_scan += src_step;
        p_dst_scan += dst_step;
    }

    return ippStsNoErr;
}

IppStatus ippiSwapChannels_8u_C3IR(Ipp8u *p_src_dst, int src_dst_step, IppiSize roi_size, const int dst_order[3])
{
    // Swap the channel of a 3-channel image in-place.

    if (p_src_dst == NULL || dst_order == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    // For C3IR operation, dst_order[i] should be in {0, 1, 2}.
    for (int i = 0; i < 3; i++)
        if (dst_order[i] < 0 || dst_order[i] >= 3)
            return ippStsChannelOrderErr;

    BYTE buffer[3] = {0};
    BYTE *p_src_dst_scan = p_src_dst;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            memcpy(buffer, p_src_dst_scan + 3 * x, 3);

            p_src_dst_scan[3 * x    ] = buffer[dst_order[0]];
            p_src_dst_scan[3 * x + 1] = buffer[dst_order[1]];
            p_src_dst_scan[3 * x + 2] = buffer[dst_order[2]];
        }

        p_src_dst_scan += src_dst_step;
    }

    return ippStsNoErr;
}

IppStatus ippiResizeGetBufSize(IppiRect src_roi, IppiRect dst_roi, int channels, int interpolation, int *p_size)
{
    if (p_size == NULL)
        return ippStsNullPtrErr;

    if (channels != 1 && channels != 3 && channels != 4)
        return ippStsNumChannelsErr;

    const int src_width = src_roi.width;
    const int src_height = src_roi.height;
    const int dst_width = dst_roi.width;
    const int dst_height = dst_roi.height;

    if (src_width <= 0 || src_height <= 0 || dst_width <= 0 || dst_height <= 0)
        return ippStsSizeErr;

    if (interpolation == IPPI_INTER_NN)
    {
        if (channels == 1)
        {
            // No additional buffer required. We still return 1 as a dummy positive size.
            *p_size = 1;
        }
        else
        {
            // We only support NN interpolation for 1-channel image.
            return ippStsNumChannelsErr;
        }
    }
    else if (interpolation == IPPI_INTER_LINEAR)
    {
        // 5 buffers are required, return the total size.
        int total_buffer_size = sizeof(int) * dst_width;
        total_buffer_size += sizeof(int) * dst_height;
        total_buffer_size += sizeof(short) * 2 * dst_width;
        total_buffer_size += sizeof(short) * 2 * dst_height;
        total_buffer_size += sizeof(short) * src_width * channels * dst_height;

        *p_size = total_buffer_size;
    }
    else if (interpolation == IPPI_INTER_SUPER)
    {
        if (src_width >= dst_width && src_height >= dst_height)
        {
            // 7 buffers are required, return the total size.
            int total_buffer_size = sizeof(unsigned short) * src_width * channels * dst_height;
            total_buffer_size += sizeof(short) * dst_width;
            total_buffer_size += sizeof(short) * dst_width;
            total_buffer_size += sizeof(unsigned short) * 2 * dst_width;
            total_buffer_size += sizeof(short) * dst_height;
            total_buffer_size += sizeof(short) * dst_height;
            total_buffer_size += sizeof(unsigned short) * 2 * dst_height;

            *p_size = total_buffer_size;
        }
        else
        {
            // For Super resize, the source ROI size must be >= destination ROI size.
            return ippStsSizeErr;
        }
    }
    else
    {
        return ippStsInterpolationErr;
    }

    return ippStsNoErr;
}

void BilinearInitialize(int src_width, int src_height, int dst_width, int dst_height,
                        int channels, BYTE *p_buffer, short *&p_temp_buffer,
                        int *&p_bilinear_horizontal_position, short *&p_bilinear_horizontal_weight, 
                        int *&p_bilinear_vertical_position, short *&p_bilinear_vertical_weight)
{
    // Arrange the buffer pointers and precompute the coefficients for bilinear image resize.

    // WARNING: Assuming that the whole buffer is 32-aligned (by ippsMalloc_8u()),
    //          Each int/short element must be 4/2 aligned to ensure correct alignment in Android.
    //          We do this by putting int buffers before the short buffers.

    _MYASSERT(p_buffer);

    BYTE *p_buffer_start = p_buffer;
    p_bilinear_horizontal_position = (int *)p_buffer_start;

    p_buffer_start += sizeof(int) * dst_width;
    p_bilinear_vertical_position = (int *)p_buffer_start;

    p_buffer_start += sizeof(int) * dst_height;
    p_bilinear_horizontal_weight = (short *)p_buffer_start;

    p_buffer_start += sizeof(short) * 2 * dst_width;
    p_bilinear_vertical_weight = (short *)p_buffer_start;

    p_buffer_start += sizeof(short) * 2 * dst_height;
    p_temp_buffer = (short *)p_buffer_start;

    int *p_position = p_bilinear_horizontal_position;
    short *p_weight = p_bilinear_horizontal_weight;

    for (int i = 0; i < dst_width; i++)
    {
        int tmp = -1;
        float center = -1.0f;

        center = (float)((i + 0.5f) * src_width) / dst_width - 0.5f;
        tmp = (int)center;

        if (tmp < src_width - 1)
        {
            p_position[i] = tmp;
            p_weight[2 * i + 1] = shortfastRound((center - tmp) * (1 << UPSCALE_SHIFT));
            p_weight[2 * i] = (1 << UPSCALE_SHIFT) - p_weight[2 * i + 1];
        }
        else
        {
            p_position[i] = src_width - 2;
            p_weight[2 * i] = 0;
            p_weight[2 * i + 1] = (1 << UPSCALE_SHIFT);
        }
    }

    p_position = p_bilinear_vertical_position;
    p_weight = p_bilinear_vertical_weight;

    for (int i = 0; i < dst_height; i++)
    {
        int tmp = -1;
        float center = -1.f;

        center = (float)((i + 0.5f) * src_height) / dst_height - 0.5f;
        tmp = (int)center;

        if (tmp < src_height - 1)
        {
            p_position[i] = tmp;
            p_weight[2 * i + 1] = shortfastRound((center - tmp) * (1 << UPSCALE_SHIFT));
            p_weight[2 * i] = (1 << UPSCALE_SHIFT) - p_weight[2 * i + 1];
        }
        else
        {
            p_position[i] = src_height - 2;
            p_weight[2 * i] = 0;
            p_weight[2 * i + 1] = (1 << UPSCALE_SHIFT);
        }
    }
}

void BilinearVertical(const BYTE *p_src_start, int src_width, int src_height, int src_step, int channels,
                      int dst_height, short *p_temp_buffer,
                      const int *p_bilinear_vertical_position, const short *p_bilinear_vertical_weight)
{
    _MYASSERT(p_src_start);
    _MYASSERT(p_temp_buffer);
    _MYASSERT(p_bilinear_vertical_position);
    _MYASSERT(p_bilinear_vertical_weight);

    const int tmp_step = src_width * channels;

    const int *p_position = p_bilinear_vertical_position;
    const short *p_weight = p_bilinear_vertical_weight;
    short *p_temp_scan = p_temp_buffer;

    for (int y = 0; y < dst_height; y++)
    {
        const BYTE *p_src_scan = p_src_start + p_position[y] * src_step;

        for (int x = 0; x < src_width; x++)
        {
            for (int i = 0; i < channels; i++)
            {
                int sum = p_src_scan[x * channels + i] * p_weight[2 * y];
                sum += p_src_scan[x * channels + i + src_step] * p_weight[2 * y + 1];

                p_temp_scan[x * channels + i] = (short)(sum >> (UPSCALE_SHIFT - 6));
            }
        }

        p_temp_scan += tmp_step;
    }
}

void BilinearHorizontal(BYTE *p_dst_start, int dst_width, int dst_height, int dst_step, int channels,
                        int src_width, const IppiRect &dst_roi_bound, short *p_temp_buffer,
                        const int *p_bilinear_horizontal_position, const short *p_bilinear_horizontal_weight)
{
    _MYASSERT(p_dst_start);
    _MYASSERT(p_temp_buffer);
    _MYASSERT(p_bilinear_horizontal_position);
    _MYASSERT(p_bilinear_horizontal_weight);

    const int tmp_step = src_width * channels;

    const int *p_position = p_bilinear_horizontal_position;
    const short *p_weight = p_bilinear_horizontal_weight;

    const int x_start = dst_roi_bound.x;
    const int y_start = dst_roi_bound.y;
    const int x_end = x_start + dst_roi_bound.width;
    const int y_end = y_start + dst_roi_bound.height;
    _MYASSERT(x_start >= 0);
    _MYASSERT(y_start >= 0);
    _MYASSERT(x_end <= dst_width);
    _MYASSERT(y_end <= dst_height);

    for (int y = y_start; y < y_end; y++)
    {
        short *p_temp_scan = p_temp_buffer + y * tmp_step;
        BYTE *p_dst_scan = p_dst_start + y * dst_step;

        for (int x = x_start; x < x_end; x++)
        {
            const int pos = p_position[x];
            for (int i = 0; i < channels; i++)
            {
                int sum = p_temp_scan[pos * channels + i] * p_weight[2 * x];
                sum += p_temp_scan[(pos + 1) * channels + i] * p_weight[2 * x + 1];

                sum = ((sum + (1 << (UPSCALE_SHIFT + 5))) >> (UPSCALE_SHIFT + 6));
                p_dst_scan[x * channels + i] = (BYTE)MACRO_CLAMP(sum, 255, 0);
            }
        }
    }
}

void SuperResizeInitialize(int src_width, int src_height, int dst_width, int dst_height,
                           int channels, BYTE *p_buffer, unsigned short *&p_super_middle,
                           short *&p_super_horizontal_position, short *&p_super_horizontal_unit_weight_skip,
                           unsigned short *&p_super_horizontal_weight,
                           short *&p_super_vertical_position, short *&p_super_vertical_unit_weight_skip,
                           unsigned short *&p_super_vertical_weight,
                           float &super_vertical_scale, float &super_horizontal_scale)
{
    // Arrange the buffer pointers and precompute the coefficients for super image resize.

    _MYASSERT(p_buffer);

    BYTE *p_buffer_start = p_buffer;
    p_super_middle = (unsigned short *)p_buffer_start;

    p_buffer_start += sizeof(unsigned short) * src_width * channels * dst_height;
    p_super_horizontal_position = (short *)p_buffer_start;

    p_buffer_start += sizeof(short) * dst_width;
    p_super_horizontal_unit_weight_skip = (short *)p_buffer_start;

    p_buffer_start += sizeof(short) * dst_width;
    p_super_horizontal_weight = (unsigned short *)p_buffer_start;

    p_buffer_start += sizeof(unsigned short) * 2 * dst_width;
    p_super_vertical_position = (short *)p_buffer_start;

    p_buffer_start += sizeof(short) * dst_height;
    p_super_vertical_unit_weight_skip = (short *)p_buffer_start;

    p_buffer_start += sizeof(short) * dst_height;
    p_super_vertical_weight = (unsigned short *)p_buffer_start;

    const float horizontal_share = (float) src_width / dst_width;
    const float vertical_share = (float) src_height / dst_height;

    short *p_position = p_super_horizontal_position;
    unsigned short *p_weight = p_super_horizontal_weight;
    short *p_weight_skip = p_super_horizontal_unit_weight_skip;

    super_vertical_scale = (float)(SUPER_UPSCALE) / vertical_share;
    super_horizontal_scale = (float)(SUPER_UPSCALE) / horizontal_share;

    unsigned short vertical_unit_weight = ushortfastRound(super_vertical_scale);
    unsigned short horizontal_unit_weight = ushortfastRound(super_horizontal_scale);

    float position_start = 0.0f;

    for (int i = 0; i < dst_width; i++)
    {
        float position_end = position_start + horizontal_share;

        short trunc_position_start = (short)position_start;
        short trunc_position_end = (short)position_end;

        p_position[i] = trunc_position_start;
        p_weight[2 * i] = ushortfastRound(((float) trunc_position_start + 1 - position_start) * super_horizontal_scale);
        
        if (trunc_position_end >= src_width)
        {
            trunc_position_end = src_width - 1;
            p_weight[2 * i + 1] = horizontal_unit_weight;
        }
        else
        {
            p_weight[2 * i + 1] = ushortfastRound((position_end - (float)trunc_position_end) * super_horizontal_scale);
        }

        p_weight_skip[i] = trunc_position_end - trunc_position_start - 1;

        position_start += horizontal_share;
    }

    p_position = p_super_vertical_position;
    p_weight = p_super_vertical_weight;
    p_weight_skip = p_super_vertical_unit_weight_skip;

    position_start = 0.0f;

    for (int i = 0; i < dst_height; i++)
    {
        float position_end = position_start + vertical_share;

        short trunc_position_start = (short)position_start;
        short trunc_position_end = (short)position_end;

        p_position[i] = trunc_position_start;

        p_weight[2 * i] = ushortfastRound(((float)trunc_position_start + 1 - position_start) * super_vertical_scale);

        if (trunc_position_end >= src_height)
        {
            trunc_position_end = src_height - 1;
            p_weight[2 * i + 1] = vertical_unit_weight;
        }
        else
        {
            p_weight[2 * i + 1] = ushortfastRound((position_end - (float)trunc_position_end) * super_vertical_scale);
        }

        p_weight_skip[i] = trunc_position_end - trunc_position_start - 1;

        position_start += vertical_share;
    }
}

void SuperResizeVertical(const BYTE *p_src_start, int src_width, int src_height, int src_step, int channels,
                         int dst_height, unsigned short *p_super_middle,
                         const short *p_super_vertical_position, const short *p_super_vertical_unit_weight_skip,
                         const unsigned short *p_super_vertical_weight, float super_vertical_scale)
{
    _MYASSERT(p_src_start);
    _MYASSERT(p_super_middle);
    _MYASSERT(p_super_vertical_position);
    _MYASSERT(p_super_vertical_unit_weight_skip);
    _MYASSERT(p_super_vertical_weight);

    const int middle_step = src_width * channels;

    const short *p_position = p_super_vertical_position;
    const short *p_unit_weight_skip = p_super_vertical_unit_weight_skip;
    const unsigned short *p_weight = p_super_vertical_weight;
    unsigned short *p_middle_scan = p_super_middle;

    unsigned int unit_weight = ushortfastRound(super_vertical_scale);

    for (int y = 0; y < dst_height; y++)
    {
        const BYTE *p_src_scan = p_src_start + p_position[y] * src_step;

        short unit_weight_skip = p_unit_weight_skip[y];

        for (int x = 0; x < src_width; x++)
        {
            for (int i = 0; i < channels; i++)
            {
                unsigned int sum = p_src_scan[x * channels + i] * p_weight[2 * y];

                unsigned int skip_sum = 0;
                for (int j = 0; j < unit_weight_skip; j++)
                    skip_sum += p_src_scan[x * channels + i + (1 + j) * src_step];
                
                sum += skip_sum * unit_weight;
                sum += p_src_scan[x * channels + i + (1 + unit_weight_skip) * src_step] * p_weight[2 * y + 1];

                p_middle_scan[x * channels + i] = (unsigned short)(sum >> 8);
            }
        }

        p_middle_scan += middle_step;
    }
}

void SuperResizeHorizontal(BYTE *p_dst_start, int dst_width, int dst_height, int dst_step, int channels,
                           int src_width, const IppiRect &dst_roi_bound, unsigned short *p_super_middle,
                           const short *p_super_horizontal_position, const short *p_super_horizontal_unit_weight_skip,
                           const unsigned short *p_super_horizontal_weight, float super_horizontal_scale)
{
    _MYASSERT(p_dst_start);
    _MYASSERT(p_super_middle);
    _MYASSERT(p_super_horizontal_position);
    _MYASSERT(p_super_horizontal_unit_weight_skip);
    _MYASSERT(p_super_horizontal_weight);

    const int middle_step = src_width * channels;

    const short *p_position = p_super_horizontal_position;
    const unsigned short *p_weight = p_super_horizontal_weight;
    const short *p_unit_weight_skip = p_super_horizontal_unit_weight_skip;

    const int x_start = dst_roi_bound.x;
    const int y_start = dst_roi_bound.y;
    const int x_end = x_start + dst_roi_bound.width;
    const int y_end = y_start + dst_roi_bound.height;
    _MYASSERT(x_start >= 0);
    _MYASSERT(y_start >= 0);
    _MYASSERT(x_end <= dst_width);
    _MYASSERT(y_end <= dst_height);

    unsigned int unit_weight = ushortfastRound(super_horizontal_scale);

    int reverse_shift = 2 * SUPER_UPSCALE_SHIFT_BIT - 8;

    for (int y = y_start; y < y_end; y++)
    {
        unsigned short *p_middle_scan = p_super_middle + y * middle_step;
        BYTE *p_dst_scan = p_dst_start + y * dst_step;

        for (int x = x_start; x < x_end; x++)
        {
            const short unit_weight_skip = p_unit_weight_skip[x];
            const int pos = p_position[x];

            for (int i = 0; i < channels; i++)
            {
                unsigned int sum = p_middle_scan[pos * channels + i] * p_weight[2 * x];

                unsigned int skip_sum = 0;

                for (int j = 0; j < unit_weight_skip; j++)
                    skip_sum += p_middle_scan[(pos + j + 1) * channels + i];

                sum += skip_sum * unit_weight;
                sum += p_middle_scan[(pos + unit_weight_skip + 1) * channels + i] * p_weight[2 * x + 1];

                sum = (sum + (1 << (reverse_shift - 1))) >> (reverse_shift);

                p_dst_scan[x * channels + i] = (BYTE)ch_Min(sum, (unsigned int)255); 
            }
        }
    }
}

IppStatus ippiResizeSqrPixel_8u_C1R(const Ipp8u *p_src, IppiSize src_size, int src_step, IppiRect src_roi,
                                    Ipp8u* p_dst, int dst_step, IppiRect dst_roi,
                                    double x_factor, double y_factor, double x_shift, double y_shift, int interpolation, Ipp8u *p_buffer)
{
    if (p_src == NULL || p_dst == NULL || p_buffer == NULL)
        return ippStsNullPtrErr;

    if (x_factor <= 0.0 || y_factor <= 0.0)
        return ippStsResizeFactorErr;

    // For Super resize, the factors cannot be greater than 1.0.
    if ((interpolation == IPPI_INTER_SUPER) && (x_factor > 1.0 || y_factor > 1.0))
        return ippStsResizeFactorErr;

    if (src_roi.width <= 0 || src_roi.height <= 0 || dst_roi.width <= 0 || dst_roi.height <= 0)
        return ippStsSizeErr;

    // Locate source rectangle and transform by x_factor/y_factor/x_shift/y_shift.
    const int src_left = src_roi.x;
    const int src_top = src_roi.y;
    const int src_width = src_roi.width;
    const int src_height = src_roi.height;
    const int src_right = src_left + src_width;
    const int src_bottom = src_top + src_height;

    const int dst_left   = ch_Round(src_left   * x_factor + x_shift);
    const int dst_top    = ch_Round(src_top    * y_factor + y_shift);
    const int dst_right  = ch_Round(src_right  * x_factor + x_shift);
    const int dst_bottom = ch_Round(src_bottom * y_factor + y_shift);
    const int dst_width = dst_right - dst_left;
    const int dst_height = dst_bottom - dst_top;

    const BYTE *p_src_start = p_src + src_top * src_step + src_left;
    BYTE *p_dst_start = p_dst + dst_top * dst_step + dst_left;

    // We interpolate the source image ROI (src_roi)
    // to the destination region computed by the transform parameters.
    // However, only the pixels in the destination image ROI (dst_roi) are filled.
    const int dst_roi_left = dst_roi.x;
    const int dst_roi_top = dst_roi.y;
    const int dst_roi_right = dst_roi_left + dst_roi.width;
    const int dst_roi_bottom = dst_roi_top + dst_roi.height;

    const int x_start = ch_Max(dst_roi_left - dst_left, 0);
    const int x_end = ch_Min(dst_roi_right - dst_left, dst_width);
    const int y_start = ch_Max(dst_roi_top - dst_top, 0);
    const int y_end = ch_Min(dst_roi_bottom - dst_top, dst_height);

    if (interpolation == IPPI_INTER_NN)
    {
        BYTE *p_dst_scan = p_dst_start;
        const float dst_center_x = (dst_width - 1) / 2.0f;
        const float dst_center_y = (dst_height - 1) / 2.0f;
        const float src_center_x = (src_width - 1) / 2.0f;
        const float src_center_y = (src_height - 1) / 2.0f;
        const float round = 0.5f - FLT_EPSILON;
        double ratio_x = dst_width / (double)src_width;
        double ratio_y = dst_height / (double)src_height;

        for (int y = y_start; y < y_end; y++)
        {
            const int src_y = (int)((y - dst_center_y) / ratio_y + src_center_y + round);

            const BYTE *p_src_scan = p_src_start + src_y * src_step;

            for (int x = x_start; x < x_end; x++)
            {
                const int src_x = (int)((x - dst_center_x) / ratio_x + src_center_x + round);

                p_dst_scan[x] = p_src_scan[src_x];
            }

            p_dst_scan += dst_step;
        }
    }
    else if (interpolation == IPPI_INTER_LINEAR)
    {
        short *p_temp_buffer = NULL;
        int *p_bilinear_horizontal_position = NULL;
        short *p_bilinear_horizontal_weight = NULL;
        int *p_bilinear_vertical_position = NULL;
        short *p_bilinear_vertical_weight = NULL;

        BilinearInitialize(src_width, src_height, dst_width, dst_height, 1, p_buffer, p_temp_buffer,
                           p_bilinear_horizontal_position, p_bilinear_horizontal_weight, 
                           p_bilinear_vertical_position, p_bilinear_vertical_weight);

        BilinearVertical(p_src_start, src_width, src_height, src_step, 1, dst_height,
                         p_temp_buffer, p_bilinear_vertical_position, p_bilinear_vertical_weight);

        IppiRect dst_roi_bound = {x_start, y_start, x_end - x_start, y_end - y_start};
        BilinearHorizontal(p_dst_start, dst_width, dst_height, dst_step, 1, src_width, dst_roi_bound,
                           p_temp_buffer, p_bilinear_horizontal_position, p_bilinear_horizontal_weight);
    }
    else if (interpolation == IPPI_INTER_SUPER)
    {
        unsigned short *p_super_middle = NULL;
        short *p_super_horizontal_position = NULL;
        short *p_super_horizontal_unit_weight_skip = NULL;
        unsigned short *p_super_horizontal_weight = NULL;
        short *p_super_vertical_position = NULL;
        short *p_super_vertical_unit_weight_skip = NULL;
        unsigned short *p_super_vertical_weight = NULL;

        float super_vertical_scale = 0.0f;
        float super_horizontal_scale = 0.0f;

        SuperResizeInitialize(src_width, src_height, dst_width, dst_height, 1, p_buffer, p_super_middle,
                              p_super_horizontal_position, p_super_horizontal_unit_weight_skip, p_super_horizontal_weight,
                              p_super_vertical_position, p_super_vertical_unit_weight_skip, p_super_vertical_weight,
                              super_vertical_scale, super_horizontal_scale);

        SuperResizeVertical(p_src_start, src_width, src_height, src_step, 1,
                            dst_height, p_super_middle,
                            p_super_vertical_position, p_super_vertical_unit_weight_skip,
                            p_super_vertical_weight, super_vertical_scale);

        IppiRect dst_roi_bound = {x_start, y_start, x_end - x_start, y_end - y_start};
        SuperResizeHorizontal(p_dst_start, dst_width, dst_height, dst_step, 1,
                              src_width, dst_roi_bound, p_super_middle,
                              p_super_horizontal_position, p_super_horizontal_unit_weight_skip,
                              p_super_horizontal_weight, super_horizontal_scale);
    }
    else
    {
        return ippStsInterpolationErr;
    }

    return ippStsNoErr;
}

IppStatus ippiResizeSqrPixel_8u_C3R(const Ipp8u *p_src, IppiSize src_size, int src_step, IppiRect src_roi,
                                    Ipp8u* p_dst, int dst_step, IppiRect dst_roi,
                                    double x_factor, double y_factor, double x_shift, double y_shift, int interpolation, Ipp8u *p_buffer)
{
    if (p_src == NULL || p_dst == NULL || p_buffer == NULL)
        return ippStsNullPtrErr;

    if (x_factor <= 0.0 || y_factor <= 0.0)
        return ippStsResizeFactorErr;

    // For Super resize, the factors cannot be greater than 1.0.
    if ((interpolation == IPPI_INTER_SUPER) && (x_factor > 1.0 || y_factor > 1.0))
        return ippStsResizeFactorErr;

    if (src_roi.width <= 0 || src_roi.height <= 0 || dst_roi.width <= 0 || dst_roi.height <= 0)
        return ippStsSizeErr;

    // Locate source rectangle and transform by x_factor/y_factor/x_shift/y_shift.
    const int src_left = src_roi.x;
    const int src_top = src_roi.y;
    const int src_width = src_roi.width;
    const int src_height = src_roi.height;
    const int src_right = src_left + src_width;
    const int src_bottom = src_top + src_height;

    const int dst_left   = ch_Round(src_left   * x_factor + x_shift);
    const int dst_top    = ch_Round(src_top    * y_factor + y_shift);
    const int dst_right  = ch_Round(src_right  * x_factor + x_shift);
    const int dst_bottom = ch_Round(src_bottom * y_factor + y_shift);
    const int dst_width = dst_right - dst_left;
    const int dst_height = dst_bottom - dst_top;

    const BYTE *p_src_start = p_src + src_top * src_step + src_left * 3;
    BYTE *p_dst_start = p_dst + dst_top * dst_step + dst_left * 3;

    // We interpolate the source image ROI (src_roi)
    // to the destination region computed by the transform parameters.
    // However, only the pixels in the destination image ROI (dst_roi) are filled.
    const int dst_roi_left = dst_roi.x;
    const int dst_roi_top = dst_roi.y;
    const int dst_roi_right = dst_roi_left + dst_roi.width;
    const int dst_roi_bottom = dst_roi_top + dst_roi.height;

    const int x_start = ch_Max(dst_roi_left - dst_left, 0);
    const int x_end = ch_Min(dst_roi_right - dst_left, dst_width);
    const int y_start = ch_Max(dst_roi_top - dst_top, 0);
    const int y_end = ch_Min(dst_roi_bottom - dst_top, dst_height);

    if (interpolation == IPPI_INTER_LINEAR)
    {
        short *p_temp_buffer = NULL;
        int *p_bilinear_horizontal_position = NULL;
        short *p_bilinear_horizontal_weight = NULL;
        int *p_bilinear_vertical_position = NULL;
        short *p_bilinear_vertical_weight = NULL;

        BilinearInitialize(src_width, src_height, dst_width, dst_height, 3, p_buffer, p_temp_buffer,
                           p_bilinear_horizontal_position, p_bilinear_horizontal_weight, 
                           p_bilinear_vertical_position, p_bilinear_vertical_weight);

        BilinearVertical(p_src_start, src_width, src_height, src_step, 3, dst_height,
                         p_temp_buffer, p_bilinear_vertical_position, p_bilinear_vertical_weight);

        IppiRect dst_roi_bound = {x_start, y_start, x_end - x_start, y_end - y_start};
        BilinearHorizontal(p_dst_start, dst_width, dst_height, dst_step, 3, src_width, dst_roi_bound,
                           p_temp_buffer, p_bilinear_horizontal_position, p_bilinear_horizontal_weight);
    }
    else if (interpolation == IPPI_INTER_SUPER)
    {
        unsigned short *p_super_middle = NULL;
        short *p_super_horizontal_position = NULL;
        short *p_super_horizontal_unit_weight_skip = NULL;
        unsigned short *p_super_horizontal_weight = NULL;
        short *p_super_vertical_position = NULL;
        short *p_super_vertical_unit_weight_skip = NULL;
        unsigned short *p_super_vertical_weight = NULL;

        float super_vertical_scale = 0.0f;
        float super_horizontal_scale = 0.0f;

        SuperResizeInitialize(src_width, src_height, dst_width, dst_height, 3, p_buffer, p_super_middle,
                              p_super_horizontal_position, p_super_horizontal_unit_weight_skip, p_super_horizontal_weight,
                              p_super_vertical_position, p_super_vertical_unit_weight_skip, p_super_vertical_weight,
                              super_vertical_scale, super_horizontal_scale);

        SuperResizeVertical(p_src_start, src_width, src_height, src_step, 3,
                            dst_height, p_super_middle,
                            p_super_vertical_position, p_super_vertical_unit_weight_skip,
                            p_super_vertical_weight, super_vertical_scale);

        IppiRect dst_roi_bound = {x_start, y_start, x_end - x_start, y_end - y_start};
        SuperResizeHorizontal(p_dst_start, dst_width, dst_height, dst_step, 3,
                              src_width, dst_roi_bound, p_super_middle,
                              p_super_horizontal_position, p_super_horizontal_unit_weight_skip,
                              p_super_horizontal_weight, super_horizontal_scale);
    }
    else
    {
        return ippStsInterpolationErr;
    }

    return ippStsNoErr;
}

IppStatus ippiResizeSqrPixel_8u_C4R(const Ipp8u *p_src, IppiSize src_size, int src_step, IppiRect src_roi,
                                    Ipp8u* p_dst, int dst_step, IppiRect dst_roi,
                                    double x_factor, double y_factor, double x_shift, double y_shift, int interpolation, Ipp8u *p_buffer)
{
    if (p_src == NULL || p_dst == NULL || p_buffer == NULL)
        return ippStsNullPtrErr;

    if (x_factor <= 0.0 || y_factor <= 0.0)
        return ippStsResizeFactorErr;

    // For Super resize, the factors cannot be greater than 1.0.
    if ((interpolation == IPPI_INTER_SUPER) && (x_factor > 1.0 || y_factor > 1.0))
        return ippStsResizeFactorErr;

    if (src_roi.width <= 0 || src_roi.height <= 0 || dst_roi.width <= 0 || dst_roi.height <= 0)
        return ippStsSizeErr;

    // Locate source rectangle and transform by x_factor/y_factor/x_shift/y_shift.
    const int src_left = src_roi.x;
    const int src_top = src_roi.y;
    const int src_width = src_roi.width;
    const int src_height = src_roi.height;
    const int src_right = src_left + src_width;
    const int src_bottom = src_top + src_height;

    const int dst_left   = ch_Round(src_left   * x_factor + x_shift);
    const int dst_top    = ch_Round(src_top    * y_factor + y_shift);
    const int dst_right  = ch_Round(src_right  * x_factor + x_shift);
    const int dst_bottom = ch_Round(src_bottom * y_factor + y_shift);
    const int dst_width = dst_right - dst_left;
    const int dst_height = dst_bottom - dst_top;

    const BYTE *p_src_start = p_src + src_top * src_step + src_left * 4;
    BYTE *p_dst_start = p_dst + dst_top * dst_step + dst_left * 4;

    // We interpolate the source image ROI (src_roi)
    // to the destination region computed by the transform parameters.
    // However, only the pixels in the destination image ROI (dst_roi) are filled.
    const int dst_roi_left = dst_roi.x;
    const int dst_roi_top = dst_roi.y;
    const int dst_roi_right = dst_roi_left + dst_roi.width;
    const int dst_roi_bottom = dst_roi_top + dst_roi.height;

    const int x_start = ch_Max(dst_roi_left - dst_left, 0);
    const int x_end = ch_Min(dst_roi_right - dst_left, dst_width);
    const int y_start = ch_Max(dst_roi_top - dst_top, 0);
    const int y_end = ch_Min(dst_roi_bottom - dst_top, dst_height);

    if (interpolation == IPPI_INTER_LINEAR)
    {
        short *p_temp_buffer = NULL;
        int *p_bilinear_horizontal_position = NULL;
        short *p_bilinear_horizontal_weight = NULL;
        int *p_bilinear_vertical_position = NULL;
        short *p_bilinear_vertical_weight = NULL;

        BilinearInitialize(src_width, src_height, dst_width, dst_height, 4, p_buffer, p_temp_buffer,
                           p_bilinear_horizontal_position, p_bilinear_horizontal_weight, 
                           p_bilinear_vertical_position, p_bilinear_vertical_weight);

        BilinearVertical(p_src_start, src_width, src_height, src_step, 4, dst_height,
                         p_temp_buffer, p_bilinear_vertical_position, p_bilinear_vertical_weight);

        IppiRect dst_roi_bound = {x_start, y_start, x_end - x_start, y_end - y_start};
        BilinearHorizontal(p_dst_start, dst_width, dst_height, dst_step, 4, src_width, dst_roi_bound,
                           p_temp_buffer, p_bilinear_horizontal_position, p_bilinear_horizontal_weight);
    }
    else if (interpolation == IPPI_INTER_SUPER)
    {
        unsigned short *p_super_middle = NULL;
        short *p_super_horizontal_position = NULL;
        short *p_super_horizontal_unit_weight_skip = NULL;
        unsigned short *p_super_horizontal_weight = NULL;
        short *p_super_vertical_position = NULL;
        short *p_super_vertical_unit_weight_skip = NULL;
        unsigned short *p_super_vertical_weight = NULL;

        float super_vertical_scale = 0.0f;
        float super_horizontal_scale = 0.0f;

        SuperResizeInitialize(src_width, src_height, dst_width, dst_height, 4, p_buffer, p_super_middle,
                              p_super_horizontal_position, p_super_horizontal_unit_weight_skip, p_super_horizontal_weight,
                              p_super_vertical_position, p_super_vertical_unit_weight_skip, p_super_vertical_weight,
                              super_vertical_scale, super_horizontal_scale);

        SuperResizeVertical(p_src_start, src_width, src_height, src_step, 4,
                            dst_height, p_super_middle,
                            p_super_vertical_position, p_super_vertical_unit_weight_skip,
                            p_super_vertical_weight, super_vertical_scale);

        IppiRect dst_roi_bound = {x_start, y_start, x_end - x_start, y_end - y_start};
        SuperResizeHorizontal(p_dst_start, dst_width, dst_height, dst_step, 4,
                              src_width, dst_roi_bound, p_super_middle,
                              p_super_horizontal_position, p_super_horizontal_unit_weight_skip,
                              p_super_horizontal_weight, super_horizontal_scale);
    }
    else
    {
        return ippStsInterpolationErr;
    }

    return ippStsNoErr;
}

IppStatus ippiSet_8u_C1R(Ipp8u value, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        BYTE *p_dst_scan = p_dst + y * dst_step;

        memset(p_dst_scan, value, width);
    }

    return ippStsNoErr;
}

IppStatus ippiSet_8u_C3R(const Ipp8u value[3], Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (value == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            memcpy(p_dst_scan, value, 3);

            p_dst_scan += 3;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiSet_8u_AC4R(const Ipp8u value[3], Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (value == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    // The alpha channel should not be modified, so the input value only have 3 channels.

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            memcpy(p_dst_scan, value, 3);

            p_dst_scan += 4;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiSet_8u_C1MR(Ipp8u value, Ipp8u *p_dst, int dst_step, IppiSize roi_size, const Ipp8u *p_mask, int mask_step)
{
    if (p_dst == NULL || p_mask == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        BYTE *p_dst_scan = p_dst + y * dst_step;
        const BYTE *p_mask_scan = p_mask + y * mask_step;

        for (int x = 0; x < width; x++)
            if (p_mask_scan[x] != 0)
                p_dst_scan[x] = value;
    }

    return ippStsNoErr;
}

IppStatus ippiSet_8u_C3MR(const Ipp8u value[3], Ipp8u *p_dst, int dst_step, IppiSize roi_size, const Ipp8u *p_mask, int mask_step)
{
    if (value == NULL || p_dst == NULL || p_mask == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        BYTE *p_dst_scan = p_dst + y * dst_step;
        const BYTE *p_mask_scan = p_mask + y * mask_step;

        for (int x = 0; x < width; x++)
        {
            if (p_mask_scan[x] != 0)
                memcpy(p_dst_scan, value, 3);

            p_dst_scan += 3;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiSet_8u_AC4MR(const Ipp8u value[3], Ipp8u *p_dst, int dst_step, IppiSize roi_size, const Ipp8u *p_mask, int mask_step)
{
    if (value == NULL || p_dst == NULL || p_mask == NULL)
        return ippStsNullPtrErr;

    // The alpha channel should not be modified, so the input value only have 3 channels.

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        BYTE *p_dst_scan = p_dst + y * dst_step;
        const BYTE *p_mask_scan = p_mask + y * mask_step;

        for (int x = 0; x < width; x++)
        {
            if (p_mask_scan[x] != 0)
                memcpy(p_dst_scan, value, 3);

            p_dst_scan += 4;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiSet_32s_C1R(Ipp32s value, Ipp32s *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        int *p_dst_scan = (int *)((BYTE *)p_dst + y * dst_step);

        for (int x = 0; x < width; x++)
            p_dst_scan[x] = value;
    }

    return ippStsNoErr;
}

IppStatus ippiSet_32f_C1R(Ipp32f value, Ipp32f *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        float *p_dst_scan = (float *)((BYTE *)p_dst + y * dst_step);

        for (int x = 0; x < width; x++)
            p_dst_scan[x] = value;
    }

    return ippStsNoErr;
}

IppStatus ippiAnd_8u_C1IR(const Ipp8u *p_src, int src_step, Ipp8u *p_srcdst, int srcdst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_srcdst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_srcdst_scan = p_srcdst + y * srcdst_step;

        for (int x = 0; x < width; x++)
        {
            BYTE old_value = p_srcdst_scan[x];
            p_srcdst_scan[x] = (p_src_scan[x] & old_value);
        }
    }

    return ippStsNoErr;
}

IppStatus ippiOr_8u_C1IR(const Ipp8u *p_src, int src_step, Ipp8u *p_srcdst, int srcdst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_srcdst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_srcdst_scan = p_srcdst + y * srcdst_step;

        for (int x = 0; x < width; x++)
        {
            BYTE old_value = p_srcdst_scan[x];
            p_srcdst_scan[x] = (p_src_scan[x] | old_value);
        }
    }

    return ippStsNoErr;
}

IppStatus ippiXor_8u_C1R(const Ipp8u *p_src1, int src1_step, const Ipp8u *p_src2, int src2_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src1 == NULL || p_src2 == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    if (src1_step <= 0 || src2_step <= 0 || dst_step <= 0)
        return ippStsStepErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src1_scan = p_src1 + y * src1_step;
        const BYTE *p_src2_scan = p_src2 + y * src2_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            p_dst_scan[x] = p_src1_scan[x] ^ p_src2_scan[x];
        }
    }

    return ippStsNoErr;
}

IppStatus ippiXor_8u_C1IR(const Ipp8u *p_src, int src_step, Ipp8u *p_srcdst, int srcdst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_srcdst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_srcdst_scan = p_srcdst + y * srcdst_step;

        for (int x = 0; x < width; x++)
        {
            BYTE old_value = p_srcdst_scan[x];
            p_srcdst_scan[x] = (p_src_scan[x] ^ old_value);
        }
    }

    return ippStsNoErr;
}

IppStatus ippiNot_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            p_dst_scan[x] = ~p_src_scan[x];
        }
    }

    return ippStsNoErr;
}

IppStatus ippiNot_8u_C1IR(Ipp8u *p_src_dst, int src_dst_step, IppiSize roi_size)
{
    if (p_src_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    if (src_dst_step <= 0)
        return ippStsStepErr;

    for (int y = 0; y < height; y++)
    {
        BYTE *p_src_dst_scan = p_src_dst + y * src_dst_step;

        for (int x = 0; x < width; x++)
        {
            p_src_dst_scan[x] = ~p_src_dst_scan[x];
        }
    }

    return ippStsNoErr;
}

// The B/G/R/Constant factor for BGR To YCbCr conversion.
const float ycbcr_y_shift = 16.0f;
const float ycbcr_cb_shift = 128.0f;
const float ycbcr_cr_shift = 128.0f;

const float bgr_to_y_factor[4]  = { 0.098f,  0.504f,  0.257f,  ycbcr_y_shift};
const float bgr_to_cb_factor[4] = { 0.439f, -0.291f, -0.148f, ycbcr_cb_shift};
const float bgr_to_cr_factor[4] = {-0.071f, -0.368f,  0.439f, ycbcr_cr_shift};

const float y_to_bgr_factor = 1.164f;
const float cb_to_b_factor = 2.017f; // no Cr
const float cbcr_to_g_factor[2] = {-0.392f, -0.813f};
const float cr_to_r_factor = 1.596f; // no Cb

inline void BGRPixelToYCbCr422(const BYTE *p_pixel1, const BYTE *p_pixel2, BYTE &y0, BYTE &cb, BYTE &y1, BYTE &cr)
{
    _MYASSERT(p_pixel1);
    _MYASSERT(p_pixel2);

    const float b0 = (float)p_pixel1[0];
    const float g0 = (float)p_pixel1[1];
    const float r0 = (float)p_pixel1[2];
    const float b1 = (float)p_pixel2[0];
    const float g1 = (float)p_pixel2[1];
    const float r1 = (float)p_pixel2[2];

    float y_0 = b0 * bgr_to_y_factor[0] + g0 * bgr_to_y_factor[1] + r0 * bgr_to_y_factor[2] + bgr_to_y_factor[3];
    float cb0 = b0 * bgr_to_cb_factor[0] + g0 * bgr_to_cb_factor[1] + r0 * bgr_to_cb_factor[2] + bgr_to_cb_factor[3];
    float cr0 = b0 * bgr_to_cr_factor[0] + g0 * bgr_to_cr_factor[1] + r0 * bgr_to_cr_factor[2] + bgr_to_cr_factor[3];
    float y_1 = b1 * bgr_to_y_factor[0] + g1 * bgr_to_y_factor[1] + r1 * bgr_to_y_factor[2] + bgr_to_y_factor[3];
    float cb1 = b1 * bgr_to_cb_factor[0] + g1 * bgr_to_cb_factor[1] + r1 * bgr_to_cb_factor[2] + bgr_to_cb_factor[3];
    float cr1 = b1 * bgr_to_cr_factor[0] + g1 * bgr_to_cr_factor[1] + r1 * bgr_to_cr_factor[2] + bgr_to_cr_factor[3];

    // Round the sum of cb/cr values before dividing by 2. The result is more close to IPP 7.1.
    y0 = (BYTE)FitInRange(ch_Round(y_0), 0, 255);
    cb = (BYTE)FitInRange(ch_Round(cb0 + cb1) / 2, 0, 255);
    y1 = (BYTE)FitInRange(ch_Round(y_1), 0, 255);
    cr = (BYTE)FitInRange(ch_Round(cr0 + cr1) / 2, 0, 255);
}

inline void YCbCr422ToBGRPixel(const BYTE *p_ycbcr422, BYTE *p_pixel1, BYTE *p_pixel2)
{
    _MYASSERT(p_ycbcr422);
    _MYASSERT(p_pixel1);
    _MYASSERT(p_pixel2);

    const float y0 = (float)p_ycbcr422[0];
    const float cb = (float)p_ycbcr422[1];
    const float y1 = (float)p_ycbcr422[2];
    const float cr = (float)p_ycbcr422[3];

    float cb_shift = cb - ycbcr_cb_shift;
    float cr_shift = cr - ycbcr_cr_shift;

    float b_part = cb_to_b_factor * cb_shift;
    float g_part = cbcr_to_g_factor[0] * cb_shift + cbcr_to_g_factor[1] * cr_shift;
    float r_part = cr_to_r_factor * cr_shift;

    float y0_part = y_to_bgr_factor * (y0 - ycbcr_y_shift);
    float y1_part = y_to_bgr_factor * (y1 - ycbcr_y_shift);

    float b0 = y0_part + b_part;
    float g0 = y0_part + g_part;
    float r0 = y0_part + r_part;
    float b1 = y1_part + b_part;
    float g1 = y1_part + g_part;
    float r1 = y1_part + r_part;

    p_pixel1[0] = (BYTE)FitInRange(ch_Round(b0), 0, 255);
    p_pixel1[1] = (BYTE)FitInRange(ch_Round(g0), 0, 255);
    p_pixel1[2] = (BYTE)FitInRange(ch_Round(r0), 0, 255);
    p_pixel2[0] = (BYTE)FitInRange(ch_Round(b1), 0, 255);
    p_pixel2[1] = (BYTE)FitInRange(ch_Round(g1), 0, 255);
    p_pixel2[2] = (BYTE)FitInRange(ch_Round(r1), 0, 255);
}

inline void RGBPixelToYCbCr(const BYTE *p_pixel, BYTE &y, BYTE &cb, BYTE &cr)
{
    _MYASSERT(p_pixel);

    const float r = (float)p_pixel[0];
    const float g = (float)p_pixel[1];
    const float b = (float)p_pixel[2];

    float y_0 = b * bgr_to_y_factor[0] + g * bgr_to_y_factor[1] + r * bgr_to_y_factor[2] + bgr_to_y_factor[3];
    float cb0 = b * bgr_to_cb_factor[0] + g * bgr_to_cb_factor[1] + r * bgr_to_cb_factor[2] + bgr_to_cb_factor[3];
    float cr0 = b * bgr_to_cr_factor[0] + g * bgr_to_cr_factor[1] + r * bgr_to_cr_factor[2] + bgr_to_cr_factor[3];

    // Round the sum of cb/cr values before dividing by 2. The result is more close to IPP 7.1.
    y = (BYTE)FitInRange(ch_Round(y_0), 0, 255);
    cb = (BYTE)FitInRange(ch_Round(cb0), 0, 255);
    cr = (BYTE)FitInRange(ch_Round(cr0), 0, 255);
}

// The H value saved in a pixel has the range [0, 255]. It is scaled from [0, 6].
const float hue_scale_factor = 255.0f / 6.0f;
const float inverse_hue_scale_factor = 6.0f / 255.0f;

inline void RGBPixelToHSVPixel(const BYTE *p_src_pixel, BYTE *p_dst_pixel)
{
    _MYASSERT(p_src_pixel);
    _MYASSERT(p_dst_pixel);

    const BYTE r = p_src_pixel[0];
    const BYTE g = p_src_pixel[1];
    const BYTE b = p_src_pixel[2];

    const BYTE v = ch_Max3(r, g, b);

    if (v == 0)
    {
        // H = S = V = 0
        p_dst_pixel[0] = 0;
        p_dst_pixel[1] = 0;
        p_dst_pixel[2] = 0;
        return;
    }

    const BYTE min_value = ch_Min3(r, g, b);
    if (v == min_value)
    {
        // Grey color, S = 0, H is undefined and set to 0.
        p_dst_pixel[0] = 0;
        p_dst_pixel[1] = 0;
        p_dst_pixel[2] = v;
        return;
    }

    const float max_value = (float)v;
    const float max_min_difference = max_value - (float)min_value;

    float saturation = max_min_difference / max_value;
    const BYTE s = (BYTE)ch_Round(saturation * 255.0f);

    float fr = (float)r;
    float fg = (float)g;
    float fb = (float)b;

    float inverse_difference = 1.0f / max_min_difference;

    float Cr = (max_value - fr) * inverse_difference;
    float Cg = (max_value - fg) * inverse_difference;
    float Cb = (max_value - fb) * inverse_difference;

    float hue = 0.0f;
    if (r == v)
        hue = Cb - Cg;
    else if (g == v)
        hue = 2.0f + Cr - Cb;
    else // (b == v)
        hue = 4.0f + Cg - Cr;

    if (hue < 0.0f)
        hue += 6.0f;

    const BYTE h = (BYTE)FitInRange(ch_Round(hue * hue_scale_factor), 0, 255);

    p_dst_pixel[0] = h;
    p_dst_pixel[1] = s;
    p_dst_pixel[2] = v;
}

inline void HSVPixelToRGBPixel(const BYTE *p_src_pixel, BYTE *p_dst_pixel)
{
    _MYASSERT(p_src_pixel);
    _MYASSERT(p_dst_pixel);

    const BYTE h = p_src_pixel[0];
    const BYTE s = p_src_pixel[1];
    const BYTE v = p_src_pixel[2];

    if (s == 0)
    {
        p_dst_pixel[0] = v;
        p_dst_pixel[1] = v;
        p_dst_pixel[2] = v;
        return;
    }

    float saturation = (float)s * inverse_255;
    float fv = (float)v;

    float fh = 0.0f;
    if (h < 255)
        fh = (float)h * inverse_hue_scale_factor;

    int I = (int)fh;
    float F = fh - (float)I;
    
    float fm = fv * (1.0f - saturation);
    float fn = fv * (1.0f - saturation * F);
    float fk = fv * (1.0f - saturation * (1.0f - F));

    const BYTE m = (BYTE)FitInRange(ch_Round(fm), 0, 255);
    const BYTE n = (BYTE)FitInRange(ch_Round(fn), 0, 255);
    const BYTE k = (BYTE)FitInRange(ch_Round(fk), 0, 255);

    if (I == 0)
    {
        p_dst_pixel[0] = v;
        p_dst_pixel[1] = k;
        p_dst_pixel[2] = m;
    }
    else if (I == 1)
    {
        p_dst_pixel[0] = n;
        p_dst_pixel[1] = v;
        p_dst_pixel[2] = m;
    }
    else if (I == 2)
    {
        p_dst_pixel[0] = m;
        p_dst_pixel[1] = v;
        p_dst_pixel[2] = k;
    }
    else if (I == 3)
    {
        p_dst_pixel[0] = m;
        p_dst_pixel[1] = n;
        p_dst_pixel[2] = v;
    }
    else if (I == 4)
    {
        p_dst_pixel[0] = k;
        p_dst_pixel[1] = m;
        p_dst_pixel[2] = v;
    }
    else // (I == 5)
    {
        p_dst_pixel[0] = v;
        p_dst_pixel[1] = m;
        p_dst_pixel[2] = n;
    }
}

IppStatus ippiRGBToYCbCr_8u_C3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            RGBPixelToYCbCr(p_src_scan, p_dst_scan[0], p_dst_scan[1], p_dst_scan[2]);

            p_src_scan += 3;
            p_dst_scan += 3;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiRGBToYCbCr_8u_AC4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            RGBPixelToYCbCr(p_src_scan, p_dst_scan[0], p_dst_scan[1], p_dst_scan[2]);

            p_src_scan += 4;
            p_dst_scan += 4;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiBGRToYCbCr422_8u_C3C2R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    // width must be at least 2.
    if (width <= 1 || height <= 0)
        return ippStsSizeErr;

    const int valid_width = (width / 2) * 2;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < valid_width; x += 2)
        {
            BGRPixelToYCbCr422(p_src_scan, p_src_scan + 3, p_dst_scan[0], p_dst_scan[1], p_dst_scan[2], p_dst_scan[3]);
            
            p_src_scan += 6;
            p_dst_scan += 4;
        }

        // process the last pixel if the ROI width is odd.
        if (valid_width < width)
        {
            const float b0 = (float)p_src_scan[0];
            const float g0 = (float)p_src_scan[1];
            const float r0 = (float)p_src_scan[2];

            float y0 = b0 * bgr_to_y_factor[0] + g0 * bgr_to_y_factor[1] + r0 * bgr_to_y_factor[2] + bgr_to_y_factor[3];
            float cb0 = b0 * bgr_to_cb_factor[0] + g0 * bgr_to_cb_factor[1] + r0 * bgr_to_cb_factor[2] + bgr_to_cb_factor[3];
            
            p_dst_scan[0] = (BYTE)FitInRange(ch_Round(y0), 0, 255);
            p_dst_scan[1] = (BYTE)FitInRange(ch_Round(cb0), 0, 255);
        }
    }

    return ippStsNoErr;
}

IppStatus ippiBGRToYCbCr422_8u_AC4C2R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    // width must be at least 2.
    if (width <= 1 || height <= 0)
        return ippStsSizeErr;

    const int valid_width = (width / 2) * 2;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < valid_width; x += 2)
        {
            BGRPixelToYCbCr422(p_src_scan, p_src_scan + 4, p_dst_scan[0], p_dst_scan[1], p_dst_scan[2], p_dst_scan[3]);
            
            p_src_scan += 8;
            p_dst_scan += 4;
        }

        // process the last pixel if the ROI width is odd.
        if (valid_width < width)
        {
            const float b0 = (float)p_src_scan[0];
            const float g0 = (float)p_src_scan[1];
            const float r0 = (float)p_src_scan[2];

            float y0 = b0 * bgr_to_y_factor[0] + g0 * bgr_to_y_factor[1] + r0 * bgr_to_y_factor[2] + bgr_to_y_factor[3];
            float cb0 = b0 * bgr_to_cb_factor[0] + g0 * bgr_to_cb_factor[1] + r0 * bgr_to_cb_factor[2] + bgr_to_cb_factor[3];
            
            p_dst_scan[0] = (BYTE)FitInRange(ch_Round(y0), 0, 255);
            p_dst_scan[1] = (BYTE)FitInRange(ch_Round(cb0), 0, 255);
        }
    }

    return ippStsNoErr;
}

IppStatus ippiBGRToYCbCr422_8u_C3P3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst[3], int dst_step[3], IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL || dst_step == NULL)
        return ippStsNullPtrErr;

    if (p_dst[0] == NULL || p_dst[1] == NULL || p_dst[2] == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    // width must be at least 2.
    if (width <= 1 || height <= 0)
        return ippStsSizeErr;

    // For C3P3R format, if the ROI width is odd,
    // the last pixel is only converted to Y and fill in p_dst[0],
    // The Cb/Cr value of the pixel is unchanged in p_dst[1]/p_dst[2] even if there are enough spaces.

    const int valid_width = (width / 2) * 2;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_y_scan = p_dst[0] + y * dst_step[0];
        BYTE *p_dst_cb_scan = p_dst[1] + y * dst_step[1];
        BYTE *p_dst_cr_scan = p_dst[2] + y * dst_step[2];

        for (int x = 0; x < valid_width; x += 2)
        {
            BGRPixelToYCbCr422(p_src_scan, p_src_scan + 3, p_dst_y_scan[0], p_dst_cb_scan[0], p_dst_y_scan[1], p_dst_cr_scan[0]);

            p_src_scan += 6;
            p_dst_y_scan += 2;
            p_dst_cb_scan++;
            p_dst_cr_scan++;
        }

        // process the last pixel if the ROI width is odd. (only for Y plane)
        if (valid_width < width)
        {
            const float b0 = (float)p_src_scan[0];
            const float g0 = (float)p_src_scan[1];
            const float r0 = (float)p_src_scan[2];

            float y0 = b0 * bgr_to_y_factor[0] + g0 * bgr_to_y_factor[1] + r0 * bgr_to_y_factor[2] + bgr_to_y_factor[3];

            p_dst_y_scan[0] = (BYTE)FitInRange(ch_Round(y0), 0, 255);
        }
    }

    return ippStsNoErr;
}

IppStatus ippiBGRToYCbCr422_8u_AC4P3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst[3], int dst_step[3], IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL || dst_step == NULL)
        return ippStsNullPtrErr;

    if (p_dst[0] == NULL || p_dst[1] == NULL || p_dst[2] == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    // width must be at least 2.
    if (width <= 1 || height <= 0)
        return ippStsSizeErr;

    // For C3P3R format, if the ROI width is odd,
    // the last pixel is only converted to Y and fill in p_dst[0],
    // The Cb/Cr value of the pixel is unchanged in p_dst[1]/p_dst[2] even if there are enough spaces.

    const int valid_width = (width / 2) * 2;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_y_scan = p_dst[0] + y * dst_step[0];
        BYTE *p_dst_cb_scan = p_dst[1] + y * dst_step[1];
        BYTE *p_dst_cr_scan = p_dst[2] + y * dst_step[2];

        for (int x = 0; x < valid_width; x += 2)
        {
            BGRPixelToYCbCr422(p_src_scan, p_src_scan + 4, p_dst_y_scan[0], p_dst_cb_scan[0], p_dst_y_scan[1], p_dst_cr_scan[0]);

            p_src_scan += 8;
            p_dst_y_scan += 2;
            p_dst_cb_scan++;
            p_dst_cr_scan++;
        }

        // process the last pixel if the ROI width is odd. (only for Y plane)
        if (valid_width < width)
        {
            const float b0 = (float)p_src_scan[0];
            const float g0 = (float)p_src_scan[1];
            const float r0 = (float)p_src_scan[2];

            float y0 = b0 * bgr_to_y_factor[0] + g0 * bgr_to_y_factor[1] + r0 * bgr_to_y_factor[2] + bgr_to_y_factor[3];

            p_dst_y_scan[0] = (BYTE)FitInRange(ch_Round(y0), 0, 255);
        }
    }

    return ippStsNoErr;
}

IppStatus ippiYCbCr422ToBGR_8u_C2C3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    // width must be at least 2.
    if (width <= 1 || height <= 0)
        return ippStsSizeErr;

    const int valid_width = (width / 2) * 2;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < valid_width; x += 2)
        {
            YCbCr422ToBGRPixel(p_src_scan, p_dst_scan, p_dst_scan + 3);
            
            p_src_scan += 4;
            p_dst_scan += 6;
        }

        // process the last pixel if the ROI width is odd.
        // The Cr value of the pixel is the previous Cr value in the row.
        if (valid_width < width)
        {
            float y = p_src_scan[0];
            float cb = p_src_scan[1];
            float cr = p_src_scan[-1];

            float y_shift = y - ycbcr_y_shift;
            float cb_shift = cb - ycbcr_cb_shift;
            float cr_shift = cr - ycbcr_cr_shift;

            float b = y_to_bgr_factor * y_shift + cb_to_b_factor * cb_shift;
            float g = y_to_bgr_factor * y_shift + cbcr_to_g_factor[0] * cb_shift + cbcr_to_g_factor[1] * cr_shift;
            float r = y_to_bgr_factor * y_shift + cr_to_r_factor * cr_shift;

            p_dst_scan[0] = (BYTE)FitInRange(ch_Round(b), 0, 255);
            p_dst_scan[1] = (BYTE)FitInRange(ch_Round(g), 0, 255);
            p_dst_scan[2] = (BYTE)FitInRange(ch_Round(r), 0, 255);
        }
    }

    return ippStsNoErr;
}

IppStatus ippiYCbCr422ToBGR_8u_C2C4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size, Ipp8u alpha_value)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    // width must be at least 2.
    if (width <= 1 || height <= 0)
        return ippStsSizeErr;

    const int valid_width = (width / 2) * 2;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < valid_width; x += 2)
        {
            YCbCr422ToBGRPixel(p_src_scan, p_dst_scan, p_dst_scan + 4);
            p_dst_scan[3] = alpha_value;
            p_dst_scan[7] = alpha_value;
            
            p_src_scan += 4;
            p_dst_scan += 8;
        }

        // process the last pixel if the ROI width is odd.
        // The Cr value of the pixel is the previous Cr value in the row.
        if (valid_width < width)
        {
            float y = p_src_scan[0];
            float cb = p_src_scan[1];
            float cr = p_src_scan[-1];

            float y_shift = y - ycbcr_y_shift;
            float cb_shift = cb - ycbcr_cb_shift;
            float cr_shift = cr - ycbcr_cr_shift;

            float b = y_to_bgr_factor * y_shift + cb_to_b_factor * cb_shift;
            float g = y_to_bgr_factor * y_shift + cbcr_to_g_factor[0] * cb_shift + cbcr_to_g_factor[1] * cr_shift;
            float r = y_to_bgr_factor * y_shift + cr_to_r_factor * cr_shift;

            p_dst_scan[0] = (BYTE)FitInRange(ch_Round(b), 0, 255);
            p_dst_scan[1] = (BYTE)FitInRange(ch_Round(g), 0, 255);
            p_dst_scan[2] = (BYTE)FitInRange(ch_Round(r), 0, 255);
            p_dst_scan[3] = alpha_value;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiRGBToHSV_8u_C3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            RGBPixelToHSVPixel(p_src_scan, p_dst_scan);

            p_src_scan += 3;
            p_dst_scan += 3;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiRGBToHSV_8u_AC4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            RGBPixelToHSVPixel(p_src_scan, p_dst_scan);

            p_src_scan += 4;
            p_dst_scan += 4;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiHSVToRGB_8u_C3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            HSVPixelToRGBPixel(p_src_scan, p_dst_scan);

            p_src_scan += 3;
            p_dst_scan += 3;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiHSVToRGB_8u_AC4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            HSVPixelToRGBPixel(p_src_scan, p_dst_scan);

            p_src_scan += 4;
            p_dst_scan += 4;
        }
    }

    return ippStsNoErr;
}

struct DoublePoint
{
    double x;
    double y;

    DoublePoint()
    {
        x = 0.0;
        y = 0.0;
    }

    DoublePoint(double _x, double _y)
    {
        x = _x;
        y = _y;
    }
};

// Point Rotation function similar to hyRotate(), but all computations are done under double float.
// WARNING: This function is used for precise computation in ippiGetRotateShift() and ippiGetRotateBound().
//          It should not be used on every pixels in the image ROI.
inline DoublePoint RotatePoint(const DoublePoint &point, const DoublePoint &center, double theta)
{
    DoublePoint dst_point;
    double dx = point.x - center.x;
    double dy = point.y - center.y;
    dst_point.x = dx * cos(theta) - dy * sin(theta) + center.x;
    dst_point.y = dx * sin(theta) + dy * cos(theta) + center.y;
    return dst_point;
}

const double degree_to_radian_factor = HY_PI / 180.0;

IppStatus ippiGetRotateShift(double x_center, double y_center, double angle, double *p_x_shift, double *p_y_shift)
{
    if (p_x_shift == NULL || p_y_shift == NULL)
        return ippStsNullPtrErr;

    double theta = -angle * degree_to_radian_factor;

    DoublePoint origin(0.0, 0.0);
    DoublePoint center(x_center, y_center);
    DoublePoint rotated_center = RotatePoint(center, origin, theta);

    // Shift the rotated center back to the position.
    (*p_x_shift) = center.x - rotated_center.x;
    (*p_y_shift) = center.y - rotated_center.y;

    return ippStsNoErr;
}

IppStatus ippiGetRotateBound(IppiRect src_roi, double bound[2][2], double angle, double x_shift, double y_shift)
{
    if (bound == NULL)
        return ippStsNullPtrErr;

    const int width = src_roi.width;
    const int height = src_roi.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    double roi_left   = (double)(src_roi.x);
    double roi_top    = (double)(src_roi.y);
    double roi_right  = (double)(src_roi.x + src_roi.width - 1);
    double roi_bottom = (double)(src_roi.y + src_roi.height - 1);

    double theta = -angle * degree_to_radian_factor;

    DoublePoint top_left(roi_left, roi_top);
    DoublePoint top_right(roi_right, roi_top);
    DoublePoint bottom_left(roi_left, roi_bottom);
    DoublePoint bottom_right(roi_right, roi_bottom);

    DoublePoint origin(0.0, 0.0);

    DoublePoint rotated_top_left = RotatePoint(top_left, origin, theta);
    DoublePoint rotated_top_right = RotatePoint(top_right, origin, theta);
    DoublePoint rotated_bottom_left = RotatePoint(bottom_left, origin, theta);
    DoublePoint rotated_bottom_right = RotatePoint(bottom_right, origin, theta);

    rotated_top_left.x += x_shift;
    rotated_top_left.y += y_shift;
    rotated_top_right.x += x_shift;
    rotated_top_right.y += y_shift;
    rotated_bottom_left.x += x_shift;
    rotated_bottom_left.y += y_shift;
    rotated_bottom_right.x += x_shift;
    rotated_bottom_right.y += y_shift;

    bound[0][0] = ch_Min4(rotated_top_left.x, rotated_top_right.x, rotated_bottom_left.x, rotated_bottom_right.x);
    bound[0][1] = ch_Min4(rotated_top_left.y, rotated_top_right.y, rotated_bottom_left.y, rotated_bottom_right.y);
    bound[1][0] = ch_Max4(rotated_top_left.x, rotated_top_right.x, rotated_bottom_left.x, rotated_bottom_right.x);
    bound[1][1] = ch_Max4(rotated_top_left.y, rotated_top_right.y, rotated_bottom_left.y, rotated_bottom_right.y);

    return ippStsNoErr;
}

inline bool GetInterpolationWeight(float src_x, float src_y, int floor_x, int floor_y, int ceiling_x, int ceiling_y,
                                          int src_roi_left, int src_roi_top, int src_roi_right, int src_roi_bottom, float weight[4])
{
    // Get interpolation weights for the neighboring 4 pixels:
    // p0  p1
    //
    // p2  p3

    // According to Intel IPP image rotation algorithm,
    // the interpolation is only performed if all 4 pixels are in ROI.
    // Return true if all pixels are valid, and false if not.

    _MYASSERT(weight);

    if (floor_x < src_roi_left || ceiling_x >= src_roi_right || floor_y < src_roi_top || ceiling_y >= src_roi_bottom)
        return false;

    float left_weight = (float)ceiling_x - src_x;
    float right_weight = 1.0f - left_weight;
    float top_weight = (float)ceiling_y - src_y;
    float bottom_weight = 1.0f - top_weight;

    weight[0] = left_weight * top_weight;
    weight[1] = right_weight * top_weight;
    weight[2] = left_weight * bottom_weight;
    weight[3] = right_weight * bottom_weight;

    return true;
}

__forceinline BYTE BilinearInterpolation(int upper_left, int upper_right, int bottom_left, 
                                         int bottom_right, int offset_x, int offset_y)
{
    const int w4 = offset_x * offset_y;
    const int w3 = (offset_y << _PREC_SHIFT) - w4;
    const int w2 = (offset_x << _PREC_SHIFT) - w4;
    const int w1 = (1 << _PREC_SHIFT) * (1 << _PREC_SHIFT) - w4 - w3 - w2;

    return (BYTE)((w1 * upper_left + w2 * upper_right + w3 * bottom_left + w4 * bottom_right) >> (_PREC_SHIFT * 2));
}

IppStatus ippiRotate_8u_C1R(const Ipp8u *p_src, IppiSize src_size, int src_step, IppiRect src_roi,
                            Ipp8u* p_dst, int dst_step, IppiRect dst_roi, double angle, double x_shift, double y_shift, int interpolation)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    // Only support linear interpolation.
    if (interpolation != IPPI_INTER_LINEAR)
        return ippStsInterpolationErr;

    const int src_width = src_size.width;
    const int src_height = src_size.height;
    if (src_width <= 0 || src_height <= 0)
        return ippStsSizeErr;

    const int src_roi_left = ch_Max(src_roi.x, 0);
    const int src_roi_top = ch_Max(src_roi.y, 0);
    const int src_roi_right = ch_Min(src_roi.x + src_roi.width, src_width);
    const int src_roi_bottom = ch_Min(src_roi.y + src_roi.height, src_height);

    if (src_roi_left >= src_roi_right || src_roi_top >= src_roi_bottom)
        return ippStsRectErr;

    const int dst_roi_left = dst_roi.x;
    const int dst_roi_top = dst_roi.y;
    const int dst_roi_right = dst_roi_left + dst_roi.width;
    const int dst_roi_bottom = dst_roi_top + dst_roi.height;

    // Still use float to compute the coordinates for speed concern.
    const float fx_shift = (float)x_shift;
    const float fy_shift = (float)y_shift;

    const double theta = angle * degree_to_radian_factor;
    const float cos_theta = (float)(cos(theta));
    const float sin_theta = (float)(sin(theta));

#if defined(ANDROID_ARM) || (defined(IOS_ARM) && !defined(__EMSCRIPTEN__))

    int height_increase_x = -(int)(sin_theta * (1 << _PREC_SHIFT));
    int height_increase_y = (int)(cos_theta * (1 << _PREC_SHIFT));
    int width_increase_x = (int)(cos_theta * (1 << _PREC_SHIFT));
    int width_increase_y = (int)(sin_theta * (1 << _PREC_SHIFT));

    const int width_increase_x_2x = width_increase_x * 2;
    const int width_increase_x_3x = width_increase_x * 3;
    const int width_increase_x_4x = width_increase_x * 4;
    const int width_increase_y_2x = width_increase_y * 2;
    const int width_increase_y_3x = width_increase_y * 3;
    const int width_increase_y_4x = width_increase_y * 4;

    const float src_x_start = (-fx_shift * cos_theta + fy_shift * sin_theta) * (1 << _PREC_SHIFT);
    const float src_y_start = (-fx_shift * sin_theta - fy_shift * cos_theta) * (1 << _PREC_SHIFT);

    const int aligned_width = (dst_roi.width / 4) * 4;
    const int shift_mul = (1 << _PREC_SHIFT) * (1 << _PREC_SHIFT);

    int32x4_t upleft, upright, btleft, btright;
    int32x4_t ofsetx, ofsety, w1, w2, w3, w4;
    int32x4_t w1UL, w2UR, w3BL, w4BR;
    int32x4_t sftmul = vdupq_n_s32(shift_mul);
    int32x4_t start_x, start_y, start_shift_x, start_shift_y;
    int32x4_t srcStep = vdupq_n_s32(src_step);
    int32x4_t temp_offset;
    int32x4_t zero = vdupq_n_s32(0);
    int32x4_t width_bound = vdupq_n_s32(src_width - 2);
    int32x4_t height_bound = vdupq_n_s32(src_height - 2);
    uint32x4_t bound_check;

    int32x4_t width_inc_x_4x = vdupq_n_s32(width_increase_x_4x);
    int32x4_t width_inc_y_4x = vdupq_n_s32(width_increase_y_4x);

    BYTE *p_dst_scan = p_dst + dst_roi_top * dst_step;
    for (int y = dst_roi_top; y < dst_roi_bottom; y++)
    {
        int src_shift_x = (int)(src_x_start + y * height_increase_x + dst_roi_left * width_increase_x);
        int src_shift_y = (int)(src_y_start + y * height_increase_y + dst_roi_left * width_increase_y);

        int start_x_buffer[4] = {src_shift_x,
                                 src_shift_x + width_increase_x, 
                                 src_shift_x + width_increase_x_2x,
                                 src_shift_x + width_increase_x_3x};

        int start_y_buffer[4] = {src_shift_y,
                                 src_shift_y + width_increase_y,
                                 src_shift_y + width_increase_y_2x,
                                 src_shift_y + width_increase_y_3x};

        start_shift_x = vld1q_s32(start_x_buffer);
        start_shift_y = vld1q_s32(start_y_buffer);

        for (int x = dst_roi_left; x < dst_roi_left + aligned_width; x += 4)
        {
            start_x = vshrq_n_s32(start_shift_x, _PREC_SHIFT);
            start_y = vshrq_n_s32(start_shift_y, _PREC_SHIFT);

            bound_check = vcgeq_s32(start_x, zero);
            bound_check = vandq_u32(bound_check, vcleq_s32(start_x, width_bound));
            bound_check = vandq_u32(bound_check, vcgeq_s32(start_y, zero));
            bound_check = vandq_u32(bound_check, vcleq_s32(start_y, height_bound));

            temp_offset = vmulq_s32(start_y, srcStep);
            temp_offset = vaddq_s32(temp_offset, start_x);

            int pos_offset[4];
            unsigned char *p_pos[4];
            unsigned int valid_pos[4];
            vst1q_u32(valid_pos, bound_check);
            vst1q_s32(pos_offset, temp_offset);

            for (int i = 0; i < 4; i++)
            {
                if (valid_pos[i])
                    p_pos[i] = (unsigned char *)p_src + pos_offset[i];
                else
                    p_pos[i] = (unsigned char *)p_src;
            }

            ofsetx = vsubq_s32(start_shift_x, vshlq_n_s32(start_x, _PREC_SHIFT));
            ofsety = vsubq_s32(start_shift_y, vshlq_n_s32(start_y, _PREC_SHIFT));

            int upper_left_buffer[4];
            int upper_right_buffer[4];
            int bottom_left_buffer[4];
            int bottom_right_buffer[4];

            for (int i = 0; i < 4; i++)
            {
                upper_left_buffer[i] = *(p_pos[i]);
                upper_right_buffer[i] = *(p_pos[i] + 1);
                bottom_left_buffer[i] = *(p_pos[i] + src_step);
                bottom_right_buffer[i] = *(p_pos[i] + src_step + 1);
            }

            upleft = vld1q_s32(upper_left_buffer);
            upright = vld1q_s32(upper_right_buffer);
            btleft = vld1q_s32(bottom_left_buffer);
            btright = vld1q_s32(bottom_right_buffer);

            w4 = vmulq_s32(ofsetx, ofsety);
            ofsety = vshlq_n_s32(ofsety, _PREC_SHIFT);
            w3 = vsubq_s32(ofsety, w4);
            ofsetx = vshlq_n_s32(ofsetx, _PREC_SHIFT);
            w2 = vsubq_s32(ofsetx, w4);
            w1 = vsubq_s32(sftmul, w4);
            w1 = vsubq_s32(w1, w3);
            w1 = vsubq_s32(w1, w2);

            w1UL = vmulq_s32(w1, upleft);
            w2UR = vmulq_s32(w2, upright);
            w3BL = vmulq_s32(w3, btleft);
            w4BR = vmulq_s32(w4, btright);

            w1UL = vaddq_s32(w1UL, w2UR);
            w1UL = vaddq_s32(w1UL, w3BL);
            w1UL = vaddq_s32(w1UL, w4BR);

            int dst_pixel[4];
            w1UL = vshrq_n_s32(w1UL, _PREC_SHIFT * 2);
            vst1q_s32(dst_pixel, w1UL);

            for (int i = 0; i < 4; i++)
            {
                if (valid_pos[i])
                    p_dst_scan[x + i] = dst_pixel[i];
            }

            start_shift_x = vaddq_s32(start_shift_x, width_inc_x_4x);
            start_shift_y = vaddq_s32(start_shift_y, width_inc_y_4x);
        }

        src_shift_x += aligned_width * width_increase_x;
        src_shift_y += aligned_width * width_increase_y;

        for (int x = dst_roi_left + aligned_width; x < dst_roi_right; x++)
        {
            int src_x = (src_shift_x >> _PREC_SHIFT);
            int src_y = (src_shift_y >> _PREC_SHIFT);

            if ((src_x >= 0) && (src_x <= src_width - 2) && (src_y >= 0) && (src_y <= src_height - 2))
            {
                const unsigned char *p_pos = p_src + src_y * src_step + src_x;
                const int offset_x = (src_shift_x - (src_x << _PREC_SHIFT));
                const int offset_y = (src_shift_y - (src_y << _PREC_SHIFT));

                int upper_left = p_pos[0];
                int upper_right = p_pos[1];
                int bottom_left = p_pos[src_step];
                int bottom_right = p_pos[src_step + 1];

                p_dst_scan[x] = (BYTE)BilinearInterpolation(upper_left, upper_right, bottom_left, bottom_right, offset_x, offset_y);
            }

            src_shift_x += width_increase_x;
            src_shift_y += width_increase_y;
        }

        p_dst_scan += dst_step;
    }

#else

    for (int y = dst_roi_top; y < dst_roi_bottom; y++)
    {
        BYTE *p_dst_scan = p_dst + y * dst_step;

        // precompute for shared coefficients in a row
        const float current_y = (float)y - fy_shift;
        const float y_sin_theta = current_y * sin_theta;
        const float y_cos_theta = current_y * cos_theta;

        for (int x = dst_roi_left; x < dst_roi_right; x++)
        {
            // Transform each point back to the source image.
            const float current_x = (float)x - fx_shift;

            float src_x = current_x * cos_theta - y_sin_theta;
            float src_y = current_x * sin_theta + y_cos_theta;

            int floor_x = ch_Round(floorf(src_x));
            int floor_y = ch_Round(floorf(src_y));
            int ceiling_x = floor_x + 1;
            int ceiling_y = floor_y + 1;

            float weight[4] = {0};
            bool is_valid = GetInterpolationWeight(src_x, src_y, floor_x, floor_y, ceiling_x, ceiling_y,
                                                   src_roi_left, src_roi_top, src_roi_right, src_roi_bottom, weight);

            if (is_valid)
            {
                const BYTE *p_src_pixel = p_src + floor_y * src_step + floor_x; // top-left pixel
                float value = 0.0f;

                value += weight[0] * (float)p_src_pixel[0];
                value += weight[1] * (float)p_src_pixel[1];
                value += weight[2] * (float)p_src_pixel[src_step];
                value += weight[3] * (float)p_src_pixel[src_step + 1];
                p_dst_scan[x] = (BYTE)FitInRange(ch_Round(value), 0, 255);
            }
        }
    }

#endif

    return ippStsNoErr;
}

IppStatus ippiRotate_8u_C3R(const Ipp8u *p_src, IppiSize src_size, int src_step, IppiRect src_roi,
                            Ipp8u* p_dst, int dst_step, IppiRect dst_roi, double angle, double x_shift, double y_shift, int interpolation)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    // Only support linear interpolation.
    if (interpolation != IPPI_INTER_LINEAR)
        return ippStsInterpolationErr;

    const int src_width = src_size.width;
    const int src_height = src_size.height;
    if (src_width <= 0 || src_height <= 0)
        return ippStsSizeErr;

    const int src_roi_left = ch_Max(src_roi.x, 0);
    const int src_roi_top = ch_Max(src_roi.y, 0);
    const int src_roi_right = ch_Min(src_roi.x + src_roi.width, src_width);
    const int src_roi_bottom = ch_Min(src_roi.y + src_roi.height, src_height);

    if (src_roi_left >= src_roi_right || src_roi_top >= src_roi_bottom)
        return ippStsRectErr;

    const int dst_roi_left = dst_roi.x;
    const int dst_roi_top = dst_roi.y;
    const int dst_roi_right = dst_roi_left + dst_roi.width;
    const int dst_roi_bottom = dst_roi_top + dst_roi.height;

    // Still use float to compute the coordinates for speed concern.
    const float fx_shift = (float)x_shift;
    const float fy_shift = (float)y_shift;

    const double theta = angle * degree_to_radian_factor;
    const float cos_theta = (float)(cos(theta));
    const float sin_theta = (float)(sin(theta));

    for (int y = dst_roi_top; y < dst_roi_bottom; y++)
    {
        BYTE *p_dst_scan = p_dst + y * dst_step;

        // precompute for shared coefficients in a row
        const float current_y = (float)y - fy_shift;
        const float y_sin_theta = current_y * sin_theta;
        const float y_cos_theta = current_y * cos_theta;

        for (int x = dst_roi_left; x < dst_roi_right; x++)
        {
            // Transform each point back to the source image.
            const float current_x = (float)x - fx_shift;

            float src_x = current_x * cos_theta - y_sin_theta;
            float src_y = current_x * sin_theta + y_cos_theta;

            int floor_x = ch_Round(floorf(src_x));
            int floor_y = ch_Round(floorf(src_y));
            int ceiling_x = floor_x + 1;
            int ceiling_y = floor_y + 1;

            float weight[4] = {0};
            bool is_valid = GetInterpolationWeight(src_x, src_y, floor_x, floor_y, ceiling_x, ceiling_y,
                                                   src_roi_left, src_roi_top, src_roi_right, src_roi_bottom, weight);

            if (is_valid)
            {
                const BYTE *p_src_pixel = p_src + floor_y * src_step + floor_x * 3; // top-left pixel
                BYTE *p_dst_pixel = p_dst_scan + x * 3;

                for (int k = 0; k < 3; k++)
                {
                    float value = 0.0f;
                    value += weight[0] * (float)p_src_pixel[k];
                    value += weight[1] * (float)p_src_pixel[3 + k];
                    value += weight[2] * (float)p_src_pixel[src_step + k];
                    value += weight[3] * (float)p_src_pixel[src_step + 3 + k];
                    p_dst_pixel[k] = (BYTE)FitInRange(ch_Round(value), 0, 255);
                }
            }
        }
    }

    return ippStsNoErr;
}

IppStatus ippiRotate_8u_C4R(const Ipp8u *p_src, IppiSize src_size, int src_step, IppiRect src_roi,
                            Ipp8u* p_dst, int dst_step, IppiRect dst_roi, double angle, double x_shift, double y_shift, int interpolation)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    // Only support linear interpolation.
    if (interpolation != IPPI_INTER_LINEAR)
        return ippStsInterpolationErr;

    const int src_width = src_size.width;
    const int src_height = src_size.height;
    if (src_width <= 0 || src_height <= 0)
        return ippStsSizeErr;

    const int src_roi_left = ch_Max(src_roi.x, 0);
    const int src_roi_top = ch_Max(src_roi.y, 0);
    const int src_roi_right = ch_Min(src_roi.x + src_roi.width, src_width);
    const int src_roi_bottom = ch_Min(src_roi.y + src_roi.height, src_height);

    if (src_roi_left >= src_roi_right || src_roi_top >= src_roi_bottom)
        return ippStsRectErr;

    const int dst_roi_left = dst_roi.x;
    const int dst_roi_top = dst_roi.y;
    const int dst_roi_right = dst_roi_left + dst_roi.width;
    const int dst_roi_bottom = dst_roi_top + dst_roi.height;

    // Still use float to compute the coordinates for speed concern.
    const float fx_shift = (float)x_shift;
    const float fy_shift = (float)y_shift;

    const double theta = angle * degree_to_radian_factor;
    const float cos_theta = (float)(cos(theta));
    const float sin_theta = (float)(sin(theta));

    for (int y = dst_roi_top; y < dst_roi_bottom; y++)
    {
        BYTE *p_dst_scan = p_dst + y * dst_step;

        // precompute for shared coefficients in a row
        const float current_y = (float)y - fy_shift;
        const float y_sin_theta = current_y * sin_theta;
        const float y_cos_theta = current_y * cos_theta;

        for (int x = dst_roi_left; x < dst_roi_right; x++)
        {
            // Transform each point back to the source image.
            const float current_x = (float)x - fx_shift;

            float src_x = current_x * cos_theta - y_sin_theta;
            float src_y = current_x * sin_theta + y_cos_theta;

            int floor_x = ch_Round(floorf(src_x));
            int floor_y = ch_Round(floorf(src_y));
            int ceiling_x = floor_x + 1;
            int ceiling_y = floor_y + 1;

            float weight[4] = {0};
            bool is_valid = GetInterpolationWeight(src_x, src_y, floor_x, floor_y, ceiling_x, ceiling_y,
                                                   src_roi_left, src_roi_top, src_roi_right, src_roi_bottom, weight);

            if (is_valid)
            {
                const BYTE *p_src_pixel = p_src + floor_y * src_step + floor_x * 4; // top-left pixel
                BYTE *p_dst_pixel = p_dst_scan + x * 4;

                // Also need to interpolate the alpha values.
                for (int k = 0; k < 4; k++)
                {
                    float value = 0.0f;
                    value += weight[0] * (float)p_src_pixel[k];
                    value += weight[1] * (float)p_src_pixel[4 + k];
                    value += weight[2] * (float)p_src_pixel[src_step + k];
                    value += weight[3] * (float)p_src_pixel[src_step + 4 + k];
                    p_dst_pixel[k] = (BYTE)FitInRange(ch_Round(value), 0, 255);
                }
            }
        }
    }

    return ippStsNoErr;
}

IppStatus ippiRotate_32f_C3R(const Ipp32f *p_src, IppiSize src_size, int src_step, IppiRect src_roi,
                            Ipp32f* p_dst, int dst_step, IppiRect dst_roi, double angle, double x_shift, double y_shift, int interpolation)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;
    
    int src_step_float = src_step / sizeof(float);
    int dst_step_float = dst_step / sizeof(float);

    // Only support linear interpolation.
    if (interpolation != IPPI_INTER_LINEAR)
        return ippStsInterpolationErr;

    const int src_width = src_size.width;
    const int src_height = src_size.height;
    if (src_width <= 0 || src_height <= 0)
        return ippStsSizeErr;

    const int src_roi_left = ch_Max(src_roi.x, 0);
    const int src_roi_top = ch_Max(src_roi.y, 0);
    const int src_roi_right = ch_Min(src_roi.x + src_roi.width, src_width);
    const int src_roi_bottom = ch_Min(src_roi.y + src_roi.height, src_height);

    if (src_roi_left >= src_roi_right || src_roi_top >= src_roi_bottom)
        return ippStsRectErr;

    const int dst_roi_left = dst_roi.x;
    const int dst_roi_top = dst_roi.y;
    const int dst_roi_right = dst_roi_left + dst_roi.width;
    const int dst_roi_bottom = dst_roi_top + dst_roi.height;

    // Still use float to compute the coordinates for speed concern.
    const float fx_shift = (float)x_shift;
    const float fy_shift = (float)y_shift;

    const double theta = angle * degree_to_radian_factor;
    const float cos_theta = (float)(cos(theta));
    const float sin_theta = (float)(sin(theta));

    for (int y = dst_roi_top; y < dst_roi_bottom; y++)
    {
        float *p_dst_scan = p_dst + y * dst_step_float;

        // precompute for shared coefficients in a row
        const float current_y = (float)y - fy_shift;
        const float y_sin_theta = current_y * sin_theta;
        const float y_cos_theta = current_y * cos_theta;

        for (int x = dst_roi_left; x < dst_roi_right; x++)
        {
            // Transform each point back to the source image.
            const float current_x = (float)x - fx_shift;

            float src_x = current_x * cos_theta - y_sin_theta;
            float src_y = current_x * sin_theta + y_cos_theta;

            int floor_x = ch_Round(floorf(src_x));
            int floor_y = ch_Round(floorf(src_y));
            int ceiling_x = floor_x + 1;
            int ceiling_y = floor_y + 1;

            float weight[4] = {0};
            bool is_valid = GetInterpolationWeight(src_x, src_y, floor_x, floor_y, ceiling_x, ceiling_y,
                src_roi_left, src_roi_top, src_roi_right, src_roi_bottom, weight);

            if (is_valid)
            {
                const float *p_src_pixel = p_src + floor_y * src_step_float + floor_x * 3; // top-left pixel
                float *p_dst_pixel = p_dst_scan + x * 3;

                for (int k = 0; k < 3; k++)
                {
                    float value = 0.0f;
                    value += weight[0] * p_src_pixel[k];
                    value += weight[1] * p_src_pixel[3 + k];
                    value += weight[2] * p_src_pixel[src_step_float + k];
                    value += weight[3] * p_src_pixel[src_step_float + 3 + k];
                    p_dst_pixel[k] = value;
                }
            }
        }
    }

    return ippStsNoErr;
}

IppStatus ippiCopyReplicateBorder_8u_C1R(const Ipp8u *p_src, int src_step, IppiSize src_roi_size,
                                         Ipp8u* p_dst, int dst_step, IppiSize dst_roi_size, int top_border_height, int left_border_width)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int src_width = src_roi_size.width;
    const int src_height = src_roi_size.height;
    const int dst_width = dst_roi_size.width;
    const int dst_height = dst_roi_size.height;

    if (src_width <= 0 || src_height <= 0 || dst_width <= 0 || dst_height <= 0)
        return ippStsSizeErr;

    if (top_border_height < 0 || left_border_width < 0)
        return ippStsSizeErr;

    if (dst_width < src_width + left_border_width || dst_height < src_height + top_border_height)
        return ippStsSizeErr;

    const int right_border_width = (dst_width - src_width) - left_border_width;
    const int bottom_border_height = (dst_height - src_height) - top_border_height;

    BYTE *p_dst_origin = p_dst + top_border_height * dst_step + left_border_width; 

    // Copy the source data and do left/right padding.
    for (int y = 0; y < src_height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst_origin + y * dst_step;

        memcpy(p_dst_scan, p_src_scan, src_width);

        if (left_border_width > 0)
        {
            const BYTE value = p_dst_scan[0];
            memset(p_dst_scan - left_border_width, value, left_border_width);
        }

        if (right_border_width > 0)
        {
            const BYTE value = p_dst_scan[src_width - 1];
            memset(p_dst_scan + src_width, value, right_border_width);
        }
    }

    // Do top/bottom padding for the whole row (dst_width).
    BYTE *p_top_row = p_dst + top_border_height * dst_step;
    for (int y = 0; y < top_border_height; y++)
        memcpy(p_dst + y * dst_step, p_top_row, dst_width);

    BYTE *p_bottom_row = p_top_row + (src_height - 1) * dst_step;
    for (int y = 0; y < bottom_border_height; y++)
        memcpy(p_bottom_row + (y + 1) * dst_step, p_bottom_row, dst_width);

    return ippStsNoErr;
}

IppStatus ippiCopyReplicateBorder_8u_C3R(const Ipp8u *p_src, int src_step, IppiSize src_roi_size,
                                         Ipp8u* p_dst, int dst_step, IppiSize dst_roi_size, int top_border_height, int left_border_width)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int src_width = src_roi_size.width;
    const int src_height = src_roi_size.height;
    const int dst_width = dst_roi_size.width;
    const int dst_height = dst_roi_size.height;

    if (src_width <= 0 || src_height <= 0 || dst_width <= 0 || dst_height <= 0)
        return ippStsSizeErr;

    if (top_border_height < 0 || left_border_width < 0)
        return ippStsSizeErr;

    if (dst_width < src_width + left_border_width || dst_height < src_height + top_border_height)
        return ippStsSizeErr;

    const int right_border_width = (dst_width - src_width) - left_border_width;
    const int bottom_border_height = (dst_height - src_height) - top_border_height;

    BYTE *p_dst_origin = p_dst + top_border_height * dst_step + left_border_width * 3; 

    // Copy the source data and do left/right padding.
    for (int y = 0; y < src_height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst_origin + y * dst_step;

        memcpy(p_dst_scan, p_src_scan, src_width * 3);

        const BYTE *p_left_pixel = p_dst_scan;
        BYTE *p_dst_left_pad = p_dst_scan - left_border_width * 3;
        for (int x = 0; x < left_border_width; x++)
        {
            memcpy(p_dst_left_pad, p_left_pixel, 3);
            p_dst_left_pad += 3;
        }

        const BYTE *p_right_pixel = p_dst_scan + (src_width - 1) * 3;
        BYTE *p_dst_right_pad = p_dst_scan + src_width * 3;
        for (int x = 0; x < right_border_width; x++)
        {
            memcpy(p_dst_right_pad, p_right_pixel, 3);
            p_dst_right_pad += 3;
        }
    }

    // Do top/bottom padding for the whole row (dst_width).
    BYTE *p_top_row = p_dst + top_border_height * dst_step;
    for (int y = 0; y < top_border_height; y++)
        memcpy(p_dst + y * dst_step, p_top_row, dst_width * 3);

    BYTE *p_bottom_row = p_top_row + (src_height - 1) * dst_step;
    for (int y = 0; y < bottom_border_height; y++)
        memcpy(p_bottom_row + (y + 1) * dst_step, p_bottom_row, dst_width * 3);

    return ippStsNoErr;
}

IppStatus ippiCopyReplicateBorder_8u_C4R(const Ipp8u *p_src, int src_step, IppiSize src_roi_size,
                                         Ipp8u* p_dst, int dst_step, IppiSize dst_roi_size, int top_border_height, int left_border_width)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int src_width = src_roi_size.width;
    const int src_height = src_roi_size.height;
    const int dst_width = dst_roi_size.width;
    const int dst_height = dst_roi_size.height;

    if (src_width <= 0 || src_height <= 0 || dst_width <= 0 || dst_height <= 0)
        return ippStsSizeErr;

    if (top_border_height < 0 || left_border_width < 0)
        return ippStsSizeErr;

    if (dst_width < src_width + left_border_width || dst_height < src_height + top_border_height)
        return ippStsSizeErr;

    const int right_border_width = (dst_width - src_width) - left_border_width;
    const int bottom_border_height = (dst_height - src_height) - top_border_height;

    BYTE *p_dst_origin = p_dst + top_border_height * dst_step + left_border_width * 4; 

    // Copy the source data and do left/right padding.
    for (int y = 0; y < src_height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst_origin + y * dst_step;

        memcpy(p_dst_scan, p_src_scan, src_width * 4);

        const BYTE *p_left_pixel = p_dst_scan;
        BYTE *p_dst_left_pad = p_dst_scan - left_border_width * 4;
        for (int x = 0; x < left_border_width; x++)
        {
            memcpy(p_dst_left_pad, p_left_pixel, 4);
            p_dst_left_pad += 4;
        }

        const BYTE *p_right_pixel = p_dst_scan + (src_width - 1) * 4;
        BYTE *p_dst_right_pad = p_dst_scan + src_width * 4;
        for (int x = 0; x < right_border_width; x++)
        {
            memcpy(p_dst_right_pad, p_right_pixel, 4);
            p_dst_right_pad += 4;
        }
    }

    // Do top/bottom padding for the whole row (dst_width).
    BYTE *p_top_row = p_dst + top_border_height * dst_step;
    for (int y = 0; y < top_border_height; y++)
        memcpy(p_dst + y * dst_step, p_top_row, dst_width * 4);

    BYTE *p_bottom_row = p_top_row + (src_height - 1) * dst_step;
    for (int y = 0; y < bottom_border_height; y++)
        memcpy(p_bottom_row + (y + 1) * dst_step, p_bottom_row, dst_width * 4);

    return ippStsNoErr;
}

IppStatus ippiMirror_8u_C1IR(Ipp8u *p_srcdst, int srcdst_step, IppiSize roi_size, IppiAxis flip_axis)
{
    if (p_srcdst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    if (flip_axis != ippAxsHorizontal && flip_axis != ippAxsVertical && flip_axis != ippAxsBoth)
        return ippStsMirrorFlipErr;

    const bool is_flip_x = (flip_axis != ippAxsHorizontal);
    const bool is_flip_y = (flip_axis != ippAxsVertical);

    // Horizontal flip (vertical axis)
    if (is_flip_x)
    {
        const int half_width = width / 2;

        for (int y = 0; y < height; y++)
        {
            BYTE *p_srcdst_scan = p_srcdst + y * srcdst_step;

            for (int x = 0; x < half_width; x++)
            {
                int flip_x = (width - 1) - x;
                ch_Swap<BYTE>(p_srcdst_scan[x], p_srcdst_scan[flip_x]);
            }
        }
    }

    if (is_flip_y)
    {
        const int half_height = height / 2;

        for (int x = 0; x < width; x++)
        {
            BYTE *p_srcdst_scan = p_srcdst + x;

            for (int y = 0; y < half_height; y++)
            {
                int flip_y = (height - 1) - y;
                ch_Swap<BYTE>(p_srcdst_scan[y * srcdst_step], p_srcdst_scan[flip_y * srcdst_step]);
            }
        }
    }

    return ippStsNoErr;
}

IppStatus ippiMirror_8u_C3IR(Ipp8u *p_srcdst, int srcdst_step, IppiSize roi_size, IppiAxis flip_axis)
{
    if (p_srcdst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    if (flip_axis != ippAxsHorizontal && flip_axis != ippAxsVertical && flip_axis != ippAxsBoth)
        return ippStsMirrorFlipErr;

    const bool is_flip_x = (flip_axis != ippAxsHorizontal);
    const bool is_flip_y = (flip_axis != ippAxsVertical);

    // Horizontal flip (vertical axis)
    if (is_flip_x)
    {
        const int half_width = width / 2;

        for (int y = 0; y < height; y++)
        {
            BYTE *p_srcdst_scan = p_srcdst + y * srcdst_step;

            for (int x = 0; x < half_width; x++)
            {
                int flip_x = (width - 1) - x;

                BYTE *p_left_pixel = p_srcdst_scan + x * 3;
                BYTE *p_right_pixel = p_srcdst_scan + flip_x * 3;
                ch_Swap<BYTE>(p_left_pixel[0], p_right_pixel[0]);
                ch_Swap<BYTE>(p_left_pixel[1], p_right_pixel[1]);
                ch_Swap<BYTE>(p_left_pixel[2], p_right_pixel[2]);
            }
        }
    }

    if (is_flip_y)
    {
        const int half_height = height / 2;

        for (int x = 0; x < width; x++)
        {
            BYTE *p_srcdst_scan = p_srcdst + x * 3;

            for (int y = 0; y < half_height; y++)
            {
                int flip_y = (height - 1) - y;

                BYTE *p_top_pixel = p_srcdst_scan + y * srcdst_step;
                BYTE *p_bottom_pixel = p_srcdst_scan + flip_y * srcdst_step;
                ch_Swap<BYTE>(p_top_pixel[0], p_bottom_pixel[0]);
                ch_Swap<BYTE>(p_top_pixel[1], p_bottom_pixel[1]);
                ch_Swap<BYTE>(p_top_pixel[2], p_bottom_pixel[2]);
            }
        }
    }

    return ippStsNoErr;
}

IppStatus ippiMirror_8u_C4IR(Ipp8u *p_srcdst, int srcdst_step, IppiSize roi_size, IppiAxis flip_axis)
{
    if (p_srcdst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    if (flip_axis != ippAxsHorizontal && flip_axis != ippAxsVertical && flip_axis != ippAxsBoth)
        return ippStsMirrorFlipErr;

    const bool is_flip_x = (flip_axis != ippAxsHorizontal);
    const bool is_flip_y = (flip_axis != ippAxsVertical);

    // Horizontal flip (vertical axis)
    if (is_flip_x)
    {
        const int half_width = width / 2;

        for (int y = 0; y < height; y++)
        {
            BYTE *p_srcdst_scan = p_srcdst + y * srcdst_step;

            for (int x = 0; x < half_width; x++)
            {
                int flip_x = (width - 1) - x;

                BYTE *p_left_pixel = p_srcdst_scan + x * 4;
                BYTE *p_right_pixel = p_srcdst_scan + flip_x * 4;
                ch_Swap<BYTE>(p_left_pixel[0], p_right_pixel[0]);
                ch_Swap<BYTE>(p_left_pixel[1], p_right_pixel[1]);
                ch_Swap<BYTE>(p_left_pixel[2], p_right_pixel[2]);
                ch_Swap<BYTE>(p_left_pixel[3], p_right_pixel[3]);
            }
        }
    }

    if (is_flip_y)
    {
        const int half_height = height / 2;

        for (int x = 0; x < width; x++)
        {
            BYTE *p_srcdst_scan = p_srcdst + x * 4;

            for (int y = 0; y < half_height; y++)
            {
                int flip_y = (height - 1) - y;

                BYTE *p_top_pixel = p_srcdst_scan + y * srcdst_step;
                BYTE *p_bottom_pixel = p_srcdst_scan + flip_y * srcdst_step;
                ch_Swap<BYTE>(p_top_pixel[0], p_bottom_pixel[0]);
                ch_Swap<BYTE>(p_top_pixel[1], p_bottom_pixel[1]);
                ch_Swap<BYTE>(p_top_pixel[2], p_bottom_pixel[2]);
                ch_Swap<BYTE>(p_top_pixel[3], p_bottom_pixel[3]);
            }
        }
    }

    return ippStsNoErr;
}

IppStatus ippiTranspose_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    if (src_step <= 0 || dst_step <= 0)
        return ippStsStepErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y;

        for (int x = 0; x < width; x++)
        {
            p_dst_scan[x * dst_step] = p_src_scan[x];
        }
    }

    return ippStsNoErr;
}

IppStatus ippiErode_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize dst_roi_size,
                           const Ipp8u *p_mask, IppiSize mask_size, IppiPoint anchor)
{
    if (p_src == NULL || p_dst == NULL || p_mask == NULL)
        return ippStsNullPtrErr;

    const int width = dst_roi_size.width;
    const int height = dst_roi_size.height;
    const int mask_width = mask_size.width;
    const int mask_height = mask_size.height;

    if (width <= 0 || height <= 0 || mask_width <= 0 || mask_height <= 0)
        return ippStsSizeErr;

    if (anchor.x < 0 || anchor.y < 0 || anchor.x >= mask_width || anchor.y >= mask_height)
        return ippStsAnchorErr;

    bool is_zero_mask = true;
    for (int i = 0; i < mask_width * mask_height; i++)
    {
        if (p_mask[i] > 0)
        {
            is_zero_mask = false;
            break;
        }
    }

    if (is_zero_mask)
        return ippStsZeroMaskValuesErr;

    // Precompute the offsets from a source pixels to all valid pixels indicated by the mask.
    // (Only non-zero mask pixels are considered.)

    int *p_offsets = new int[mask_width * mask_height];
    int valid_count = 0;

    for (int y = 0; y < mask_height; y++)
    {
        const int dy = y - anchor.y;
        const BYTE *p_mask_scan = p_mask + y * mask_width;
        
        for (int x = 0; x < mask_width; x++)
        {
            if (p_mask_scan[x] > 0)
            {
                const int dx = x - anchor.x;

                p_offsets[valid_count] = dy * src_step + dx;
                valid_count++;
            }
        }
    }

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            const BYTE *p_src_pixel = p_src_scan + x;
            BYTE min_value = 255;

            for (int i = 0; i < valid_count; i++)
                min_value = ch_Min(min_value, p_src_pixel[p_offsets[i]]);

            p_dst_scan[x] = min_value;
        }
    }

    delete [] p_offsets;

    return ippStsNoErr;
}

IppStatus ippiDilate_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize dst_roi_size,
                           const Ipp8u *p_mask, IppiSize mask_size, IppiPoint anchor)
{
    if (p_src == NULL || p_dst == NULL || p_mask == NULL)
        return ippStsNullPtrErr;

    const int width = dst_roi_size.width;
    const int height = dst_roi_size.height;
    const int mask_width = mask_size.width;
    const int mask_height = mask_size.height;

    if (width <= 0 || height <= 0 || mask_width <= 0 || mask_height <= 0)
        return ippStsSizeErr;

    if (anchor.x < 0 || anchor.y < 0 || anchor.x >= mask_width || anchor.y >= mask_height)
        return ippStsAnchorErr;

    bool is_zero_mask = true;
    for (int i = 0; i < mask_width * mask_height; i++)
    {
        if (p_mask[i] > 0)
        {
            is_zero_mask = false;
            break;
        }
    }

    if (is_zero_mask)
        return ippStsZeroMaskValuesErr;

    // Precompute the offsets from a source pixels to all valid pixels indicated by the mask.
    // (Only non-zero mask pixels are considered.)

    int *p_offsets = new int[mask_width * mask_height];
    int valid_count = 0;

    for (int y = 0; y < mask_height; y++)
    {
        const int dy = y - anchor.y;
        const BYTE *p_mask_scan = p_mask + y * mask_width;
        
        for (int x = 0; x < mask_width; x++)
        {
            if (p_mask_scan[x] > 0)
            {
                const int dx = x - anchor.x;

                p_offsets[valid_count] = dy * src_step + dx;
                valid_count++;
            }
        }
    }

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            const BYTE *p_src_pixel = p_src_scan + x;
            BYTE max_value = 0;

            for (int i = 0; i < valid_count; i++)
                max_value = ch_Max(max_value, p_src_pixel[p_offsets[i]]);

            p_dst_scan[x] = max_value;
        }
    }

    delete [] p_offsets;

    return ippStsNoErr;
}

// Border-replicated Erosion / Dilation:
// A structure IppiMorphState is used to record the mask/anchor information and provide the buffer.
// It is allocated by ippiMorphologyInitAlloc_<type>, and freed by ippiMorphologyFree.

// Since we cannot know the actual structure of IppiMorphState in Intel IPP,
// we implement the structure to fit the implemented erode/dilate algorithm.

IppStatus ippiMorphologyInitAlloc_8u_C1R(int roi_width, const Ipp8u *p_mask, IppiSize mask_size, IppiPoint anchor, IppiMorphState **pp_state)
{
    if (p_mask == NULL || pp_state == NULL)
        return ippStsNullPtrErr;

    const int mask_width = mask_size.width;
    const int mask_height = mask_size.height;

    if (roi_width <= 0 || mask_width <= 0 || mask_height <= 0)
        return ippStsSizeErr;

    if (anchor.x < 0 || anchor.y < 0 || anchor.x >= mask_width || anchor.y >= mask_height)
        return ippStsAnchorErr;

    (*pp_state) = new IppiMorphState;
    IppiMorphState &state = **pp_state;

    state.roi_width = roi_width;
    state.mask_size = mask_size;
    state.anchor = anchor;

    _IPP_ALIGNED_MALLOC_PTR(state.p_mask, BYTE, mask_width * mask_height);
    memcpy(state.p_mask, p_mask, mask_width * mask_height);

    // The buffer is used to store the padded data of the top/bottom part for the source image.

    const int top_pad = anchor.y;
    const int bottom_pad = (mask_height - 1) - anchor.y;

    int top_pad_area = (roi_width + (mask_width - 1)) * (top_pad + (mask_height - 1));
    int bottom_pad_area = (roi_width + (mask_width - 1)) * (bottom_pad + (mask_height - 1));
    int max_buffer_size = ch_Max(top_pad_area, bottom_pad_area);

    _IPP_ALIGNED_MALLOC_PTR(state.p_buffer, BYTE, max_buffer_size);
    state.buffer_size = max_buffer_size;

    return ippStsNoErr;
}

IppStatus ippiMorphologyFree(IppiMorphState *p_state)
{
    if (p_state == NULL)
        return ippStsNullPtrErr;

    _IPP_ALIGNED_FREE_PTR(p_state->p_mask);
    _IPP_ALIGNED_FREE_PTR(p_state->p_buffer);

    delete p_state;

    return ippStsNoErr;
}

// The source image is padded until it is enough to perform the operation.
// We divide the ROI into 5 parts:
// |-------------------------|
// |            1            |
// |-----|-------------|-----|
// |     |             |     |
// |  2  |      3      |  4  |
// |     |             |     |
// |-----|-------------|-----|
// |            5            |
// |-------------------------|
//
// Part 3 is the maximal region that does not require any padding.
// Part 1 and 5 use the buffer in IppiMorphState to pad the source image.
// Part 2 and 4 use the buffer if its size or enough, or use internal allocated buffer.
// (The size of part 1, 5 is known in ippiMorphologyInitAlloc_<type>, but the size of part 2, 4 is not.)

inline void MorphImageByPadding_8u_C1R(const BYTE *p_src, int src_step, const IppiSize &src_size,
                                              BYTE *p_dst, int dst_step, const HyRect &roi_rect,
                                              const BYTE *p_mask, const IppiSize &mask_size, const IppiPoint &anchor,
                                              BYTE *p_external_buffer, int external_buffer_size, bool is_erode)
{
    _MYASSERT(p_src);
    _MYASSERT(p_dst);

    // A generalized function to perform erosion or dilation for the ROI of an image.
    // The image is padded to have enough size for the operation.
    // The results are put into the same ROI of the destination image.

    // p_buffer is a pre-allocated buffer with a known size.
    // If it is enough, the buffer is used. Otherwise, internal-allocated buffer is used.

    if (roi_rect.width <= 0 || roi_rect.height <= 0)
        return;

    // The space from the anchor to the mask boundary.
    const int left_space = anchor.x;
    const int top_space = anchor.y;
    const int right_space = (mask_size.width - 1) - anchor.x;
    const int bottom_space = (mask_size.height - 1) - anchor.y;

    // The enlarged rectangle with enough size for the operation.
    HyRect buffer_rect = hyEnlargeRect(roi_rect, left_space, top_space, right_space, bottom_space);
    IppiSize buffer_rect_size = {buffer_rect.width, buffer_rect.height};

    // The valid region for the source image. The invalid part must be padded.
    HyRect valid_rect = hyIntersectRect(buffer_rect, hyRect(0, 0, src_size.width, src_size.height));
    IppiSize valid_size = {valid_rect.width, valid_rect.height};

    // Check the buffer size and prepare the buffer.
    BYTE *p_internal_buffer = NULL;
    const int buffer_size = buffer_rect.width * buffer_rect.height;

    BYTE *p_buffer = p_external_buffer;
    if (buffer_size > external_buffer_size)
    {
        _IPP_ALIGNED_MALLOC_PTR(p_internal_buffer, BYTE, buffer_size);
        p_buffer = p_internal_buffer;
    }

    // Copy and pad the source image to the buffer.
    ippiCopyReplicateBorder_8u_C1R(p_src + valid_rect.y * src_step + valid_rect.x, src_step, valid_size,
                                   p_buffer, buffer_rect.width, buffer_rect_size,
                                   valid_rect.y - buffer_rect.y, valid_rect.x - buffer_rect.x);

    _MYASSERT(p_buffer);

    // Perform the morph operation.
    const BYTE *p_buffer_start = p_buffer + top_space * buffer_rect.width + left_space;
    BYTE *p_dst_start = p_dst + roi_rect.y * dst_step + roi_rect.x;
    const IppiSize roi_size = {roi_rect.width, roi_rect.height};

    if (is_erode)
        ippiErode_8u_C1R(p_buffer_start, buffer_rect.width, p_dst_start, dst_step, roi_size, p_mask, mask_size, anchor);
    else
        ippiDilate_8u_C1R(p_buffer_start, buffer_rect.width, p_dst_start, dst_step, roi_size, p_mask, mask_size, anchor);

    _IPP_ALIGNED_FREE_PTR(p_internal_buffer);
}

IppStatus ippiErodeBorderReplicate_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                                          IppiSize roi_size, IppiBorderType border_type, IppiMorphState *p_state)
{
    if (p_src == NULL || p_dst == NULL || p_state == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    // ROI width can be smaller than or equal to the width in morph state, but cannot be greater.
    if (width > p_state->roi_width)
        return ippStsSizeErr;

    if (border_type != ippBorderRepl)
        return ippStsBorderErr;

    const BYTE *p_mask = p_state->p_mask;
    const IppiSize &mask_size = p_state->mask_size;
    const IppiPoint &anchor = p_state->anchor;
    BYTE *p_buffer = p_state->p_buffer;
    const int buffer_size = p_state->buffer_size;

    bool is_zero_mask = true;
    for (int i = 0; i < mask_size.width * mask_size.height; i++)
    {
        if (p_mask[i] > 0)
        {
            is_zero_mask = false;
            break;
        }
    }

    if (is_zero_mask)
    {
        // For all-zero mask, follow the behavior of Intel IPP:
        // Just copy the source data to destination data and return OK.

        ippiCopy_8u_C1R(p_src, src_step, p_dst, dst_step, roi_size);

        return ippStsNoErr;
    }

    const int left_pad = anchor.x;
    const int top_pad = anchor.y;
    const int right_pad = (mask_size.width - 1) - anchor.x;
    const int bottom_pad = (mask_size.height - 1) - anchor.y;

    // Perform the center part that does not require padding.
    int center_left = left_pad;
    int center_top = top_pad;
    int center_right = width - right_pad;
    int center_bottom = height - bottom_pad;

    if (center_right > center_left && center_bottom > center_top)
    {
        const BYTE *p_src_start = p_src + center_top * src_step + center_left;
        BYTE *p_dst_start = p_dst + center_top * dst_step + center_left;
        IppiSize center_roi_size = {center_right - center_left, center_bottom - center_top};

        ippiErode_8u_C1R(p_src_start, src_step, p_dst_start, dst_step, center_roi_size, p_mask, mask_size, anchor);
    }

    // Perform the top part.
    int bottom_of_top_roi = ch_Min(top_pad, height);
    HyRect top_roi_rect = hyRect(0, 0, width, bottom_of_top_roi);

    MorphImageByPadding_8u_C1R(p_src, src_step, roi_size, p_dst, dst_step, top_roi_rect,
                               p_mask, mask_size, anchor, p_buffer, buffer_size, true);

    // Perform the bottom part.
    // If the ROI height is very small, the top and bottom parts can cover the ROI and we can skip the other parts.
    int top_of_bottom_roi = ch_Max(height - bottom_pad, 0);
    HyRect bottom_roi_rect = hyRect(0, top_of_bottom_roi, width, height - top_of_bottom_roi);

    bool is_cover = false;
    if (bottom_roi_rect.y <= bottom_of_top_roi)
    {
        bottom_roi_rect.y = bottom_of_top_roi;
        bottom_roi_rect.height = height - bottom_of_top_roi;
        is_cover = true;
    }

    // Sometimes only the top ROI cover the whole height. Perform bottom ROI only when needed.
    if (bottom_roi_rect.height > 0)
    {
        MorphImageByPadding_8u_C1R(p_src, src_step, roi_size, p_dst, dst_step, bottom_roi_rect,
                                   p_mask, mask_size, anchor, p_buffer, buffer_size, true);
    }

    if (is_cover == false)
    {
        const int y_start = bottom_of_top_roi;
        const int y_range = top_of_bottom_roi - bottom_of_top_roi;

        // Perform the left part.
        int right_of_left_roi = ch_Min(left_pad, width);
        HyRect left_roi_rect = hyRect(0, y_start, right_of_left_roi, y_range);

        MorphImageByPadding_8u_C1R(p_src, src_step, roi_size, p_dst, dst_step, left_roi_rect,
                                   p_mask, mask_size, anchor, p_buffer, buffer_size, true);

        // Perform the right part.
        int left_of_right_roi = ch_Max(width - right_pad, 0);
        HyRect right_roi_rect = hyRect(left_of_right_roi, y_start, width - left_of_right_roi, y_range);

        if (right_roi_rect.x <= right_of_left_roi)
        {
            right_roi_rect.x = right_of_left_roi;
            right_roi_rect.width = width - right_of_left_roi;
        }

        // Sometimes only the left ROI cover the whole width. Perform right ROI only when needed.
        if (right_roi_rect.width > 0)
        {
            MorphImageByPadding_8u_C1R(p_src, src_step, roi_size, p_dst, dst_step, right_roi_rect,
                                       p_mask, mask_size, anchor, p_buffer, buffer_size, true);
        }
    }

    return ippStsNoErr;
}

IppStatus ippiDilateBorderReplicate_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                                           IppiSize roi_size, IppiBorderType border_type, IppiMorphState *p_state)
{
    if (p_src == NULL || p_dst == NULL || p_state == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    // ROI width can be smaller than or equal to the width in morph state, but cannot be greater.
    if (width > p_state->roi_width)
        return ippStsSizeErr;

    if (border_type != ippBorderRepl)
        return ippStsBorderErr;

    const BYTE *p_mask = p_state->p_mask;
    const IppiSize &mask_size = p_state->mask_size;
    const IppiPoint &anchor = p_state->anchor;
    BYTE *p_buffer = p_state->p_buffer;
    const int buffer_size = p_state->buffer_size;

    bool is_zero_mask = true;
    for (int i = 0; i < mask_size.width * mask_size.height; i++)
    {
        if (p_mask[i] > 0)
        {
            is_zero_mask = false;
            break;
        }
    }

    if (is_zero_mask)
    {
        // For all-zero mask, follow the behavior of Intel IPP:
        // Just copy the source data to destination data and return OK.

        ippiCopy_8u_C1R(p_src, src_step, p_dst, dst_step, roi_size);

        return ippStsNoErr;
    }

    const int left_pad = anchor.x;
    const int top_pad = anchor.y;
    const int right_pad = (mask_size.width - 1) - anchor.x;
    const int bottom_pad = (mask_size.height - 1) - anchor.y;

    // Perform the center part that does not require padding.
    int center_left = left_pad;
    int center_top = top_pad;
    int center_right = width - right_pad;
    int center_bottom = height - bottom_pad;

    if (center_right > center_left && center_bottom > center_top)
    {
        const BYTE *p_src_start = p_src + center_top * src_step + center_left;
        BYTE *p_dst_start = p_dst + center_top * dst_step + center_left;
        IppiSize center_roi_size = {center_right - center_left, center_bottom - center_top};

        ippiDilate_8u_C1R(p_src_start, src_step, p_dst_start, dst_step, center_roi_size, p_mask, mask_size, anchor);
    }

    // Perform the top part.
    int bottom_of_top_roi = ch_Min(top_pad, height);
    HyRect top_roi_rect = hyRect(0, 0, width, bottom_of_top_roi);

    MorphImageByPadding_8u_C1R(p_src, src_step, roi_size, p_dst, dst_step, top_roi_rect,
                               p_mask, mask_size, anchor, p_buffer, buffer_size, false);

    // Perform the bottom part.
    // If the ROI height is very small, the top and bottom parts can cover the ROI and we can skip the other parts.
    int top_of_bottom_roi = ch_Max(height - bottom_pad, 0);
    HyRect bottom_roi_rect = hyRect(0, top_of_bottom_roi, width, height - top_of_bottom_roi);

    bool is_cover = false;
    if (bottom_roi_rect.y <= bottom_of_top_roi)
    {
        bottom_roi_rect.y = bottom_of_top_roi;
        bottom_roi_rect.height = height - bottom_of_top_roi;
        is_cover = true;
    }

    // Sometimes only the top ROI cover the whole height. Perform bottom ROI only when needed.
    if (bottom_roi_rect.height > 0)
    {
        MorphImageByPadding_8u_C1R(p_src, src_step, roi_size, p_dst, dst_step, bottom_roi_rect,
                                   p_mask, mask_size, anchor, p_buffer, buffer_size, false);
    }

    if (is_cover == false)
    {
        const int y_start = bottom_of_top_roi;
        const int y_range = top_of_bottom_roi - bottom_of_top_roi;

        // Perform the left part.
        int right_of_left_roi = ch_Min(left_pad, width);
        HyRect left_roi_rect = hyRect(0, y_start, right_of_left_roi, y_range);

        MorphImageByPadding_8u_C1R(p_src, src_step, roi_size, p_dst, dst_step, left_roi_rect,
                                   p_mask, mask_size, anchor, p_buffer, buffer_size, false);

        // Perform the right part.
        int left_of_right_roi = ch_Max(width - right_pad, 0);
        HyRect right_roi_rect = hyRect(left_of_right_roi, y_start, width - left_of_right_roi, y_range);

        if (right_roi_rect.x <= right_of_left_roi)
        {
            right_roi_rect.x = right_of_left_roi;
            right_roi_rect.width = width - right_of_left_roi;
        }

        // Sometimes only the left ROI cover the whole width. Perform right ROI only when needed.
        if (right_roi_rect.width > 0)
        {
            MorphImageByPadding_8u_C1R(p_src, src_step, roi_size, p_dst, dst_step, right_roi_rect,
                                       p_mask, mask_size, anchor, p_buffer, buffer_size, false);
        }
    }

    return ippStsNoErr;
}

IppStatus ippiIntegral_8u32s_C1R(const Ipp8u *p_src, int src_step, Ipp32s *p_dst, int dst_step, IppiSize roi_size, Ipp32s add_value)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    // dst_step should be divisible by units.
    if (dst_step % 4 != 0)
        return ippStsNotEvenStepErr;

    const int sum_width = width + 1;
    const int sum_height = height + 1;

    // Set top row to zero
    memset(p_dst, 0, sizeof(int) * sum_width);

    // Set left column to zero
    for (int y = 1; y < sum_height; y++)
    {
        int *p_dst_scan = (int *)((BYTE *)p_dst + y * dst_step);
        p_dst_scan[0] = 0;
    }

    const BYTE *p_src_data = p_src;
    int *p_sum = (int *)((BYTE *)p_dst + dst_step) + 1;

    for (int y = 0; y < height; y++)
    {
        int *p_prev_row_sum = (int *)((BYTE *)p_sum - dst_step);

        int top_left_sum = p_prev_row_sum[-1];
        int left_sum = p_sum[-1];

        for (int x = 0; x < width; x++)
        {
            int value = (int)p_src_data[x];

            int sum = left_sum - top_left_sum + value;
            top_left_sum = p_prev_row_sum[x];
            sum += top_left_sum;
            left_sum = sum;
            p_sum[x] = sum;
        }

        p_src_data += src_step;
        p_sum = (int *)((BYTE *)p_sum + dst_step);
    }

    if (add_value != 0)
    {
        for (int y = 0; y < sum_height; y++)
        {
            int *p_dst_scan = (int *)((BYTE *)p_dst + y * dst_step);

            for (int x = 0; x < sum_width; x++)
                p_dst_scan[x] += add_value;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiSqrIntegral_8u32s_C1R(const Ipp8u *p_src, int src_step, Ipp32s *p_dst, int dst_step,
                                    Ipp32s *p_square, int square_step, IppiSize roi_size, Ipp32s add_value, Ipp32s add_value_square)
{
    if (p_src == NULL || p_dst == NULL || p_square == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    // dst_step and square_step should be divisible by units.
    if (dst_step % 4 != 0 || square_step % 4 != 0)
        return ippStsNotEvenStepErr;

    const int sum_width = width + 1;
    const int sum_height = height + 1;

    // Set top row to zero
    memset(p_dst, 0, sizeof(int) * sum_width);
    memset(p_square, 0, sizeof(int) * sum_width);

    // Set left column to zero
    for (int y = 1; y < sum_height; y++)
    {
        int *p_dst_scan = (int *)((BYTE *)p_dst + y * dst_step);
        int *p_square_scan = (int *)((BYTE *)p_square + y * square_step);

        p_dst_scan[0] = 0;
        p_square_scan[0] = 0;
    }

    const BYTE *p_src_data = p_src;
    int *p_sum = (int *)((BYTE *)p_dst + dst_step) + 1;
    int *p_square_sum = (int *)((BYTE *)p_square + square_step) + 1;

    for (int y = 0; y < height; y++)
    {
        int *p_prev_row_sum = (int *)((BYTE *)p_sum - dst_step);
        int *p_prev_row_square_sum = (int *)((BYTE *)p_square_sum - square_step);

        int top_left_sum = p_prev_row_sum[-1];
        int left_sum = p_sum[-1];

        int top_left_square_sum = p_prev_row_square_sum[-1];
        int left_square_sum = p_square_sum[-1];

        for (int x = 0; x < width; x++)
        {
            int value = (int)p_src_data[x];
            int square_value = value * value;

            int sum = left_sum - top_left_sum + value;
            top_left_sum = p_prev_row_sum[x];
            sum += top_left_sum;
            left_sum = sum;
            p_sum[x] = sum;

            int square_sum = left_square_sum - top_left_square_sum + square_value;
            top_left_square_sum = p_prev_row_square_sum[x];
            square_sum += top_left_square_sum;
            left_square_sum = square_sum;
            p_square_sum[x] = square_sum;
        }

        p_src_data += src_step;
        p_sum = (int *)((BYTE *)p_sum + dst_step);
        p_square_sum = (int *)((BYTE *)p_square_sum + square_step);
    }

    if (add_value != 0)
    {
        for (int y = 0; y < sum_height; y++)
        {
            int *p_dst_scan = (int *)((BYTE *)p_dst + y * dst_step);

            for (int x = 0; x < sum_width; x++)
                p_dst_scan[x] += add_value;
        }
    }

    if (add_value_square != 0)
    {
        for (int y = 0; y < sum_height; y++)
        {
            int *p_square_scan = (int *)((BYTE *)p_square + y * square_step);

            for (int x = 0; x < sum_width; x++)
                p_square_scan[x] += add_value_square;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiSqrIntegral_8u32f64f_C1R(const Ipp8u *p_src, int src_step, Ipp32f* p_dst, int dst_step,
                                       Ipp64f *p_square, int square_step, IppiSize roi_size, Ipp32f add_value, Ipp64f add_value_square)
{
    if (p_src == NULL || p_dst == NULL || p_square == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    // dst_step and square_step should be divisible by units.
    if (dst_step % 4 != 0 || square_step % 8 != 0)
        return ippStsNotEvenStepErr;

    const int sum_width = width + 1;
    const int sum_height = height + 1;

    // Set top row to zero
    memset(p_dst, 0, sizeof(float) * sum_width);
    memset(p_square, 0, sizeof(double) * sum_width);

    // Set left column to zero
    for (int y = 1; y < sum_height; y++)
    {
        float *p_dst_scan = (float *)((BYTE *)p_dst + y * dst_step);
        double *p_square_scan = (double *)((BYTE *)p_square + y * square_step);

        p_dst_scan[0] = 0.0f;
        p_square_scan[0] = 0.0;
    }

    const BYTE *p_src_data = p_src;
    float *p_sum = (float *)((BYTE *)p_dst + dst_step) + 1;
    double *p_square_sum = (double *)((BYTE *)p_square + square_step) + 1;

    for (int y = 0; y < height; y++)
    {
        float *p_prev_row_sum = (float *)((BYTE *)p_sum - dst_step);
        double *p_prev_row_square_sum = (double *)((BYTE *)p_square_sum - square_step);

        float top_left_sum = p_prev_row_sum[-1];
        float left_sum = p_sum[-1];

        double top_left_square_sum = p_prev_row_square_sum[-1];
        double left_square_sum = p_square_sum[-1];

        for (int x = 0; x < width; x++)
        {
            float value = (float)p_src_data[x];
            double square_value = (double)(value * value);

            float sum = 0.0f;
            sum = left_sum - top_left_sum + value;
            top_left_sum = p_prev_row_sum[x];
            sum += top_left_sum;
            left_sum = sum;
            p_sum[x] = sum;

            double square_sum = 0;
            square_sum = left_square_sum - top_left_square_sum + square_value;
            top_left_square_sum = p_prev_row_square_sum[x];
            square_sum += top_left_square_sum;
            left_square_sum = square_sum;
            p_square_sum[x] = square_sum;
        }

        p_src_data += src_step;
        p_sum = (float *)((BYTE *)p_sum + dst_step);
        p_square_sum = (double *)((BYTE *)p_square_sum + square_step);
    }

    if (add_value != 0.0f)
    {
        for (int y = 0; y < sum_height; y++)
        {
            float *p_dst_scan = (float *)((BYTE *)p_dst + y * dst_step);

            for (int x = 0; x < sum_width; x++)
                p_dst_scan[x] += add_value;
        }
    }

    if (add_value_square != 0.0)
    {
        for (int y = 0; y < sum_height; y++)
        {
            double *p_square_scan = (double *)((BYTE *)p_square + y * square_step);

            for (int x = 0; x < sum_width; x++)
                p_square_scan[x] += add_value_square;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiTiltedIntegral_8u32f_C1R(const Ipp8u *p_src, int src_step, Ipp32f* p_dst, int dst_step, IppiSize roi_size, Ipp32f add_value)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    // dst_step should be divisible by units.
    if (dst_step % 4 != 0)
        return ippStsNotEvenStepErr;

    // For tilted sum, the width and height of sum image are increased by 2.
    const int sum_width = width + 2;
    const int sum_height = height + 2;

    for (int y = 0; y < 2; y++)
    {
        float *p_dst_scan = (float *)((BYTE *)p_dst + y * dst_step);
        memset(p_dst_scan, 0, sizeof(float) * sum_width);
    }

    const BYTE *p_src_data = p_src;
    float *p_tilted_sum = (float *)((BYTE *)p_dst + 2 * dst_step);

    for (int y = 0; y < height; y++)
    {
        float *p_prev1_row_tilted_sum = (float *)((BYTE *)p_tilted_sum - dst_step);
        float *p_prev2_row_tilted_sum = (float *)((BYTE *)p_tilted_sum - 2 * dst_step);

        for (int x = 0; x < sum_width; x++)
        {       
            if (x <= 0)
            {
                p_tilted_sum[x] = p_prev1_row_tilted_sum[x + 1];
            }
            else if (x < width + 1)
            {
                float value = 0.0f;
                if (x < width)
                    value += (float)p_src_data[x];
                if (x - 1 < width)
                    value += (float)p_src_data[x - 1];

                p_tilted_sum[x] = p_prev1_row_tilted_sum[x - 1]
                                + p_prev1_row_tilted_sum[x + 1]
                                - p_prev2_row_tilted_sum[x] + value;
            }
            else
            {
                p_tilted_sum[x] = p_prev1_row_tilted_sum[x - 1];
            }
        }

        p_src_data += src_step;
        p_tilted_sum = (float *)((BYTE *)p_tilted_sum + dst_step);
    }

    if (add_value != 0.0f)
    {
        for (int y = 0; y < sum_height; y++)
        {
            float *p_dst_scan = (float *)((BYTE *)p_dst + y * dst_step);

            for (int x = 0; x < sum_width; x++)
                p_dst_scan[x] += add_value;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiTiltedIntegral_8u32s_C1R(const Ipp8u *p_src, int src_step, Ipp32s *p_dst, int dst_step, IppiSize roi_size, Ipp32s add_value)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;

    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    // dst_step should be divisible by units.
    if (dst_step % 4 != 0)
        return ippStsNotEvenStepErr;

    // For tilted sum, the width and height of sum image are increased by 2.
    const int sum_width = width + 2;
    const int sum_height = height + 2;

    for (int y = 0; y < 2; y++)
    {
        int *p_dst_scan = (int *)((BYTE *)p_dst + y * dst_step);
        memset(p_dst_scan, 0, sizeof(int) * sum_width);
    }

    const BYTE *p_src_data = p_src;
    int *p_tilted_sum = (int *)((BYTE *)p_dst + 2 * dst_step);

    for (int y = 0; y < height; y++)
    {
        int *p_prev1_row_tilted_sum = (int *)((BYTE *)p_tilted_sum - dst_step);
        int *p_prev2_row_tilted_sum = (int *)((BYTE *)p_tilted_sum - 2 * dst_step);

        for (int x = 0; x < sum_width; x++)
        {       
            if (x <= 0)
            {
                p_tilted_sum[x] = p_prev1_row_tilted_sum[x + 1];
            }
            else if (x < width + 1)
            {
                int value = 0;
                if (x < width)
                    value += (int)p_src_data[x];
                if (x - 1 < width)
                    value += (int)p_src_data[x - 1];

                p_tilted_sum[x] = p_prev1_row_tilted_sum[x - 1]
                                + p_prev1_row_tilted_sum[x + 1]
                                - p_prev2_row_tilted_sum[x] + value;
            }
            else
            {
                p_tilted_sum[x] = p_prev1_row_tilted_sum[x - 1];
            }
        }

        p_src_data += src_step;
        p_tilted_sum = (int *)((BYTE *)p_tilted_sum + dst_step);
    }

    if (add_value != 0)
    {
        for (int y = 0; y < sum_height; y++)
        {
            int *p_dst_scan = (int *)((BYTE *)p_dst + y * dst_step);

            for (int x = 0; x < sum_width; x++)
                p_dst_scan[x] += add_value;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiConvert_8u32f_C1R(const Ipp8u *p_src, int src_step, Ipp32f *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        float *p_dst_scan = (float *)((BYTE *)p_dst + y * dst_step);

        for (int x = 0; x < width; x++)
            p_dst_scan[x] = (float)p_src_scan[x];
    }

    return ippStsNoErr;
}

IppStatus ippiAdd_32f_C1R(const Ipp32f *p_src1, int src1_step, const Ipp32f *p_src2, int src2_step, Ipp32f *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src1 == NULL || p_src2 == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        const float *p_src1_scan = (const float *)((BYTE *)p_src1 + y * src1_step);
        const float *p_src2_scan = (const float *)((BYTE *)p_src2 + y * src2_step);
        float *p_dst_scan = (float *)((BYTE *)p_dst + y * dst_step);

        for (int x = 0; x < width; x++)
            p_dst_scan[x] = p_src1_scan[x] + p_src2_scan[x];
    }

    return ippStsNoErr;
}

IppStatus ippiSub_32f_C1R(const Ipp32f *p_src1, int src1_step, const Ipp32f *p_src2, int src2_step, Ipp32f *p_dst, int dst_step, IppiSize roi_size)
{
    // For IPP subtraction function, the result value is src2 - src1.

    if (p_src1 == NULL || p_src2 == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        const float *p_src1_scan = (const float *)((BYTE *)p_src1 + y * src1_step);
        const float *p_src2_scan = (const float *)((BYTE *)p_src2 + y * src2_step);
        float *p_dst_scan = (float *)((BYTE *)p_dst + y * dst_step);

        for (int x = 0; x < width; x++)
            p_dst_scan[x] = p_src2_scan[x] - p_src1_scan[x];
    }

    return ippStsNoErr;
}

IppStatus ippiThreshold_LTVal_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                                     IppiSize roi_size, Ipp8u threshold, Ipp8u value)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    if (src_step <= 0 || dst_step <= 0)
        return ippStsStepErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            if (p_src_scan[x] < threshold)
                p_dst_scan[x] = value;
            else
                p_dst_scan[x] = p_src_scan[x];
        }
    }

    return ippStsNoErr;
}

IppStatus ippiThreshold_GTVal_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                                     IppiSize roi_size, Ipp8u threshold, Ipp8u value)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    if (src_step <= 0 || dst_step <= 0)
        return ippStsStepErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            if (p_src_scan[x] > threshold)
                p_dst_scan[x] = value;
            else
                p_dst_scan[x] = p_src_scan[x];
        }
    }

    return ippStsNoErr;
}

IppStatus ippiThreshold_LTVal_8u_C1IR(Ipp8u *p_src_dst, int src_dst_step, IppiSize roi_size, Ipp8u threshold, Ipp8u value)
{
    if (p_src_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        BYTE *p_src_dst_scan = p_src_dst + y * src_dst_step;

        for (int x = 0; x < width; x++)
            if (p_src_dst_scan[x] < threshold)
                p_src_dst_scan[x] = value;
    }

    return ippStsNoErr;
}

IppStatus ippiThreshold_GTVal_8u_C1IR(Ipp8u *p_src_dst, int src_dst_step, IppiSize roi_size, Ipp8u threshold, Ipp8u value)
{
    if (p_src_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        BYTE *p_src_dst_scan = p_src_dst + y * src_dst_step;

        for (int x = 0; x < width; x++)
            if (p_src_dst_scan[x] > threshold)
                p_src_dst_scan[x] = value;
    }

    return ippStsNoErr;
}

IppStatus ippiCompare_8u_C1R(const Ipp8u *p_src1, int src1_step, const Ipp8u *p_src2, int src2_step,
                             Ipp8u *p_dst, int dst_step, IppiSize roi_size, IppCmpOp ipp_compare_op)
{
    if (p_src1 == NULL || p_src2 == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    if (src1_step <= 0 || src2_step <= 0 || dst_step <= 0)
        return ippStsStepErr;

    switch (ipp_compare_op)
    {
    case ippCmpLess:
        {
            for (int y = 0; y < height; y++)
            {
                const BYTE *p_src1_scan = p_src1 + y * src1_step;
                const BYTE *p_src2_scan = p_src2 + y * src2_step;
                BYTE *p_dst_scan = p_dst + y * dst_step;

                for (int x = 0; x < width; x++)
                {
                    if (p_src1_scan[x] < p_src2_scan[x])
                        p_dst_scan[x] = IPP_MAX_8U;
                    else
                        p_dst_scan[x] = 0;
                }
            }

            break;
        }
    case ippCmpLessEq:
        {
            for (int y = 0; y < height; y++)
            {
                const BYTE *p_src1_scan = p_src1 + y * src1_step;
                const BYTE *p_src2_scan = p_src2 + y * src2_step;
                BYTE *p_dst_scan = p_dst + y * dst_step;

                for (int x = 0; x < width; x++)
                {
                    if (p_src1_scan[x] <= p_src2_scan[x])
                        p_dst_scan[x] = IPP_MAX_8U;
                    else
                        p_dst_scan[x] = 0;
                }
            }

            break;
        }
    case ippCmpEq:
        {
            for (int y = 0; y < height; y++)
            {
                const BYTE *p_src1_scan = p_src1 + y * src1_step;
                const BYTE *p_src2_scan = p_src2 + y * src2_step;
                BYTE *p_dst_scan = p_dst + y * dst_step;

                for (int x = 0; x < width; x++)
                {
                    if (p_src1_scan[x] == p_src2_scan[x])
                        p_dst_scan[x] = IPP_MAX_8U;
                    else
                        p_dst_scan[x] = 0;
                }
            }

            break;
        }
    case ippCmpGreaterEq:
        {
            for (int y = 0; y < height; y++)
            {
                const BYTE *p_src1_scan = p_src1 + y * src1_step;
                const BYTE *p_src2_scan = p_src2 + y * src2_step;
                BYTE *p_dst_scan = p_dst + y * dst_step;

                for (int x = 0; x < width; x++)
                {
                    if (p_src1_scan[x] >= p_src2_scan[x])
                        p_dst_scan[x] = IPP_MAX_8U;
                    else
                        p_dst_scan[x] = 0;
                }
            }

            break;
        }
    case ippCmpGreater:
        {
            for (int y = 0; y < height; y++)
            {
                const BYTE *p_src1_scan = p_src1 + y * src1_step;
                const BYTE *p_src2_scan = p_src2 + y * src2_step;
                BYTE *p_dst_scan = p_dst + y * dst_step;

                for (int x = 0; x < width; x++)
                {
                    if (p_src1_scan[x] > p_src2_scan[x])
                        p_dst_scan[x] = IPP_MAX_8U;
                    else
                        p_dst_scan[x] = 0;
                }
            }

            break;
        }
    default:
        {
            // undefined, do nothing
            break;
        }
    }
    return ippStsNoErr;
}

IppStatus ippiCompareC_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u value,
                              Ipp8u* p_dst, int dst_step, IppiSize roi_size, IppCmpOp ipp_compare_op)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    if (src_step <= 0 || dst_step <= 0)
        return ippStsStepErr;

    switch (ipp_compare_op)
    {
    case ippCmpLess:
        {
            for (int y = 0; y < height; y++)
            {
                const BYTE *p_src_scan = p_src + y * src_step;
                BYTE *p_dst_scan = p_dst + y * dst_step;

                for (int x = 0; x < width; x++)
                {
                    if (p_src_scan[x] < value)
                        p_dst_scan[x] = IPP_MAX_8U;
                    else
                        p_dst_scan[x] = 0;
                }
            }

            break;
        }
    case ippCmpLessEq:
        {
            for (int y = 0; y < height; y++)
            {
                const BYTE *p_src_scan = p_src + y * src_step;
                BYTE *p_dst_scan = p_dst + y * dst_step;

                for (int x = 0; x < width; x++)
                {
                    if (p_src_scan[x] <= value)
                        p_dst_scan[x] = IPP_MAX_8U;
                    else
                        p_dst_scan[x] = 0;
                }
            }

            break;
        }
    case ippCmpEq:
        {
            for (int y = 0; y < height; y++)
            {
                const BYTE *p_src_scan = p_src + y * src_step;
                BYTE *p_dst_scan = p_dst + y * dst_step;

                for (int x = 0; x < width; x++)
                {
                    if (p_src_scan[x] == value)
                        p_dst_scan[x] = IPP_MAX_8U;
                    else
                        p_dst_scan[x] = 0;
                }
            }

            break;
        }
    case ippCmpGreaterEq:
        {
            for (int y = 0; y < height; y++)
            {
                const BYTE *p_src_scan = p_src + y * src_step;
                BYTE *p_dst_scan = p_dst + y * dst_step;

                for (int x = 0; x < width; x++)
                {
                    if (p_src_scan[x] >= value)
                        p_dst_scan[x] = IPP_MAX_8U;
                    else
                        p_dst_scan[x] = 0;
                }
            }

            break;
        }
    case ippCmpGreater:
        {
            for (int y = 0; y < height; y++)
            {
                const BYTE *p_src_scan = p_src + y * src_step;
                BYTE *p_dst_scan = p_dst + y * dst_step;

                for (int x = 0; x < width; x++)
                {
                    if (p_src_scan[x] > value)
                        p_dst_scan[x] = IPP_MAX_8U;
                    else
                        p_dst_scan[x] = 0;
                }
            }

            break;
        }
    default:
        {
            // undefined, do nothing
            break;
        }
    }
    return ippStsNoErr;
}

IppStatus ippiMin_8u_C1R(const Ipp8u *p_src, int src_step, IppiSize roi_size, Ipp8u *p_min)
{
    if (p_src == NULL || p_min == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    BYTE min_value = 255;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;

        for (int x = 0; x < width; x++)
            min_value = ch_Min(min_value, p_src_scan[x]);
    }

    (*p_min) = min_value;

    return ippStsNoErr;
}

IppStatus ippiMax_8u_C1R(const Ipp8u *p_src, int src_step, IppiSize roi_size, Ipp8u *p_max)
{
    if (p_src == NULL || p_max == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    BYTE max_value = 0;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;

        for (int x = 0; x < width; x++)
            max_value = ch_Max(max_value, p_src_scan[x]);
    }

    (*p_max) = max_value;

    return ippStsNoErr;
}

IppStatus ippiAbsDiff_8u_C1R(const Ipp8u *p_src1, int src1_step, const Ipp8u *p_src2, int src2_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size)
{
    if (p_src1 == NULL || p_src2 == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src1_scan = p_src1 + y * src1_step;
        const BYTE *p_src2_scan = p_src2 + y * src2_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            int abs_difference = ch_Abs((int)p_src1_scan[x] - (int)p_src2_scan[x]);
            p_dst_scan[x] = (BYTE)abs_difference;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiSum_8u_C1R(const Ipp8u *p_src, int src_step, IppiSize roi_size, Ipp64f *p_sum)
{
    if (p_src == NULL || p_sum == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    __int64 sum = 0;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;

        for (int x = 0; x < width; x++)
            sum += (__int64)p_src_scan[x];
    }

    (*p_sum) = (double)sum;

    return ippStsNoErr;
}

IppStatus ippiCountInRange_8u_C1R(const Ipp8u *p_src, int src_step, IppiSize roi_size, int *p_counts, Ipp8u lowerBound, Ipp8u upperBound)
{
    if (p_src == NULL || p_counts == NULL)
        return ippStsNullPtrErr;

    if (lowerBound > upperBound)
        return ippStsRangeErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    int count = 0;

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;

        for (int x = 0; x < width; x++)
        {
            BYTE value = p_src_scan[x];
            if (value >= lowerBound && value <= upperBound)
                count++;
        }
    }

    (*p_counts) = count;

    return ippStsNoErr;
}

IppStatus ippiAbs_16s_C1IR(Ipp16s *p_src_dst, int src_dst_step, IppiSize roi_size)
{
    if (p_src_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    for (int y = 0; y < height; y++)
    {
        short *p_src_dst_scan = (short *)((BYTE *)p_src_dst + y * src_dst_step);

        for (int x = 0; x < width; x++)
        {
            short value = p_src_dst_scan[x];
            if (value == -32768)
                p_src_dst_scan[x] = 32767;
            else if (value < 0)
                p_src_dst_scan[x] = -value;
            else
                p_src_dst_scan[x] = value;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiFilterBox_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize dst_roi_size, IppiSize mask_size, IppiPoint anchor)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = dst_roi_size.width;
    const int height = dst_roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    const int mask_width = mask_size.width;
    const int mask_height = mask_size.height;
    if (mask_width <= 0 || mask_height <= 0)
        return ippStsMaskSizeErr;

    if (anchor.x < 0 || anchor.y < 0 || anchor.x >= mask_width || anchor.y >= mask_height)
        return ippStsAnchorErr;

    // Precompute the offsets from a source pixel to all pixels in the mask size.
    const int mask_area = mask_width * mask_height;
    int *p_offsets = new int[mask_area];

    for (int y = 0; y < mask_height; y++)
    {
        const int dy = y - anchor.y;
        
        for (int x = 0; x < mask_width; x++)
        {
            const int dx = x - anchor.x;

            p_offsets[y * mask_width + x] = dy * src_step + dx;
        }
    }

    const int half_mask_area = mask_area / 2; // used for rounding

    int sum_value = 0;

    if (mask_width >= 3)
    {
        // Compute the sum value of the first pixel of each row,
        // then compute only the difference between the consecutive pixels.
        // (The leftmost and rightmost column.)

        int *p_left_offsets = new int[mask_height];
        int *p_right_offsets = new int[mask_height];
        const int dx_left = (-1) - anchor.x;
        const int dx_right = (mask_width - 1) - anchor.x;
        for (int i = 0; i < mask_height; i++)
        {
            const int dy = i - anchor.y;

            p_left_offsets[i] = dy * src_step + dx_left;
            p_right_offsets[i] = dy * src_step + dx_right;
        }

        for (int y = 0; y < height; y++)
        {
            const BYTE *p_src_scan = p_src + y * src_step;
            BYTE *p_dst_scan = p_dst + y * dst_step;

            // first pixel
            const BYTE *p_first_pixel = p_src_scan;
            sum_value = 0;

            for (int i = 0; i < mask_area; i++)
                sum_value += (int)p_first_pixel[p_offsets[i]];

            int mean_value = (sum_value + half_mask_area) / mask_area;

            p_dst_scan[0] = (BYTE)FitInRange(mean_value, 0, 255);

            for (int x = 1; x < width; x++)
            {
                const BYTE *p_src_pixel = p_src_scan + x;

                // subtract left column
                for (int i = 0; i < mask_height; i++)
                    sum_value -= (int)p_src_pixel[p_left_offsets[i]];

                // add right column
                for (int i = 0; i < mask_height; i++)
                    sum_value += (int)p_src_pixel[p_right_offsets[i]];

                int mean_value = (sum_value + half_mask_area) / mask_area;

                p_dst_scan[x] = (BYTE)FitInRange(mean_value, 0, 255);
            }
        }

        delete [] p_left_offsets;
        delete [] p_right_offsets;
    }
    else
    {
        // If mask_width < 3, it is slower to use the above algorithm.
        // Just compute the sum pixel values actually.

        for (int y = 0; y < height; y++)
        {
            const BYTE *p_src_scan = p_src + y * src_step;
            BYTE *p_dst_scan = p_dst + y * dst_step;

            for (int x = 0; x < width; x++)
            {
                const BYTE *p_src_pixel = p_src_scan + x;
                sum_value = 0;

                for (int i = 0; i < mask_area; i++)
                    sum_value += (int)p_src_pixel[p_offsets[i]];

                int mean_value = (sum_value + half_mask_area) / mask_area;

                p_dst_scan[x] = (BYTE)FitInRange(mean_value, 0, 255);
            }
        }
    }

    delete [] p_offsets;

    return ippStsNoErr;
}

const int gaussian_3x3_kernel[9] =
{
    1, 2, 1,
    2, 4, 2,
    1, 2, 1
};
const int gaussian_3x3_kernel_sum = 16;

const int gaussian_5x5_kernel[25] =
{
     2,  7,  12,  7, 2,
     7, 31,  52, 31, 7,
    12, 52, 127, 52, 12,
     7, 31,  52, 31, 7,
     2,  7,  12,  7, 2
};
const int gaussian_5x5_kernel_sum = 571;

IppStatus ippiFilterGauss_8u_C1R(const Ipp8u* p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size, IppiMaskSize mask)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    // Gaussian kernel only supports 3x3 or 5x5 size.
    if (mask != ippMskSize3x3 && mask != ippMskSize5x5)
        return ippStsMaskSizeErr;

    int mask_width = 3;
    int mask_height = 3;
    IppiPoint anchor = {1, 1};
    const int *p_mask_data = gaussian_3x3_kernel;
    int mask_sum = gaussian_3x3_kernel_sum;
    if (mask == ippMskSize5x5)
    {
        mask_width = 5;
        mask_height = 5;
        anchor.x = 2;
        anchor.y = 2;
        p_mask_data = gaussian_5x5_kernel;
        mask_sum = gaussian_5x5_kernel_sum;
    }

    // Precompute the offsets from a source pixel to all pixels in the mask size.
    const int mask_area = mask_width * mask_height;
    int *p_offsets = new int[mask_area];

    for (int y = 0; y < mask_height; y++)
    {
        const int dy = y - anchor.y;
        
        for (int x = 0; x < mask_width; x++)
        {
            const int dx = x - anchor.x;

            p_offsets[y * mask_width + x] = dy * src_step + dx;
        }
    }

    const int half_mask_sum = mask_sum / 2; // used for rounding

    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_scan = p_src + y * src_step;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        for (int x = 0; x < width; x++)
        {
            const BYTE *p_src_pixel = p_src_scan + x;
            int sum_value = 0;

            for (int i = 0; i < mask_area; i++)
                sum_value += (int)p_src_pixel[p_offsets[i]] * p_mask_data[i];

            int mean_value = (sum_value + half_mask_sum) / mask_sum;

            p_dst_scan[x] = (BYTE)FitInRange(mean_value, 0, 255);
        }
    }

    delete [] p_offsets;

    return ippStsNoErr;
}

IppStatus ippiConvValid_32f_C1R(const Ipp32f *p_src1, int src1_step, IppiSize src1_size,
                                const Ipp32f *p_src2, int src2_step, IppiSize src2_size, Ipp32f *p_dst, int dst_step)
{
    if (p_src1 == NULL || p_src2 == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int src1_width = src1_size.width;
    const int src1_height = src1_size.height;
    if (src1_width <= 0 || src1_height <= 0)
        return ippStsSizeErr;

    const int src2_width = src2_size.width;
    const int src2_height = src2_size.height;
    if (src2_width <= 0 || src2_height <= 0)
        return ippStsSizeErr;

    // The size of one source image must be >= the other.
    int width1 = 0;
    int height1 = 0;
    int stride1 = 0;
    int width2 = 0;
    int height2 = 0;
    int stride2 = 0;
    const float *p_data1 = NULL;
    const float *p_data2 = NULL;
    if (src1_width >= src2_width && src1_height >= src2_height)
    {
        width1 = src1_width;
        height1 = src1_height;
        stride1 = src1_step;
        width2 = src2_width;
        height2 = src2_height;
        stride2 = src2_step;
        p_data1 = p_src1;
        p_data2 = p_src2;
    }
    else if (src2_width >= src1_width && src2_height >= src1_height)
    {
        width1 = src2_width;
        height1 = src2_height;
        stride1 = src2_step;
        width2 = src1_width;
        height2 = src1_height;
        stride2 = src1_step;
        p_data1 = p_src2;
        p_data2 = p_src1;
    }
    else
    {
        return ippStsSizeErr;
    }

    // The destination size is (width1 - width2 + 1) x (height1 - height2 + 1).
    const int dst_width = width1 - width2 + 1;
    const int dst_height = height1 - height2 + 1;

    for (int y = 0; y < dst_height; y++)
    {
        const float *p_data1_scan = (const float *)((BYTE *)p_data1 + y * stride1);
        float *p_dst_scan = (float *)((BYTE *)p_dst + y * dst_step);
          
        for (int x = 0; x < dst_width; x++)
        {
            const float *p_data1_pixel = p_data1_scan + x;
            float sum_value = 0.0f;

            for (int b = 0; b < height2; b++)
            {
                const float *p_conv1_scan = (const float *)((BYTE *)p_data1_pixel + b * stride1);
                const float *p_conv2_scan = (const float *)((BYTE *)p_data2 + (height2 - 1 - b) * stride2);

                for (int a = 0; a < width2; a++)
                    sum_value += p_conv1_scan[a] * p_conv2_scan[width2 - 1 - a];
            }

            p_dst_scan[x] = sum_value;
        }
    }

    return ippStsNoErr;
}

inline BYTE GetMedianValue(const int histogram[256], int center_amount)
{
    _MYASSERT(histogram);

    int median = 0;
    int sum = 0;
    for (int i = 0; i < 256; i++)
    {
        median = i;
        sum += histogram[i];
        if (sum >= center_amount)
            break;
    }

    return (BYTE)median;
}

IppStatus ippiFilterMedian_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize dst_roi_size, IppiSize mask_size, IppiPoint anchor)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = dst_roi_size.width;
    const int height = dst_roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    if (src_step <= 0 || dst_step <= 0)
        return ippStsStepErr;

    const int mask_width = mask_size.width;
    const int mask_height = mask_size.height;
    if (mask_width <= 0 || mask_height <= 0)
        return ippStsMaskSizeErr;

    // mask size cannot be even for IPP median filter.
    if (mask_width % 2 == 0 || mask_height % 2 == 0)
        return ippStsMaskSizeErr;

    if (anchor.x < 0 || anchor.y < 0 || anchor.x >= mask_width || anchor.y >= mask_height)
        return ippStsAnchorErr;

    const int center_amount = (mask_width * mask_height + 1) / 2; // for finding median in histogram

    // Use histogram-based algorithm for median filter.
    // A histogram is maintained for the values under the moving window.
    int histogram[256];

    for (int y = 0; y < height; y++)
    {
        // Initialize the histogram at the leftmost position of every row.
        // When the window moves rightward, update the histogram by the columns entering/leaving the window.

        const BYTE *p_window_start = p_src + (y - anchor.y) * src_step - anchor.x;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        ZeroMemory(histogram, sizeof(int) * 256);
        for (int n = 0; n < mask_height; n++)
        {
            const BYTE *p_window_scan = p_window_start + n * src_step;
            for (int m = 0; m < mask_width; m++)
                histogram[p_window_scan[m]]++;
        }

        p_dst_scan[0] = GetMedianValue(histogram, center_amount);

        const BYTE *p_window_left_column = p_window_start;
        const BYTE *p_window_right_column = p_window_start + mask_width;
        for (int x = 1; x < width; x++)
        {
            for (int n = 0; n < mask_height; n++)
            {
                int offset = n * src_step;
                histogram[p_window_left_column[offset]]--;
                histogram[p_window_right_column[offset]]++;
            }

            p_dst_scan[x] = GetMedianValue(histogram, center_amount);

            p_window_left_column++;
            p_window_right_column++;
        }
    }

    return ippStsNoErr;
}

IppStatus ippiFilterMedian_8u_AC4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize dst_roi_size, IppiSize mask_size, IppiPoint anchor)
{
    if (p_src == NULL || p_dst == NULL)
        return ippStsNullPtrErr;

    const int width = dst_roi_size.width;
    const int height = dst_roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    if (src_step <= 0 || dst_step <= 0)
        return ippStsStepErr;

    const int mask_width = mask_size.width;
    const int mask_height = mask_size.height;
    if (mask_width <= 0 || mask_height <= 0)
        return ippStsMaskSizeErr;

    // mask size cannot be even for IPP median filter.
    if (mask_width % 2 == 0 || mask_height % 2 == 0)
        return ippStsMaskSizeErr;

    if (anchor.x < 0 || anchor.y < 0 || anchor.x >= mask_width || anchor.y >= mask_height)
        return ippStsAnchorErr;

    const int center_amount = (mask_width * mask_height + 1) / 2; // for finding median in histogram

    // Use histogram-based algorithm for median filter.
    // A histogram is maintained for the values under the moving window.
    int histogram[3][256];

    for (int y = 0; y < height; y++)
    {
        // Initialize the histogram at the leftmost position of every row.
        // When the window moves rightward, update the histogram by the columns entering/leaving the window.

        const BYTE *p_window_start = p_src + (y - anchor.y) * src_step  - anchor.x * 4;
        BYTE *p_dst_scan = p_dst + y * dst_step;

        ZeroMemory(histogram[0], sizeof(int) * 256);
        ZeroMemory(histogram[1], sizeof(int) * 256);
        ZeroMemory(histogram[2], sizeof(int) * 256);

        const BYTE *p_window_scan = p_window_start;
        for (int n = 0; n < mask_height; n++)
        {
            for (int m = 0; m < mask_width; m++)
            {
                histogram[0][p_window_scan[4 * m    ]]++;
                histogram[1][p_window_scan[4 * m + 1]]++;
                histogram[2][p_window_scan[4 * m + 2]]++;
            }

            p_window_scan += src_step;
        }

        p_dst_scan[0] = GetMedianValue(histogram[0], center_amount);
        p_dst_scan[1] = GetMedianValue(histogram[1], center_amount);
        p_dst_scan[2] = GetMedianValue(histogram[2], center_amount);

        for (int x = 1; x < width; x++)
        {
            const BYTE *p_window_left_column = p_window_start + 4 * (x - 1);
            const BYTE *p_window_right_column = p_window_start + 4 * (mask_width + x - 1);

            for (int n = 0; n < mask_height; n++)
            {
                histogram[0][p_window_left_column[0]]--;
                histogram[1][p_window_left_column[1]]--;
                histogram[2][p_window_left_column[2]]--;
                p_window_left_column += src_step;

                histogram[0][p_window_right_column[0]]++;
                histogram[1][p_window_right_column[1]]++;
                histogram[2][p_window_right_column[2]]++;
                p_window_right_column += src_step;
            }

            p_dst_scan[x * 4    ] = GetMedianValue(histogram[0], center_amount);
            p_dst_scan[x * 4 + 1] = GetMedianValue(histogram[1], center_amount);
            p_dst_scan[x * 4 + 2] = GetMedianValue(histogram[2], center_amount);
        }
    }

    return ippStsNoErr;
}

IppStatus ippiFloodFillGetSize(IppiSize roi_size, int *p_buffer_size)
{
    if (p_buffer_size == NULL)
        return ippStsNullPtrErr;

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    // For current algorithm we do not require any additional buffers.
    // Return 1 as a dummy positive size.

    (*p_buffer_size) = 1;

    return ippStsNoErr;
}

IppStatus ippiFloodFill_4Con_8u_C1IR(Ipp8u *p_image, int image_step, IppiSize roi_size, IppiPoint seed,
                                     Ipp8u new_value, IppiConnectedComp *p_region, Ipp8u *p_buffer)
{
    if (p_image == NULL || p_region == NULL || p_buffer == NULL)
        return ippStsNullPtrErr;

    // p_buffer is not used in current design.
    // To sync the Intel IPP function behavior, we still return ippStsNullPtrErr if it is not given.

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    if (seed.x < 0 || seed.y < 0 || seed.x >= width || seed.y >= height)
        return ippStsOutOfRangeErr;

    FloodFillTool flood_fill_tool;
    flood_fill_tool.Initialize(width, height);

    ConnectedComponent region;
    flood_fill_tool.FloodFill_4Connect(p_image, image_step, hyPoint(seed.x, seed.y), new_value, region);

    flood_fill_tool.UnInitialize();

    p_region->area = (double)region.area;
    p_region->rect.x = region.rect.x;
    p_region->rect.y = region.rect.y;
    p_region->rect.width = region.rect.width;
    p_region->rect.height = region.rect.height;
    p_region->value[0] = (double)region.value;
    p_region->value[1] = 0.0;
    p_region->value[2] = 0.0;

    return ippStsNoErr;
}

IppStatus ippiFloodFill_8Con_8u_C1IR(Ipp8u *p_image, int image_step, IppiSize roi_size, IppiPoint seed,
                                     Ipp8u new_value, IppiConnectedComp *p_region, Ipp8u *p_buffer)
{
    if (p_image == NULL || p_region == NULL || p_buffer == NULL)
        return ippStsNullPtrErr;

    // p_buffer is not used in current design.
    // To sync the Intel IPP function behavior, we still return ippStsNullPtrErr if it is not given.

    const int width = roi_size.width;
    const int height = roi_size.height;
    if (width <= 0 || height <= 0)
        return ippStsSizeErr;

    if (seed.x < 0 || seed.y < 0 || seed.x >= width || seed.y >= height)
        return ippStsOutOfRangeErr;

    FloodFillTool flood_fill_tool;
    flood_fill_tool.Initialize(width, height);

    ConnectedComponent region;
    flood_fill_tool.FloodFill_8Connect(p_image, image_step, hyPoint(seed.x, seed.y), new_value, region);

    flood_fill_tool.UnInitialize();

    p_region->area = (double)region.area;
    p_region->rect.x = region.rect.x;
    p_region->rect.y = region.rect.y;
    p_region->rect.width = region.rect.width;
    p_region->rect.height = region.rect.height;
    p_region->value[0] = (double)region.value;
    p_region->value[1] = 0.0;
    p_region->value[2] = 0.0;

    return ippStsNoErr;
}

