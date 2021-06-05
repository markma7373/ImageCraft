#pragma once
#include "Common.h"
#include <math.h>
#include <map>

#include "use_half.h"

///////////////////////////////////////////////////
// Const

#define HY_DEPTH_SIGN           0x80000000
#define HY_WIDTHSTEP_ALIGN_NUM  16

#define HY_DEPTH_8U  8
#define HY_DEPTH_16S  16
#define HY_DEPTH_32F 32

///////////////////////////////////////////////////
// Macros

#define HY_ALIGN(x, y)  (((x) + (y) - 1) & ~((y) - 1))

#define HY_PI 3.1415926535897932384626433832795028841971
#define HY_F_PI 3.14159265f

#define HY_RGB(r, g, b)  (int)((BYTE)(b) + ((BYTE)(g) << 8) + ((BYTE)(r) << 16))

#define HY_GetBValue(rgb)      (BYTE)((int)(rgb))
#define HY_GetGValue(rgb)      (BYTE)((int)(rgb) >> 8)
#define HY_GetRValue(rgb)      (BYTE)((int)(rgb) >> 16)

#define HY_COLOR_BLACK      HY_RGB(0x00, 0x00, 0x00)
#define HY_COLOR_WHITE      HY_RGB(0xff, 0xff, 0xff)
#define HY_COLOR_GRAY       HY_RGB(0xc0, 0xc0, 0xc0)
#define HY_COLOR_RED        HY_RGB(0xff, 0x00, 0x00)
#define HY_COLOR_GREEN      HY_RGB(0x00, 0xff, 0x00)
#define HY_COLOR_BLUE       HY_RGB(0x00, 0x00, 0xff)
#define HY_COLOR_CYAN       HY_RGB(0x00, 0xff, 0xff)
#define HY_COLOR_YELLOW     HY_RGB(0xff, 0xff, 0x00)
#define HY_COLOR_PINK       HY_RGB(0xff, 0x00, 0xff)
#define HY_COLOR_ORANGE     HY_RGB(0xff, 0xa5, 0x00)
#define HY_COLOR_OLIVE      HY_RGB(0x80, 0x80, 0x00)
#define HY_COLOR_PURPLE     HY_RGB(0x80, 0x00, 0x80)

#define HY_FILLED   0

#define HY_ZEROIMAGE(p_image)                                                \
if ((p_image) && ((p_image)->imageData))                                     \
{                                                                            \
    memset((p_image)->imageData, 0, (p_image->widthStep * p_image->height)); \
}

#define HY_SETIMAGE(p_image, value)                                          \
if ((p_image) && ((p_image)->imageData))                                         \
{                                                                                \
    memset((p_image)->imageData, value, (p_image->widthStep * p_image->height)); \
}

enum HyStatus
{
    HY_OK = 0,
    HY_FAIL
};

struct HyPoint
{
    int x;
    int y;

    HyPoint()
    {
        x = y = 0;
    }
    HyPoint(const int _x, const int _y)
    {
        x = _x;
        y = _y;
    }

    bool operator==(const HyPoint &point) const
    {
        return (x == point.x) && (y == point.y);
    }
    bool operator!=(const HyPoint &point) const
    {
        return !(*this == point);
    }

    inline HyPoint operator* (const ImageScale &scale) const
    {
        HyPoint p;
        p.x = ch_Round((float)x * scale.x);
        p.y = ch_Round((float)y * scale.y);

        return p;
    }
    int norm1() const
    {
        return abs(x) + abs(y);
    }
    friend HyPoint operator-(const HyPoint &point);
};

inline HyPoint operator-(const HyPoint &point) 
{
    return HyPoint(-point.x, -point.y);
}

inline HyPoint hyPoint(const int x, const int y)
{
    return HyPoint(x, y);
}

struct HyPoint2D16h
{
    half x;
    half y;

    HyPoint2D16h()
    {
    }

    HyPoint2D16h(const half _x, const half _y)
    {
        x = _x;
        y = _y;
    }

    bool operator==(const HyPoint2D16h &point) const
    {
        return (x == point.x) && (y == point.y);
    }
    bool operator!=(const HyPoint2D16h &point) const
    {
        return !(*this == point);
    }
};

inline HyPoint2D16h hyPoint2D16h(const half x, const half y)
{
    return HyPoint2D16h(x, y);
}

struct HyPoint2D32f
{
    float x;
    float y;

    HyPoint2D32f()
    {
        x = y = 0;
    }
    HyPoint2D32f(const float _x, const float _y)
    {
        x = _x;
        y = _y;
    }
    HyPoint2D32f(const HyPoint &point)
    {
        x = (float)point.x;
        y = (float)point.y;
    }
    HyPoint2D32f(const HyPoint2D16h &point_16h)
    {
        x = (float)point_16h.x;
        y = (float)point_16h.y;
    }

    bool operator==(const HyPoint2D32f &point) const
    {
        return (x == point.x) && (y == point.y);
    }
    bool operator!=(const HyPoint2D32f &point) const
    {
        return !(*this == point);
    }

    operator HyPoint2D16h() const
    {
        return HyPoint2D16h((half)x, (half)y);
    }

};

inline HyPoint2D32f hyPoint2D32f(const float x, const float y)
{
    return HyPoint2D32f(x, y);
}

struct HyPoint2D64d
{
    double x;
    double y;

    HyPoint2D64d()
    {
        x = y = 0.0;
    }
    HyPoint2D64d(const double _x, const double _y)
    {
        x = _x;
        y = _y;
    }
};

