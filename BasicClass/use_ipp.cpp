
#include "stdafx.h"
#include "use_ipp.h"

IppStatus ipp_dummy = ippInit();

IppStatus ippiResize_8u_C1R(const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, 
                            Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, 
                            double xFactor, double yFactor, int interpolation)
{
    int buffer_size;
    IppiRect dstRoi = {0, 0, dstRoiSize.width, dstRoiSize.height};
    ippiResizeGetBufSize(srcRoi, dstRoi, 1, interpolation, &buffer_size);
    Ipp8u *pBuffer = ippsMalloc_8u(buffer_size); 

    double x_shift = (double)(-srcRoi.x) * xFactor;
    double y_shift = (double)(-srcRoi.y) * yFactor;

    IppStatus status = ippiResizeSqrPixel_8u_C1R(pSrc, srcSize, srcStep, srcRoi, 
                                                 pDst, dstStep, dstRoi, 
                                                 xFactor, yFactor, x_shift, y_shift, interpolation, pBuffer);
    ippsFree(pBuffer);

    return status;
}

IppStatus ippiResize_8u_C3R(const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, 
                            Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, 
                            double xFactor, double yFactor, int interpolation)
{
    int buffer_size;
    IppiRect dstRoi = {0, 0, dstRoiSize.width, dstRoiSize.height};
    ippiResizeGetBufSize(srcRoi, dstRoi, 3, interpolation, &buffer_size);
    Ipp8u *pBuffer = ippsMalloc_8u(buffer_size);

    double x_shift = (double)(-srcRoi.x) * xFactor;
    double y_shift = (double)(-srcRoi.y) * yFactor;

    IppStatus status = ippiResizeSqrPixel_8u_C3R(pSrc, srcSize, srcStep, srcRoi, 
                                                 pDst, dstStep, dstRoi, 
                                                 xFactor, yFactor, x_shift, y_shift, interpolation, pBuffer);
    ippsFree(pBuffer);

    return status;
}

IppStatus ippiResize_8u_C4R(const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, 
                             Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, 
                             double xFactor, double yFactor, int interpolation)
{
    int buffer_size;
    IppiRect dstRoi = {0, 0, dstRoiSize.width, dstRoiSize.height};
    ippiResizeGetBufSize(srcRoi, dstRoi, 4, interpolation, &buffer_size);
    Ipp8u *pBuffer = ippsMalloc_8u(buffer_size); 

    double x_shift = (double)(-srcRoi.x) * xFactor;
    double y_shift = (double)(-srcRoi.y) * yFactor;

    IppStatus status = ippiResizeSqrPixel_8u_C4R(pSrc, srcSize, srcStep, srcRoi, 
                                                 pDst, dstStep, dstRoi, 
                                                 xFactor, yFactor, x_shift, y_shift, interpolation, pBuffer);
    ippsFree(pBuffer);

    return status;
}



const float ipp_bgr_to_gray_coeff[3] = { 0.114f, 0.587f, 0.299f }; //  Y' = 0.299 * R' + 0.587 * G' + 0.114 * B';

int ippiBGRToGray(const HyImage *src, HyImage *dst)
{
    if (src == NULL || dst == NULL)
        return -1;
    if (src->nChannels != 3 || dst->nChannels != 1)
        return -1;

    BYTE *pSrc = (BYTE *)src->imageData;
    BYTE *pDst = (BYTE *)dst->imageData;
    // HyImage is in BGR order
    IppStatus ipp_status = ippiColorToGray_8u_C3C1R(pSrc, src->widthStep, pDst, dst->widthStep, ippiSize(hyGetSize(src)), ipp_bgr_to_gray_coeff);

    if (ipp_status != ippStsNoErr)
        return -1;

    return 0;
}

