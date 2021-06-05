#include "stdafx.h"
#include "use_hylib.h"

#include <algorithm>
#include <functional>

#ifdef UNIX_OS
#ifdef MAC_OSX
#include <ApplicationServices/ApplicationServices.h>
#endif
#if IS_ANDROID || defined(LINUX_SERVER)
extern "C" {
#include "jpeglib.h"
#include "_inc/libpng/png.h"
}
#include <setjmp.h>
#endif
#include "use_ipp.h"
#else
#ifndef _WINRT_METRO
#include "use_gdiplus.h"
#else
#include "use_ipp.h"
#endif
#endif

const TCHAR *g_ext_table[] = 
{
    _T(".bmp"),
    _T(".jpg"),
    _T(".png")
};

const TCHAR *g_encoder_table[] = 
{
    _T("image/bmp"),
    _T("image/jpeg"),
    _T("image/png")
};

inline bool IsValidPixelDepth(int depth)
{
    return ((depth == HY_DEPTH_8U) || (depth == HY_DEPTH_16S) || (depth == HY_DEPTH_32F));
}

inline int hyGetPixelByteSize(const HyImage *p_image)
{
    _MYASSERT(p_image);
    return ((p_image->depth & 255) >> 3) * p_image->nChannels;
}

inline void hyExtractColor(int color, BYTE* buffer)
{
    if (buffer)
    {
        buffer[0] = (BYTE)color;
        buffer[1] = (BYTE)(color >> 8);
        buffer[2] = (BYTE)(color >> 16);
    }
}

inline void hySetPixel(HyImage *p_image, const HyPoint& point, BYTE *color)
{
    if (p_image == NULL || color == NULL)
        return;

    if ((point.x < 0) || (point.x >= p_image->width) || (point.y < 0) || (point.y >= p_image->height))
        return;

    int stride = p_image->widthStep;
    int channel = p_image->nChannels;

    BYTE *p_data = p_image->imageData + point.y * stride;
    int index = point.x * channel;
    memcpy(p_data + index, color, channel);
}

inline void hyFillRow(HyImage *p_image, int column_index, int row_start, int row_end, BYTE *color)
{
    if (p_image == NULL || color == NULL)
        return;

    int stride = p_image->widthStep;
    int channel = p_image->nChannels;

    BYTE *p_data = p_image->imageData + column_index * stride;
    int index = row_start * channel;

    if (channel == 1)
    {
        memset(p_data + index, color[0], row_end - row_start + 1);
    }
    else
    {
        for (int i = row_start; i <= row_end; i++)
        {
            memcpy(p_data + index, color, channel);
            index += channel;
        }
    }
}

inline void hySetRowFrontEnd(HyImage *p_image, int column_index, int row_start, int row_end, BYTE *color)
{
    if (p_image == NULL || color == NULL)
        return;

    int stride = p_image->widthStep;
    int channel = p_image->nChannels;
    BYTE *p_data = p_image->imageData + column_index * stride;

    if (channel == 1)
    {
        *(p_data + row_start) = color[0];
        *(p_data + row_end) = color[0];
    }
    else
    {
        memcpy(p_data + row_start * channel, color, channel);
        memcpy(p_data + row_end * channel, color, channel);
    }
}

inline void hyReplaceRow(HyImage *p_image, int column_index, int row_start, int row_end, BYTE *color, BYTE *old_color)
{
    if (p_image == NULL || color == NULL || old_color == NULL)
        return;

    int stride = p_image->widthStep;
    int channel = p_image->nChannels;

    BYTE *p_data = p_image->imageData + column_index * stride + row_start * channel;

    for (int i = row_start; i <= row_end; i++)
    {
        bool is_old_color = true;
        for (int j = 0; j < channel; j++)
            if (p_data[j] != old_color[j])
                is_old_color = false;

        if (is_old_color)
            memcpy(p_data, color, channel);

        p_data += channel;
    }
}

inline bool IsFileExtHyRaw(LPCTSTR path)
{
    return ch_GetFileExtName(path) == _T(".hyr");
}

HyImage* hyCreateImageHeader(const HySize &size, const int depth, const int channel)
{
    if (!IsValidPixelDepth(depth))
        return NULL;

    return new HyImage(size, depth, channel);
};

void hyReleaseImageHeader(HyImage **pp_image)
{
    if (pp_image == NULL)
        return;

    if (*pp_image)
    {
        delete *pp_image;
        *pp_image = NULL;
    }
}

HyImage* hyCreateImage(const HySize &size, const int depth, const int channel)
{
    if (!IsValidPixelDepth(depth))
        return NULL;

    if (size.width <= 0 || size.height <= 0 || channel <= 0)
        return NULL;

    HyImage *p_image = new HyImage(size, depth, channel);

    if (!p_image)
        return NULL;

    const int total_size = p_image->widthStep * p_image->height;
    _ALIGNED_MALLOC_PTR(p_image->imageData, BYTE, total_size);

    if (!p_image->imageData)
    {
        delete p_image;
        p_image = NULL;
    }

    return p_image;
};

void hyReleaseImage(HyImage **pp_image)
{
    if (pp_image == NULL)
        return;

    if (*pp_image)
    {
        if (!(*pp_image)->is_attached_buffer)
            _ALIGNED_FREE_PTR((*pp_image)->imageData);

        delete *pp_image;
        *pp_image = NULL;
    }
}

HyStatus hySetImageData(HyImage *p_image, BYTE *p_data, const int step)
{
    if (p_image == NULL)
        return HY_FAIL;

    int min_step = hyGetPixelByteSize(p_image) * p_image->width;
    if (step < min_step)
        return HY_FAIL; // impossible image step

    p_image->widthStep = step;
    p_image->imageData = p_data;
    p_image->is_attached_buffer = true;
    return HY_OK;
}

HySize hyGetSize(const HyImage *p_image)
{
    if (p_image == NULL)
        return HySize(0, 0);

    return HySize(p_image->width, p_image->height);
}

void hySetImageROI(HyImage *p_image, HyRect roi)
{
    if (p_image == NULL)
        return;

    p_image->roi = roi;
}

HyRect hySafelySetImageROI(HyImage *p_image, HyRect roi)
{
    HyRect safety_roi = hyIntersectRect(roi, hyRect(hyGetSize(p_image)));
    hySetImageROI(p_image, safety_roi);

    return safety_roi;
}

HyRect hyGetImageROI(const HyImage *p_image)
{
    if (p_image == NULL)
        return HyRect(0, 0, 0, 0);

    return p_image->roi;
}

void hyResetImageROI(HyImage *p_image)
{
    if (p_image)
        p_image->ResetROI();
}

void hyRotateImage90(HyImage **pp_img)
{
    if (pp_img == NULL)
        return;

    if ((*pp_img)->depth != HY_DEPTH_8U)
        return;

    int im_width = (*pp_img)->width;
    int im_height = (*pp_img)->height;
    int im_channels = (*pp_img)->nChannels;
    HyImage *p2 = hyCreateImage(hySize(im_height, im_width), HY_DEPTH_8U, im_channels);

    BYTE *pSrc = (*pp_img)->imageData;
    BYTE *pDest = p2->imageData;

    int src_stride = (*pp_img)->widthStep;
    int dest_stride = p2->widthStep;
    int i, j, p;
    for(i = 0; i < im_height; i++) {
        for(j = 0; j < im_width; j++) {
            int src_pos = i * src_stride + j * im_channels;
            int dest_pos = j * dest_stride + (im_height - i - 1) * im_channels;
            for(p = 0; p < im_channels; p++)
                pDest[dest_pos + p] = pSrc[src_pos + p];
        }
    }

    hyReleaseImage(pp_img);
    *pp_img = p2;
}

void hyRotateImage180(HyImage **pp_img)
{
    if (pp_img == NULL)
        return;

    if ((*pp_img)->depth != HY_DEPTH_8U)
        return;

    int im_width = (*pp_img)->width;
    int im_height = (*pp_img)->height;
    int im_channels = (*pp_img)->nChannels;
    HyImage *p2 = hyCreateImage(hySize(im_width, im_height), HY_DEPTH_8U, im_channels);

    BYTE *pSrc = (*pp_img)->imageData;
    BYTE *pDest = p2->imageData;

    int src_stride = (*pp_img)->widthStep;
    int dest_stride = p2->widthStep;
    int i, j, p;
    for(i = 0; i < im_height; i++) {
        for(j = 0; j < im_width; j++) {
            int src_pos = i * src_stride + j * im_channels;
            int dest_pos = (im_height - i - 1) * dest_stride + (im_width - j - 1) * im_channels;
            for(p = 0; p < im_channels; p++)
                pDest[dest_pos + p] = pSrc[src_pos + p];
        }
    }

    hyReleaseImage(pp_img);
    *pp_img = p2;
}

void hyRotateImage270(HyImage **pp_img)
{
    if (pp_img == NULL)
        return;

    if ((*pp_img)->depth != HY_DEPTH_8U)
        return;

    int im_width = (*pp_img)->width;
    int im_height = (*pp_img)->height;
    int im_channels = (*pp_img)->nChannels;
    HyImage *p2 = hyCreateImage(HySize(im_height, im_width), HY_DEPTH_8U, im_channels);

    BYTE *pSrc = (*pp_img)->imageData;
    BYTE *pDest = p2->imageData;

    int src_stride = (*pp_img)->widthStep;
    int dest_stride = p2->widthStep;
    int i, j, p;
    for(i = 0; i < im_height; i++) {
        for(j = 0; j < im_width; j++) {
            int src_pos = i * src_stride + j * im_channels;
            int dest_pos = (im_width - j - 1) * dest_stride + i * im_channels;
            for(p = 0; p < im_channels; p++)
                pDest[dest_pos + p] = pSrc[src_pos + p];
        }
    }

    hyReleaseImage(pp_img);
    *pp_img = p2;
}