inline HyPoint2D64d hyPoint2D64d(const double x, const double y)
{
    return HyPoint2D64d(x, y);
}

struct HySize
{
    int width;
    int height;

    HySize()
    {
        width = 0;
        height = 0;
    }
    HySize(const int _width, const int _height)
    {
        width = _width;
        height = _height;
    }

    bool operator==(const HySize &dst_size) const
    {
        return ((width == dst_size.width) && (height == dst_size.height));
    }

    bool operator!=(const HySize &dst_size) const
    {
        return ((width != dst_size.width) || (height != dst_size.height));
    }
};

inline HySize hySize(const int width, const int height)
{
    return HySize(width, height);
}

struct HyRect
{
    int x;
    int y;
    int width;
    int height;

    HyRect()
    {
        x = 0;
        y = 0;
        width = 0;
        height = 0;
    }
    HyRect(const int _x, const int _y, const int _width, const int _height)
    {
        x = _x;
        y = _y;
        width = _width;
        height = _height;
    }
    HyRect(const HyRect &rect)
    {
        x = rect.x;
        y = rect.y;
        width = rect.width;
        height = rect.height;
    }

    int Left() const {return x;}
    int Top()  const {return y;}
    int Right()  const {return x + width;}
    int Bottom() const {return y + height;}

    bool IsPtInRect(const HyPoint &point) const
    {
        return ((point.x >= Left()) && (point.x < Right())
             && (point.y >= Top()) && (point.y < Bottom()));
    }
    
    bool IsRectInRect(const HyRect &rect) const
    {
        return ((rect.Left() >= Left()) && (rect.Top() >= Top())
             && (rect.Right() <= Right()) && (rect.Bottom() <= Bottom()));
    }

    void OffsetRect(const HyPoint &point)
    {
        x += point.x;
        y += point.y;
    }

    bool operator==(const HyRect &r) const
    {
        return ((x == r.x) && (y == r.y) && (width == r.width) && (height == r.height));
    }

    bool operator!=(const HyRect &r) const
    {
        return !(*this == r);
    }

    inline HyRect operator+ (const HyPoint &point) const
    {
        HyRect r;

        r.x      = x + point.x;
        r.y      = y + point.y;
        r.width  = width;
        r.height = height;

        return r;
    }

    inline HyRect operator- (const HyPoint &point) const
    {
        HyRect r;

        r.x      = x - point.x;
        r.y      = y - point.y;
        r.width  = width;
        r.height = height;

        return r;
    }

    inline HyRect& operator+= (const HyPoint &point)
    {
        x += point.x;
        y += point.y;
        return *this;
    }

    inline HyRect& operator-= (const HyPoint &point)
    {
        x -= point.x;
        y -= point.y;
        return *this;
    }
};

inline HyRect hyRect(const int x, const int y, const int width, const int height)
{
    return HyRect(x, y, width, height);
}

inline HyRect hyRect(const HyRect &rect)
{
    return HyRect(rect);
}

inline HyRect hyRect(const HyPoint &point, const HySize &size) 
{ 
    return HyRect(point.x, point.y, size.width, size.height); 
}

inline HyRect hyRect(const HyPoint &point_lt, 
                     const HyPoint &point_rb, 
                     bool is_point_rb_inside_rect = false) 
{ 
    return hyRect(point_lt, 
                  HySize(point_rb.x - point_lt.x + (int)is_point_rb_inside_rect, 
                         point_rb.y - point_lt.y + (int)is_point_rb_inside_rect)); 
}

inline HyRect hyRect(const HySize &size) { return hyRect(HyPoint(0, 0), size); }

struct HyImage
{
    int width;
    int height;
    int depth;
    int nChannels;
    int widthStep;

    HyRect roi;

    BYTE *imageData;

    bool is_attached_buffer;

    HyImage()
    {
        width = height = nChannels = depth = widthStep = 0;
        imageData = NULL;

        is_attached_buffer = false;
    }
    HyImage(const HySize &_size, const int _depth, const int _channel)
    {
        width = _size.width;
        height = _size.height;
        depth = _depth;
        nChannels = _channel;
        widthStep = HY_ALIGN((width * nChannels * (depth & ~HY_DEPTH_SIGN) + 7)/8, HY_WIDTHSTEP_ALIGN_NUM);

        imageData = NULL;
        ResetROI();

        is_attached_buffer = false;
    }

    void ResetROI()
    {
        roi.x = 0;
        roi.y = 0;
        roi.width = width;
        roi.height = height;
    }

    //////////////////////////////////////////////////////////////////////////
    // Template member functions for non-BYTE return type
    template <typename U>
    U& pixel(int x, int y, int c = 0)
    {
        _MYASSERT(imageData);
        return ((U*)(imageData + widthStep * y))[x * nChannels + c];
    }

    template <typename U>
    const U& pixel(int x, int y, int c = 0) const
    {
        _MYASSERT(imageData);
        return ((U*)(imageData + widthStep * y))[x * nChannels + c];
    }

    template <typename U, typename T>
    U &pixel(const T &point, int c = 0)
    {
        return pixel<U>(point.x, point.y, c);
    }

    template <typename U, typename T>
    const U &pixel(const T &point, int c = 0) const
    {
        return pixel<U>(point.x, point.y, c);
    }

    template <typename U>
    U *get_pixels(int x, int y)
    {
        return &pixel<U>(x, y);
    }