int ippiBGRAToGray(const HyImage *src, HyImage *dst)
{
    if (src == NULL || dst == NULL)
        return -1;
    if (src->nChannels != 4 || dst->nChannels != 1)
        return -1;

    BYTE *pSrc = (BYTE *)src->imageData;
    BYTE *pDst = (BYTE *)dst->imageData;
    // HyImage is in BGR order
    IppStatus ipp_status = ippiColorToGray_8u_AC4C1R(pSrc, src->widthStep, pDst, dst->widthStep, ippiSize(hyGetSize(src)), ipp_bgr_to_gray_coeff);

    if (ipp_status != ippStsNoErr)
        return -1;

    return 0;
}

int ippiGrayToBGR(const HyImage *src, HyImage *dst)
{
    if (src == NULL || dst == NULL)
        return -1;
    if (src->nChannels != 1 || dst->nChannels != 3)
        return -1;

    HyRect src_roi = hyGetImageROI(src);
    HyRect dst_roi = hyGetImageROI(dst);
    if (src_roi.width != dst_roi.width || src_roi.height != dst_roi.height)
        return -1;

    BYTE *pSrc = (BYTE *)src->imageData + src_roi.y * src->widthStep + src_roi.x;
    BYTE *pDst = (BYTE *)dst->imageData + dst_roi.y * dst->widthStep + dst_roi.x * dst->nChannels;
    ippiDup_8u_C1C3R(pSrc, src->widthStep, pDst, dst->widthStep, ippiSize(src_roi.width, src_roi.height));

    return 0;
}

int ippiGrayToBGRA(const HyImage *src, HyImage *dst)
{
    if (src == NULL || dst == NULL)
        return -1;
    if (src->nChannels != 1 || dst->nChannels != 4)
        return -1;

    HyRect src_roi = hyGetImageROI(src);
    HyRect dst_roi = hyGetImageROI(dst);
    if (src_roi.width != dst_roi.width || src_roi.height != dst_roi.height)
        return -1;

    BYTE *pSrc = (BYTE *)src->imageData + src_roi.y * src->widthStep + src_roi.x;
    BYTE *pDst = (BYTE *)dst->imageData + dst_roi.y * dst->widthStep + dst_roi.x * dst->nChannels;
    ippiDup_8u_C1C4R(pSrc, src->widthStep, pDst, dst->widthStep, ippiSize(src_roi.width, src_roi.height));

    return 0;
}

int ippiBGRToYCbCr422(HyImage *p_bgr_image, HyImage *p_ycbcr_image)
{
    if (p_bgr_image == NULL || p_ycbcr_image == NULL)
        return -1;

    if (p_bgr_image->nChannels != 3 && p_bgr_image->nChannels != 4)
        return -1;

    if (p_ycbcr_image->nChannels != 2)
        return -1;

    HyRect src_roi = hyGetImageROI(p_bgr_image);
    HyRect dst_roi = hyGetImageROI(p_ycbcr_image);

    if (src_roi.width != dst_roi.width || src_roi.height != dst_roi.height)
        return -1;

    BYTE *p_src = (BYTE *)p_bgr_image->imageData + src_roi.y * p_bgr_image->widthStep
                                                 + src_roi.x * p_bgr_image->nChannels;
    BYTE *p_dst = (BYTE *)p_ycbcr_image->imageData + dst_roi.y * p_ycbcr_image->widthStep
                                                   + dst_roi.x * p_ycbcr_image->nChannels;
    int src_step = p_bgr_image->widthStep;
    int dst_step = p_ycbcr_image->widthStep;
    IppiSize roi_size = ippiSize(src_roi.width, src_roi.height);

    if (p_bgr_image->nChannels == 3)
        ippiBGRToYCbCr422_8u_C3C2R(p_src, src_step, p_dst, dst_step, roi_size);
    else if (p_bgr_image->nChannels == 4)
        ippiBGRToYCbCr422_8u_AC4C2R(p_src, src_step, p_dst, dst_step, roi_size);

    return 0;
}

