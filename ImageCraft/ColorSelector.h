#pragma once

#include "ColorStaticST.h"
#include "use_hylib.h"

class ColorSelector : public CColorStaticST
{
public:
    ColorSelector();
    virtual ~ColorSelector();

    void SetColor(int color);
    int GetColor();

protected:

    DECLARE_MESSAGE_MAP()

private:
    int m_color; // in HY_RGB()

public:
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
};