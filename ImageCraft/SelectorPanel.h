#pragma once

#include "use_hylib.h"
#include "use_ipp.h"

class SelectorPanel
{
public:
    SelectorPanel();
    ~SelectorPanel();

    HySize GetIconSize()
    {
        return hySize(m_icon_width, m_icon_height);
    }
    void GetSelectedIndex(int &row_index, int &col_index)
    {
        row_index = m_selected_row_index;
        col_index = m_selected_col_index;
    }
    HyImage *GetDisplayImage()
    {
        return mp_display_image;
    }

    void SetStructure(int width, int height, int row_count, 
                      const int *p_row_item_counts, int icon_size_alignment = 1);
    void SetIconImage(HyImage *p_image, int row_index, int col_index);

    bool GetItemIndex(int x, int y, int &row_index, int &col_index);
    bool Select(int row_index, int col_index);

private:
    bool m_is_initialized;

    int m_icon_width;
    int m_icon_height;
    int m_border_space;
    std::vector<int> m_row_item_counts;
    HyImage *mp_base_image;
    HyImage *mp_display_image;

    int m_background_color;
    int m_selection_color;

    int m_selected_row_index;
    int m_selected_col_index;

    __forceinline HyRect GetIconRect(int row_index, int col_index);
};