#ifndef UNIX_OS
std_tstring GetEncoder(LPCTSTR path)
{
    std_tstring ext_name = ch_GetFileExtName(path);

    int amount = sizeof(g_ext_table) / sizeof(TCHAR*);
    for (int i = 0; i < amount; i++)
    {
        if (ext_name == g_ext_table[i])
        {
            return std_tstring(g_encoder_table[i]);
        }
    }

    return std_tstring(_T(""));
}

#ifndef _WINRT_METRO
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    if (format == NULL || pClsid == NULL)
        return -1;

    using namespace Gdiplus;

    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if(size == 0)
        return -1;  // Failure

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for(UINT j = 0; j < num; ++j)
    {
        if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }    
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

void hySave3ChannelImage(LPCTSTR path_, const HyImage *p_image)
{
    if (p_image == NULL)
        return;

    if (p_image->depth != HY_DEPTH_8U)
        return;

    _MYASSERT(p_image->nChannels == 3);

    using namespace Gdiplus;

    const wchar_t *path;
#ifdef _UNICODE
    path = path_;
#else
    std::wstring file_string = utf8_to_unicode(path_);
    path = file_string.c_str();
#endif

    std_tstring encoder_name_ = GetEncoder(path_);
    std::wstring encoder_name;
#ifdef _UNICODE
    encoder_name = encoder_name_;
#else
    encoder_name = utf8_to_unicode(encoder_name_.c_str());
#endif
    CLSID encoder_clsid;

    if (GetEncoderClsid(encoder_name.c_str(), &encoder_clsid) < 0)
    {
        MessageBoxA(NULL, "The encoder is not supported !", "Error", MB_OK | MB_ICONSTOP);
        return;
    }

    EncoderParameters encoder_parameters;
    ULONG quality = 100;

    encoder_parameters.Count = 1;
    encoder_parameters.Parameter[0].Guid = EncoderQuality;
    encoder_parameters.Parameter[0].Type = EncoderParameterValueTypeLong;
    encoder_parameters.Parameter[0].NumberOfValues = 1;
    encoder_parameters.Parameter[0].Value = &quality;

    int width = p_image->width;
    int height = p_image->height;
    int align_stride = CLALIGN(width * 3, 4);

    if (p_image->widthStep == align_stride)
    {
        Gdiplus::Bitmap *p_gimage = new Bitmap(width, height, align_stride,
                                               PixelFormat24bppRGB, p_image->imageData);

        if (p_gimage != NULL)
        {        
            p_gimage->Save(path, &encoder_clsid, NULL);
            delete p_gimage;
        }
    }
    else
    {
        // Create an image with aligned width step.
        HyImage *p_align_image = hyCreateImage(hySize(width, height), HY_DEPTH_8U, 3);
        _MYASSERT(p_align_image);
        ippiCopy(p_image, p_align_image);

        Gdiplus::Bitmap *p_gimage = new Bitmap(width, height, p_align_image->widthStep,
                                               PixelFormat24bppRGB, p_align_image->imageData);

        if (p_gimage != NULL)
        {        
            p_gimage->Save(path, &encoder_clsid, &encoder_parameters);
            delete p_gimage;
        }

        hyReleaseImage(&p_align_image);
    }
}

void hySave4ChannelImage(LPCTSTR path_, const HyImage *p_image)
{
    if (p_image == NULL)
        return;

    _MYASSERT(p_image->nChannels == 4);

    using namespace Gdiplus;

    const wchar_t *path;
#ifdef _UNICODE
    path = path_;
#else
    std::wstring file_string = utf8_to_unicode(path_);
    path = file_string.c_str();
#endif

    std_tstring encoder_name_ = GetEncoder(path_);
    std::wstring encoder_name;
#ifdef _UNICODE
    encoder_name = encoder_name_;
#else
    encoder_name = utf8_to_unicode(encoder_name_.c_str());
#endif
    CLSID encoder_clsid;

    if (GetEncoderClsid(encoder_name.c_str(), &encoder_clsid) < 0)
    {
        MessageBoxA(NULL, "The encoder is not supported !", "Error", MB_OK | MB_ICONSTOP);
        return;
    }

    EncoderParameters encoder_parameters;
    ULONG quality = 100;

    encoder_parameters.Count = 1;
    encoder_parameters.Parameter[0].Guid = EncoderQuality;
    encoder_parameters.Parameter[0].Type = EncoderParameterValueTypeLong;
    encoder_parameters.Parameter[0].NumberOfValues = 1;
    encoder_parameters.Parameter[0].Value = &quality;

    int width = p_image->width;
    int height = p_image->height;
    int align_stride = p_image->widthStep;

    Gdiplus::Bitmap *p_gimage = new Bitmap(width, height, align_stride,
        PixelFormat32bppARGB, p_image->imageData);

    if (p_gimage != NULL)
    {        
        p_gimage->Save(path, &encoder_clsid, &encoder_parameters);
        delete p_gimage;
    }
}

#endif

void hySaveImage(LPCTSTR path, HyImage *p_image)
{
#ifndef _WINRT_METRO
    if (p_image == NULL)
        return;

    if (p_image->depth != HY_DEPTH_8U)
        return;

    if (IsFileExtHyRaw(path))
    {
        if (!hySaveImageToRawData(path, p_image))
        {
            MessageBoxA(NULL, "Image save in .hyr format failed!", "Error", MB_OK | MB_ICONSTOP);
        }
        return;
    }

    HyRect old_roi = hyGetImageROI(p_image);
    hyResetImageROI(p_image);

    if (p_image->nChannels == 1)
    {
        HyImage *p_color_image = hyCreateImage(hyGetSize(p_image), HY_DEPTH_8U, 3);
        ippiGrayToBGR(p_image, p_color_image);
        hySave3ChannelImage(path, p_color_image);
        hyReleaseImage(&p_color_image);
    }
    else if (p_image->nChannels == 3)
    {
        hySave3ChannelImage(path, p_image);
    }
    else if (p_image->nChannels == 4)
    {
        hySave4ChannelImage(path, p_image);
    }

    hySetImageROI(p_image, old_roi);
#endif
}
#else
#ifdef MAC_OSX
void SaveCGImageAsPNG(CGImageRef imgRef, const char* fpath)
{
    CFStringRef strRef = CFStringCreateWithCString(NULL, fpath, kCFStringEncodingMacRoman);
    CFStringRef typRef = CFStringCreateWithCString(NULL, "public.png", kCFStringEncodingMacRoman);
    CFURLRef    urlRef = CFURLCreateWithFileSystemPath(NULL, strRef, 0, false);

    CGImageDestinationRef dstRef = CGImageDestinationCreateWithURL(urlRef, typRef, 1, NULL);

    CGImageDestinationAddImage(dstRef, imgRef, NULL);
    CGImageDestinationFinalize(dstRef);
    CFRelease(dstRef);
}