    template <typename U>
    const U *get_pixels(int x, int y) const
    {
        return &pixel<U>(x, y);
    }

    template <typename U, typename T>
    U *get_pixels(const T &point)
    {
        return &pixel<U>(point.x, point.y);
    }

    template <typename U, typename T>
    void get_pixels(const T &point, U *p_pixel)
    {
        for (int c = 0; c < nChannels; c++)
        {
            p_pixel[c] = pixel<U>(point, c);
        }
    }

    template <typename U, typename T>
    const U *get_pixels(const T &point) const 
    {
        return &pixel<U>(point.x, point.y);
    }

    template <typename U, typename T>
    void set_pixels(const T &point, const U *p_pixel)
    {
        for (int c = 0; c < nChannels; c++)
        {
            pixel<U>(point, c) = p_pixel[c];
        }
    }

    template <typename U>
    void set_pixels(const int x, const int y, const U *p_pixel)
    {
        _MYASSERT(p_pixel);
        for (int c = 0; c < nChannels; c++)
        {
            pixel<U>(x, y, c) = p_pixel[c];
        }
    }

    template <typename U, typename T>
    void set_pixels(const T &point, const U &pixel_value)
    {
        for (int c = 0; c < nChannels; c++)
        {
            pixel<U>(point, c) = pixel_value;
        }
    }

    template <typename U>
    void set_pixels(const int x, const int y, const U &pixel_value)
    {
        for (int c = 0; c < nChannels; c++)
        {
            pixel<U>(x, y, c) = pixel_value;
        }
    }
};

HyStatus hySetImageData(HyImage*p_image, BYTE *p_data, const int step);
HySize hyGetSize(const HyImage *p_image);

void hySetImageROI(HyImage *p_image, HyRect roi);
HyRect hySafelySetImageROI(HyImage *p_image, HyRect roi);
HyRect hyGetImageROI(const HyImage *p_image);
void hyResetImageROI(HyImage *p_image);

HySize hySize(const HyRect& r);
inline HySize hyGetROISize(const HyImage *p_image)
{
    return hySize(hyGetImageROI(p_image));
}

HyImage* hyCreateImageHeader(const HySize &size, const int depth, const int channel);
void hyReleaseImageHeader(HyImage **pp_image);
HyImage* hyCreateImage(const HySize &size, const int depth, const int channel);
void hyReleaseImage(HyImage **pp_image);

void hySaveImage(HyImage *pImage, LPCTSTR fmt, ...);
void hySaveImage(LPCTSTR path, HyImage *p_image);

#ifdef MAC_OSX
HyImage *hyLoadImageByCGImage(LPCTSTR szFilePath, const int iscolor = 1);
#endif

#if IS_ANDROID || defined(LINUX_SERVER)
HyImage *hyLoadImageByLIBJpeg(const char *filename);
HyImage *hyLoadImageByLIBPng(const char *filename, const bool is_premultiplied = true);
#endif

HyImage *hyLoadImageFromRawData(LPCTSTR path);
bool hyLoadImageSizeFromRawData(LPCTSTR path, HySize &size);
bool hySaveImageToRawData(LPCTSTR path, HyImage *p_image);

class SmartHyImagePtr
{
public:
    SmartHyImagePtr()
    {
        mp_image = NULL;
        m_is_assign_image = false;
    }
    ~SmartHyImagePtr()
    {
        free();
    }
    int attach_smart_buffer(ChSmartPtr<BYTE> &p_buffer, const int width,
        const int height, const int channels)
    {
        return attach_smart_buffer(p_buffer, HySize(width, height), channels);
    }
    int attach_smart_buffer(ChSmartPtr<BYTE> &p_buffer, const HySize &image_size,
        const int channels)
    {
        free();

        int succeeded = 0;
        mp_image = hyCreateImageHeader(image_size, HY_DEPTH_8U, channels);
        succeeded = (mp_image != NULL);
        if (succeeded)
            succeeded = (p_buffer.smartptr_alloc(mp_image->height * mp_image->widthStep) != NULL);
        if (succeeded)
            hySetImageData(mp_image, p_buffer, mp_image->widthStep);

        if (!succeeded)
            free();

        return succeeded;
    }
    void free()
    {
        if (!m_is_assign_image)
            hyReleaseImageHeader(&mp_image);
        reset();
    }
    void swap(SmartHyImagePtr &p_dst_image)
    {
        SmartHyImagePtr temp_image = p_dst_image;
        p_dst_image = *this;
        *this = temp_image;

        temp_image.reset();
    }
    void assign(HyImage *p_image)
    {
        free();

        mp_image = p_image;
        m_is_assign_image = true;
    }

    operator const HyImage *()
    {
        return mp_image;
    }
    operator HyImage *()
    {
        return mp_image;
    }
    HyImage *operator->() const
    {
        return mp_image;
    }

private:
    SmartHyImagePtr(const SmartHyImagePtr &_p)
    {
        mp_image = _p.mp_image;
        m_is_assign_image = _p.m_is_assign_image;
    }
    void operator=(const SmartHyImagePtr &_p)
    {
        mp_image = _p.mp_image;
        m_is_assign_image = _p.m_is_assign_image;
    }

    void reset()
    {
        mp_image = NULL;
        m_is_assign_image = false;
    }

    HyImage *mp_image;
    bool m_is_assign_image;
};

///////////////////////////////////////////////////
// API functions