int ippiBGRToYCbCr422_P3(HyImage *p_bgr_image, HyImage *p_y_image, HyImage *p_cb_image, HyImage *p_cr_image)
{
    if (p_bgr_image == NULL || p_y_image == NULL || p_cb_image == NULL || p_cr_image == NULL)
        return -1;

    if (p_bgr_image->nChannels != 3 && p_bgr_image->nChannels != 4)
        return -1;

    if ((p_y_image->nChannels != 1) || (p_cb_image->nChannels != 1) || (p_cr_image->nChannels != 1))
        return -1;

    HyRect src_roi = hyGetImageROI(p_bgr_image);
    HyRect dst_y_roi = hyGetImageROI(p_y_image);
    HyRect dst_cb_roi = hyGetImageROI(p_cb_image);
    HyRect dst_cr_roi = hyGetImageROI(p_cr_image);

    if ((src_roi.width != dst_y_roi.width || src_roi.height != dst_y_roi.height)
        || (src_roi.width != (dst_cb_roi.width * 2) || src_roi.height != dst_cb_roi.height)
        || (src_roi.width != (dst_cr_roi.width * 2) || src_roi.height != dst_cr_roi.height))
    {
        return -1;
    }

    BYTE *p_src = (BYTE *)p_bgr_image->imageData + src_roi.y * p_bgr_image->widthStep
                          + src_roi.x * p_bgr_image->nChannels;
    int src_step = p_bgr_image->widthStep;
    
    BYTE *p_dst[3];
    int dst_step[3];
    p_dst[0] = (BYTE *)p_y_image->imageData + dst_y_roi.y * p_y_image->widthStep
                       + dst_y_roi.x * p_y_image->nChannels;
    p_dst[1] = (BYTE *)p_cb_image->imageData + dst_cb_roi.y * p_cb_image->widthStep
                       + dst_cb_roi.x * p_cb_image->nChannels;
    p_dst[2] = (BYTE *)p_cr_image->imageData + dst_cr_roi.y * p_cr_image->widthStep
                       + dst_cr_roi.x * p_cr_image->nChannels;
    dst_step[0] = p_y_image->widthStep;
    dst_step[1] = p_cb_image->widthStep;
    dst_step[2] = p_cr_image->widthStep;

    IppiSize roi_size = ippiSize(src_roi.width, src_roi.height);

    if (p_bgr_image->nChannels == 3)
        ippiBGRToYCbCr422_8u_C3P3R(p_src, src_step, p_dst, dst_step, roi_size);
    else if (p_bgr_image->nChannels == 4)
        ippiBGRToYCbCr422_8u_AC4P3R(p_src, src_step, p_dst, dst_step, roi_size);

    return 0;
}

int ippiYCbCr422ToBGR(HyImage *p_ycbcr_image, HyImage *p_bgr_image)
{
    if (p_bgr_image == NULL || p_ycbcr_image == NULL)
        return -1;

    if (p_bgr_image->nChannels != 3 && p_bgr_image->nChannels != 4)
        return -1;

    if (p_ycbcr_image->nChannels != 2)
        return -1;

    HyRect src_roi = hyGetImageROI(p_ycbcr_image);
    HyRect dst_roi = hyGetImageROI(p_bgr_image);

    if (src_roi.width != dst_roi.width || src_roi.height != dst_roi.height)
        return -1;

    BYTE *p_src = (BYTE *)p_ycbcr_image->imageData + src_roi.y * p_ycbcr_image->widthStep
                          + src_roi.x * p_ycbcr_image->nChannels;
    BYTE *p_dst = (BYTE *)p_bgr_image->imageData + dst_roi.y * p_bgr_image->widthStep
                          + dst_roi.x * p_bgr_image->nChannels;
    int src_step = p_ycbcr_image->widthStep;
    int dst_step = p_bgr_image->widthStep;
    IppiSize roi_size = ippiSize(src_roi.width, src_roi.height);

    if (p_bgr_image->nChannels == 3)
        ippiYCbCr422ToBGR_8u_C2C3R(p_src, src_step, p_dst, dst_step, roi_size);
    else if (p_bgr_image->nChannels == 4)
        ippiYCbCr422ToBGR_8u_C2C4R(p_src, src_step, p_dst, dst_step, roi_size, 255);

    return 0;
}

