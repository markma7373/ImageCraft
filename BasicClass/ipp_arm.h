#pragma once

#include "Common.h"
#include "use_hylib.h"

#if defined(ANDROID_ARM) || defined(IOS_ARM)
#ifndef __EMSCRIPTEN__
#include <arm_neon.h>
#endif
#endif

// Implementation of IPP functions that can be used in Android ARM platform.
// They are C codes w/o NEON optimization.

typedef unsigned char   Ipp8u;
typedef unsigned short  Ipp16u;
typedef unsigned int    Ipp32u;

typedef signed char    Ipp8s;
typedef signed short   Ipp16s;
typedef signed int     Ipp32s;
typedef float   Ipp32f;
typedef __int64 Ipp64s;
typedef unsigned long long Ipp64u;
typedef double  Ipp64f;

#define IPP_MAX_8U     ( 0xFF )

typedef struct {
    int x;
    int y;
    int width;
    int height;
} IppiRect;

typedef struct {
    int x;
    int y;
} IppiPoint;

typedef struct {
    int width;
    int height;
} IppiSize;

enum {
    IPPI_INTER_NN     = 1,
    IPPI_INTER_LINEAR = 2,
    IPPI_INTER_SUPER  = 8,
};

typedef enum {
    ippAxsHorizontal,
    ippAxsVertical,
    ippAxsBoth
} IppiAxis;

typedef struct
{
    int roi_width;
    BYTE *p_mask;
    IppiSize mask_size;
    IppiPoint anchor;
    BYTE *p_buffer;
    int buffer_size; // in BYTEs
} IppiMorphState;

typedef enum _IppiBorderType {
    ippBorderRepl      =  1,
} IppiBorderType;

typedef enum _IppiMaskSize {
    ippMskSize1x3 = 13,
    ippMskSize1x5 = 15,
    ippMskSize3x1 = 31,
    ippMskSize3x3 = 33,
    ippMskSize5x1 = 51,
    ippMskSize5x5 = 55
} IppiMaskSize;

typedef enum {
    ippCmpLess,
    ippCmpLessEq,
    ippCmpEq,
    ippCmpGreaterEq,
    ippCmpGreater
} IppCmpOp;

typedef struct _IppiConnectedComp {
    Ipp64f   area;    /*  area of the segmented component  */
    Ipp64f   value[3];/*  gray scale value of the segmented component  */
    IppiRect rect;    /*  bounding rectangle of the segmented component  */
} IppiConnectedComp;

// The connected component used for FloodFillTool. It will be converted into IppiConnectedComp.
struct ConnectedComponent
{
    int area;
    BYTE value;
    HyRect rect;

    ConnectedComponent()
    {
        area = 0;
        value = 0;
        rect = hyRect(0, 0, 0, 0);
    }
};

template<typename T>
class CLQueue
{
public:
    CLQueue()
    {
        mp_buffer = NULL;
        m_size = 0;
        m_buffer_size = 0;
        m_data_offset = 0;
    }

    // copy constructor
    CLQueue(const CLQueue &queue)
    {
        if (queue.mp_buffer)
        {
            m_size = queue.m_size;
            m_buffer_size = queue.m_buffer_size;

            // Allocate [m_buffer_size] elements, but put the data at the front of the array.
            mp_buffer = NULL;
            _ALIGNED_MALLOC_PTR(mp_buffer, T, m_buffer_size);
            memcpy(mp_buffer, queue.mp_data, sizeof(T) * m_size);
            m_data_offset = 0;
        }
        else
        {
            mp_buffer = NULL;
            m_size = 0;
            m_buffer_size = 0;
            m_data_offset = 0;
        }
    }

    // copy assignment
    CLQueue &operator=(const CLQueue &queue)
    {
        if (this != &queue) // not self-assignment
        {
            if (queue.mp_buffer)
            {
                m_size = queue.m_size;
                m_buffer_size = queue.m_buffer_size;

                // Allocate [m_buffer_size] elements, but put the data at the front of the array.
                _ALIGNED_MALLOC_PTR(mp_buffer, T, m_buffer_size);
                memcpy(mp_buffer, queue.mp_data, sizeof(T) * m_size);
                m_data_offset = 0;
            }
            else
            {
                mp_buffer = NULL;
                m_size = 0;
                m_buffer_size = 0;
                m_data_offset = 0;
            }
        }

        return *this;
    }

    ~CLQueue()
    {
        Clear();
    }