HyRect hyEnlargeROI(const HyRect &src_roi, const HySize &image_size, const int left_space, const int top_space, const int right_space, const int bottom_space, bool is_shift);
HyRect hyEnlargeROI(const HyRect &src_roi, const HySize &image_size, const float enlarge_ratio, bool is_shift);
HyRect hyEnlargeROI(const HyRect &src_roi, const HySize &image_size, const float left_ratio, const float top_ratio, const float right_ratio, const float bottom_ratio, bool is_shift);
HyRect hyEnlargeRect(const HyRect &src_rect, const int left_space, const int top_space, const int right_space, const int bottom_space);
HyRect hyEnlargeRect(const HyRect &src_rect, const float enlarge_ratio);
HyRect hyEnlargeRect(const HyRect &src_rect, const float left_ratio, const float top_ratio, const float right_ratio, const float bottom_ratio);
HyRect hyShrinkRect(const HyRect &src_rect, const int left_space, const int top_space, const int right_space, const int bottom_space);
HyRect hyShrinkRect(const HyRect &src_rect, const float shrink_ratio);

void hyRotateImage90(HyImage **pp_img);
void hyRotateImage270(HyImage **pp_img);
void hyRotateImage180(HyImage **pp_img);

/* For 3 channel color image, use macro HY_RGB() to specify color
   For 1 channel gray image, specify the color value to the color parameter */
void hyFillCircle(HyImage* p_image, const HyPoint &center, int radius, int color);
void hyReplaceCircle(HyImage* p_image, const HyPoint &center, int radius, int color, int old_color);
void hyCircle(HyImage* p_image, const HyPoint &center, int radius, int color, int thickness);
void hyDonut(HyImage* p_image, const HyPoint &center, float inner_radius, float outer_radius, BYTE alpha);

void hyLine(HyImage *p_image, const HyPoint &start_point, const HyPoint &end_point, int color);

void hyFillConvexPoly(HyImage *p_image, const HyPoint *pts, int pts_amount, int color );
void hyThickLine(HyImage *p_image, 
                 const HyPoint &start_point, const HyPoint &end_point, 
                 const int radius, int color, 
                 bool is_cut_head = false, bool is_cut_tail = false);
void hyReplaceConvexPoly(HyImage *p_image, const HyPoint *pts, int pts_amount, int color, int old_color);
void hyReplaceThickLine(HyImage *p_image, const HyPoint &start_point, const HyPoint &end_point, const int radius, int color, int old_color);

void hyFillRectangle(HyImage *p_image, const HyRect &rect, int color);
void hyRectangle(HyImage *p_image, const HyRect &rect, int color, int thickness);

// hyPutText only supports 3 channel image, and use macro HY_RGB() to specify color
void hyPutText(HyImage *p_image, LPCTSTR text, HyPoint position, int color = HY_RGB(0, 0, 255), int size = 32);
void hyPutText(HyImage *p_image, HyPoint position, int color, int size, const TCHAR *format, ...);
void hyPutText(HyImage *p_image, HyPoint position, const TCHAR *format, ...);

///////////////////////////////////////////////////

inline HyPoint hyPoint(const HyPoint2D32f& p)
{
    HyPoint r;
    r.x = (int) p.x;
    r.y = (int) p.y;
    return r;
}

inline HyPoint2D32f hyPoint2D32f(const HyPoint& p)
{
    HyPoint2D32f r;
    r.x = (float) p.x;
    r.y = (float) p.y;
    return r;
}

inline HySize hySize(const HyRect& r)
{
    HySize s;
    s.width = r.width;
    s.height = r.height;
    return s;
}

inline HyPoint hyRectCenter(const HyRect& r)
{
    HyPoint p;
    p.x = r.x + r.width / 2;
    p.y = r.y + r.height / 2;
    return p;
}

inline bool hyEmptyRect(const HyRect& r)
{
    return (r.width == 0 || r.height == 0);
}

inline int hyArea(const HyRect& r)
{
    return r.width * r.height;
}

inline int hyArea(const HySize& size)
{
    return size.width * size.height;
}

inline int hyArea(const HyImage *p_image)
{
    return hyArea(hyGetSize(p_image));
}

inline void hyPointRotate90(HyPoint& pt, int /*width*/, int height)
{
    HyPoint t = pt;
    t.x = height - pt.y;
    t.y = pt.x;
    pt = t;
}

inline void hyPointRotate270(HyPoint& pt, int width, int /*height*/)
{
    HyPoint t = pt;
    t.x = pt.y;
    t.y = width - pt.x;
    pt = t;
}

inline void hyRectRotate90(HyRect& rect, int width, int height)
{
    HyPoint pt = hyPoint(rect.x, rect.y + rect.height);
    hyPointRotate90(pt, width, height);
    rect = hyRect(pt.x, pt.y, rect.height, rect.width);
}

inline void hyRectRotate270(HyRect& rect, int width, int height)
{
    HyPoint pt = hyPoint(rect.x + rect.width, rect.y);
    hyPointRotate270(pt, width, height);
    rect = hyRect(pt.x, pt.y, rect.height, rect.width);
}

///////////////////////////////////////////////////

inline HyPoint hyRotate(const HyPoint& p, const HyPoint& r, float theta)
{
    HyPoint tmp;
    tmp.x = int((p.x - r.x) * cosf(theta) - (p.y - r.y) * sinf(theta) + r.x);
    tmp.y = int((p.x - r.x) * sinf(theta) + (p.y - r.y) * cosf(theta) + r.y);
    return tmp;
}

