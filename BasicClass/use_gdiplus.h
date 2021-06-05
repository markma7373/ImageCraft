
#pragma once

#ifndef min
#define min(x,y) (((x)<(y))?(x):(y))
#endif
#ifndef max
#define max(x,y) (((x)>(y))?(x):(y))
#endif

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#include "use_hylib.h"
#include "use_ipp.h"

inline HyImage *hyLoadImageByGdiplus(LPCTSTR szFilePath, const int iscolor = 1)
{
    using namespace Gdiplus;
    Bitmap img(szFilePath);
    if(img.GetLastStatus() != Ok)
        return NULL;
    Rect r(0, 0, img.GetWidth(), img.GetHeight());
    BitmapData bmData;
    HyImage *src_img = NULL;
    if(img.LockBits(&r, ImageLockModeRead, PixelFormat24bppRGB, &bmData) == Ok) 
    {
        src_img = hyCreateImage(hySize(img.GetWidth(), img.GetHeight()), HY_DEPTH_8U, 3);
        ippiCopy_8u_C3R((BYTE*)bmData.Scan0, bmData.Stride,
                        src_img->imageData, src_img->widthStep,
                        ippiSize(img.GetWidth(), img.GetHeight()));

        img.UnlockBits(&bmData);
        if(!iscolor) {
            HyImage *gray = hyCreateImage(hyGetSize(src_img), HY_DEPTH_8U, 1);
            ippiBGRToGray(src_img, gray);
            hyReleaseImage(&src_img);
            src_img = gray;
        }
    }
    
    return src_img;
}

inline HyImage *hyLoad4ChannelImageByGdiplus(LPCTSTR szFilePath, const int iscolor = 1)
{
    using namespace Gdiplus;
    Bitmap img(szFilePath);
    if(img.GetLastStatus() != Ok)
        return NULL;
    Rect r(0, 0, img.GetWidth(), img.GetHeight());
    BitmapData bmData;
    HyImage *src_img = NULL;
    if(img.LockBits(&r, ImageLockModeRead, PixelFormat32bppARGB, &bmData) == Ok) 
    {
        src_img = hyCreateImage(hySize(img.GetWidth(), img.GetHeight()), HY_DEPTH_8U, 4);
        ippiCopy_8u_C4R((BYTE*)bmData.Scan0, bmData.Stride,
                        src_img->imageData, src_img->widthStep,
                        ippiSize(img.GetWidth(), img.GetHeight()));

        img.UnlockBits(&bmData);
        if(!iscolor) {
            HyImage *gray = hyCreateImage(hyGetSize(src_img), HY_DEPTH_8U, 1);
            ippiBGRAToGray(src_img, gray);
            hyReleaseImage(&src_img);
            src_img = gray;
        }
    }
    return src_img;
}

inline LONGLONG ch_GetExifPhotoDate(Gdiplus::Image *img)
{
    if (img == NULL)
        return 0;

    using namespace Gdiplus;
    int size = img->GetPropertyItemSize(PropertyTagExifDTDigitized);
    PropertyItem *propertyItem = (PropertyItem *) malloc(size);

    if (propertyItem == NULL)
        return 0;

    LONGLONG tmDate = -1;
    if(img->GetPropertyItem(PropertyTagExifDTDigitized, size, propertyItem) == Ok) {
        struct tm ph_tm;
        int rdcnt = sscanf((const char *)propertyItem->value, "%04d:%02d:%02d %02d:%02d:%02d", 
            &(ph_tm.tm_year), &(ph_tm.tm_mon), &(ph_tm.tm_mday), 
            &(ph_tm.tm_hour), &(ph_tm.tm_min), &(ph_tm.tm_sec));
        if(rdcnt != 6 || (ph_tm.tm_year == 0 && ph_tm.tm_mon == 0 && ph_tm.tm_mday == 0) || (ph_tm.tm_year < 1980 || ph_tm.tm_year > 2100))
            tmDate = -1;
        else {
            ph_tm.tm_year -= 1900;
            ph_tm.tm_mon--;
            tmDate = (LONGLONG) mktime(&ph_tm);
        }
    }
    free(propertyItem);

    return tmDate;
}

/////////////////////////////////////////////////////////////////////////////////
//   1        2       3      4         5            6           7          8
//
// 888888  888888      88  88      8888888888  88                  88  8888888888
// 88          88      88  88      88  88      88  88          88  88      88  88
// 8888      8888    8888  8888    88          8888888888  8888888888          88
// 88          88      88  88
// 88          88  888888  888888
/////////////////////////////////////////////////////////////////////////////////

#define _EXIF_ORIEN_ROTATE_NORMAL   1
#define _EXIF_ORIEN_ROTATE_LEFT     8
#define _EXIF_ORIEN_ROTATE_RIGHT    6

inline int ch_GetExifPhotoOrientation(Gdiplus::Image *img)
{
    if (img == NULL)
        return _EXIF_ORIEN_ROTATE_NORMAL;
    
    using namespace Gdiplus;
    int size = img->GetPropertyItemSize(PropertyTagOrientation);
    PropertyItem *propertyItem = (PropertyItem *) malloc(size);

    if (propertyItem == NULL)
        return _EXIF_ORIEN_ROTATE_NORMAL;

    int orientation = 1;
    if(img->GetPropertyItem(PropertyTagOrientation, size, propertyItem) == Ok) {
        unsigned short *p = (unsigned short *) propertyItem->value;
        orientation = p[0];
        if(img->GetWidth() < img->GetHeight())
            orientation = 1;
    }
    free(propertyItem);
    
    return orientation;
}

inline int ch_GetExifPhotoOrientationByFileName(LPCTSTR szFileName)
{
    Gdiplus::Image img(szFileName, FALSE);
    return ch_GetExifPhotoOrientation(&img);
}