    void Clear()
    {
        _ALIGNED_FREE_PTR(mp_buffer);
        m_size = 0;
        m_buffer_size = 0;
        m_data_offset = 0;
    }

    inline bool IsEmpty()
    {
        return (m_size == 0);
    }

    inline int GetSize()
    {
        return m_size;
    }

    inline int GetBufferSize()
    {
        return m_buffer_size;
    }

    inline void Push(const T &item)
    {
        if (m_data_offset + m_size == m_buffer_size)
        {
            // Re-allocate the queue buffer.
            // If the buffer size was 0, allocate [base_allocate_size] elements.
            // If the buffer size was not 0:
            //     If the ratio of free space (m_data_offset / m_buffer_size) is small, double the buffer size.
            //     Else, use the same buffer size and m_data_offset becomes the free space after the data.

            // In theory, the ratio threshold can be 0.0.
            // But if it is too small, we may re-allocate the buffer too frequently.
            const float max_free_space_ratio = 0.5f;

            if (m_buffer_size == 0)
            {
                m_buffer_size = base_allocate_size;
            }
            else
            {
                int free_space_threshold = ch_Round(max_free_space_ratio * m_buffer_size);
                free_space_threshold = ch_Max(free_space_threshold, 1); // one space for the new item

                if (m_data_offset < free_space_threshold)
                    m_buffer_size *= 2;
            }

            _MYASSERT(m_size < m_buffer_size);

            // When the buffer is re-allocated, put the data at the front of the array.
            T *p_new_buffer = NULL;
            _ALIGNED_MALLOC_PTR(p_new_buffer, T, m_buffer_size);
            if (m_size > 0 && mp_buffer != NULL)
                memcpy(p_new_buffer, mp_buffer + m_data_offset, sizeof(T) * m_size);

            m_data_offset = 0;

            _ALIGNED_FREE_PTR(mp_buffer);
            mp_buffer = p_new_buffer;
        }

        _MYASSERT(mp_buffer);
        mp_buffer[m_data_offset + m_size] = item;
        m_size++;
    }

    inline void Pop()
    {
        if (m_size > 0)
        {
            m_data_offset++;
            m_size--;
        }

        // Do not release the buffer array.
        // We keep the buffer even if m_size becomes 0.
    }

    // WARNING: Top() should never be called if the stack may be empty.
    inline T &Front()
    {
        _MYASSERT(mp_buffer);
        return mp_buffer[m_data_offset];
    }

    inline const T &Front() const
    {
        _MYASSERT(mp_buffer);
        return mp_buffer[m_data_offset];
    }

private:
    static const int base_allocate_size = 16; // in elements

    T *mp_buffer;
    int m_size;         // existing elements
    int m_buffer_size;  // buffer size of mp_buffer
    int m_data_offset;  // offset of current data
};

// A 4-BYTE Point structure used for flood fill search.
struct ShortPoint
{
    USHORT x;
    USHORT y;

    ShortPoint()
    {
        x = 0;
        y = 0;
    }

    ShortPoint(USHORT _x, USHORT _y)
    {
        x = _x;
        y = _y;
    }

    ShortPoint(int _x, int _y)
    {
        x = (USHORT)_x;
        y = (USHORT)_y;
    }
};

enum FloodFillConnection
{
    Connection_4Neighbor = 0,
    Connection_8Neighbor,
};

// A class to perform flood fill without using Intel IPP functions.
// It requires smaller computational buffer than Intel IPP (6x ROI size).
// Current support:
// - Element type: BYTE
// - Channel count: 1 channel
// - Connection: 4-Connect, 8-Connect
// - Gradient search (like ippiFloodFill_Grad): NOT supported
// - Max ROI size: 65536 x 65536 (We use unsigned short to record the coordinates)
class FloodFillTool
{
public:
    FloodFillTool()
    {
        m_width = 0;
        m_height = 0;

        m_is_initialized = false;
    }

    ~FloodFillTool()
    {
        UnInitialize();
    }

    bool Initialize(int width, int height);
    void UnInitialize();

    bool FloodFill_4Connect(BYTE *p_image_data, int image_step, const HyPoint &seed,
                            BYTE new_value, ConnectedComponent &region);
    bool FloodFill_8Connect(BYTE *p_image_data, int image_step, const HyPoint &seed,
                            BYTE new_value, ConnectedComponent &region);

private:
    int m_width;
    int m_height;

    bool m_is_initialized;

    CLQueue<ShortPoint> m_top_search_queue;
    CLQueue<ShortPoint> m_bottom_search_queue;