int ippiRGBToHSV(HyImage *p_rgb_image, HyImage *p_hsv_image)
{
	if ((p_rgb_image == NULL) || (p_hsv_image == NULL))
		return -1;

	if (p_rgb_image->nChannels != p_hsv_image->nChannels)
		return -1;

	if ((p_rgb_image->nChannels != 3) && (p_rgb_image->nChannels != 4))
		return -1;

	HyRect rgb_roi = hyGetImageROI(p_rgb_image);
	HyRect hsv_roi = hyGetImageROI(p_hsv_image);

	if ((hsv_roi.width > rgb_roi.width) || (hsv_roi.height > rgb_roi.height))
		return -1;

	int channels = p_rgb_image->nChannels;

	int rgb_step = p_rgb_image->widthStep;
	int hsv_step = p_hsv_image->widthStep;

	Ipp8u *p_rgb_data = (Ipp8u*)(p_rgb_image->imageData + rgb_roi.y * rgb_step) + rgb_roi.x * channels;
	Ipp8u *p_hsv_data = (Ipp8u*)(p_hsv_image->imageData + hsv_roi.y * hsv_step) + hsv_roi.x * channels;
	if (channels == 3)
	{
		ippiRGBToHSV_8u_C3R(p_rgb_data, rgb_step,
			p_hsv_data, hsv_step, ippiSize(hsv_roi.width, hsv_roi.height));
	}
	else if (channels == 4)
	{
		ippiRGBToHSV_8u_AC4R(p_rgb_data, rgb_step,
			p_hsv_data, hsv_step, ippiSize(hsv_roi.width, hsv_roi.height));
	}

	return 0;
}

int ippiHSVToRGB(HyImage *p_hsv_image, HyImage *p_rgb_image)
{
	if ((p_rgb_image == NULL) || (p_hsv_image == NULL))
		return -1;

	if (p_rgb_image->nChannels != p_hsv_image->nChannels)
		return -1;

	if ((p_rgb_image->nChannels != 3) && (p_rgb_image->nChannels != 4))
		return -1;

	HyRect rgb_roi = hyGetImageROI(p_rgb_image);
	HyRect hsv_roi = hyGetImageROI(p_hsv_image);

	if ((rgb_roi.width > hsv_roi.width) || (rgb_roi.height > hsv_roi.height))
		return -1;

	int channels = p_rgb_image->nChannels;

	int rgb_step = p_rgb_image->widthStep;
	int hsv_step = p_hsv_image->widthStep;

	Ipp8u *p_rgb_data = (Ipp8u*)(p_rgb_image->imageData + rgb_roi.y * rgb_step) + rgb_roi.x * channels;
	Ipp8u *p_hsv_data = (Ipp8u*)(p_hsv_image->imageData + hsv_roi.y * hsv_step) + hsv_roi.x * channels;
	if (channels == 3)
	{
		ippiHSVToRGB_8u_C3R(p_hsv_data, hsv_step,
			p_rgb_data, rgb_step, ippiSize(rgb_roi.width, rgb_roi.height));
	}
	else if (channels == 4)
	{
		ippiHSVToRGB_8u_AC4R(p_hsv_data, hsv_step,
			p_rgb_data, rgb_step, ippiSize(rgb_roi.width, rgb_roi.height));
	}

	return 0;
}

