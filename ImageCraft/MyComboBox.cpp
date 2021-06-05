#include "stdafx.h"
#include "MyComboBox.h"

MyComboBox::MyComboBox()   
{
    m_marked_text_color = RGB(0, 255, 255);
    m_marked_background_color = RGB(255, 0, 0);
}

MyComboBox::~MyComboBox()   
{

}

BEGIN_MESSAGE_MAP(MyComboBox, CComboBox)
END_MESSAGE_MAP()

void MyComboBox::SetMarkedColor(COLORREF marked_text_color, COLORREF marked_background_color)
{
    m_marked_text_color = marked_text_color;
    m_marked_background_color = marked_background_color;
}

void MyComboBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)    
{   
    if (GetCount() == 0)
        return;

    CString item_str;   
    GetLBText(lpDrawItemStruct->itemID, item_str);   
    CDC dc;   
      
    dc.Attach(lpDrawItemStruct->hDC);   

    // Save these value to restore them when done drawing.   
    COLORREF old_text_color = dc.GetTextColor();   
    COLORREF old_background_color = dc.GetBkColor();   

    const bool is_enabled = (IsWindowEnabled() != 0);
    const bool is_dropdown = (GetDroppedState() != 0);
    COLORREF text_color = ::GetSysColor(COLOR_WINDOWTEXT);
    COLORREF background_color = ::GetSysColor(COLOR_WINDOW);
    if (is_enabled == false)
    {
        text_color = ::GetSysColor(COLOR_GRAYTEXT);
    }
    else if (is_dropdown)
    {
        if ((lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED))
        {
            text_color = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
            background_color = ::GetSysColor(COLOR_HIGHLIGHT);
        }
        else if ((lpDrawItemStruct->itemState & ODS_COMBOBOXEDIT) == 0)
        {
            MyComboBoxItem item_data;
            if (m_map.Lookup(lpDrawItemStruct->itemID, item_data))
            {
                if (item_data.is_marked)
                {
                    text_color = m_marked_text_color;
                    background_color = m_marked_background_color;
                }
            }
        }
    }

    dc.SetTextColor(text_color);   
    dc.SetBkColor(background_color);
    dc.FillSolidRect(&lpDrawItemStruct->rcItem, background_color);   
 
    CRect rect(lpDrawItemStruct->rcItem);   
    dc.DrawText(item_str, -1, &rect, DT_LEFT|DT_SINGLELINE|DT_VCENTER);

    dc.SetTextColor(old_text_color);   
    dc.SetBkColor(old_background_color);   

    dc.Detach();
}

void MyComboBox::SetItemMarked(int item, bool is_marked)
{
    MyComboBoxItem item_data;
    item_data.is_marked = is_marked;

    m_map.SetAt(item, item_data);
}

void MyComboBox::SetAllItemsUnmarked()
{
    int item_count = GetCount();
    for (int i = 0; i < item_count; i++)
    {
        MyComboBoxItem item_data;
        if (m_map.Lookup(i, item_data))   
        {
            item_data.is_marked = false;
            m_map.SetAt(i, item_data);
        }
    }
}

void MyComboBox::ClearItemMap()
{
    m_map.RemoveAll();
}