    inline int GetSeedPoints(const BYTE *p_data_list, BYTE target_value, int *p_seed_list, int length);

    bool FloodFill_Kernel(BYTE *p_image_data, int image_step, const HyPoint &seed,
                          BYTE new_value, ConnectedComponent &region, FloodFillConnection connection);
    void FloodFill_Kernel_Search(CLQueue<ShortPoint> &active_queue,
                                 BYTE *p_image_data, int image_step, BYTE seed_value, BYTE new_value, 
                                 ConnectedComponent &region, FloodFillConnection connection);
};

// We only support partial IPP status enumerations.

typedef enum {
    /* errors */
    ippStsNotSupportedModeErr    = -9999,/* The requested mode is currently not supported.  */
    ippStsBorderErr             = -225,  /* Illegal value for border type.*/
    ippStsNotEvenStepErr        = -108,  /* Step value is not pixel multiple. */
    ippStsChannelOrderErr       = -60,   /* Incorrect order of the destination channels. */
    ippStsZeroMaskValuesErr     = -59,   /* All values of the mask are equal to zero. */
    ippStsRectErr               = -57,   /* Size of the rectangle region is less than, or equal to 1. */
    ippStsNumChannelsErr        = -53,   /* Number of channels is incorrect, or not supported. */
    ippStsAnchorErr             = -34,   /* Anchor point is outside the mask. */
    ippStsMaskSizeErr           = -33,   /* Invalid mask size. */
    ippStsResizeFactorErr       = -23,   /* Resize factor(s) is less than, or equal to zero. */
    ippStsInterpolationErr      = -22,   /* Invalid interpolation mode. */
    ippStsMirrorFlipErr         = -21,   /* Invalid flip mode. */
    ippStsStepErr               = -14,   /* Step value is not valid. */
    ippStsOutOfRangeErr         = -11,   /* Argument is out of range, or point is outside the image. */
    ippStsNullPtrErr            =  -8,   /* Null pointer error. */
    ippStsRangeErr              =  -7,   /* Incorrect values for bounds: the lower bound is greater than the upper bound. */
    ippStsSizeErr               =  -6,   /* Incorrect value for data size. */

    /* no errors */
    ippStsNoErr                 =   0,   /* No errors. */

    /* warnings  */
    ippStsNoOperation           =   1,   /* No operation has been executed */
} IppStatus;

#define ippStsOk ippStsNoErr;

///////////////////////////////////////////////////////////////////////////////////
// Declaration of IPP functions

// IPP Initialize function
IppStatus ippInit();

// Memory Allocate/Free
Ipp8u *ippsMalloc_8u(int size);
void ippsFree(void *pointer);

// Copy
IppStatus ippiCopy_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiCopy_8u_C3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiCopy_8u_C4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiCopy_8u_AC4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiCopy_8u_C3AC4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiCopy_8u_C3P3R(const Ipp8u *p_src, int src_step, Ipp8u *const p_dst[3], int dst_step, IppiSize roi_size);
IppStatus ippiCopy_8u_P3C3R(const Ipp8u *const p_src[3], int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiCopy_16s_C1R(const Ipp16s *p_src, int src_step, Ipp16s *p_dst, int dst_step, IppiSize roi_size);

// Masked Copy
IppStatus ippiCopy_8u_C1MR(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size, const Ipp8u *p_mask, int mask_step);
IppStatus ippiCopy_8u_C3MR(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size, const Ipp8u *p_mask, int mask_step);
IppStatus ippiCopy_8u_C4MR(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size, const Ipp8u *p_mask, int mask_step);
IppStatus ippiCopy_8u_AC4MR(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size, const Ipp8u *p_mask, int mask_step);

// Color to Gray
IppStatus ippiColorToGray_8u_C3C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size, const Ipp32f coeffs[3]);
IppStatus ippiColorToGray_8u_AC4C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size, const Ipp32f coeffs[3]);

// Gray to Color (Channel Duplication)
IppStatus ippiDup_8u_C1C3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiDup_8u_C1C4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);

// 3/4 Channels Conversion & Swap
IppStatus ippiSwapChannels_8u_C3C4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                                    IppiSize roi_size, const int dst_order[4], Ipp8u value);
IppStatus ippiCopy_8u_AC4C3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiSwapChannels_8u_C3IR(Ipp8u* p_src_dst, int src_dst_step, IppiSize roi_size, const int dst_order[3]);