int ippiResize(const HyImage *src, HyImage *dst, int method /* = IPPI_INTER_LINEAR*/)
{
    if (src == NULL || dst == NULL)
        return -1;
    if (src->nChannels != dst->nChannels)
        return -1;

    HyRect src_roi = hyGetImageROI(src);
    HyRect dst_roi = hyGetImageROI(dst);
    BYTE *pSrc = (BYTE *)(src->imageData);
    BYTE *pDst = (BYTE *)(dst->imageData + dst_roi.y * dst->widthStep + dst_roi.x * dst->nChannels);
    double x_factor = double(dst_roi.width) / src_roi.width;
    double y_facror = double(dst_roi.height) / src_roi.height;

    if (src->nChannels == 1)
    {
        ippiResize_8u_C1R(pSrc, ippiSize(src->width, src->height), src->widthStep, ippiRect(src_roi),
                          pDst, dst->widthStep, ippiSize(dst_roi.width, dst_roi.height), x_factor, y_facror, method);
    }
    else if (src->nChannels == 3)
    {
        ippiResize_8u_C3R(pSrc, ippiSize(src->width, src->height), src->widthStep, ippiRect(src_roi),
                          pDst, dst->widthStep, ippiSize(dst_roi.width, dst_roi.height), x_factor, y_facror, method);
    }
    else if (src->nChannels == 4)
    {
        ippiResize_8u_C4R(pSrc, ippiSize(src->width, src->height), src->widthStep, ippiRect(src_roi),
                          pDst, dst->widthStep, ippiSize(dst_roi.width, dst_roi.height), x_factor, y_facror, method);
    }

    return 0;
}

int ippiCopy(const HyImage *src, HyImage *dst)
{
    if (src == NULL || dst == NULL)
        return -1;
    if (src->nChannels != dst->nChannels)
        return -1;

    HyRect src_roi = hyGetImageROI(src);
    HyRect dst_roi = hyGetImageROI(dst);
    BYTE *pSrc = (BYTE *)(src->imageData + src_roi.y * src->widthStep + src_roi.x * src->nChannels);
    BYTE *pDst = (BYTE *)(dst->imageData + dst_roi.y * dst->widthStep + dst_roi.x * dst->nChannels);

    if (src->nChannels == 1)
    {
        ippiCopy_8u_C1R(pSrc, src->widthStep, pDst, dst->widthStep, ippiSize(src_roi.width, src_roi.height));
    }
    else if (src->nChannels == 3)
    {
        ippiCopy_8u_C3R(pSrc, src->widthStep, pDst, dst->widthStep, ippiSize(src_roi.width, src_roi.height));
    }
    else if (src->nChannels == 4)
    {
        ippiCopy_8u_C4R(pSrc, src->widthStep, pDst, dst->widthStep, ippiSize(src_roi.width, src_roi.height));
    }

    return 0;
}

int ippiROICopy(const HyImage *p_src_image, const HyImage *p_dst_image, const HyRect &roi)
{
    if (p_src_image == NULL || p_dst_image == NULL)
        return -1;
    if (p_src_image->nChannels != p_dst_image->nChannels)
        return -1;
    if (IsValidRoi(roi, p_src_image) == false || IsValidRoi(roi, p_dst_image) == false)
        return -1;

    const int channels = p_src_image->nChannels;

    // DO NOT change the ROI of the images, access the content directly.
    const BYTE *p_src_start = p_src_image->imageData + roi.y * p_src_image->widthStep + roi.x * channels;
    BYTE *p_dst_start = p_dst_image->imageData + roi.y * p_dst_image->widthStep + roi.x * channels;

    if (channels == 1)
    {
        ippiCopy_8u_C1R(p_src_start, p_src_image->widthStep, p_dst_start, p_dst_image->widthStep, ippiSize(roi.width, roi.height));
    }
    else if (channels == 3)
    {
        ippiCopy_8u_C3R(p_src_start, p_src_image->widthStep, p_dst_start, p_dst_image->widthStep, ippiSize(roi.width, roi.height));
    }
    else if (channels == 4)
    {
        ippiCopy_8u_C4R(p_src_start, p_src_image->widthStep, p_dst_start, p_dst_image->widthStep, ippiSize(roi.width, roi.height));
    }

    return 0;
}

