
#include "stdafx.h"
// #include "winheaders.h"
#include "DrawStatic.h"
#include "Common.h"
#include "use_hylib.h"
#include "use_gdiplus.h"
#include "use_ipp.h"
using namespace Gdiplus;

// CDrawStatic

IMPLEMENT_DYNAMIC(CDrawStatic, CStatic)
CDrawStatic::CDrawStatic()
{
    m_Width = m_Height = -1;
    m_pMyBuffer = NULL;
    m_bStretch = false;
    m_bErase = true;
}

CDrawStatic::~CDrawStatic()
{
    _DELETE_PTRS(m_pMyBuffer);
}

BEGIN_MESSAGE_MAP(CDrawStatic, CStatic)
END_MESSAGE_MAP()

// CDrawStatic message handlers

void CDrawStatic::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CRect client_rect;
    GetClientRect(client_rect);

    CClientDC dc(this);

    ChAutoLock myLock(&m_csBuffer);
    if(m_pMyBuffer != NULL) {
        HyImage *p_src_image = hyCreateImageHeader(hySize(m_Width, m_Height), HY_DEPTH_8U, 3);
        hySetImageData(p_src_image, m_pMyBuffer, m_Stride);

        HyImage *p_dst_image = hyCreateImageHeader(hySize(client_rect.Width(), client_rect.Height()), HY_DEPTH_8U, 3);
        BYTE *p_buffer = NULL;
        int stride = ALIGN(p_dst_image->width * 3, 4);
        _NEW_PTRS(p_buffer, BYTE, stride * p_dst_image->height);
        hySetImageData(p_dst_image, p_buffer, stride);

        ZeroMemory(p_dst_image->imageData, p_dst_image->widthStep * p_dst_image->height);

        int new_width = m_Width;
        int new_height = m_Height;
        m_csBuffer.Lock();
        if (ch_NeedScaleFix(new_width, new_height, client_rect.Width(), client_rect.Height()))
        {
            new_width = ch_Min(new_width, client_rect.Width());
            new_height = ch_Min(new_height, client_rect.Height());
            hySetImageROI(p_dst_image, hyRect((client_rect.Width() - new_width) / 2,
                (client_rect.Height() - new_height) / 2,
                new_width,
                new_height));
            ippiResize(p_src_image, p_dst_image);
            hyResetImageROI(p_dst_image);
        }
        else
        {
            ippiCopy(p_src_image, p_dst_image);
        }
        m_csBuffer.Unlock();

        Graphics gx(dc.GetSafeHdc());

        Bitmap img(p_dst_image->width, p_dst_image->height, PixelFormat24bppRGB);
        Rect r(0, 0, p_dst_image->width, p_dst_image->height);
        BitmapData bmData;
        img.LockBits(&r, ImageLockModeWrite, PixelFormat24bppRGB, &bmData);
        memcpy(bmData.Scan0, (BYTE*)p_dst_image->imageData, p_dst_image->widthStep * p_dst_image->height);
        img.UnlockBits(&bmData);
        gx.DrawImage(&img, client_rect.left, client_rect.top, client_rect.Width(), client_rect.Height());

        hyReleaseImageHeader(&p_src_image);
        _DELETE_PTRS(p_buffer);
        hyReleaseImageHeader(&p_dst_image);
    }
}

bool CDrawStatic::SetColorImage(BYTE *pBuff, int width, int height, int stride, int src_channels)
{
    _MYASSERT(pBuff);

    if(width * height <= 0)
        return false;

    ChAutoLock myLock(&m_csBuffer);

    int dst_channels = 3;
    //int src_channels = 1;

    m_Width = width;
    m_Height = height;
    m_Stride = ALIGN(width * 3, 4);
        
    _NEW_PTRS(m_pMyBuffer, BYTE, m_Stride * m_Height);
    ZeroMemory(m_pMyBuffer, m_Stride * m_Height);
    
    if (src_channels == 1)
    {
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                *(m_pMyBuffer + y * m_Stride + x * dst_channels) = *(m_pMyBuffer + y * m_Stride + x * dst_channels + 1) = *(m_pMyBuffer + y * m_Stride + x * dst_channels + 2) = *(pBuff + y * stride + x);
        
                //memcpy(m_pMyBuffer + y * m_Stride + x * dst_channels, pBuff + y * stride + x * src_channels, dst_channels);
            }
        }
    }
    else if (src_channels == 3)
    {
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                *(m_pMyBuffer + y * m_Stride + x * dst_channels) = *(pBuff + y * stride + x * src_channels + 0);
                *(m_pMyBuffer + y * m_Stride + x * dst_channels + 1) = *(pBuff + y * stride + x * src_channels + 1);
                *(m_pMyBuffer + y * m_Stride + x * dst_channels + 2) = *(pBuff + y * stride + x * src_channels + 2);
            }
        }
    }
    else if (src_channels >= 4)
    {
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int alpha = *(pBuff + y * stride + x * src_channels + 3);
                *(m_pMyBuffer + y * m_Stride + x * dst_channels) = (*(pBuff + y * stride + x * src_channels + 0) * alpha) >> 8;
                *(m_pMyBuffer + y * m_Stride + x * dst_channels + 1) = (*(pBuff + y * stride + x * src_channels + 1) * alpha) >> 8;
                *(m_pMyBuffer + y * m_Stride + x * dst_channels + 2) = (*(pBuff + y * stride + x * src_channels + 2) * alpha) >> 8;
            }
        }
    }

    return true;
}

bool CDrawStatic::SetImage(BYTE *pBuff, int width, int height)
{
    if(width * height <= 0)
        return false;

    ChAutoLock myLock(&m_csBuffer);

    m_Width = width;
    m_Height = height;
    m_Stride = ALIGN(width * 3, 4);

    HyImage *p1 = hyCreateImageHeader(hySize(m_Width, m_Height), HY_DEPTH_8U, 1);
    hySetImageData(p1, pBuff, p1->widthStep);
    HyImage *p2 = hyCreateImageHeader(hySize(m_Width, m_Height), HY_DEPTH_8U, 3);

    _NEW_PTRS(m_pMyBuffer, BYTE, m_Stride * m_Height);
    ZeroMemory(m_pMyBuffer, m_Stride * m_Height);
    hySetImageData(p2, m_pMyBuffer, p2->widthStep);
    ippiBGRToGray(p1, p2);
    
    hyReleaseImageHeader(&p1);
    hyReleaseImageHeader(&p2);

    return true;
}

void CDrawStatic::SetStretch(const bool bEnable)
{
    m_bStretch = bEnable;
}

void CDrawStatic::SetErase(const bool is_to_erase)
{
    m_bErase = is_to_erase;
}

bool CDrawStatic::ZeroImage()
{
    if (!m_pMyBuffer || m_Stride * m_Height == 0) 
    {
        return false;
    }
    else
    {
        ZeroMemory(m_pMyBuffer, m_Stride * m_Height);
        return true;
    }
}