// Resize - Get buffer size
IppStatus ippiResizeGetBufSize(IppiRect src_roi, IppiRect dst_roi, int channels, int interpolation, int *p_size);

// Resize
// WARNING: The factor parameters must be correctly corresponded to the src/dst ROI size in ippiResizeGetBufSize().
//          Otherwise, the buffer size may not be enough for computation.
// Specifically, if src_roi transformed by (x_factor, y_factor, x_shift, y_shift)
// is larger than dst_roi, out-of-bound accession will occur for the computational buffers.
IppStatus ippiResizeSqrPixel_8u_C1R(const Ipp8u *p_src, IppiSize src_size, int src_step, IppiRect src_roi,
                                    Ipp8u* p_dst, int dst_step, IppiRect dst_roi,
                                    double x_factor, double y_factor, double x_shift, double y_shift, int interpolation, Ipp8u *p_buffer);

IppStatus ippiResizeSqrPixel_8u_C3R(const Ipp8u *p_src, IppiSize src_size, int src_step, IppiRect src_roi,
                                    Ipp8u* p_dst, int dst_step, IppiRect dst_roi,
                                    double x_factor, double y_factor, double x_shift, double y_shift, int interpolation, Ipp8u *p_buffer);

IppStatus ippiResizeSqrPixel_8u_C4R(const Ipp8u *p_src, IppiSize src_size, int src_step, IppiRect src_roi,
                                    Ipp8u* p_dst, int dst_step, IppiRect dst_roi,
                                    double x_factor, double y_factor, double x_shift, double y_shift, int interpolation, Ipp8u *p_buffer);

// Set value
IppStatus ippiSet_8u_C1R(Ipp8u value, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiSet_8u_C3R(const Ipp8u value[3], Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiSet_8u_AC4R(const Ipp8u value[3], Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiSet_8u_C1MR(Ipp8u value, Ipp8u *p_dst, int dst_step, IppiSize roi_size, const Ipp8u *p_mask, int mask_step);
IppStatus ippiSet_8u_C3MR(const Ipp8u value[3], Ipp8u *p_dst, int dst_step, IppiSize roi_size, const Ipp8u *p_mask, int mask_step);
IppStatus ippiSet_8u_AC4MR(const Ipp8u value[3], Ipp8u *p_dst, int dst_step, IppiSize roi_size, const Ipp8u *p_mask, int mask_step);
IppStatus ippiSet_32s_C1R(Ipp32s value, Ipp32s *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiSet_32f_C1R(Ipp32f value, Ipp32f *p_dst, int dst_step, IppiSize roi_size);

// Bitwise operation
IppStatus ippiAnd_8u_C1IR(const Ipp8u *p_src, int src_step, Ipp8u *p_srcdst, int srcdst_step, IppiSize roi_size);
IppStatus ippiOr_8u_C1IR(const Ipp8u *p_src, int src_step, Ipp8u *p_srcdst, int srcdst_step, IppiSize roi_size);
IppStatus ippiXor_8u_C1R(const Ipp8u *p_src1, int src1_step, const Ipp8u *p_src2, int src2_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiXor_8u_C1IR(const Ipp8u *p_src, int src_step, Ipp8u *p_srcdst, int srcdst_step, IppiSize roi_size);
IppStatus ippiNot_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiNot_8u_C1IR(Ipp8u *p_src_dst, int src_dst_step, IppiSize roi_size);

// Color Convert
IppStatus ippiRGBToYCbCr_8u_C3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiRGBToYCbCr_8u_AC4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiBGRToYCbCr422_8u_C3C2R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiBGRToYCbCr422_8u_AC4C2R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiBGRToYCbCr422_8u_C3P3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst[3], int dst_step[3], IppiSize roi_size);
IppStatus ippiBGRToYCbCr422_8u_AC4P3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst[3], int dst_step[3], IppiSize roi_size);
IppStatus ippiYCbCr422ToBGR_8u_C2C3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiYCbCr422ToBGR_8u_C2C4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size, Ipp8u alpha_value);

IppStatus ippiRGBToHSV_8u_C3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiRGBToHSV_8u_AC4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiHSVToRGB_8u_C3R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiHSVToRGB_8u_AC4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);

// Image Rotation
IppStatus ippiGetRotateShift(double x_center, double y_center, double angle, double *p_x_shift, double *p_y_shift);
IppStatus ippiGetRotateBound(IppiRect src_roi, double bound[2][2], double angle, double x_shift, double y_shift);
IppStatus ippiRotate_8u_C1R(const Ipp8u *p_src, IppiSize src_size, int src_step, IppiRect src_roi,
                            Ipp8u* p_dst, int dst_step, IppiRect dst_roi, double angle, double x_shift, double y_shift, int interpolation);
IppStatus ippiRotate_8u_C3R(const Ipp8u *p_src, IppiSize src_size, int src_step, IppiRect src_roi,
                            Ipp8u* p_dst, int dst_step, IppiRect dst_roi, double angle, double x_shift, double y_shift, int interpolation);
IppStatus ippiRotate_8u_C4R(const Ipp8u *p_src, IppiSize src_size, int src_step, IppiRect src_roi,
                            Ipp8u* p_dst, int dst_step, IppiRect dst_roi, double angle, double x_shift, double y_shift, int interpolation);
IppStatus ippiRotate_32f_C3R(const Ipp32f *p_src, IppiSize src_size, int src_step, IppiRect src_roi,
                            Ipp32f* p_dst, int dst_step, IppiRect dst_roi, double angle, double x_shift, double y_shift, int interpolation);

// Border-Replicated Copy
IppStatus ippiCopyReplicateBorder_8u_C1R(const Ipp8u *p_src, int src_step, IppiSize src_roi_size,
                                         Ipp8u* p_dst, int dst_step, IppiSize dst_roi_size, int top_border_height, int left_border_width);
IppStatus ippiCopyReplicateBorder_8u_C3R(const Ipp8u *p_src, int src_step, IppiSize src_roi_size,
                                         Ipp8u* p_dst, int dst_step, IppiSize dst_roi_size, int top_border_height, int left_border_width);
IppStatus ippiCopyReplicateBorder_8u_C4R(const Ipp8u *p_src, int src_step, IppiSize src_roi_size,
                                         Ipp8u* p_dst, int dst_step, IppiSize dst_roi_size, int top_border_height, int left_border_width);

// Mirror & Transpose
IppStatus ippiMirror_8u_C1IR(Ipp8u *p_srcdst, int srcdst_step, IppiSize roi_size, IppiAxis flip_axis);
IppStatus ippiMirror_8u_C3IR(Ipp8u *p_srcdst, int srcdst_step, IppiSize roi_size, IppiAxis flip_axis);
IppStatus ippiMirror_8u_C4IR(Ipp8u *p_srcdst, int srcdst_step, IppiSize roi_size, IppiAxis flip_axis);
IppStatus ippiTranspose_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);