int ippiCopyROIInfo(const HyROIInfo &src_info, HyImage *dst)
{
    if (src_info.imageData == NULL || dst == NULL)
        return -1;
    if (src_info.nChannels != dst->nChannels)
        return -1;

    HyRect dst_roi = hyGetImageROI(dst);
    BYTE *pDst = (BYTE *)(dst->imageData + dst_roi.y * dst->widthStep + dst_roi.x * dst->nChannels);

    if (src_info.nChannels == 1)
    {
        ippiCopy_8u_C1R(src_info.imageData, src_info.widthStep, 
                        pDst, dst->widthStep, ippiSize(src_info.size));
    }
    else if (src_info.nChannels == 3)
    {
        ippiCopy_8u_C3R(src_info.imageData, src_info.widthStep, 
                        pDst, dst->widthStep, ippiSize(src_info.size));
    }
    else if (src_info.nChannels == 4)
    {
        ippiCopy_8u_C4R(src_info.imageData, src_info.widthStep, 
                        pDst, dst->widthStep, ippiSize(src_info.size));
    }

    return 0;
}

int ippiMaskCopy(const HyImage *src, const HyImage *mask, HyImage *dst)
{
    if (src == NULL || dst == NULL || mask == NULL)
        return -1;
    if (src->nChannels != dst->nChannels)
        return -1;
    if (mask->nChannels != 1)
        return -1;

    HyRect src_roi = hyGetImageROI(src);
    HyRect mask_roi = hyGetImageROI(mask);
    HyRect dst_roi = hyGetImageROI(dst);

    if (src_roi.width != mask_roi.width || src_roi.height != mask_roi.height)
        return -1;

    BYTE *pSrc = (BYTE *)(src->imageData + src_roi.y * src->widthStep + src_roi.x * src->nChannels);
    BYTE *pMask = (BYTE *)(mask->imageData + mask_roi.y * mask->widthStep + mask_roi.x);
    BYTE *pDst = (BYTE *)(dst->imageData + dst_roi.y * dst->widthStep + dst_roi.x * dst->nChannels);

    if (src->nChannels == 1)
    {
        ippiCopy_8u_C1MR(pSrc, src->widthStep, pDst, dst->widthStep, ippiSize(src_roi.width, src_roi.height), 
                         pMask, mask->widthStep);
    }
    else if (src->nChannels == 3)
    {
        ippiCopy_8u_C3MR(pSrc, src->widthStep, pDst, dst->widthStep, ippiSize(src_roi.width, src_roi.height),
                         pMask, mask->widthStep);
    }
    else if (src->nChannels == 4)
    {
        ippiCopy_8u_C4MR(pSrc, src->widthStep, pDst, dst->widthStep, ippiSize(src_roi.width, src_roi.height),
                         pMask, mask->widthStep);
    }

    return 0;
}

// Safe erosion / dilation function, use IPP BorderReplicate function to handle the boundary.
// NOTE: These functions only support BYTE data, and the square-size mask with centered anchor point.
int ippiSafeErode(const Ipp8u *p_src, int src_step, Ipp8u* p_dst, int dst_step,
                  IppiSize roi_size, const Ipp8u *p_mask, const IppiSize &mask_size, const IppiPoint &anchor)
{
    if (p_src == NULL || p_dst == NULL || p_mask == NULL)
        return -1;

    if (roi_size.width <= 0 || roi_size.height <= 0)
        return -1; // invalid size

    IppiMorphState *p_morph_state;
    ippiMorphologyInitAlloc_8u_C1R(roi_size.width, p_mask, mask_size, anchor, &p_morph_state);

    int status = ippiErodeBorderReplicate_8u_C1R(p_src, src_step, p_dst, dst_step,
                                                 roi_size, ippBorderRepl, p_morph_state);

    ippiMorphologyFree(p_morph_state);

    if (status == ippStsNoErr)
        return 0;
    else
        return -1;
}