void hySaveImage(LPCTSTR path, HyImage *p_image)
{
    if (!p_image) return;
    if (p_image->nChannels != 1 && p_image->nChannels != 3 && p_image->nChannels != 4)
    {
        return;
    }

    std_tstring path_string(path);
    if (path_string.length() == 0) return;

    if (IsFileExtHyRaw(path))
    {
        hySaveImageToRawData(path, p_image);
        return;
    }

    // Ensure extension of file to be png.
    size_t dot_pos =  path_string.rfind(_T("."));
    if (dot_pos == std::string::npos)
    {
        path_string += _T(".png");
    }
    else
    {
        std_tstring save_type_str = path_string.substr(dot_pos + 1);
        if (save_type_str.length() == 0)
        {
            path_string += _T("png");
        }
        else
        {
            if (save_type_str.compare(_T("png")) != 0 &&
                save_type_str.compare(_T("PNG")) != 0)
            {
                path_string = path_string.substr(0, dot_pos + 1) + _T("png");
            }
        }
    }

    HyImage *p_image_4channel = hyCreateImage(hyGetSize(p_image), HY_DEPTH_8U, 4);
    if (p_image->nChannels == 4)
    {
        ippiCopy_8u_C4R(p_image->imageData,
                        p_image->widthStep,
                        p_image_4channel->imageData,
                        p_image_4channel->widthStep,
                        ippiSize(hyGetSize(p_image)));
    }
    else
    {
        if (p_image->nChannels == 1)
        {
            BYTE *p_planes[] = {p_image->imageData, 
                                p_image->imageData, 
                                p_image->imageData, 
                                p_image->imageData};
            ippiCopy_8u_P4C4R(p_planes,
                              p_image->widthStep,
                              p_image_4channel->imageData,
                              p_image_4channel->widthStep,
                              ippiSize(hyGetSize(p_image)));
        }
        else // 3-channel
        {
            ippiCopy_8u_C3AC4R(p_image->imageData,
                               p_image->widthStep,
                               p_image_4channel->imageData,
                               p_image_4channel->widthStep,
                               ippiSize(hyGetSize(p_image)));
        }
    }

    int pixel_order[] = {2, 1, 0, 3};
    ippiSwapChannels_8u_C4IR(p_image_4channel->imageData,
                             p_image_4channel->widthStep,
                             ippiSize(hyGetSize(p_image_4channel)),
                             pixel_order);
    CGColorSpaceRef p_colorspace = CGColorSpaceCreateDeviceRGB();
    CGImageAlphaInfo alpha_info = (p_image->nChannels == 4) ? kCGImageAlphaPremultipliedLast 
                                                            : kCGImageAlphaNoneSkipLast;
    CGContextRef p_cg_context = CGBitmapContextCreate(p_image_4channel->imageData,
                                                      p_image_4channel->width,
                                                      p_image_4channel->height,
                                                      8,
                                                      p_image_4channel->widthStep,
                                                      p_colorspace,
                                                      alpha_info | kCGRenderingIntentDefault);
    CGColorSpaceRelease(p_colorspace);
    CGImageRef p_cg_image = CGBitmapContextCreateImage(p_cg_context);
    CGContextRelease(p_cg_context);

    SaveCGImageAsPNG(p_cg_image, path_string.c_str());
    CGImageRelease(p_cg_image);
    hyReleaseImage(&p_image_4channel);
}
#elif IS_ANDROID || defined(LINUX_SERVER)
void write_JPEG_file (const HyImage *p_image, const char * filename, int quality = 100)
{
    if (!p_image) return;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE * outfile;     /* target file */
    JSAMPROW row_pointer[1];    /* pointer to JSAMPLE row[s] */

    const int width = p_image->width;
    const int height = p_image->height;
    const int stride = p_image->widthStep;
    const BYTE *p_bgr_data = p_image->imageData;

    // Convert BGR to RGB
    BYTE *p_data = NULL;
    _NEW_PTRS(p_data, BYTE, height * stride);
    for (int y = 0; y < height; y++)
    {
        const BYTE *p_src_row = p_bgr_data + stride * y;
        BYTE *p_dst_row = p_data + stride * y;
        for (int x = 0; x < width; x++)
        {
            p_dst_row[x * 3] = p_src_row[x * 3 + 2];
            p_dst_row[x * 3 + 1] = p_src_row[x * 3 + 1];
            p_dst_row[x * 3 + 2] = p_src_row[x * 3];
        }
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    if ((outfile = fopen(filename, "wb")) == NULL) {
        fprintf(stderr, "can't open %s\n", filename);
        _DELETE_PTRS(p_data);
        exit(1);
    }
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = width;  /* image width and height, in pixels */
    cinfo.image_height = height;
    cinfo.input_components = 3;     /* # of color components per pixel */
    cinfo.in_color_space = JCS_RGB;     /* colorspace of input image */

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

    jpeg_start_compress(&cinfo, TRUE);

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = & p_data[cinfo.next_scanline * stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);

    _DELETE_PTRS(p_data);
    jpeg_destroy_compress(&cinfo);
}
void hySaveImage(LPCTSTR path, HyImage *p_image)
{
    if (!p_image) return;
    if (p_image->nChannels != 1 && 
        p_image->nChannels != 3 && 
        p_image->nChannels != 4)
    {
        return;
    }

    std_tstring path_string(path);
    if (path_string.length() == 0) return;

    if (IsFileExtHyRaw(path))
    {
        hySaveImageToRawData(path, p_image);
        return;
    }

    // Ensure extension of file to be jpg.
    size_t dot_pos =  path_string.rfind(_T("."));
    if (dot_pos == std::string::npos)
    {
        path_string += _T(".jpg");
    }
    else
    {
        std_tstring save_type_str = path_string.substr(dot_pos + 1);
        if (save_type_str.length() == 0)
        {
            path_string += _T("jpg");
        }
        else
        {
            if (save_type_str.compare(_T("jpg")) != 0 &&
                save_type_str.compare(_T("JPG")) != 0 && 
                save_type_str.compare(_T("jpeg")) != 0 &&
                save_type_str.compare(_T("JPEG")) != 0)
            {
                path_string = path_string.substr(0, dot_pos + 1) + _T("jpg");
            }
        }
    }

    HyImage *p_image_3channel = hyCreateImage(hyGetSize(p_image), HY_DEPTH_8U, 3);
    if (p_image->nChannels == 3)
    {
        ippiCopy_8u_C3R(p_image->imageData,
                        p_image->widthStep,
                        p_image_3channel->imageData,
                        p_image_3channel->widthStep,
                        ippiSize(hyGetSize(p_image)));
    }
    else
    {
        if (p_image->nChannels == 1)
        {
            ippiGrayToBGR(p_image, p_image_3channel);
        }
        else // 4-channel
        {
            ippiCopy_8u_AC4C3R(p_image->imageData,
                               p_image->widthStep,
                               p_image_3channel->imageData,
                               p_image_3channel->widthStep,
                               ippiSize(hyGetSize(p_image)));
        }
    }
    write_JPEG_file(p_image_3channel, path_string.c_str());
    hyReleaseImage(&p_image_3channel);
}
#else
void hySaveImage(LPCTSTR path, HyImage *p_image)
{
}
#endif
#endif

void hySaveImage(HyImage *pImage, LPCTSTR fmt, ...)
{
    _TCHAR buffer[256];
    va_list marker;
    va_start(marker, fmt);
    _vstprintf(buffer, fmt, marker);
    hySaveImage(buffer, pImage);
    va_end (marker);
}

HyRect hyEnlargeROI(const HyRect &src_roi, const HySize &image_size, const int left_space, const int top_space, const int right_space, const int bottom_space, bool is_shift)
{
    HyRect dst_roi;

    // for horizontal enlarge
    dst_roi.x = ch_Max(0, src_roi.x - left_space);
    if (is_shift)
    {
        dst_roi.width = src_roi.width + left_space + right_space;
        if ((dst_roi.x + dst_roi.width) > image_size.width)
        {
            dst_roi.x = image_size.width - dst_roi.width;
            if (dst_roi.x < 0)
            {
                dst_roi.x = 0;
                dst_roi.width = image_size.width;
            }
        }
    }
    else
    {
        int right = ch_Min(image_size.width, src_roi.x + src_roi.width + right_space);
        dst_roi.width = right - dst_roi.x;
    }

    // for vertical enlarge
    dst_roi.y = ch_Max(0, src_roi.y - top_space);
    if (is_shift)
    {
        dst_roi.height = src_roi.height + top_space + bottom_space;
        if ((dst_roi.y + dst_roi.height) > image_size.height)
        {
            dst_roi.y = image_size.height - dst_roi.height;
            if (dst_roi.y < 0)
            {
                dst_roi.y = 0;
                dst_roi.height = image_size.height;
            }
        }
    }
    else
    {
        int bottom = ch_Min(image_size.height, src_roi.y + src_roi.height + bottom_space);
        dst_roi.height = bottom - dst_roi.y;
    }

    return dst_roi;
}

HyRect hyEnlargeROI(const HyRect &src_roi, const HySize &image_size, const float enlarge_ratio, bool is_shift)
{
    int h_enlarge_pixel = (int)(src_roi.width * enlarge_ratio);
    int v_enlarge_pixel = (int)(src_roi.height * enlarge_ratio);
    return hyEnlargeROI(src_roi, image_size, h_enlarge_pixel, v_enlarge_pixel, h_enlarge_pixel, v_enlarge_pixel, is_shift);
}

HyRect hyEnlargeROI(const HyRect &src_roi, const HySize &image_size, const float left_ratio, const float top_ratio, const float right_ratio, const float bottom_ratio, bool is_shift)
{
    const int width = src_roi.width;
    const int height = src_roi.height;

    const int left_space   = ch_Round(left_ratio * width);
    const int top_space    = ch_Round(top_ratio * height);
    const int right_space  = ch_Round(right_ratio * width);
    const int bottom_space = ch_Round(bottom_ratio * height);

    return hyEnlargeROI(src_roi, image_size, left_space, top_space, right_space, bottom_space, is_shift);
}

HyRect hyEnlargeRect(const HyRect &src_rect, const int left_space, const int top_space, const int right_space, const int bottom_space)
{
    HyRect dst_rect;

    // for horizontal enlarge
    dst_rect.x = src_rect.x - left_space;
    dst_rect.width = left_space + src_rect.width + right_space;

    // for vertical enlarge
    dst_rect.y = src_rect.y - top_space;
    dst_rect.height = top_space + src_rect.height + bottom_space;

    return dst_rect;
}

HyRect hyEnlargeRect(const HyRect &src_rect, const float enlarge_ratio)
{
    int h_enlarge_pixel = (int)(src_rect.width * enlarge_ratio);
    int v_enlarge_pixel = (int)(src_rect.height * enlarge_ratio);
    return hyEnlargeRect(src_rect, h_enlarge_pixel, v_enlarge_pixel, h_enlarge_pixel, v_enlarge_pixel);
}

HyRect hyEnlargeRect(const HyRect &src_rect, const float left_ratio, const float top_ratio, const float right_ratio, const float bottom_ratio)
{
    const int width = src_rect.width;
    const int height = src_rect.height;

    const int left_space   = ch_Round(left_ratio * width);
    const int top_space    = ch_Round(top_ratio * height);
    const int right_space  = ch_Round(right_ratio * width);
    const int bottom_space = ch_Round(bottom_ratio * height);

    return hyEnlargeRect(src_rect, left_space, top_space, right_space, bottom_space);
}

HyRect hyShrinkRect(const HyRect &src_rect, const int left_space, const int top_space, const int right_space, const int bottom_space)
{
    HyRect dst_rect;

    // for horizontal shrink
    dst_rect.x = src_rect.x + left_space;
    dst_rect.width = src_rect.width - left_space - right_space;

    // for vertical shrink
    dst_rect.y = src_rect.y + top_space;
    dst_rect.height = src_rect.height - top_space - bottom_space;

    return dst_rect;
}

HyRect hyShrinkRect(const HyRect &src_rect, const float shrink_ratio)
{
    int h_shrink_pixel = (int)(src_rect.width * shrink_ratio);
    int v_shrink_pixel = (int)(src_rect.height * shrink_ratio);
    return hyShrinkRect(src_rect, h_shrink_pixel, v_shrink_pixel, h_shrink_pixel, v_shrink_pixel);
}

void hyFillCircle(HyImage* p_image, const HyPoint &center, int radius, int color)
{
    if (p_image == NULL)
        return;

    if (p_image->depth != HY_DEPTH_8U)
        return;

    // use mid-point circle algorithm to accelerate the circle filling

    BYTE color_buffer[4] = {0, 0, 0, 255};
    
    if (p_image->nChannels >= 3)
        hyExtractColor(color, color_buffer);
    else if (p_image->nChannels == 1)
        color_buffer[0] = (BYTE) color;
    else
        _MYASSERT(false);

    HySize size = hyGetSize(p_image);

    int error = 0;
    int dx = radius;
    int dy = 0;
    int plus = 1;
    int minus = (radius << 1) - 1;  // radius/2 - 1

    int inside = ((center.x >= radius) && (center.x < size.width - radius)
                 &&
                 (center.y >= radius) && (center.y < size.height - radius));

    while (dx >= dy)
    {
        int y11 = center.y - dy, y12 = center.y + dy, y21 = center.y - dx, y22 = center.y + dx;
        int x11 = center.x - dx, x12 = center.x + dx, x21 = center.x - dy, x22 = center.x + dy;

        if (inside)
        {
            hyFillRow(p_image, y11, x11, x12, color_buffer);
            hyFillRow(p_image, y12, x11, x12, color_buffer);
            hyFillRow(p_image, y21, x21, x22, color_buffer);
            hyFillRow(p_image, y22, x21, x22, color_buffer);
        }
        else if (x11 < size.width && x12 >= 0 && y21 < size.height && y22 >= 0 )
        {
            x11 = ch_Max(x11, 0);
            x12 = ch_Min(x12, size.width - 1);

            if (y11 >= 0 && y11 < size.height)
                hyFillRow(p_image, y11, x11, x12, color_buffer);

            if (y12 >= 0 && y12 < size.height)
                hyFillRow(p_image, y12, x11, x12, color_buffer);

            if (x21 < size.width && x22 >= 0)
            {
                x21 = ch_Max(x21, 0);
                x22 = ch_Min(x22, size.width - 1);

                if (y21 >= 0 && y21 < size.height)
                    hyFillRow(p_image, y21, x21, x22, color_buffer);

                if (y22 >= 0 && y22 < size.height )
                    hyFillRow(p_image, y22, x21, x22, color_buffer);
            }
        }

        dy++;
        error += plus;
        plus += 2;
        
        if (error > 0)
        {
            error -= minus;
            dx--;
            minus -= 2;
        }
    }
}

void hyCircle(HyImage* p_image, const HyPoint &center, int radius, int color, int thickness)
{
    if (p_image == NULL)
        return;

    if (p_image->depth != HY_DEPTH_8U)
        return;

    if (thickness == 0)
        return;
    else if (thickness < 0)
    {
        hyFillCircle(p_image, center, radius, color);
        return;
    }

    // use mid-point circle algorithm to accelerate the circle filling

    BYTE color_buffer[4];
    color_buffer[3] = 255;
    
    if (p_image->nChannels >= 3)
        hyExtractColor(color, color_buffer);
    else if (p_image->nChannels == 1)
        color_buffer[0] = (BYTE) color;
    else
        _MYASSERT(false);

    HySize size = hyGetSize(p_image);

    int error = 0;
    int dx = radius;
    int dy = 0;
    int plus = 1;
    int minus = (radius << 1) - 1;  // radius/2 - 1

    int inside = ((center.x >= radius) && (center.x < size.width - radius)
                 &&
                 (center.y >= radius) && (center.y < size.height - radius));

    while (dx >= dy)
    {
        int y11 = center.y - dy, y12 = center.y + dy, y21 = center.y - dx, y22 = center.y + dx;
        int x11 = center.x - dx, x12 = center.x + dx, x21 = center.x - dy, x22 = center.x + dy;

        if (inside)
        {
            hySetRowFrontEnd(p_image, y11, x11, x12, color_buffer);
            hySetRowFrontEnd(p_image, y12, x11, x12, color_buffer);
            hySetRowFrontEnd(p_image, y21, x21, x22, color_buffer);
            hySetRowFrontEnd(p_image, y22, x21, x22, color_buffer);
        }
        else if (x11 < size.width && x12 >= 0 && y21 < size.height && y22 >= 0 )
        {
            x11 = ch_Max(x11, 0);
            x12 = ch_Min(x12, size.width - 1);

            if (y11 >= 0 && y11 < size.height)
                hySetRowFrontEnd(p_image, y11, x11, x12, color_buffer);

            if (y12 >= 0 && y12 < size.height)
                hySetRowFrontEnd(p_image, y12, x11, x12, color_buffer);

            if (x21 < size.width && x22 >= 0)
            {
                x21 = ch_Max(x21, 0);
                x22 = ch_Min(x22, size.width - 1);

                if (y21 >= 0 && y21 < size.height)
                    hySetRowFrontEnd(p_image, y21, x21, x22, color_buffer);

                if (y22 >= 0 && y22 < size.height )
                    hySetRowFrontEnd(p_image, y22, x21, x22, color_buffer);
            }
        }

        dy++;
        error += plus;
        plus += 2;
        
        if (error > 0)
        {
            error -= minus;
            dx--;
            minus -= 2;
        }
    }
}
void hyReplaceCircle(HyImage* p_image, const HyPoint &center, int radius, int color, int old_color)
{
    if (p_image == NULL)
        return;

    if (p_image->depth != HY_DEPTH_8U)
        return;

    // use mid-point circle algorithm to accelerate the circle filling

    BYTE color_buffer[3];
    BYTE old_color_buffer[3];
    
    if (p_image->nChannels == 3)
    {
        hyExtractColor(color, color_buffer);
        hyExtractColor(old_color, old_color_buffer);
    }
    else if (p_image->nChannels == 1)
    {
        color_buffer[0] = (BYTE) color;
        old_color_buffer[0] = (BYTE) old_color;
    }
    else
    {
        _MYASSERT(false);
    }

    HySize size = hyGetSize(p_image);

    int error = 0;
    int dx = radius;
    int dy = 0;
    int plus = 1;
    int minus = (radius << 1) - 1;  // radius/2 - 1

    int inside = ((center.x >= radius) && (center.x < size.width - radius)
                 &&
                 (center.y >= radius) && (center.y < size.height - radius));

    while (dx >= dy)
    {
        int y11 = center.y - dy, y12 = center.y + dy, y21 = center.y - dx, y22 = center.y + dx;
        int x11 = center.x - dx, x12 = center.x + dx, x21 = center.x - dy, x22 = center.x + dy;

        if (inside)
        {
            hyReplaceRow(p_image, y11, x11, x12, color_buffer, old_color_buffer);
            hyReplaceRow(p_image, y12, x11, x12, color_buffer, old_color_buffer);
            hyReplaceRow(p_image, y21, x21, x22, color_buffer, old_color_buffer);
            hyReplaceRow(p_image, y22, x21, x22, color_buffer, old_color_buffer);
        }
        else if (x11 < size.width && x12 >= 0 && y21 < size.height && y22 >= 0 )
        {
            x11 = ch_Max(x11, 0);
            x12 = ch_Min(x12, size.width - 1);

            if (y11 >= 0 && y11 < size.height)
                hyReplaceRow(p_image, y11, x11, x12, color_buffer, old_color_buffer);

            if (y12 >= 0 && y12 < size.height)
                hyReplaceRow(p_image, y12, x11, x12, color_buffer, old_color_buffer);

            if (x21 < size.width && x22 >= 0)
            {
                x21 = ch_Max(x21, 0);
                x22 = ch_Min(x22, size.width - 1);

                if (y21 >= 0 && y21 < size.height)
                    hyReplaceRow(p_image, y21, x21, x22, color_buffer, old_color_buffer);

                if (y22 >= 0 && y22 < size.height )
                    hyReplaceRow(p_image, y22, x21, x22, color_buffer, old_color_buffer);
            }
        }

        dy++;
        error += plus;
        plus += 2;
        
        if (error > 0)
        {
            error -= minus;
            dx--;
            minus -= 2;
        }
    }
}

void hyDonut(HyImage* p_image, const HyPoint &center, float inner_radius, float outer_radius, BYTE alpha)
{
    if (p_image == NULL)
        return;

    if (p_image->depth != HY_DEPTH_8U)
        return;

    if (inner_radius <= 0)
        return;

    if (inner_radius >= outer_radius)
        return;

    int x_min = ch_Round(floorf(center.x - outer_radius));
    int y_min = ch_Round(floorf(center.y - outer_radius));
    int x_max = ch_Round(ceilf(center.x + outer_radius));
    int y_max = ch_Round(ceilf(center.y + outer_radius));
    x_min = ch_Max(x_min, 0);
    y_min = ch_Max(y_min, 0);
    x_max = ch_Min(x_max, p_image->width - 1);
    y_max = ch_Min(y_max, p_image->height - 1);

    const float outer_radius_square = outer_radius * outer_radius;
    const float inner_radius_square = inner_radius * inner_radius;

    for (int y = y_min; y <= y_max; y++)
    {
        BYTE *p_image_scan = p_image->imageData + y * p_image->widthStep;
        float dy = (float)y - center.y;
        float dy2 = dy * dy;

        for (int x = x_min; x <= x_max; x++)
        {
            float dx = (float)x - center.x;
            float d2 = dx * dx + dy2;

            if (d2 < outer_radius_square && d2 > inner_radius_square)
            {
                p_image_scan[x] = alpha;
            }
        }
    }
}

struct ConvexPolyEdgeInfo
{
    int end_vertex_index;
    int vertex_index_jump;

    int process_x;
    int x_step;
    int bottom_y;
};

enum ConvexEdgeIndex
{
    CEI_LEFT = 0,
    CEI_RIGHT
};

void hyFillConvexPoly(HyImage *p_image, const HyPoint *pts, int pts_amount, int color)
{
    if (p_image == NULL)
        return;

    if (pts == NULL)
        return;

    if (pts_amount < 3)
        return;

    if (p_image->depth != HY_DEPTH_8U)
        return;

    BYTE color_buffer[4];

    color_buffer[3] = 255;
    if (p_image->nChannels == 3 || p_image->nChannels == 4)
        hyExtractColor(color, color_buffer);
    else if (p_image->nChannels == 1)
        color_buffer[0] = (BYTE) color;
    else
        _MYASSERT(false);

    HySize size = hyGetSize(p_image);

    int min_y = INT_MAX, max_y = INT_MIN;
    int min_x = INT_MAX, max_x = INT_MIN;

    int index_min_y = -1;
    for (int i = 0; i < pts_amount; i++)
    {
        if (pts[i].y < min_y)
        {
            index_min_y = i;
            min_y = pts[i].y;
        }
        max_y = ch_Max(max_y, pts[i].y);

        max_x = ch_Max(max_x, pts[i].x);
        min_x = ch_Min(min_x, pts[i].x);
    }

    if ((max_x < 0) || (max_y < 0) || (min_x >= size.width) || (min_y >= size.height))
        return;

    if (min_y == max_y)
    {
        // Special case: The polygon's height is only 1 pixel.        

        if (min_y < size.height - 1)
        {
            int fill_left = ch_Max(min_x, 0);
            int fill_right = ch_Min(max_x, size.width - 1);
            hyFillRow(p_image, min_y, fill_left, fill_right, color_buffer);
        }

        return;
    }

    max_y = ch_Min(max_y, size.height - 1);

    ConvexPolyEdgeInfo boundary_edges[2];

    boundary_edges[0].end_vertex_index = index_min_y;
    boundary_edges[1].end_vertex_index = index_min_y;
    boundary_edges[0].vertex_index_jump = 1;
    boundary_edges[1].vertex_index_jump = pts_amount - 1;
    boundary_edges[0].bottom_y = min_y;
    boundary_edges[1].bottom_y = min_y;

    int process_y = min_y;

    const int shift = 16;  // use integer to simulate float rounding
    const int rounding_pad = 1 << (shift - 1);

    int pending_edge = pts_amount;

    // records of previous valid row (y index), and its left and right boundary
    int previous_y = INT_MIN;
    int previous_left = INT_MAX;
    int previous_right = INT_MIN;

    const int right_bound = size.width - 1;

    while (process_y <= max_y) 
    {
        if (process_y < max_y)
        {
            // for each horizontal line, find its left and right boundary
            // do this only if the processed row is not the last row

            for (int i = 0; i < 2; i++)
            {
                if (process_y == boundary_edges[i].bottom_y)
                {
                    int index = boundary_edges[i].end_vertex_index;
                    int index_jump = boundary_edges[i].vertex_index_jump;

                    int x_start = 0;
                    while ((process_y >= pts[index].y) && (pending_edge > 0))
                    {
                        x_start = pts[index].x;

                        index += index_jump;
                        if (index >= pts_amount)
                            index -= pts_amount;
                        pending_edge--;
                    }

                    int x_end = pts[index].x;
                    int y_end = pts[index].y;
                    _MYASSERT(y_end > process_y);

                    boundary_edges[i].bottom_y = y_end;
                    boundary_edges[i].x_step = (((x_end - x_start) << (shift + 1)) + (y_end - process_y)) 
                                               / ((y_end - process_y) << 1);
                    boundary_edges[i].process_x = x_start << shift;
                    boundary_edges[i].end_vertex_index = index;
                }
            }
        }

        if (boundary_edges[CEI_LEFT].process_x > boundary_edges[CEI_RIGHT].process_x)
            ch_Swap(boundary_edges[CEI_LEFT], boundary_edges[CEI_RIGHT]);

        int left_x = boundary_edges[CEI_LEFT].process_x;
        int right_x = boundary_edges[CEI_RIGHT].process_x;

        if (process_y >= 0)
        {
            int current_left = (left_x + rounding_pad) >> shift;
            int current_right = (right_x + rounding_pad) >> shift;

            if ((current_right >= 0) && (current_left <= right_bound))
            {
                int correct_left = ch_Max(0, current_left);
                int correct_right = ch_Min(right_bound, current_right);

                hyFillRow(p_image, process_y, correct_left, correct_right, color_buffer);
            }

            // Check the left / right boundary of previous row (previous_y) and current row (process_y).
            // If they are not connected, we make them to be connected.
            if (previous_y >= 0 && process_y == previous_y + 1 &&
                previous_left <= previous_right && current_left <= current_right)
            {
                if (current_left >= previous_right + 2)
                {
                    int middle_left = (previous_right + current_left) / 2;
                    int middle_right = middle_left + 1;

                    if (middle_left >= 0 && middle_left <= right_bound)
                        hyFillRow(p_image, previous_y, ch_Max(previous_right + 1, 0), middle_left, color_buffer);
                    if (middle_right >= 0 && middle_right <= right_bound)
                        hyFillRow(p_image, process_y, middle_right, ch_Min(current_left - 1, right_bound), color_buffer);
                }
                else if (previous_left >= current_right + 2)
                {
                    int middle_left = (current_right + previous_left) / 2;
                    int middle_right = middle_left + 1;

                    if (middle_right >= 0 && middle_right <= right_bound)
                        hyFillRow(p_image, previous_y, middle_right, ch_Min(previous_left - 1, right_bound), color_buffer);
                    if (middle_left >= 0 && middle_left <= right_bound)
                        hyFillRow(p_image, process_y, ch_Max(current_right + 1, 0), middle_left, color_buffer);
                }
            }

            previous_y = process_y;
            previous_left = current_left;
            previous_right = current_right;
        }
        
        left_x += boundary_edges[CEI_LEFT].x_step;
        right_x += boundary_edges[CEI_RIGHT].x_step;

        boundary_edges[CEI_LEFT].process_x = left_x;
        boundary_edges[CEI_RIGHT].process_x = right_x;

        process_y++;
    }
}

void hyLine(HyImage *p_image, const HyPoint &start_point, const HyPoint &end_point, int color)
{
    if (p_image == NULL)
        return;

    if (p_image->depth != HY_DEPTH_8U)
        return;

    BYTE color_buffer[4] = {0, 0, 0, 255};

    if (p_image->nChannels >= 3)
        hyExtractColor(color, color_buffer);
    else if (p_image->nChannels == 1)
        color_buffer[0] = (BYTE) color;
    else
        _MYASSERT(false);

    const int dx = abs(start_point.x - end_point.x);
    const int dy = abs(start_point.y - end_point.y);

    const int x_step = (start_point.x < end_point.x) ? 1 : -1;
    const int y_step = (start_point.y < end_point.y) ? 1 : -1;

    int error = dx - dy;
    HyPoint current_point = start_point;

    for (;;)
    {
        hySetPixel(p_image, current_point, color_buffer);

        if (current_point.x == end_point.x && current_point.y == end_point.y)
            break;

        const int error_2x = error * 2;

        if (error_2x > -dy)
        {
            error -= dy;
            current_point.x += x_step;
        }

        if (error_2x < dx)
        {
            error += dx;
            current_point.y += y_step;
        }
    }
}

void hyThickLine(HyImage *p_image, 
                 const HyPoint &start_point, const HyPoint &end_point, 
                 const int radius, int color, 
                 bool is_cut_head /*= false*/, bool is_cut_tail /*= false*/)
{
    if (p_image == NULL)
        return;

    if (radius < 0)
        return;

    if (p_image->depth != HY_DEPTH_8U)
        return;

    int dx = start_point.x - end_point.x;
    int dy = end_point.y - start_point.y;
    int square_distance = dx * dx + dy * dy;

    if (square_distance > 0)
    {
        float ratio = radius * (1.0f / sqrtf((float)square_distance));

        HyPoint shift_pt;
        shift_pt.x = ch_Round(dy * ratio);
        shift_pt.y = ch_Round(dx * ratio);

        HyPoint polygon[4];
        polygon[0].x = start_point.x + shift_pt.x;
        polygon[0].y = start_point.y + shift_pt.y;
        polygon[1].x = start_point.x - shift_pt.x;
        polygon[1].y = start_point.y - shift_pt.y;
        polygon[2].x = end_point.x - shift_pt.x;
        polygon[2].y = end_point.y - shift_pt.y;
        polygon[3].x = end_point.x + shift_pt.x;
        polygon[3].y = end_point.y + shift_pt.y;

        hyFillConvexPoly(p_image, polygon, 4, color);

        if (radius > 0)
        {
            if (!is_cut_head)
                hyFillCircle(p_image, start_point, radius, color);
            if (!is_cut_tail)
                hyFillCircle(p_image, end_point, radius, color);
        }
    }
    else
    {
        // start_point == end_point
        // Do not draw the polygon, and only need to draw one circle.
        hyFillCircle(p_image, start_point, radius, color);
    }
}

void hyReplaceConvexPoly(HyImage *p_image, const HyPoint *pts, int pts_amount, int color, int old_color)
{
    if (p_image == NULL)
        return;

    if (pts == NULL)
        return;

    if (pts_amount < 3)
        return;

    if (p_image->depth != HY_DEPTH_8U)
        return;

    BYTE color_buffer[3];
    BYTE old_color_buffer[3];
    
    if (p_image->nChannels == 3)
    {
        hyExtractColor(color, color_buffer);
        hyExtractColor(old_color, old_color_buffer);
    }
    else if (p_image->nChannels == 1)
    {
        color_buffer[0] = (BYTE) color;
        old_color_buffer[0] = (BYTE) old_color;
    }
    else
    {
        _MYASSERT(false);
    }

    HySize size = hyGetSize(p_image);

    int min_y = INT_MAX, max_y = INT_MIN;
    int min_x = INT_MAX, max_x = INT_MIN;

    int index_min_y = -1;
    for (int i = 0; i < pts_amount; i++)
    {
        if (pts[i].y < min_y)
        {
            index_min_y = i;
            min_y = pts[i].y;
        }
        max_y = ch_Max(max_y, pts[i].y);

        max_x = ch_Max(max_x, pts[i].x);
        min_x = ch_Min(min_x, pts[i].x);
    }

    if ((max_x < 0) || (max_y < 0) || (min_x >= size.width) || (min_y >= size.height))
        return;

    if (min_y == max_y)
    {
        // Special case: The polygon's height is only 1 pixel.

        if (min_y < size.height - 1)
        {
            int fill_left = ch_Max(min_x, 0);
            int fill_right = ch_Min(max_x, size.width - 1);
            hyReplaceRow(p_image, min_y, fill_left, fill_right, color_buffer, old_color_buffer);
        }

        return;
    }

    max_y = ch_Min(max_y, size.height - 1);

    ConvexPolyEdgeInfo boundary_edges[2];

    boundary_edges[0].end_vertex_index = index_min_y;
    boundary_edges[1].end_vertex_index = index_min_y;
    boundary_edges[0].vertex_index_jump = 1;
    boundary_edges[1].vertex_index_jump = pts_amount - 1;
    boundary_edges[0].bottom_y = min_y;
    boundary_edges[1].bottom_y = min_y;

    int process_y = min_y;

    const int shift = 16;  // use integer to simulate float rounding
    const int rounding_pad = 1 << (shift - 1);

    int pending_edge = pts_amount;

    // records of previous valid row (y index), and its left and right boundary
    int previous_y = INT_MIN;
    int previous_left = INT_MAX;
    int previous_right = INT_MIN;

    const int right_bound = size.width - 1;

    while (process_y <= max_y) 
    {
        if (process_y < max_y)
        {
            // for each horizontal line, find its left and right boundary
            // do this only if the processed row is not the last row

            for (int i = 0; i < 2; i++)
            {
                if (process_y == boundary_edges[i].bottom_y)
                {
                    int index = boundary_edges[i].end_vertex_index;
                    int index_jump = boundary_edges[i].vertex_index_jump;

                    int x_start = 0;
                    while ((process_y >= pts[index].y) && (pending_edge > 0))
                    {
                        x_start = pts[index].x;

                        index += index_jump;
                        if (index >= pts_amount)
                            index -= pts_amount;
                        pending_edge--;
                    }

                    int x_end = pts[index].x;
                    int y_end = pts[index].y;
                    _MYASSERT(y_end > process_y);

                    boundary_edges[i].bottom_y = y_end;
                    boundary_edges[i].x_step = (((x_end - x_start) << (shift + 1)) + (y_end - process_y)) 
                                               / ((y_end - process_y) << 1);
                    boundary_edges[i].process_x = x_start << shift;
                    boundary_edges[i].end_vertex_index = index;
                }
            }
        }

        if (boundary_edges[CEI_LEFT].process_x > boundary_edges[CEI_RIGHT].process_x)
            ch_Swap(boundary_edges[CEI_LEFT], boundary_edges[CEI_RIGHT]);

        int left_x = boundary_edges[CEI_LEFT].process_x;
        int right_x = boundary_edges[CEI_RIGHT].process_x;

        if (process_y >= 0)
        {
            int current_left = (left_x + rounding_pad) >> shift;
            int current_right = (right_x + rounding_pad) >> shift;

            if ((current_right >= 0) && (current_left <= right_bound))
            {
                int correct_left = ch_Max(0, current_left);
                int correct_right = ch_Min(right_bound, current_right);

                hyReplaceRow(p_image, process_y, correct_left, correct_right, color_buffer, old_color_buffer);
            }

            // Check the left / right boundary of previous row (previous_y) and current row (process_y).
            // If they are not connected, we make them to be connected.
            if (previous_y >= 0 && process_y == previous_y + 1 &&
                previous_left <= previous_right && current_left <= current_right)
            {
                if (current_left >= previous_right + 2)
                {
                    int middle_left = (previous_right + current_left) / 2;
                    int middle_right = middle_left + 1;

                    if (middle_left >= 0 && middle_left <= right_bound)
                        hyReplaceRow(p_image, previous_y, ch_Max(previous_right + 1, 0), middle_left, color_buffer, old_color_buffer);
                    if (middle_right >= 0 && middle_right <= right_bound)
                        hyReplaceRow(p_image, process_y, middle_right, ch_Min(current_left - 1, right_bound), color_buffer, old_color_buffer);
                }
                else if (previous_left >= current_right + 2)
                {
                    int middle_left = (current_right + previous_left) / 2;
                    int middle_right = middle_left + 1;

                    if (middle_right >= 0 && middle_right <= right_bound)
                        hyReplaceRow(p_image, previous_y, middle_right, ch_Min(previous_left - 1, right_bound), color_buffer, old_color_buffer);
                    if (middle_left >= 0 && middle_left <= right_bound)
                        hyReplaceRow(p_image, process_y, ch_Max(current_right + 1, 0), middle_left, color_buffer, old_color_buffer);
                }
            }

            previous_y = process_y;
            previous_left = current_left;
            previous_right = current_right;
        }
        
        left_x += boundary_edges[CEI_LEFT].x_step;
        right_x += boundary_edges[CEI_RIGHT].x_step;

        boundary_edges[CEI_LEFT].process_x = left_x;
        boundary_edges[CEI_RIGHT].process_x = right_x;

        process_y++;
    }
}

void hyReplaceThickLine(HyImage *p_image, const HyPoint &start_point, const HyPoint &end_point, const int radius, int color, int old_color)
{
    if (p_image == NULL)
        return;

    if (radius < 0)
        return;

    if (p_image->depth != HY_DEPTH_8U)
        return;

    int dx = start_point.x - end_point.x;
    int dy = end_point.y - start_point.y;
    int square_distance = dx * dx + dy * dy;

    if (square_distance > 0)
    {
        float ratio = radius * (1.0f / sqrtf((float)square_distance));

        HyPoint shift_pt;
        shift_pt.x = ch_Round(dy * ratio);
        shift_pt.y = ch_Round(dx * ratio);

        HyPoint polygon[4];
        polygon[0].x = start_point.x + shift_pt.x;
        polygon[0].y = start_point.y + shift_pt.y;
        polygon[1].x = start_point.x - shift_pt.x;
        polygon[1].y = start_point.y - shift_pt.y;
        polygon[2].x = end_point.x - shift_pt.x;
        polygon[2].y = end_point.y - shift_pt.y;
        polygon[3].x = end_point.x + shift_pt.x;
        polygon[3].y = end_point.y + shift_pt.y;

        hyReplaceConvexPoly(p_image, polygon, 4, color, old_color);

        if (radius > 0)
        {
            hyReplaceCircle(p_image, start_point, radius, color, old_color);
            hyReplaceCircle(p_image, end_point, radius, color, old_color);
        }
    }
    else
    {
        // start_point == end_point
        // Do not draw the polygon, and only need to draw one circle.
        hyReplaceCircle(p_image, start_point, radius, color, old_color);
    }
}

void hyFillRectangle(HyImage *p_image, const HyRect &rect, int color)
{
    if (p_image == NULL)
        return;

    if (rect.width <= 0 || rect.height <= 0)
        return;

    _MYASSERT(p_image->nChannels == 4 || p_image->nChannels == 3 || p_image->nChannels == 1);

    int left = rect.x;
    int top = rect.y;
    int right = left + rect.width;
    int bottom = top + rect.height;

    left = ch_Max(left, 0);
    top = ch_Max(top, 0);
    right = ch_Min(right, p_image->width);
    bottom = ch_Min(bottom, p_image->height);

    int image_step = p_image->widthStep;
    IppiSize roi_size = ippiSize(right - left, bottom - top);

    if (p_image->nChannels == 4)
    {
        BYTE color_buffer[3];
        hyExtractColor(color, color_buffer);

        ippiSet_8u_AC4R(color_buffer, p_image->imageData + top * image_step + left * 4, image_step, roi_size);
    }
    else if (p_image->nChannels == 3)
    {
        BYTE color_buffer[3];
        hyExtractColor(color, color_buffer);

        ippiSet_8u_C3R(color_buffer, p_image->imageData + top * image_step + left * 3, image_step, roi_size);
    }
    else if (p_image->nChannels == 1)
    {
        ippiSet_8u_C1R((BYTE)color, p_image->imageData + top * image_step + left, image_step, roi_size);
    }
}

void hyRectangle(HyImage *p_image, const HyRect &rect, int color, int thickness)
{
    // Draw a rectangle by filling it or drawing the four boundary lines.
    // NOTE: This function does not need to check the image size.
    //       It will call hyFillRectangle() and do boundary checking inside it.

    if (p_image == NULL)
        return;

    if (rect.width <= 0 || rect.height <= 0)
        return;

    _MYASSERT(p_image->nChannels == 4 || p_image->nChannels == 3 || p_image->nChannels == 1);

    if (thickness <= 0) // HY_FILLED, or any negative value
    {
        // Call hyFillRectangle once to fill the whole region.

        hyFillRectangle(p_image, rect, color);
    }
    else
    {
        // Compute 4 rectangles as the boundary of given rectangle.

        int left = rect.x;
        int top = rect.y;
        int width = rect.width;
        int height = rect.height;
        int right = left + width;
        int bottom = top + height;

        HyRect fill_rect;

        // left boundary
        fill_rect = hyRect(left, top, thickness, height);
        hyFillRectangle(p_image, fill_rect, color);

        // top boundary
        fill_rect = hyRect(left, top, width, thickness);
        hyFillRectangle(p_image, fill_rect, color);

        // right boundary
        fill_rect = hyRect(right - thickness, top, thickness, height);
        hyFillRectangle(p_image, fill_rect, color);

        // bottom boundary
        fill_rect = hyRect(left, bottom - thickness, width, thickness);
        hyFillRectangle(p_image, fill_rect, color);
    }
}

void hyPutText(HyImage *p_image, LPCTSTR text_, HyPoint position, int color /*= HY_RGB(0, 0, 255)*/, int size /*= 32*/)
{
#if !defined(_WINRT_METRO) && !defined(UNIX_OS)

    if (p_image == NULL)
        return;

    if (p_image->depth != HY_DEPTH_8U)
        return;

    if ((p_image->nChannels != 3) && (p_image->nChannels != 4))
        return;

    if ((position.x < 0) 
        || (position.y < 0) 
        || (position.x >= p_image->width) 
        || (position.y >= p_image->height))
    {
        return;
    }

    using namespace Gdiplus;

    FontFamily  font_family(L"Courier New");
    Gdiplus::Font        font(&font_family, (float)size, FontStyleBold, UnitPixel);

    SolidBrush *p_brush = NULL;
    Bitmap *p_draw_image = NULL;

    int channel = p_image->nChannels;
    if (channel == 3)
    {
        p_brush = new SolidBrush(Color(HY_GetRValue(color), HY_GetGValue(color), HY_GetBValue(color)));
        p_draw_image = new Bitmap(p_image->width, p_image->height, p_image->widthStep,
                                  PixelFormat24bppRGB,
                                  p_image->imageData);
    }
    else if (channel == 4)
    {
        p_brush = new SolidBrush(Color(HY_GetRValue(color), HY_GetGValue(color), HY_GetBValue(color)));
        p_draw_image = new Bitmap(p_image->width, p_image->height, p_image->widthStep,
                                  PixelFormat32bppRGB,
                                  p_image->imageData);
    }

    const wchar_t *text;
#ifdef _UNICODE
    text = text_;
#else
    std::wstring text_string = utf8_to_unicode(text_);
    text = text_string.c_str();
#endif

    if ((p_brush != NULL) && (p_draw_image != NULL))
    {
        Graphics g(p_draw_image);
        g.DrawString(text, -1, &font, PointF((float)position.x, (float)position.y), p_brush);
    }

    if (p_brush)
        delete p_brush;
    if (p_draw_image)
        delete p_draw_image;

#endif
}

void hyPutText(HyImage *p_image, HyPoint position, int color, int size, const TCHAR *format, ...)
{
    TCHAR buffer[256];
    va_list marker;
    va_start(marker, format);
    _vstprintf(buffer, format, marker);
    hyPutText(p_image, buffer, position, color, size);
    va_end(marker);	
}

void hyPutText(HyImage *p_image, HyPoint position, const TCHAR *format, ...)
{
    TCHAR buffer[256];
    va_list marker;
    va_start(marker, format);
    _vstprintf(buffer, format, marker);
    hyPutText(p_image, buffer, position);
    va_end(marker);	
}

HyROIInfo hyGetROIInfo(const HyImage *p_image)
{
    return hyGetROIInfo(p_image, hyGetImageROI(p_image));
}

HyROIInfo hyGetROIInfo(const HyImage *p_image, const HyRect &roi)
{
    _MYASSERT(hyIsImageValid(p_image));
    _MYASSERT(p_image);
    _MYASSERT(p_image->depth == HY_DEPTH_8U);

    return HyROIInfo(p_image, roi);
}

#if IS_ANDROID || defined(LINUX_SERVER)
struct my_error_mgr {
  jpeg_error_mgr pub;    /* "public" fields */
  jmp_buf setjmp_buffer;        /* for return to caller */
};
typedef my_error_mgr * my_error_ptr;

void my_error_exit (j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  (*cinfo->err->output_message) (cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}

HyImage *hyLoadImageByLIBJpeg(const char *filename)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    FILE * infile;                /* source file */
    JSAMPARRAY buffer;            /* Output row buffer */
    int row_stride;               /* physical row width in output buffer */

    if ((infile = fopen(filename, "rb")) == NULL) {
        _MYASSERT(0);
        return NULL;
    }

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        _MYASSERT(0);
        return NULL;
    }
    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
    HyImage *p_ret_image = NULL;

    if ((cinfo.output_components != 1 && cinfo.output_components != 3) ||
        (cinfo.out_color_space != JCS_RGB && cinfo.out_color_space != JCS_GRAYSCALE))
    {
        _MYASSERT(0);
    }
    else
    {
        HyImage *p_loaded_image = 
            hyCreateImage(HySize(cinfo.output_width, cinfo.output_height), 
                          HY_DEPTH_8U, 
                          cinfo.output_components);
        row_stride = cinfo.output_width * cinfo.output_components;
        buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
        BYTE *p_curr_data = p_loaded_image->imageData;
        while (cinfo.output_scanline < cinfo.output_height) {
            jpeg_read_scanlines(&cinfo, buffer, 1);
            memcpy(p_curr_data, buffer[0], p_loaded_image->width * p_loaded_image->nChannels);
            p_curr_data += p_loaded_image->widthStep;
        }

        p_ret_image = hyCreateImage(hyGetSize(p_loaded_image), HY_DEPTH_8U, 3);
        int order[3] = {2, 1, 0};
        switch (cinfo.output_components)
        {
        case 1:
            ippiGrayToBGR(p_loaded_image, p_ret_image);
            break;

        case 3:
            ippiCopy(p_loaded_image, p_ret_image);
            ippiSwapChannels_8u_C3IR(p_ret_image->imageData,
                                     p_ret_image->widthStep,
                                     ippiSize(hyGetSize(p_ret_image)),
                                     order);
            break;

        default:
            _MYASSERT(0);
        }
        hyReleaseImage(&p_loaded_image);
    }


    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(infile);

    return p_ret_image;
}