inline HyPoint2D32f hyRotate(const HyPoint2D32f& p, const HyPoint2D32f& r, float theta)
{
    HyPoint2D32f tmp;
    tmp.x = (p.x - r.x) * cosf(theta) - (p.y - r.y) * sinf(theta) + r.x;
    tmp.y = (p.x - r.x) * sinf(theta) + (p.y - r.y) * cosf(theta) + r.y;
    return tmp;
}

#define _MAKE_HYSQUAREDISTANCE(_pt2d)                                                \
    inline float hySquareDistance(const _pt2d& p1, const _pt2d& p2)                  \
{                                                                                    \
    return float((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));     \
}

#define _MAKE_HYROOTDISTANCE(_pt2d)                                        \
    inline float hyRootDistance(const _pt2d& p1, const _pt2d& p2)          \
{                                                                          \
    return sqrtf(hySquareDistance(p1, p2));                                \
}

_MAKE_HYSQUAREDISTANCE(HyPoint);
_MAKE_HYSQUAREDISTANCE(HyPoint2D32f);
_MAKE_HYROOTDISTANCE(HyPoint);
_MAKE_HYROOTDISTANCE(HyPoint2D32f);


#define _MAKE_HYPOINT2D_MIDPOINT(_pt2d)                                \
    inline _pt2d hyMidPoint(const _pt2d& p1, const _pt2d& p2)          \
{                                                                      \
    _pt2d pt;                                                          \
    pt.x = (p1.x + p2.x) / 2;                                          \
    pt.y = (p1.y + p2.y) / 2;                                          \
    return pt;                                                         \
}

_MAKE_HYPOINT2D_MIDPOINT(HyPoint);
_MAKE_HYPOINT2D_MIDPOINT(HyPoint2D32f);

///////////////////////////////////////////////////

#define _MAKE_HYPOINT2D_OPERATOR(_pt2d, _op)                           \
    inline _pt2d operator _op (const _pt2d& p1, const _pt2d& p2)       \
{                                                                      \
    _pt2d pt;                                                          \
    pt.x = p1.x _op p2.x;                                              \
    pt.y = p1.y _op p2.y;                                              \
    return pt;                                                         \
}

_MAKE_HYPOINT2D_OPERATOR(HyPoint, +);
_MAKE_HYPOINT2D_OPERATOR(HyPoint, -);
_MAKE_HYPOINT2D_OPERATOR(HyPoint2D32f, +);
_MAKE_HYPOINT2D_OPERATOR(HyPoint2D32f, -);

#define _MAKE_HYPOINT2D_OPERATORE(_pt2d, _op)                          \
    inline _pt2d& operator _op (_pt2d& p1, const _pt2d& p2)            \
{                                                                      \
    p1.x _op p2.x;                                                     \
    p1.y _op p2.y;                                                     \
    return p1;                                                         \
}

_MAKE_HYPOINT2D_OPERATORE(HyPoint, +=);
_MAKE_HYPOINT2D_OPERATORE(HyPoint, -=);
_MAKE_HYPOINT2D_OPERATORE(HyPoint2D32f, +=);
_MAKE_HYPOINT2D_OPERATORE(HyPoint2D32f, -=);

#define _MAKE_HYPOINT2D_OPERATOR_V(_pt2d, _op)                         \
    template<class _TYPE>                                              \
    inline _pt2d operator _op (const _pt2d& p, const _TYPE& v)         \
{                                                                      \
    _pt2d pt = p;                                                      \
    pt.x = pt.x _op v;                                                 \
    pt.y = pt.y _op v;                                                 \
    return pt;                                                         \
}

_MAKE_HYPOINT2D_OPERATOR_V(HyPoint, +);
_MAKE_HYPOINT2D_OPERATOR_V(HyPoint, -);
_MAKE_HYPOINT2D_OPERATOR_V(HyPoint, *);
_MAKE_HYPOINT2D_OPERATOR_V(HyPoint, /);

_MAKE_HYPOINT2D_OPERATOR_V(HyPoint2D32f, +);
_MAKE_HYPOINT2D_OPERATOR_V(HyPoint2D32f, -);
_MAKE_HYPOINT2D_OPERATOR_V(HyPoint2D32f, *);
_MAKE_HYPOINT2D_OPERATOR_V(HyPoint2D32f, /);

template<class _TYPE>
inline HyPoint& operator += (HyPoint& pt, const _TYPE& v)
{
    pt.x = int(pt.x + v);
    pt.y = int(pt.y + v);
    return pt;
}

template<class _TYPE>
inline HyPoint& operator -= (HyPoint& pt, const _TYPE& v)
{
    pt.x = int(pt.x - v);
    pt.y = int(pt.y - v);
    return pt;
}

template<class _TYPE>
inline HyPoint& operator *= (HyPoint& pt, const _TYPE& v)
{
    pt.x = int(pt.x * v);
    pt.y = int(pt.y * v);
    return pt;
}

template<class _TYPE>
inline HyPoint& operator /= (HyPoint& pt, const _TYPE& v)
{
    pt.x = int(pt.x / v);
    pt.y = int(pt.y / v);
    return pt;
}

template<class _TYPE>
inline HyPoint2D32f& operator += (HyPoint2D32f& pt, const _TYPE& v)
{
    pt.x = float(pt.x + v);
    pt.y = float(pt.y + v);
    return pt;
}