int ippiSafeErode(const Ipp8u *p_src, int src_step, Ipp8u* p_dst, int dst_step, IppiSize roi_size, int mask_width)
{
    if (p_src == NULL || p_dst == NULL)
        return -1;

    if (mask_width % 2 != 1)
        return -1; // should be odd value

    if (roi_size.width <= 0 || roi_size.height <= 0)
        return -1; // invalid size

    int mask_area = mask_width * mask_width;
    BYTE *p_mask = new BYTE[mask_area];
    memset(p_mask, 1, mask_area);
    IppiSize mask_size = ippiSize(mask_width, mask_width);
    IppiPoint anchor = ippiPoint((mask_width - 1) / 2, (mask_width - 1) / 2);

    IppiMorphState *p_morph_state;
    ippiMorphologyInitAlloc_8u_C1R(roi_size.width, p_mask, mask_size, anchor, &p_morph_state);

    int status = ippiErodeBorderReplicate_8u_C1R(p_src, src_step, p_dst, dst_step,
                                                 roi_size, ippBorderRepl, p_morph_state);

    ippiMorphologyFree(p_morph_state);
    delete [] p_mask;

    if (status == ippStsNoErr)
        return 0;
    else
        return -1;
}

int ippiSafeDilate(const Ipp8u *p_src, int src_step, Ipp8u* p_dst, int dst_step,
                   IppiSize roi_size, const Ipp8u *p_mask, const IppiSize &mask_size, const IppiPoint &anchor)
{
    if (p_src == NULL || p_dst == NULL || p_mask == NULL)
        return -1;

    if (roi_size.width <= 0 || roi_size.height <= 0)
        return -1; // invalid size

    IppiMorphState *p_morph_state;
    ippiMorphologyInitAlloc_8u_C1R(roi_size.width, p_mask, mask_size, anchor, &p_morph_state);

    int status = ippiDilateBorderReplicate_8u_C1R(p_src, src_step, p_dst, dst_step,
                                                  roi_size, ippBorderRepl, p_morph_state);

    ippiMorphologyFree(p_morph_state);

    if (status == ippStsNoErr)
        return 0;
    else
        return -1;
}

int ippiSafeDilate(const Ipp8u *p_src, int src_step, Ipp8u* p_dst, int dst_step, IppiSize roi_size, int mask_width)
{
    if (p_src == NULL || p_dst == NULL)
        return -1;

    if (mask_width % 2 != 1)
        return -1; // should be odd value

    if (roi_size.width <= 0 || roi_size.height <= 0)
        return -1; // invalid size

    int mask_area = mask_width * mask_width;
    BYTE *p_mask = new BYTE[mask_area];
    memset(p_mask, 1, mask_area);
    IppiSize mask_size = ippiSize(mask_width, mask_width);
    IppiPoint anchor = ippiPoint((mask_width - 1) / 2, (mask_width - 1) / 2);

    IppiMorphState *p_morph_state;
    ippiMorphologyInitAlloc_8u_C1R(roi_size.width, p_mask, mask_size, anchor, &p_morph_state);

    int status = ippiDilateBorderReplicate_8u_C1R(p_src, src_step, p_dst, dst_step,
                                                  roi_size, ippBorderRepl, p_morph_state);

    ippiMorphologyFree(p_morph_state);
    delete [] p_mask;

    if (status == ippStsNoErr)
        return 0;
    else
        return -1;
}

int ippiGetEnlargeRotateCenterInfo(HySize &dstSize, double &xshift, double &yshift, HyRect src_roi, float angle, float xcenter, float ycenter)
{
	double rectRotatedImg[2][2];
	ippiGetRotateShift(xcenter, ycenter, angle, &xshift, &yshift);
	ippiGetRotateBound(ippiRect(src_roi), rectRotatedImg, angle, xshift, yshift);
	dstSize.width = int(rectRotatedImg[1][0] - rectRotatedImg[0][0]);
	dstSize.height = int(rectRotatedImg[1][1] - rectRotatedImg[0][1]);
	xshift += (double(dstSize.width) / 2 - xcenter);
	yshift += (double(dstSize.height) / 2 - ycenter);

	return 0;
}