HyImage *hyLoadImageByLIBPng(const char *filename, const bool is_premultiplied)
{
    unsigned char header[8];    // 8 is the maximum size that can be checked

    /* open file and test for it being a png */
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        _DPRINTF((_T("[read_png_file] File %s could not be opened for reading"), filename));
        return NULL;
    }
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8))
    {
        _DPRINTF((_T("[read_png_file] File %s is not recognized as a PNG file"), filename));
        return NULL;
    }

    /* initialize stuff */
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
    {
        _DPRINTF((_T("[read_png_file] png_create_read_struct failed")));
        fclose(fp);
        return NULL;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        _DPRINTF((_T("[read_png_file] png_create_info_struct failed")));
        fclose(fp);
        return NULL;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        _DPRINTF((_T("[read_png_file] Error during init_io")));
        fclose(fp);
        return NULL;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    /* Check if HyImage support the png content */
    if (color_type != PNG_COLOR_TYPE_GRAY &&
        (color_type & PNG_COLOR_TYPE_RGB) == 0 &&
        (color_type & PNG_COLOR_MASK_ALPHA) == 0)
    {
        _DPRINTF((_T("[read_png_file] HyImage doesn't support the color type")));
        fclose(fp);
        return NULL;
    }

    if (bit_depth != HY_DEPTH_8U)
    {
        _DPRINTF((_T("[read_png_file] HyImage doesn't support the bit depth")));
        fclose(fp);
        return NULL;
    }

    // if it contains RGB channel, sort it to BGR order.
    if ((color_type & PNG_COLOR_TYPE_RGB) != 0)
        png_set_bgr(png_ptr);

    // Premultiplied
    if (color_type == PNG_COLOR_TYPE_RGBA && is_premultiplied)
    {
        double gamma = 1.0;
        png_get_gAMA(png_ptr, info_ptr, &gamma);
        png_set_alpha_mode(png_ptr, PNG_ALPHA_PREMULTIPLIED, gamma);
    }

    int number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    int channels = png_get_channels(png_ptr, info_ptr);

    /* read file */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        _DPRINTF((_T("[read_png_file] Error during read_image")));
        fclose(fp);
        return NULL;
    }

    png_bytep *p_row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    for (int y = 0; y < height; y++)
        p_row_pointers[y] = (png_byte*) malloc(row_bytes);

    png_read_image(png_ptr, p_row_pointers);

    fclose(fp);

    HyImage *p_ret_image = hyCreateImage(HySize(width, height), bit_depth, channels);
    BYTE *p_ret_image_row = p_ret_image->imageData;

    for (int y = 0; y < height; y++)
    {
        memcpy(p_ret_image_row, p_row_pointers[y], row_bytes);
        p_ret_image_row += p_ret_image->widthStep;
    }

    for (int y = 0; y < height; y++)
        free(p_row_pointers[y]);
    free(p_row_pointers);

    return p_ret_image;
}
#endif

