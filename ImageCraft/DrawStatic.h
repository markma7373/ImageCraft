
#pragma once

#include "ThreadTool.h"
// CDrawStatic

class CDrawStatic : public CStatic
{
    DECLARE_DYNAMIC(CDrawStatic)
    int m_Width;
    int m_Height;
    int m_Stride;
    BYTE *m_pMyBuffer;
    bool m_bStretch;
    bool m_bErase;

    ChCritSec m_csBuffer;

public:
    CDrawStatic();
    virtual ~CDrawStatic();

    void SetStretch(const bool bEnable);
    bool SetImage(BYTE *pBuff, int width, int height);
    bool SetColorImage(BYTE *pBuff, int width, int height, int stride, int src_channels = 1);
    void SetErase(const bool is_to_erase);
    bool ZeroImage();

protected:
    DECLARE_MESSAGE_MAP()
public:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
};