template<class _TYPE>
inline HyPoint2D32f& operator -= (HyPoint2D32f& pt, const _TYPE& v)
{
    pt.x = float(pt.x - v);
    pt.y = float(pt.y - v);
    return pt;
}

template<class _TYPE>
inline HyPoint2D32f& operator *= (HyPoint2D32f& pt, const _TYPE& v)
{
    pt.x = float(pt.x * v);
    pt.y = float(pt.y * v);
    return pt;
}

template<class _TYPE>
inline HyPoint2D32f& operator /= (HyPoint2D32f& pt, const _TYPE& v)
{
    pt.x = float(pt.x / v);
    pt.y = float(pt.y / v);
    return pt;
}

///////////////////////////////////////////////////

#define _MAKE_HYSIZE_OPERATOR_V(_op)                                  \
    template<class _TYPE>                                             \
    inline HySize operator _op (const HySize& s, const _TYPE& v)      \
{                                                                     \
    HySize size = s;                                                  \
    size.width = size.width _op v;                                    \
    size.height = size.height _op v;                                  \
    return size;                                                      \
}

_MAKE_HYSIZE_OPERATOR_V(+);
_MAKE_HYSIZE_OPERATOR_V(-);
_MAKE_HYSIZE_OPERATOR_V(*);
_MAKE_HYSIZE_OPERATOR_V(/);

/////////////////////////////////////////////////////

#define _MAKE_HYRECT_OPERATOR_V(_op)                                  \
    template<class _TYPE>                                             \
    inline HyRect operator _op (const HyRect& r, const _TYPE& v)      \
{                                                                     \
    HyRect rect = r;                                                  \
    rect.x = int(rect.x _op v);                                       \
    rect.y = int(rect.y _op v);                                       \
    rect.width = int(rect.width _op v);                               \
    rect.height = int(rect.height _op v);                             \
    return rect;                                                      \
}

_MAKE_HYRECT_OPERATOR_V(+);
_MAKE_HYRECT_OPERATOR_V(-);
_MAKE_HYRECT_OPERATOR_V(*);
_MAKE_HYRECT_OPERATOR_V(/);


template<class _TYPE>
inline HyRect& operator += (HyRect& r, const _TYPE& v)
{
    r.x = (int) (r.x + v);
    r.y = (int) (r.y + v);
    r.width = (int) (r.width + v);
    r.height = (int) (r.height + v);
    return r;
}

template<class _TYPE>
inline HyRect& operator -= (HyRect& r, const _TYPE& v)
{
    r.x = (int) (r.x - v);
    r.y = (int) (r.y - v);
    r.width = (int) (r.width - v);
    r.height = (int) (r.height - v);
    return r;
}

template<class _TYPE>
inline HyRect& operator *= (HyRect& r, const _TYPE& v)
{
    r.x = (int) (r.x * v);
    r.y = (int) (r.y * v);
    r.width = (int) (r.width * v);
    r.height = (int) (r.height * v);
    return r;
}

template<class _TYPE>
inline HyRect& operator /= (HyRect& r, const _TYPE& v)
{
    r.x = (int) (r.x / v);
    r.y = (int) (r.y / v);
    r.width = (int) (r.width / v);
    r.height = (int) (r.height / v);
    return r;
}

inline HyRect operator * (const HyRect &r, const ImageScale &scale)
{
    HyRect rect;
    rect.x = ch_Round((float)(r.x) * scale.x);
    rect.y = ch_Round((float)(r.y) * scale.y);
    rect.width = ch_Round((float)(r.width) * scale.x);
    rect.height = ch_Round((float)(r.height) * scale.y);
    return rect;
}

inline HyRect& operator *= (HyRect &r, const ImageScale &scale)
{
    r.x = ch_Round((float)(r.x) * scale.x);
    r.y = ch_Round((float)(r.y) * scale.y);
    r.width = ch_Round((float)(r.width) * scale.x);
    r.height = ch_Round((float)(r.height) * scale.y);
    return r;
}

inline HyPoint2D32f operator * (const HyPoint2D32f &p, const ImageScale &scale)
{
    HyPoint2D32f point;
    point.x = p.x * scale.x;
    point.y = p.y * scale.y;
    return point;
}

inline HyPoint2D32f operator / (const HyPoint2D32f &p, const ImageScale &scale)
{
    HyPoint2D32f point;
    point.x = p.x / scale.x;
    point.y = p.y / scale.y;
    return point;
}

inline HyPoint2D32f& operator *= (HyPoint2D32f &p, const ImageScale &scale)
{
    p.x *= scale.x;
    p.y *= scale.y;
    return p;
}

inline HyPoint2D32f& operator /= (HyPoint2D32f &p, const ImageScale &scale)
{
    p.x /= scale.x;
    p.y /= scale.y;
    return p;
}

/////////////////////////////////////////////////////
inline HyRect hyIntersectRect(const HyRect &rect1, const HyRect &rect2)
{
    int max_left = ch_Max(rect1.x, rect2.x);
    int min_right = ch_Min(rect1.x + rect1.width, rect2.x + rect2.width);
    int max_top = ch_Max(rect1.y, rect2.y);
    int min_bottom = ch_Min(rect1.y + rect1.height, rect2.y + rect2.height);

    HyRect new_rect;
    new_rect.x = max_left;
    new_rect.y = max_top;
    new_rect.width = ch_Max(min_right - max_left, 0);    
    new_rect.height = ch_Max(min_bottom - max_top, 0);

    return new_rect;
}