HyImage *hyLoadImageFromRawData(LPCTSTR path)
{
    FILE *p_file = _tfopen(path, _T("rb"));
    HyImage *p_ret_image = NULL;

    if (p_file)
    {
        int image_info[4];
        fread(image_info, sizeof(int), 4, p_file);

        int width = image_info[0];
        int height = image_info[1];
        int channel = image_info[2];
        int depth_byte = image_info[3];

        // Only support positive width & height, channel 1,3,4,
        // and 1-Byte depth image currently
        if (width > 0 && height > 0 && 
            (channel == 1 || channel == 3 || channel == 4) &&
            depth_byte == 1)
        {
            p_ret_image = hyCreateImage(HySize(width, height), HY_DEPTH_8U, channel);
            for (int y = 0; y < height; y++)
            {
                int read_count = fread(p_ret_image->imageData + y * p_ret_image->widthStep,
                                       sizeof(BYTE), width * channel, p_file);
                if (read_count < width * channel)
                {
                    hyReleaseImage(&p_ret_image);
                    break;
                }
            }
        }

        fclose(p_file);
    }

    return p_ret_image;
}

bool hySaveImageToRawData(LPCTSTR path, HyImage *p_image)
{
    if (!p_image) return false;

    FILE *p_file = _tfopen(path, _T("wb"));
    if (!p_file) return false;

    int image_info[4] = {p_image->width, p_image->height, p_image->nChannels, 1};
    fwrite(image_info, sizeof(int), 4, p_file);
    for (int y = 0; y < p_image->height; y++)
    {
        fwrite(p_image->imageData + y * p_image->widthStep,
               sizeof(BYTE), p_image->width * p_image->nChannels, p_file);
    }

    fclose(p_file);

    return true;
}