// Erode & Dilate
IppStatus ippiErode_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                           IppiSize dst_roi_size, const Ipp8u *p_mask, IppiSize mask_size, IppiPoint anchor);
IppStatus ippiDilate_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                            IppiSize dst_roi_size, const Ipp8u *p_mask, IppiSize mask_size, IppiPoint anchor);

// Border-Replicated Erode & Dilate
IppStatus ippiMorphologyInitAlloc_8u_C1R(int roi_width, const Ipp8u *p_mask, IppiSize mask_size, IppiPoint anchor, IppiMorphState **pp_state);
IppStatus ippiMorphologyFree(IppiMorphState *p_state);

IppStatus ippiErodeBorderReplicate_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                                          IppiSize roi_size, IppiBorderType border_type, IppiMorphState *p_state);
IppStatus ippiDilateBorderReplicate_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                                           IppiSize roi_size, IppiBorderType border_type, IppiMorphState *p_state);

// Image Integral
IppStatus ippiIntegral_8u32s_C1R(const Ipp8u *p_src, int src_step, Ipp32s *p_dst, int dst_step, IppiSize roi_size, Ipp32s add_value);
IppStatus ippiSqrIntegral_8u32s_C1R(const Ipp8u *p_src, int src_step, Ipp32s *p_dst, int dst_step,
                                    Ipp32s *p_square, int square_step, IppiSize roi_size, Ipp32s add_value, Ipp32s add_value_square);
IppStatus ippiSqrIntegral_8u32f64f_C1R(const Ipp8u *p_src, int src_step, Ipp32f *p_dst, int dst_step,
                                       Ipp64f *p_square, int square_step, IppiSize roi_size, Ipp32f add_value, Ipp64f add_value_square);
IppStatus ippiTiltedIntegral_8u32f_C1R(const Ipp8u *p_src, int src_step, Ipp32f *p_dst, int dst_step, IppiSize roi_size, Ipp32f add_value);
IppStatus ippiTiltedIntegral_8u32s_C1R(const Ipp8u *p_src, int src_step, Ipp32s *p_dst, int dst_step, IppiSize roi_size, Ipp32s add_value);

