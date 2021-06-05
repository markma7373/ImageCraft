#include "stdafx.h"
#include "ColorSelector.h"


ColorSelector::ColorSelector()
: CColorStaticST()
{
    m_color = RGB(0, 0, 0);
}

ColorSelector::~ColorSelector()
{
}

BEGIN_MESSAGE_MAP(ColorSelector, CColorStaticST)
    ON_WM_LBUTTONUP()
    ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

void ColorSelector::SetColor(int color)
{
    m_color = color;

    SetBkColor(RGB(HY_GetRValue(color), HY_GetGValue(color), HY_GetBValue(color)));
}

int ColorSelector::GetColor()
{
    return m_color;
}

void ColorSelector::OnLButtonUp(UINT nFlags, CPoint point) 
{
    NMHDR hdr;
    hdr.code = NM_CLICK;
    hdr.hwndFrom = this->GetSafeHwnd();
    hdr.idFrom = GetDlgCtrlID();
    this->GetParent()->SendMessage(WM_NOTIFY, (WPARAM)hdr.idFrom, (LPARAM)&hdr);
}

void ColorSelector::OnRButtonUp(UINT nFlags, CPoint point) 
{
    NMHDR hdr;
    hdr.code = NM_RCLICK;
    hdr.hwndFrom = this->GetSafeHwnd();
    hdr.idFrom = GetDlgCtrlID();
    this->GetParent()->SendMessage(WM_NOTIFY, (WPARAM)hdr.idFrom, (LPARAM)&hdr);
}