bool hyLoadImageSizeFromRawData(LPCTSTR path, HySize &size)
{
    bool is_success = false;

    FILE *p_file = _tfopen(path, _T("rb"));
    if (p_file)
    {
        int image_info[2];
        size_t read_size = fread(image_info, sizeof(int), 2, p_file);

        if (read_size >= 2)
        {
            size.width = image_info[0];
            size.height = image_info[1];
            is_success = true;
        }

        fclose(p_file);
    }

    return is_success;
}

#ifdef MAC_OSX
void LoadCGImage(const char *fname, CGImageRef& imgRef)
{
    imgRef = NULL;

    CGDataProviderRef   pdrSrc = NULL;
    CGImageSourceRef    imgSrc = NULL;

    pdrSrc = CGDataProviderCreateWithFilename(fname);
    imgSrc = CGImageSourceCreateWithDataProvider(pdrSrc, NULL);
    imgRef = CGImageSourceCreateImageAtIndex(imgSrc, 0, NULL);

    if(imgRef)
    {
        CFRelease(imgSrc);
        CFRelease(pdrSrc);
    }
}

HyImage *hyLoadImageByCGImage(LPCTSTR szFilePath, const int iscolor)
{
    CGImageRef p_cg_image;
    LoadCGImage(szFilePath, p_cg_image); 

    CGColorSpaceRef p_colorspace = CGColorSpaceCreateDeviceRGB();
    HyImage *p_hy_image_4channel = hyCreateImage(HySize(CGImageGetWidth(p_cg_image), 
                                                      CGImageGetHeight(p_cg_image)), 
                                                 HY_DEPTH_8U, 
                                                 4);
    CGContextRef p_context = CGBitmapContextCreate(p_hy_image_4channel->imageData, 
                                                   p_hy_image_4channel->width, 
                                                   p_hy_image_4channel->height, 
                                                   p_hy_image_4channel->depth, 
                                                   p_hy_image_4channel->widthStep, 
                                                   p_colorspace, 
                                                   kCGImageAlphaPremultipliedLast|kCGBitmapByteOrderDefault);
    CGContextDrawImage(p_context, 
                       CGRectMake(0, 0, CGImageGetWidth(p_cg_image), CGImageGetHeight(p_cg_image)), 
                       p_cg_image);
    CGContextRelease(p_context);
    CGColorSpaceRelease(p_colorspace);
    CGImageRelease(p_cg_image);

    int swap_channel_order[] = {2, 1, 0};
    HyImage *p_ret_image = hyCreateImage(hyGetSize(p_hy_image_4channel), HY_DEPTH_8U, 3);
    ippiSwapChannels_8u_C4C3R(p_hy_image_4channel->imageData,
                              p_hy_image_4channel->widthStep,
                              p_ret_image->imageData,
                              p_ret_image->widthStep,
                              ippiSize(hyGetSize(p_hy_image_4channel)),
                              swap_channel_order);
    hyReleaseImage(&p_hy_image_4channel);

    if (iscolor == 0)
    {
        HyImage *p_bgr_image = p_ret_image;
        p_ret_image = hyCreateImage(hyGetSize(p_bgr_image), HY_DEPTH_8U, 1);
        ippiBGRToGray(p_bgr_image, p_ret_image);
        hyReleaseImage(&p_bgr_image);
    }

    return p_ret_image;
}
#endif