inline bool hyIntersectRect(HyRect &result, const HyRect &src1, const HyRect &src2) 
{
#ifdef UNIX_OS
    result = hyIntersectRect(src1, src2);
    return result.width > 0 && result.height > 0;
#else
	RECT tmpSrc1 = {src1.x, src1.y, src1.x + src1.width, src1.y + src1.height};
	RECT tmpSrc2 = {src2.x, src2.y, src2.x + src2.width, src2.y + src2.height};
	RECT tmpResult;

	bool bIntersect = IntersectRect(&tmpResult, &tmpSrc1, &tmpSrc2) ? true: false;
	result = hyRect(tmpResult.left, tmpResult.top, tmpResult.right - tmpResult.left, tmpResult.bottom - tmpResult.top);

	return bIntersect;
#endif
}

inline HyRect hyUnionRect(const HyRect &rect1, const HyRect &rect2)
{
    int min_left = ch_Min(rect1.x, rect2.x);
    int max_right = ch_Max(rect1.x + rect1.width, rect2.x + rect2.width);
    int min_top = ch_Min(rect1.y, rect2.y);
    int max_bottom = ch_Max(rect1.y + rect1.height, rect2.y + rect2.height);

    HyRect new_rect;
    new_rect.x = min_left;
    new_rect.y = min_top;
    new_rect.width = ch_Max(max_right - min_left, 0);    
    new_rect.height = ch_Max(max_bottom - min_top, 0);

    return new_rect;
}

inline bool IsValidRoi(const HyRect &roi, const HySize &size)
{
    if (roi.width <= 0 || roi.height <= 0)
        return false;
    if (roi.x < 0 || roi.x + roi.width > size.width)
        return false;
    if (roi.y < 0 || roi.y + roi.height > size.height)
        return false;

    return true;
}

inline bool IsValidRoi(const HyRect &roi, const HyImage *p_image)
{
    return IsValidRoi(roi, hyGetSize(p_image));
}

inline HyPoint& operator*= (HyPoint &p, const ImageScale &scale)
{
    p.x = ch_Round((float)p.x * scale.x);
    p.y = ch_Round((float)p.y * scale.y);

    return p;
}

inline HyPoint hyRoundPoint(const HyPoint2D32f& p)
{
    HyPoint round_p;
    round_p.x = ch_Round(p.x);
    round_p.y = ch_Round(p.y);
    return round_p;
}

inline HyPoint hyStartPoint(const HyRect &rect)
{
    return HyPoint(rect.x, rect.y);
}

inline HyPoint hyEndPoint(const HyRect &rect, const bool is_include = false)
{
    return HyPoint(rect.x + rect.width - (int)is_include, rect.y + rect.height - (int)is_include);
}

inline int GetDebugColor(float value, float min_th, float max_th)
{
    // Make a visualized color for debug message.
    // The color has red->yellow->green->cyan transition according to the given range.

    if (value < min_th)
        return HY_COLOR_RED;
    else if (value > max_th)
        return HY_COLOR_CYAN;

    float ratio = 3.0f * (value - min_th) / (max_th - min_th);
    int red = 0;
    int green = 0;
    int blue = 0;

    if (ratio < 1.0f)
    {
        red = 255;
        green = ch_Round(ratio * 255.0f);
    }
    else if (ratio < 2.0f)
    {
        green = 255;
        red = 255 - ch_Round((ratio - 1.0f) * 255.0f);
    }
    else
    {
        green = 255;
        blue = ch_Round((ratio - 2.0f) * 255.0f);
    }

    return HY_RGB(red, green, blue);
}

enum HyImagePtrSetMode {HYIMAGEPTR_SET_ALLOCATE, HYIMAGEPTR_SET_CLONE};

class HyImagePtr
{
    HyImage *mp_image;

    _DISABLE_CLASS_COPY(HyImagePtr);
    void operator=(HyImage *p);

public:
    HyImagePtr() : mp_image(NULL) {}
    HyImagePtr(HyImage *p) : mp_image(p) {}
    HyImagePtr(const HyImage *p, HyImagePtrSetMode mode) : mp_image(NULL) 
    {
        switch (mode)
        {
        case HYIMAGEPTR_SET_ALLOCATE:
            if (p) Alloc(hyGetSize(p), p->depth, p->nChannels);
            break;
        case HYIMAGEPTR_SET_CLONE:
            Clone(p);
            break;
        }
    }
    HyImagePtr(const HySize &size, const int depth, const int channel_num) : mp_image(NULL)
    {
        Alloc(size, depth, channel_num);
    }
    ~HyImagePtr() { hyReleaseImage(&mp_image); }

    void Alloc(const HySize &size, const int depth, const int channel_num)
    {
        hyReleaseImage(&mp_image);
        mp_image = hyCreateImage(size, depth, channel_num);
    }
    void Alloc(HyImage *p_image_header)
    {
        hyReleaseImage(&mp_image);
        if (p_image_header) Alloc(hyGetSize(p_image_header), p_image_header->depth, p_image_header->nChannels);
    }
    void Free() { hyReleaseImage(&mp_image); }
    void Reset(HyImage *p_image = NULL)
    {
        hyReleaseImage(&mp_image);
        mp_image = p_image;
    }

    void Clone(const HyImage *p_image)
    {
        Reset();

        if (!p_image) return;

        Alloc(hyGetSize(p_image), p_image->depth, p_image->nChannels);

        if (mp_image)
        {
            memcpy(mp_image->imageData, p_image->imageData, mp_image->height * mp_image->widthStep);

            hySetImageROI(mp_image, hyGetImageROI(p_image));
        }
    }

