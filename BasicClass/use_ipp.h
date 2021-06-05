#pragma once

#include "use_hylib.h"
#include "ipp_arm.h"

//////////////////////////////////////////////////////////////////////////

#define _MAKE_IPPISIZE_OPERATOR_V(_op) \
    template < class _TYPE > \
    inline IppiSize operator _op(const IppiSize &s, const _TYPE &v) \
    { \
        IppiSize size = s; \
        size.width = size.width _op v; \
        size.height = size.height _op v; \
        return size; \
    }

_MAKE_IPPISIZE_OPERATOR_V(+);
_MAKE_IPPISIZE_OPERATOR_V(-);
_MAKE_IPPISIZE_OPERATOR_V(*);
_MAKE_IPPISIZE_OPERATOR_V(/);

//////////////////////////////////////////////////////////////////////////

#define CONNECTION_4    4
#define CONNECTION_8    8

inline IppiSize ippiSize(const IppiRect &r)
{
    IppiSize size;
    size.width = r.width;
    size.height = r.height;
    return size;
}

inline IppiSize ippiSize(const int width, const int height)
{
    IppiSize size;
    size.width = width;
    size.height = height;
    return size;
}

inline IppiSize ippiSize(const HySize &s)
{
    IppiSize size;
    size.width = s.width;
    size.height = s.height;
    return size;
}

inline IppiSize ippiSize(const HyRect &s)
{
    IppiSize size;
    size.width = s.width;
    size.height = s.height;
    return size;
}

inline IppiSize ippiSize(const HyImage *p_image)
{
    if (p_image == NULL)
        return ippiSize(0, 0);

    return ippiSize(p_image->width, p_image->height);
}

inline IppiRect ippiRect(const int x, const int y, const int width, const int height)
{
    IppiRect rect;
    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;
    return rect;
}

inline IppiRect ippiRect(const IppiSize &s)
{
    return ippiRect(0, 0, s.width, s.height);
}

inline IppiRect ippiRect(const HyRect &r)
{
    return ippiRect(r.x, r.y, r.width, r.height);
}

inline HyRect hyRect(const IppiRect &r)
{
    return hyRect(r.x, r.y, r.width, r.height);
}

inline IppiPoint ippiPoint(const int _x, const int _y)
{
    IppiPoint point;
    point.x = _x;
    point.y = _y;
    return point;
}

inline IppiPoint ippiPoint(const HyPoint &p)
{
    return ippiPoint(p.x, p.y);
}

inline HyPoint hyPoint(const IppiPoint &p)
{
    return hyPoint(p.x, p.y);
}

inline IppiRect ippIntersectRect(const IppiRect &rect1, const IppiRect &rect2)
{
    int max_left = ch_Max(rect1.x, rect2.x);
    int min_right = ch_Min(rect1.x + rect1.width, rect2.x + rect2.width);
    int max_top = ch_Max(rect1.y, rect2.y);
    int min_bottom = ch_Min(rect1.y + rect1.height, rect2.y + rect2.height);

    IppiRect new_rect;
    new_rect.x = max_left;
    new_rect.y = max_top;
    new_rect.width = ch_Max(min_right - max_left, 0);    
    new_rect.height = ch_Max(min_bottom - max_top, 0);

    return new_rect;
}

inline void ippiGetResizeParameters(const IppiRect &src_roi, const IppiRect &dst_roi,
                                           double &x_factor, double &y_factor, double &x_shift, double &y_shift)
{
    x_factor = y_factor = 1.0;
    x_shift = y_shift = 0.0;

    if (src_roi.width != 0.0 && src_roi.height != 0.0)
    {
        x_factor = (double)dst_roi.width / src_roi.width;
        y_factor = (double)dst_roi.height / src_roi.height;
        x_shift = (double)(-src_roi.x) * x_factor + dst_roi.x;
        y_shift = (double)(-src_roi.y) * y_factor + dst_roi.y;
    }
}

//////////////////////////////////////////////////////////////////////////
extern const float ipp_bgr_to_gray_coeff[3];

//////////////////////////////////////////////////////////////////////////
int ippiBGRToGray(const HyImage *src, HyImage *dst);
int ippiBGRAToGray(const HyImage *src, HyImage *dst);
int ippiGrayToBGR(const HyImage *src, HyImage *dst);
int ippiGrayToBGRA(const HyImage *src, HyImage *dst);
int ippiBGRToYCbCr422(HyImage *p_bgr_image, HyImage *p_ycbcr_image);
int ippiBGRToYCbCr422_P3(HyImage *p_bgr_image, HyImage *p_y_image, HyImage *p_cb_image, HyImage *p_cr_image);
int ippiYCbCr422ToBGR(HyImage *p_ycbcr_image, HyImage *p_bgr_image);
int ippiRGBToHSV(HyImage *p_rgb_image, HyImage *p_hsv_image);
int ippiHSVToRGB(HyImage *p_hsv_image, HyImage *p_rgb_image);
int ippiResize(const HyImage *src, HyImage *dst, int method = IPPI_INTER_LINEAR);

IppStatus ippiResize_8u_C1R(const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, 
                            Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, 
                            double xFactor, double yFactor, int interpolation);
IppStatus ippiResize_8u_C3R(const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, 
                            Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, 
                            double xFactor, double yFactor, int interpolation);
IppStatus ippiResize_8u_C4R(const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, 
                             Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, 
                             double xFactor, double yFactor, int interpolation);

int ippiCopy(const HyImage *src, HyImage *dst);
int ippiCopyROIInfo(const HyROIInfo &src_info, HyImage *dst);
int ippiMaskCopy(const HyImage *src, const HyImage *mask, HyImage *dst);
int ippiROICopy(const HyImage *p_src_image, const HyImage *p_dst_image, const HyRect &roi);
int ippiSafeErode(const Ipp8u *p_src, int src_step, Ipp8u* p_dst, int dst_step,
                  IppiSize roi_size, const Ipp8u *p_mask, const IppiSize &mask_size, const IppiPoint &anchor);
int ippiSafeErode(const Ipp8u *p_src, int src_step, Ipp8u* p_dst, int dst_step, IppiSize roi_size, int mask_width);
int ippiSafeDilate(const Ipp8u *p_src, int src_step, Ipp8u* p_dst, int dst_step,
                   IppiSize roi_size, const Ipp8u *p_mask, const IppiSize &mask_size, const IppiPoint &anchor);
int ippiSafeDilate(const Ipp8u *p_src, int src_step, Ipp8u* p_dst, int dst_step, IppiSize roi_size, int mask_width);
int ippiGetEnlargeRotateCenterInfo(HySize &dstSize, double &xshift, double &yshift, HyRect src_roi, float angle, float xcenter, float ycenter);
int ippiRotate(HyImage *src, HyImage *dst, float angle, double xshift, double yshift);
int ippiMaskToBoundary(const BYTE *p_mask_data, int mask_step, BYTE *p_boundary_data, int boundary_step, const IppiSize &roi_size, int connection = CONNECTION_8);
int ippiMaskToBoundary(const HyImage *p_mask_image, HyImage *p_boundary_image, int connection = CONNECTION_8);
int ippiMirrorI(HyImage *src, IppiAxis flip);