HyImage *hyCreateHorizontalConcatenateImage(const HyImage *p_left, const HyImage *p_right)
{
    if (NULL == p_left)
        return NULL;

    if (NULL == p_right)
        return NULL;

    if (!hyIsImageAndROIValid(p_left) &&
        !hyIsImageAndROIValid(p_right) && 
        p_left->nChannels != p_right->nChannels)
    {
        return NULL;
    }

    int width = p_left->roi.width + p_right->roi.width;
    int height = ch_Max(p_left->roi.height, p_right->roi.height);
    HyImagePtr result(hySize(width, height), HY_DEPTH_8U, p_left->nChannels);
    HY_ZEROIMAGE(result.Content());
    hySetImageROI(result.Content(), p_left->roi);
    ippiCopy(p_left, result.Content());
    hySetImageROI(result.Content(), hyRect(hyPoint(p_left->roi.width, 0), hySize(p_right->roi)));
    ippiCopy(p_right, result.Content());
    hyResetImageROI(result.Content());

    return result.Transit();
}

HyImage *hyCreateVerticalConcatenateImage(const HyImage *p_top, const HyImage *p_bottom)
{
    if (NULL == p_top)
        return NULL;

    if (NULL == p_bottom)
        return NULL;

    if (!hyIsImageAndROIValid(p_top) ||
        !hyIsImageAndROIValid(p_bottom) ||
        p_top->nChannels != p_bottom->nChannels)
    {
        return NULL;
    }

    int width = ch_Max(p_top->roi.width, p_bottom->roi.width);
    int height = p_top->roi.height + p_top->roi.height;
    HyImagePtr result(hySize(width, height), HY_DEPTH_8U, p_top->nChannels);
    HY_ZEROIMAGE(result.Content());
    hySetImageROI(result.Content(), p_top->roi);
    ippiCopy(p_top, result.Content());
    hySetImageROI(result.Content(), hyRect(hyPoint(0, p_top->roi.height), hySize(p_bottom->roi)));
    ippiCopy(p_bottom, result.Content());
    hyResetImageROI(result.Content());

    return result.Transit();
}

void hyFillValueToChannel(HyImage *p_image, BYTE value, int channel)
{
    _MYASSERT(p_image && p_image->nChannels > channel);
    
    BYTE *p_pixels = hyStartPixels<BYTE>(p_image);
    BYTE *p_row = p_pixels;
    for (int y = 0; y < p_image->roi.height; y++, p_row += p_image->widthStep)
    {
        BYTE *p_curr = p_row;
        for (int x = 0; x < p_image->roi.width; x++, p_curr += p_image->nChannels)
        {
            p_curr[channel] = value;
        }
    }
}