    HyImage* Transit() 
    { 
        HyImage *p_ret = mp_image;
        mp_image = NULL;

        return p_ret;
    }

    HyImage* Content() const { return mp_image; }
    HyImage* operator->() const { return mp_image; }

    void *operator()(int x, int y)
    {
        _MYASSERT(mp_image);
        return (void *)mp_image->get_pixels<BYTE>(x, y);
    }
};

template <ChAutoPtr_Allocator T>
struct ChAutoPtr<HyImage, T> {};

template <typename T> bool hyIsValidPtr(T *p_ptr) { return  p_ptr != NULL; }
template <class T> bool hyIsValidSize(const T &content) { return content.width > 0 && content.height > 0; }
template <class T> bool hyIsValidRect(const T &content, const HySize &target_size = HySize(INT_MAX, INT_MAX))
{
    return hyIsValidSize(content) && 
        content.x >= 0 && content.y >= 0 && 
        content.x + content.width <= target_size.width &&
        content.y + content.height <= target_size.height;
}
inline bool hyIsChannelReasonable(const HyImage *p_image) 
{ 
    if (!p_image)
        return false;

    return (p_image->nChannels == 1 || p_image->nChannels == 3 || p_image->nChannels == 4);
}
inline bool hyIsImageValid(const HyImage *p_image)
{
    if (!p_image)
        return false;

    return p_image->imageData &&
           hyIsChannelReasonable(p_image) &&
           hyIsValidSize(hyGetSize(p_image));
}
inline bool hyIsImageAndROIValid(const HyImage *p_image)
{
    return hyIsImageValid(p_image) && hyIsValidRect(hyGetImageROI(p_image), hyGetSize(p_image));
}
inline bool hyAreImageSizeMatched(const HyImage *p_image1, const HyImage *p_image2)
{
    return hyIsImageValid(p_image1) && hyIsImageValid(p_image2) &&
           (hyGetSize(p_image1) == hyGetSize(p_image2));
}
inline bool hyAreImageROISizeMatched(const HyImage *p_image1, const HyImage *p_image2)
{
    return hyIsImageAndROIValid(p_image1) && hyIsImageAndROIValid(p_image2) &&
           (hyGetROISize(p_image1) == hyGetROISize(p_image2));
}

template <typename T>
T* hyStartPixels(const HyImage *p_image, const HyRect &roi)
{
    _MYASSERT(p_image);

    return (hyIsImageValid(p_image) && hyArea(roi) > 0)
        ? const_cast<T*>(p_image->get_pixels<T>(hyStartPoint(roi)))
        : NULL;
}

template <typename T>
T* hyStartPixels(const HyImage *p_image)
{
    return hyStartPixels<T>(p_image, hyGetImageROI(p_image));
}

template <typename T>
inline T* hyPixels(T *p_start, const int widthStep, int nChannels, const HyPoint &point)
{
    _MYASSERT(p_start);

    return (T*)((BYTE *)p_start + widthStep * point.y) + point.x * nChannels;
}

template <typename T>
inline const T* hyPixels(const T *p_start, const int widthStep, int nChannels, const HyPoint &point)
{
    return (const T*)((const BYTE *)p_start + widthStep * point.y) + point.x * nChannels;
}

template <typename T>
inline T* hyPixels(HyImage *p_image, const HyPoint &point)
{
    _MYASSERT(p_image);

    return p_image->get_pixels<T>(point);
}

template <typename T>
inline const T* hyPixels(const HyImage *p_image, const HyPoint &point)
{
    _MYASSERT(p_image);

    return p_image->get_pixels<T>(point);
}

class HyROIInfo
{
public:
    BYTE *imageData;
    int widthStep;
    int nChannels;
    HySize size;

private:
    // Should not drectly create HyROIInfo; 
    // instead, call the hyGetROIInfo to get HyROIInfo.
    HyROIInfo(const HyImage *p_image, const HyRect &roi) 
        : imageData(hyStartPixels<BYTE>(p_image, roi))
        , size(hySize(roi))
    {
        _MYASSERT(p_image);
        widthStep = p_image->widthStep;
        nChannels = p_image->nChannels;
    }

    friend HyROIInfo hyGetROIInfo(const HyImage *p_image);
    friend HyROIInfo hyGetROIInfo(const HyImage *p_image, const HyRect &roi);
};

HyROIInfo hyGetROIInfo(const HyImage *p_image);
HyROIInfo hyGetROIInfo(const HyImage *p_image, const HyRect &roi);

inline BYTE* hyPixels(const HyROIInfo &roi_info, const HyPoint &point)
{
    return hyPixels<BYTE>(roi_info.imageData, 
                          roi_info.widthStep,
                          roi_info.nChannels,
                          point);
}

inline bool hyIsROIInfoValid(const HyROIInfo &roi_info)
{
    return roi_info.imageData && 
           (roi_info.nChannels == 1 || roi_info.nChannels == 3 || roi_info.nChannels == 4) &&
           hyIsValidSize(roi_info.size);
}

HyImage *hyCreateHorizontalConcatenateImage(const HyImage *p_left, const HyImage *p_right);
HyImage *hyCreateVerticalConcatenateImage(const HyImage *p_top, const HyImage *p_bottom);

void hyFillValueToChannel(HyImage *p_image, BYTE value, int channel);