int ippiRotate(HyImage *src, HyImage *dst, float angle, double xshift, double yshift)
{
    if (src == NULL || dst == NULL)
        return -1;
    if (src->nChannels != dst->nChannels)
        return -1;

    HyRect src_roi = hyGetImageROI(src);
    HyRect dst_roi = hyGetImageROI(dst);

    if (src->nChannels == 1)
    {
        ippiRotate_8u_C1R(src->imageData, ippiSize(hyGetSize(src)), src->widthStep, ippiRect(src_roi),
                          dst->imageData, dst->widthStep, ippiRect(dst_roi),
                          angle, xshift, yshift, IPPI_INTER_LINEAR);
    }
    else if (src->nChannels == 3)
    {
        ippiRotate_8u_C3R(src->imageData, ippiSize(hyGetSize(src)), src->widthStep, ippiRect(src_roi),
                          dst->imageData, dst->widthStep, ippiRect(dst_roi),
                          angle, xshift, yshift, IPPI_INTER_LINEAR);
    }
    else if (src->nChannels == 4)
    {
        ippiRotate_8u_C4R(src->imageData, ippiSize(hyGetSize(src)), src->widthStep, ippiRect(src_roi),
                          dst->imageData, dst->widthStep, ippiRect(dst_roi),
                          angle, xshift, yshift, IPPI_INTER_LINEAR);
    }

    return 0;
}

int ippiMaskToBoundary(const BYTE *p_mask_data, int mask_step, BYTE *p_boundary_data, int boundary_step, const IppiSize &roi_size, int connection/* = CONNECTION_8*/)
{
    // connection is the connect type of the boundary itself.
    // For CONNECTION_8, the boundary can be connected in diagonal.
    // For CONNECTION_4, the boundary can only be connected in 4-neighbor.

    if (p_mask_data == NULL || p_boundary_data == NULL)
        return -1;

    _MYASSERT(connection == CONNECTION_4 || connection == CONNECTION_8);

    const int width = roi_size.width;
    const int height = roi_size.height;

    if (width <= 0 || height <= 0)
        return -1;

    if (mask_step < width || boundary_step < width)
        return -1;

    ippiSet_8u_C1R(0, p_boundary_data, boundary_step, ippiSize(width, height));


    if (width >= 3 && height >= 3)
    {
        BYTE p_mask[9] = {0, 1, 0, 1, 1, 1, 0, 1, 0};
        if (connection == CONNECTION_4)
            p_mask[0] = p_mask[2] = p_mask[6] = p_mask[8] = 1;

        ippiErode_8u_C1R(p_mask_data + mask_step + 1, mask_step,
                         p_boundary_data + boundary_step + 1, boundary_step,
                         ippiSize(width - 2, height - 2), p_mask, ippiSize(3, 3), ippiPoint(1, 1));
    }

    ippiXor_8u_C1IR(p_mask_data, mask_step, p_boundary_data, boundary_step, ippiSize(width, height));

    return 0;
}

int ippiMaskToBoundary(const HyImage *p_mask_image, HyImage *p_boundary_image, int connection/* = CONNECTION_8*/)
{
    if (p_mask_image == NULL || p_boundary_image == NULL)
        return -1;

    if (hyGetSize(p_mask_image) != hyGetSize(p_boundary_image))
        return -1;

    return ippiMaskToBoundary(p_mask_image->imageData, p_mask_image->widthStep,
                              p_boundary_image->imageData, p_boundary_image->widthStep,
                              ippiSize(p_mask_image->width, p_mask_image->height), connection);
}

int ippiMirrorI(HyImage *src, IppiAxis flip)
{
    if (src == NULL)
        return -1;
    IppiRect src_roi = ippiRect(hyGetImageROI(src));
    BYTE *pSrc = (BYTE *)(src->imageData + src_roi.y * src->widthStep + src_roi.x * src->nChannels);

    if (src->nChannels == 1)
        ippiMirror_8u_C1IR(pSrc, src->widthStep, ippiSize(src_roi), flip);
    else if (src->nChannels == 3)
        ippiMirror_8u_C3IR(pSrc, src->widthStep, ippiSize(src_roi), flip);
    else if (src->nChannels == 4)
        ippiMirror_8u_C4IR(pSrc, src->widthStep, ippiSize(src_roi), flip);

    return 0;
}
