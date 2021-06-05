#include "stdafx.h"
#include "SelectorPanel.h"

SelectorPanel::SelectorPanel()
{
    m_is_initialized = false;
    m_icon_width = 0;
    m_icon_height = 0;
    m_border_space = 0;
    mp_base_image = NULL;
    mp_display_image = NULL;

    m_background_color = HY_RGB(0, 200, 255);
    m_selection_color = HY_COLOR_RED;

    m_selected_row_index = 0;
    m_selected_col_index = 0;
}

SelectorPanel::~SelectorPanel()
{
    hyReleaseImage(&mp_base_image);
    hyReleaseImage(&mp_display_image);
}

__forceinline HyRect SelectorPanel::GetIconRect(int row_index, int col_index)
{
    HyRect icon_rect;
    icon_rect.x = (col_index + 1) * m_border_space + col_index * m_icon_width;
    icon_rect.y = (row_index + 1) * m_border_space + row_index * m_icon_height;
    icon_rect.width = m_icon_width;
    icon_rect.height = m_icon_height;

    return icon_rect;
}

void SelectorPanel::SetStructure(int width, int height, int row_count, 
                                 const int *p_row_item_counts, int icon_size_alignment)
{
    if (width <= 0 || height <= 0 || row_count <= 0 || p_row_item_counts == NULL)
        return;

    int max_row_size = 0;
    for (int i = 0; i < row_count; i++)
        max_row_size = ch_Max(max_row_size, p_row_item_counts[i]);

    if (max_row_size == 0)
        return;

    max_row_size = ch_Max(max_row_size, 8);

    // Determine suitable layout size.
    m_border_space = 2;

    int best_icon_width = (width - (max_row_size + 1) * m_border_space) / max_row_size;
    int best_icon_height = (height - (row_count + 1) * m_border_space) / row_count;

    const int min_icon_length = 8;
    m_icon_width = ch_Max(ch_Min(best_icon_width, best_icon_height), min_icon_length);
    if (icon_size_alignment > 1)
        m_icon_width = (m_icon_width / icon_size_alignment) * icon_size_alignment;

    m_icon_height = m_icon_width;

    m_row_item_counts.resize(row_count);
    for (int i = 0; i < row_count; i++)
        m_row_item_counts[i] = p_row_item_counts[i];

    hyReleaseImage(&mp_base_image);
    hyReleaseImage(&mp_display_image);
    mp_base_image = hyCreateImage(hySize(width, height), HY_DEPTH_8U, 3);
    mp_display_image = hyCreateImage(hySize(width, height), HY_DEPTH_8U, 3);

    hyFillRectangle(mp_base_image, hyRect(0, 0, mp_base_image->width, mp_base_image->height), m_background_color);

    m_selected_row_index = 0;
    m_selected_col_index = 0;

    m_is_initialized = true;
}

void SelectorPanel::SetIconImage(HyImage *p_image, int row_index, int col_index)
{
    if (m_is_initialized == false)
        return;
    if (p_image == NULL)
        return;

    HyRect icon_rect = GetIconRect(row_index, col_index);
    if (IsValidRoi(icon_rect, hyGetSize(mp_base_image)) == false)
        return;
    if (icon_rect.width > p_image->width || icon_rect.height > p_image->height)
        return;

    hySetImageROI(p_image, hyRect(0, 0, icon_rect.width, icon_rect.height));
    hySetImageROI(mp_base_image, icon_rect);
    ippiCopy(p_image, mp_base_image);
    hyResetImageROI(p_image);
    hyResetImageROI(mp_base_image);
}

bool SelectorPanel::GetItemIndex(int x, int y, int &row_index, int &col_index)
{
    if (m_is_initialized == false)
        return false;

    int row_count = m_row_item_counts.size();
    int y_end = (row_count + 1) * m_border_space + row_count * m_icon_height;
    if (y < 0 || y >= y_end)
        return false;

    row_index = (y - m_border_space) / (m_icon_height + m_border_space);
    row_index = FitInRange(row_index, 0, row_count - 1);

    int col_count = m_row_item_counts[row_index];
    int x_end = (col_count + 1) * m_border_space + col_count * m_icon_width;
    if (x < 0 || x >= x_end)
        return false;

    col_index = (x - m_border_space) / (m_icon_width + m_border_space);
    col_index = FitInRange(col_index, 0, col_count - 1);

    return true;
}

bool SelectorPanel::Select(int row_index, int col_index)
{
    if (m_is_initialized == false)
        return false;

    int row_count = m_row_item_counts.size();
    if (row_index < 0 || row_index >= row_count)
        return false;
    if (col_index < 0 || col_index >= m_row_item_counts[row_index])
        return false;

    m_selected_row_index = row_index;
    m_selected_row_index = col_index;

    HyRect icon_rect = GetIconRect(row_index, col_index);
    int enlarge_space = m_border_space;
    HyRect enlarged_rect = hyEnlargeRect(icon_rect, enlarge_space, enlarge_space, enlarge_space, enlarge_space);

    ippiCopy(mp_base_image, mp_display_image);
    hyRectangle(mp_display_image, enlarged_rect, m_selection_color, enlarge_space);

    return true;
}