// Float Value Conversion
IppStatus ippiConvert_8u32f_C1R(const Ipp8u *p_src, int src_step, Ipp32f *p_dst, int dst_step, IppiSize roi_size);

// Value Thresholding & Comparison
IppStatus ippiThreshold_LTVal_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                                     IppiSize roi_size, Ipp8u threshold, Ipp8u value);
IppStatus ippiThreshold_GTVal_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step,
                                     IppiSize roi_size, Ipp8u threshold, Ipp8u value);
IppStatus ippiThreshold_LTVal_8u_C1IR(Ipp8u *p_src_dst, int src_dst_step, IppiSize roi_size, Ipp8u threshold, Ipp8u value);
IppStatus ippiThreshold_GTVal_8u_C1IR(Ipp8u *p_src_dst, int src_dst_step, IppiSize roi_size, Ipp8u threshold, Ipp8u value);
IppStatus ippiCompare_8u_C1R(const Ipp8u *p_src1, int src1_step, const Ipp8u *p_src2, int src2_step,
                             Ipp8u *p_dst, int dst_step, IppiSize roi_size, IppCmpOp ipp_compare_op);
IppStatus ippiCompareC_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u value,
                              Ipp8u* p_dst, int dst_step, IppiSize roi_size, IppCmpOp ipp_compare_op);

// Find Minimum / Maximum
IppStatus ippiMin_8u_C1R(const Ipp8u *p_src, int src_step, IppiSize roi_size, Ipp8u *p_min);
IppStatus ippiMax_8u_C1R(const Ipp8u *p_src, int src_step, IppiSize roi_size, Ipp8u *p_max);

// Arithmetics Operation (BYTE)
IppStatus ippiAbsDiff_8u_C1R(const Ipp8u *p_src1, int src1_step, const Ipp8u *p_src2, int src2_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiSum_8u_C1R(const Ipp8u *p_src, int src_step, IppiSize roi_size, Ipp64f *p_sum);
IppStatus ippiCountInRange_8u_C1R(const Ipp8u *p_src, int src_step, IppiSize roi_size, int *p_counts, Ipp8u lowerBound, Ipp8u upperBound);

// Arithmetics Operation (short)
IppStatus ippiAbs_16s_C1IR(Ipp16s *p_src_dst, int src_dst_step, IppiSize roi_size);

// Arithmetics Operation (float)
IppStatus ippiAdd_32f_C1R(const Ipp32f *p_src1, int src1_step, const Ipp32f *p_src2, int src2_step, Ipp32f *p_dst, int dst_step, IppiSize roi_size);
IppStatus ippiSub_32f_C1R(const Ipp32f *p_src1, int src1_step, const Ipp32f *p_src2, int src2_step, Ipp32f *p_dst, int dst_step, IppiSize roi_size);

// Filter & Convolution
IppStatus ippiFilterBox_8u_C1R(const Ipp8u *p_src,int src_step, Ipp8u *p_dst, int dst_step, IppiSize dst_roi_size, IppiSize mask_size, IppiPoint anchor);
IppStatus ippiFilterGauss_8u_C1R(const Ipp8u* p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize roi_size, IppiMaskSize mask);
IppStatus ippiConvValid_32f_C1R(const Ipp32f *p_src1, int src1_step, IppiSize src1_size,
                                const Ipp32f *p_src2, int src2_step, IppiSize src2_size, Ipp32f *p_dst, int dst_step);
IppStatus ippiFilterMedian_8u_C1R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize dst_roi_size, IppiSize mask_size, IppiPoint anchor);
IppStatus ippiFilterMedian_8u_AC4R(const Ipp8u *p_src, int src_step, Ipp8u *p_dst, int dst_step, IppiSize dst_roi_size, IppiSize mask_size, IppiPoint anchor);

// Flood Fill
IppStatus ippiFloodFillGetSize(IppiSize roi_size, int *p_buffer_size);
IppStatus ippiFloodFill_4Con_8u_C1IR(Ipp8u *p_image, int image_step, IppiSize roi_size, IppiPoint seed,
                                     Ipp8u new_value, IppiConnectedComp *p_region, Ipp8u* p_buffer);
IppStatus ippiFloodFill_8Con_8u_C1IR(Ipp8u *p_image, int image_step, IppiSize roi_size, IppiPoint seed,
                                     Ipp8u new_value, IppiConnectedComp *p_region, Ipp8u* p_buffer);

