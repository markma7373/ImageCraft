#pragma once

#include <afxtempl.h>

struct MyComboBoxItem
{ 
	bool is_marked;

    MyComboBoxItem()
    {
        is_marked = false;
    }
}; 
 
class MyComboBox : public CComboBox 
{ 
public: 
	MyComboBox();
    virtual ~MyComboBox();

    void SetMarkedColor(COLORREF marked_text_color, COLORREF marked_background_color);

    void ClearItemMap();
 
	void SetItemMarked(int item, bool is_marked);
    void SetAllItemsUnmarked();
 
protected: 
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
 
	DECLARE_MESSAGE_MAP() 
	CMap<int, int, MyComboBoxItem, MyComboBoxItem> m_map;

private:
    COLORREF m_marked_text_color;
    COLORREF m_marked_background_color;
}; 