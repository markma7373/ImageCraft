#include "stdafx.h"
#include "ImageCraftShare.h"
#include "AnsiCanvas.h"
#include <stack>

#define MIN_CANVAS_WIDTH    80
#define MIN_CANVAS_HEIGHT   23

#define MAX_ACTION_HISTORY_SIZE     500

AnsiCanvas::AnsiCanvas()
: mp_colors(NULL)
, mp_old_colors(NULL)
, mp_small_square_mask(NULL)
{
    m_width = MIN_CANVAS_WIDTH;
    m_height = MIN_CANVAS_HEIGHT;

    _NEW_PTRS(mp_colors, AnsiColor, m_width * m_height);
    _NEW_PTRS(mp_old_colors, AnsiColor, m_width * m_height);
    _NEW_PTRS(mp_small_square_mask, BYTE, m_width * m_height * 2);

    ClearCanvas();
    ClearActionHistory();

    UpdateBlockSize(16);

    m_display_x_offset = 0;
    m_display_y_offset = 0;
}

AnsiCanvas::~AnsiCanvas()
{
    _DELETE_PTRS(mp_colors);
    _DELETE_PTRS(mp_old_colors);
    _DELETE_PTRS(mp_small_square_mask);
}

__forceinline int AnsiCanvas::FloorDivision(int a, int b)
{
    // Assume b > 0. Compute a / b but still round down if a < 0.

    if (a >= 0)
        return a / b;
    else
        return -(((-a) + b - 1) / b);
}

__forceinline int AnsiCanvas::ColorValue(int label, bool is_bright)
{
    if (is_bright)
        return g_bright_color_list[label];
    else
        return g_normal_color_list[label];
}

__forceinline int AnsiCanvas::ColorLabel(char color_char)
{
    if (color_char == '0')
        return 0;
    else if (color_char == '1')
        return 1;
    else if (color_char == '2')
        return 2;
    else if (color_char == '3')
        return 3;
    else if (color_char == '4')
        return 4;
    else if (color_char == '5')
        return 5;
    else if (color_char == '6')
        return 6;
    else if (color_char == '7')
        return 7;
    else
        return 0;
}

__forceinline void AnsiCanvas::StartAnsiCommand(FILE *p_file)
{
    fputc(0x1b, p_file);
    fputc(0x5b, p_file);
}

__forceinline void AnsiCanvas::EndAnsiCommand(FILE *p_file)
{
    fputc(0x6d, p_file);
}

__forceinline void AnsiCanvas::EndLine(FILE *p_file)
{
    StartAnsiCommand(p_file);
    EndAnsiCommand(p_file);

    fputc(0x0d, p_file);
    fputc(0x0a, p_file);
}

__forceinline bool AnsiCanvas::IsLineEnd(const char *p_line)
{
    return (p_line[0] == '\r') || 
           (p_line[0] == '\n') ||
           (p_line[0] == '\0');
}

__forceinline bool AnsiCanvas::IsAnsiCommand(const char *p_line)
{
    return (p_line[0] == 0x1b && p_line[1] == 0x5b);
}

__forceinline bool AnsiCanvas::IsForegroundTriangle(int x_shift, int y_shift, AnsiCharLocation char_location, int triangle_label)
{
    int dx = x_shift;
    int dy = y_shift;
    if (char_location == DOUBLE_CHAR_RIGHT)
        dx += m_half_block_size;

    if (triangle_label == 0)
        return (dx + dy < m_block_size);
    else if (triangle_label == 1)
        return (dx > dy);
    else if (triangle_label == 2)
        return (dx < dy);
    else if (triangle_label == 3)
        return (dx + dy > m_block_size);
    else
        return false;
}

__forceinline bool AnsiCanvas::IsForegroundRegularTriangle(int x_shift, int y_shift, AnsiCharLocation char_location, int triangle_label)
{
    // dx, dy is relative to the current space region.
    // This is different from IsForegroundTriangle().
    int dx = x_shift;
    int dy = y_shift;

    if (triangle_label == 0) // upward triangle
    {
        if (char_location == DOUBLE_CHAR_RIGHT)
            return (dy >= dx * 2 + 1);
        else
            return (dx * 2 + dy + 1 >= m_block_size);
    }
    else if (triangle_label == 1) // downward triangle
    {
        if (char_location == DOUBLE_CHAR_RIGHT)
            return (dx * 2 + dy + 1 < m_block_size);
        else
            return (dy < dx * 2 + 1);
    }
    else
    {
        return false;
    }
}

AnsiColor AnsiCanvas::RandomColor(bool is_enable_bright/* = true*/)
{
    int rand_fg = rand() % 8;
    int rand_bg = rand() % 8;

    AnsiColor color;
    color.bg_color = ColorValue(rand_bg, false);

    if (is_enable_bright)
        color.fg_color = ColorValue(rand_fg, (rand() % 2 == 0));
    else
        color.fg_color = ColorValue(rand_fg, false);

    return color;
}

bool AnsiCanvas::GetSingleBlockColor(const AnsiCell &cell, const AnsiColor &left_color, const AnsiColor &right_color, int &block_color)
{
    int left_space_color;
    if (GetPureSpaceColor(left_color, cell, false, left_space_color) == false)
        return false;

    int right_space_color;
    if  (GetPureSpaceColor(right_color, cell, true, right_space_color) == false)
        return false;

    if (left_space_color != right_space_color)
        return false;

    block_color = left_space_color;
    return true;
}

bool AnsiCanvas::GetSplitBlockColor(const AnsiCell &cell, const AnsiColor &left_color, const AnsiColor &right_color, int &split_color1, int &split_color2)
{
    if (IsEmptyBlock(cell))
    {
        if (left_color.bg_color != right_color.bg_color)
        {
            split_color1 = left_color.bg_color;
            split_color2 = right_color.bg_color;
            return true;
        }
    }
    else if (IsFullBlock(cell))
    {
        if (left_color.fg_color != right_color.fg_color)
        {
            split_color1 = left_color.fg_color;
            split_color2 = right_color.fg_color;
            return true;
        }
    }
    else if (cell.IsDoubleChar())
    {
        int left_space_color, right_space_color;
        if (GetPureSpaceColor(left_color, cell, false, left_space_color) &&
            GetPureSpaceColor(right_color, cell, true, right_space_color))
        {
            split_color1 = left_space_color;
            split_color2 = right_space_color;
            return true;
        }
    }

    return false;
}

bool AnsiCanvas::GetLeftBoundaryColor(int x, int y, int &boundary_color)
{
    if (y < 0 || y >= m_height)
        return false;
    if (x < 0 || x >= m_width)
        return false;

    const AnsiRow &row = m_cells[y];

    int offset;
    AnsiCharLocation location;
    bool is_success = SearchCellInfo(row, x, offset, location);
    if (is_success == false)
        return false;

    const AnsiCell &cell = row[offset];
    const AnsiColor color = CanvasColorAt(x, y);
    if (color.fg_color == color.bg_color)
    {
        boundary_color = color.fg_color;
        return true;
    }
    else if (cell.type == EMPTY)
    {
        boundary_color = color.bg_color;
        return true;
    }
    else if (cell.type == HORI_BLOCK)
    {
        if (cell.label == 8)
        {
            boundary_color = color.fg_color;
            return true;
        }
        else if (cell.label == 0)
        {
            boundary_color = color.bg_color;
            return true;
        }
    }
    else if (cell.type == VERT_BLOCK)
    {
        if (location == DOUBLE_CHAR_LEFT)
        {
            boundary_color = (cell.label > 0) ? color.fg_color : color.bg_color;
            return true;
        }
        else if (location == DOUBLE_CHAR_RIGHT)
        {
            boundary_color = (cell.label > 4) ? color.fg_color : color.bg_color;
            return true;
        }
    }
    else if (cell.type == TRIANGLE)
    {
        if (location == DOUBLE_CHAR_LEFT)
        {
            boundary_color = (cell.label == 0 || cell.label == 2) ? color.fg_color : color.bg_color;
            return true;
        }
    }
    else if (cell.type == REGULAR_TRIANGLE)
    {
        if (location == DOUBLE_CHAR_LEFT)
        {
            boundary_color = color.bg_color;
            return true;
        }
        else if (location == DOUBLE_CHAR_RIGHT)
        {
            boundary_color = color.fg_color;
            return true;
        }
    }

    return false;
}

bool AnsiCanvas::GetRightBoundaryColor(int x, int y, int &boundary_color)
{
    if (y < 0 || y >= m_height)
        return false;
    if (x < 0 || x >= m_width)
        return false;

    const AnsiRow &row = m_cells[y];

    int offset;
    AnsiCharLocation location;
    bool is_success = SearchCellInfo(row, x, offset, location);
    if (is_success == false)
        return false;

    const AnsiCell &cell = row[offset];
    const AnsiColor color = CanvasColorAt(x, y);
    if (color.fg_color == color.bg_color)
    {
        boundary_color = color.fg_color;
        return true;
    }
    else if (cell.type == EMPTY)
    {
        boundary_color = color.bg_color;
        return true;
    }
    else if (cell.type == HORI_BLOCK)
    {
        if (cell.label == 8)
        {
            boundary_color = color.fg_color;
            return true;
        }
        else if (cell.label == 0)
        {
            boundary_color = color.bg_color;
            return true;
        }
    }
    else if (cell.type == VERT_BLOCK)
    {
        if (location == DOUBLE_CHAR_LEFT)
        {
            boundary_color = (cell.label >= 4) ? color.fg_color : color.bg_color;
            return true;
        }
        else if (location == DOUBLE_CHAR_RIGHT)
        {
            boundary_color = (cell.label == 8) ? color.fg_color : color.bg_color;
            return true;
        }
    }
    else if (cell.type == TRIANGLE)
    {
        if (location == DOUBLE_CHAR_RIGHT)
        {
            boundary_color = (cell.label == 1 || cell.label == 3) ? color.fg_color : color.bg_color;
            return true;
        }
    }
    else if (cell.type == REGULAR_TRIANGLE)
    {
        if (location == DOUBLE_CHAR_LEFT)
        {
            boundary_color = color.fg_color;
            return true;
        }
        else if (location == DOUBLE_CHAR_RIGHT)
        {
            boundary_color = color.bg_color;
            return true;
        }
    }

    return false;
}

bool AnsiCanvas::GetTopBoundaryColor(int x, int y, int &boundary_color)
{
    if (y < 0 || y >= m_height)
        return false;
    if (x < 0 || x >= m_width)
        return false;

    const AnsiRow &row = m_cells[y];

    int offset;
    AnsiCharLocation location;
    bool is_success = SearchCellInfo(row, x, offset, location);
    if (is_success == false)
        return false;

    const AnsiCell &cell = row[offset];
    const AnsiColor color = CanvasColorAt(x, y);
    if (color.fg_color == color.bg_color)
    {
        boundary_color = color.fg_color;
        return true;
    }
    else if (cell.type == EMPTY)
    {
        boundary_color = color.bg_color;
        return true;
    }
    else if (cell.type == HORI_BLOCK)
    {
        boundary_color = (cell.label == 8) ? color.fg_color : color.bg_color;
        return true;
    }
    else if (cell.type == VERT_BLOCK)
    {
        if (location == DOUBLE_CHAR_LEFT)
        {
            if (cell.label >= 4)
            {
                boundary_color = color.fg_color;
                return true;
            }
            else if (cell.label == 0)
            {
                boundary_color = color.bg_color;
                return true;
            }
        }
        else if (location == DOUBLE_CHAR_RIGHT)
        {
            if (cell.label == 8)
            {
                boundary_color = color.fg_color;
                return true;
            }
            else if (cell.label <= 4)
            {
                boundary_color = color.bg_color;
                return true;
            }
        }
    }
    else if (cell.type == TRIANGLE)
    {
        boundary_color = (cell.label <= 1) ? color.fg_color : color.bg_color;
        return true;
    }
    else if (cell.type == REGULAR_TRIANGLE)
    {
        boundary_color = (cell.label == 1) ? color.fg_color : color.bg_color;
        return true;
    }

    return false;
}

bool AnsiCanvas::GetTopBoundaryColor(int x1, int x2, int y, int &boundary_color)
{
    // Check if (x1, y) (x2, y) shares a top boundary color.

    int color1;
    bool is_success = GetTopBoundaryColor(x1, y, color1);
    if (is_success == false)
        return false;

    int color2;
    is_success = GetTopBoundaryColor(x2, y, color2);
    if (is_success == false)
        return false;

    if (color1 != color2)
        return false;

    boundary_color = color1;
    return true;
}

bool AnsiCanvas::GetBottomBoundaryColor(int x, int y, int &boundary_color)
{
    if (y < 0 || y >= m_height)
        return false;
    if (x < 0 || x >= m_width)
        return false;

    const AnsiRow &row = m_cells[y];

    int offset;
    AnsiCharLocation location;
    bool is_success = SearchCellInfo(row, x, offset, location);
    if (is_success == false)
        return false;

    const AnsiCell &cell = row[offset];
    const AnsiColor color = CanvasColorAt(x, y);
    if (color.fg_color == color.bg_color)
    {
        boundary_color = color.fg_color;
        return true;
    }
    else if (cell.type == EMPTY)
    {
        boundary_color = color.bg_color;
        return true;
    }
    else if (cell.type == HORI_BLOCK)
    {
        boundary_color = (cell.label > 0) ? color.fg_color : color.bg_color;
        return true;
    }
    else if (cell.type == VERT_BLOCK)
    {
        if (location == DOUBLE_CHAR_LEFT)
        {
            if (cell.label >= 4)
            {
                boundary_color = color.fg_color;
                return true;
            }
            else if (cell.label == 0)
            {
                boundary_color = color.bg_color;
                return true;
            }
        }
        else if (location == DOUBLE_CHAR_RIGHT)
        {
            if (cell.label == 8)
            {
                boundary_color = color.fg_color;
                return true;
            }
            else if (cell.label <= 4)
            {
                boundary_color = color.bg_color;
                return true;
            }
        }
    }
    else if (cell.type == TRIANGLE)
    {
        boundary_color = (cell.label >= 2) ? color.fg_color : color.bg_color;
        return true;
    }
    else if (cell.type == REGULAR_TRIANGLE)
    {
        boundary_color = (cell.label == 0) ? color.fg_color : color.bg_color;
        return true;
    }

    return false;
}

bool AnsiCanvas::GetBottomBoundaryColor(int x1, int x2, int y, int &boundary_color)
{
    // Check if (x1, y) (x2, y) shares a bottom boundary color.

    int color1;
    bool is_success = GetBottomBoundaryColor(x1, y, color1);
    if (is_success == false)
        return false;

    int color2;
    is_success = GetBottomBoundaryColor(x2, y, color2);
    if (is_success == false)
        return false;

    if (color1 != color2)
        return false;

    boundary_color = color1;
    return true;
}

void AnsiCanvas::GetOutsideBoundaryColors(int x_offset, int y_offset,
                                          int boundary_colors[4], bool is_boundary_valid_array[4])
{
    // Get the boundary color outside of the half-width space at given offset.
    // At left/top/right/bottom side, get the color from the right/bottom/left/top side of neighboring contents.

    is_boundary_valid_array[0] = GetRightBoundaryColor(x_offset - 1, y_offset, boundary_colors[0]);
    is_boundary_valid_array[1] = GetBottomBoundaryColor(x_offset, y_offset - 1, boundary_colors[1]);
    is_boundary_valid_array[2] = GetLeftBoundaryColor(x_offset + 1, y_offset, boundary_colors[2]);
    is_boundary_valid_array[3] = GetTopBoundaryColor(x_offset, y_offset + 1, boundary_colors[3]);
}

void AnsiCanvas::GetOutsideBlockBoundaryColors(int x_offset, int y_offset,
                                               int boundary_colors[6], bool is_boundary_valid_array[6])
{
    // Similar to GetOutsideBoundaryColors, but check for a total of 6 boundaries:
    //    +--[1]--+--[2]--+
    //    |       |       |
    //    |       |       |
    //   [0]      |      [3]
    //    |       |       |
    //    |       |       |
    //    +--[5]--+--[4]--+

    is_boundary_valid_array[0] = GetRightBoundaryColor(x_offset - 1, y_offset, boundary_colors[0]);
    is_boundary_valid_array[1] = GetBottomBoundaryColor(x_offset, y_offset - 1, boundary_colors[1]);
    is_boundary_valid_array[2] = GetBottomBoundaryColor(x_offset + 1, y_offset - 1, boundary_colors[2]);
    is_boundary_valid_array[3] = GetLeftBoundaryColor(x_offset + 2, y_offset, boundary_colors[3]);
    is_boundary_valid_array[4] = GetTopBoundaryColor(x_offset + 1, y_offset + 1, boundary_colors[4]);
    is_boundary_valid_array[5] = GetTopBoundaryColor(x_offset, y_offset + 1, boundary_colors[5]);
}

void AnsiCanvas::SetBlockSize(int block_size)
{
    // Force to use even-value block size.

    ChAutoLock auto_lock(&m_data_lock);

    int valid_block_size = FitInRange(block_size, m_min_block_size, m_max_block_size);
    valid_block_size = ALIGN(valid_block_size, 2);

    UpdateBlockSize(valid_block_size);
}

void AnsiCanvas::UpdateBlockSize(int block_size)
{
    m_block_size = block_size;
    m_half_block_size = m_block_size / 2;

    m_ansi_template.SetSize(m_block_size);
}

void AnsiCanvas::ClearCanvasAction()
{
    ClearCanvas();

    RecordAction(ActionInfo(AC_EDIT_CLEAR_CANVAS));
}

void AnsiCanvas::ClearCanvas()
{
    if (mp_colors)
    {
        for (int i = 0; i < m_width * m_height; i++)
            mp_colors[i] = AnsiColor();
    }

    m_cells.clear();
    m_cells.resize(m_height);

    for (int y = 0; y < m_height; y++)
        m_cells[y].resize(m_width, AnsiCell());
}

void AnsiCanvas::SetTestContents()
{
    ChAutoLock auto_lock(&m_data_lock);

    for (int y = 0; y < m_height; y++)
    {
        AnsiColor *p_color_scan = mp_colors + y * m_width;

        int curr_x = 0;
        while (true)
        {
            int step = ch_Min(1 + rand() % 6, m_width - curr_x);

            AnsiColor random_color = RandomColor();
            for (int x = curr_x; x < curr_x + step; x++)
                p_color_scan[x] = random_color;

            curr_x += step;
            if (curr_x == m_width)
                break;
        }
    }

    m_cells.clear();
    m_cells.resize(m_height);

    for (int y = 0; y < m_height; y++)
    {
        AnsiRow &row = m_cells[y];

        if (y % 4 == 0)
        {
            row.resize(m_width, AnsiCell());
        }
        else if (y % 4 == 1)
        {
            for (int x = 0; x < m_width / 2; x++)
                    row.push_back(AnsiCell(VERT_BLOCK, x % 9));
        }
        else if (y % 4 == 2)
        {
            for (int x = 0; x < m_width / 2; x++)
                row.push_back(AnsiCell(HORI_BLOCK, x % 9));
        }
        else
        {
            for (int x = 0; x < m_width / 2; x++)
                row.push_back(AnsiCell(TRIANGLE, x % 4));
        }
    }

    ClearActionHistory();
}

void AnsiCanvas::MakeImage(HyImage *p_image)
{
    ChAutoLock auto_lock(&m_data_lock);

    if (p_image == NULL || p_image->nChannels < 3)
        return;

    HY_ZEROIMAGE(p_image);

    int image_width = p_image->width;
    int image_height = p_image->height;
    int channels = p_image->nChannels;

    // Draw contents from (m_display_x_offset, m_display_y_offset) to (m_width, m_height).
    int safe_width = ch_Min(m_width - m_display_x_offset, image_width / m_half_block_size);
    int safe_height = ch_Min(m_height - m_display_y_offset, image_height / m_block_size);

    int draw_x = 0;
    int draw_y = 0;

    for (int y = 0; y < safe_height; y++)
    {
        int row_index = m_display_y_offset + y;

        const AnsiColor *p_color_scan = mp_colors + row_index * m_width + m_display_x_offset;
        const AnsiRow &row = m_cells[row_index];

        int color_index = -m_display_x_offset; // shifted
        int cell_index = 0;
        int cell_count = (int)row.size();

        draw_y = y * m_block_size;
        
        while (true)
        {
            if (color_index >= safe_width || cell_index >= cell_count)
                break;

            const AnsiCell &cell = row[cell_index];

            int chars = cell.GetCharCount();

            for (int c = 0; c < chars; c++)
            {
                const bool is_right = (c == 1);

                if (color_index >= 0)
                {
                    draw_x = color_index * m_half_block_size;
                    m_ansi_template.DrawHalfBlock(p_image, draw_x, draw_y, p_color_scan[color_index], cell, is_right);
                }

                color_index++;
                if (color_index >= safe_width)
                    break;
            }

            cell_index++;
        }
    }
}

void AnsiCanvas::UpdateImage(HyImage *p_image, const HyRect &roi)
{
    ChAutoLock auto_lock(&m_data_lock);

    if (p_image == NULL || p_image->nChannels < 3)
        return;

    int image_width = p_image->width;
    int image_height = p_image->height;
    int channels = p_image->nChannels;

    // Update contents inside the region from (m_display_x_offset, m_display_y_offset) to (m_width, m_height).
    int safe_width = ch_Min(m_width - m_display_x_offset, image_width / m_half_block_size);
    int safe_height = ch_Min(m_height - m_display_y_offset, image_height / m_block_size);

    int x_min = roi.x / m_half_block_size;
    int y_min = roi.y / m_block_size;
    int x_max = (roi.Right() - 1) / m_half_block_size;
    int y_max = (roi.Bottom() - 1) / m_block_size;

    x_min = ch_Min(x_min, 0);
    y_min = ch_Max(y_min, 0);
    x_max = ch_Min(x_max, safe_width - 1);
    y_max = ch_Min(y_max, safe_height - 1);

    int draw_x = 0;
    int draw_y = 0;

    for (int y = y_min; y <= y_max; y++)
    {
        int row_index = m_display_y_offset + y;

        const AnsiColor *p_color_scan = mp_colors + row_index * m_width + m_display_x_offset;
        const AnsiRow &row = m_cells[row_index];

        int color_index = -m_display_x_offset; // shifted
        int cell_index = 0;
        int cell_count = (int)row.size();

        draw_y = y * m_block_size;
        
        while (true)
        {
            if (color_index > x_max || cell_index >= cell_count)
                break;

            const AnsiCell &cell = row[cell_index];

            int chars = cell.GetCharCount();

            for (int c = 0; c < chars; c++)
            {
                const bool is_right = (c == 1);

                if (color_index >= x_min)
                {
                    draw_x = color_index * m_half_block_size;
                    m_ansi_template.DrawHalfBlock(p_image, draw_x, draw_y, p_color_scan[color_index], cell, is_right);
                }

                color_index++;
                if (color_index > x_max)
                    break;
            }

            cell_index++;
        }
    }
}

void AnsiCanvas::MakeWriteRowAndColors(const AnsiRow &src_row, const AnsiColor *p_src_row_colors,
                                       AnsiRow &dst_row, AnsiColor *p_dst_row_colors, int max_width)
{
    dst_row = src_row;
    memcpy(p_dst_row_colors, p_src_row_colors, sizeof(AnsiColor) * m_width);

    if (max_width <= 0 || max_width >= m_width)
        return;

    // color: just set to black after max_width
    for (int i = max_width; i < m_width; i++)
        p_dst_row_colors[i] = AnsiColor(ANSI_BLACK, ANSI_BLACK);

    // cell: keep the last cell at offset max_width - 1
    int cell_offset;
    AnsiCharLocation location;
    if (SearchCellInfo(dst_row, max_width - 1, cell_offset, location))
    {
        int cell_count = (int)dst_row.size();
        if (cell_count > cell_offset + 1)
            dst_row.erase(dst_row.begin() + (cell_offset + 1), dst_row.end());
    }
}

void AnsiCanvas::SaveAnsi(LPCTSTR path, int max_width/* = UNLIMITED_SIZE*/)
{
    ChAutoLock auto_lock(&m_data_lock);

    if (path == NULL)
        return;

    FILE *p_file = _tfopen(path, _T("wb"));
    if (p_file == NULL)
        return;

    // Ignore invalid rows from the bottom.
    int valid_height = 0;
    for (int y = m_height - 1; y >= 0; y--)
    {
        if (IsValidRow(m_cells[y], mp_colors + y * m_width, m_width))
        {
            valid_height = y + 1;
            break;
        }
    }

    AnsiRow write_row;
    ChAutoPtr<AnsiColor> write_colors(m_width);
    for (int y = 0; y < valid_height; y++)
    {
        MakeWriteRowAndColors(m_cells[y], mp_colors + y * m_width,
                              write_row, write_colors, max_width);

        AnsiColor old_color;
        old_color.fg_color = ANSI_DEFAULT_FOREGROUND;
        old_color.bg_color = ANSI_DEFAULT_BACKGROUND;

        int color_index = 0;
        int cell_index = 0;
        int cell_count = (int)write_row.size();

        while (true)
        {
            if (cell_index >= cell_count)
                break;

            const AnsiCell &cell = write_row[cell_index];
            int char_count = cell.GetCharCount();

            if (color_index + char_count > m_width)
                break;

            BYTE character[2];
            cell.GetCharacter(character);

            for (int c = 0; c < char_count; c++)
            {
                BYTE curr_char = character[c];
                const AnsiColor &curr_color = write_colors[color_index];

                bool is_different_color = (curr_color.fg_color != old_color.fg_color || curr_color.bg_color != old_color.bg_color);
                if (is_different_color)
                {
                    AddColorCommand(p_file, curr_color, old_color);
                    old_color = curr_color;
                }

                fwrite(character + c, sizeof(BYTE), 1, p_file);

                color_index++;
            }

            cell_index++;
        }

        EndLine(p_file);
    }

    fclose(p_file);
}

bool AnsiCanvas::LoadAnsi(LPCTSTR path)
{
    ChAutoLock auto_lock(&m_data_lock);

    m_display_x_offset = 0;
    m_display_y_offset = 0;

    bool is_valid = false;
    FILE *p_file = NULL;
    if (path != NULL)
    {
        p_file = _tfopen(path, _T("rb"));
        is_valid = (p_file != NULL);
    }

    if (is_valid == false)
    {
        m_width = MIN_CANVAS_WIDTH;
        m_height = MIN_CANVAS_HEIGHT;
        _NEW_PTRS(mp_colors, AnsiColor, m_width * m_height);
        _NEW_PTRS(mp_old_colors, AnsiColor, m_width * m_height);
        _NEW_PTRS(mp_small_square_mask, BYTE, m_width * m_height * 2);
        ClearCanvas();
        ClearActionHistory();
        return false;
    }

    std::vector<AnsiRow> read_cell_rows;
    std::vector<std::vector<AnsiColor > > read_color_rows;

    const int max_read_line_size = 2048;
    ChAutoPtr<char> line_buffer(max_read_line_size);
    ChAutoPtr<char> line_data(max_read_line_size + 1);
    while (fgets(line_buffer, max_read_line_size, p_file))
    {
        read_color_rows.push_back(std::vector<AnsiColor>());
        std::vector<AnsiColor> &color_row = read_color_rows.back();

        const char *p_line_start = (const char *)line_buffer;
        const char *p_line_scan = p_line_start;
        int x = 0;
        AnsiColor curr_color;
        while (true)
        {
            if (p_line_scan - p_line_start >= max_read_line_size)
                break;
            if (IsLineEnd(p_line_scan))
                break;

            if (IsAnsiCommand(p_line_scan))
            {
                ReadAnsiCommand(p_line_scan, curr_color);
            }
            else
            {
                line_data[x] = p_line_scan[0];
                color_row.push_back(curr_color);

                x++;
                p_line_scan++;
            }
        }

        line_data[x] = '\0';

        read_cell_rows.push_back(AnsiRow());
        LineDataToAnsiRow(line_data, read_cell_rows.back());
    }

    fclose(p_file);

    TruncateInvalidRows(read_cell_rows, read_color_rows);
    InitAnsiCanvas(read_cell_rows, read_color_rows);

    ClearActionHistory();

    return true;
}

void AnsiCanvas::TruncateInvalidRows(std::vector<AnsiRow> &cell_rows, std::vector<std::vector<AnsiColor> > &color_rows)
{
    int valid_row_count = ch_Min(cell_rows.size(), color_rows.size());
    if ((int)cell_rows.size() > valid_row_count)
        cell_rows.erase(cell_rows.begin() + valid_row_count, cell_rows.end());
    if ((int)color_rows.size() > valid_row_count)
        color_rows.erase(color_rows.begin() + valid_row_count, color_rows.end());

    for (int i = valid_row_count - 1; i >= 0; i--)
    {
        AnsiRow &cell_row = cell_rows[i];
        std::vector<AnsiColor> &color_row = color_rows[i];

        bool is_valid = IsValidRow(cell_row, color_row);
        if (is_valid)
        {
            break;
        }
        else
        {
            cell_rows.erase(cell_rows.begin() + i);
            color_rows.erase(color_rows.begin() + i);
        }
    }
}

bool AnsiCanvas::IsValidRow(const AnsiRow &cell_row, const std::vector<AnsiColor> &color_row)
{
    if (cell_row.empty() || color_row.empty())
    {
        return false;
    }
    else
    {
        for (int i = 0; i < (int)cell_row.size(); i++)
        {
            if (cell_row[i].type != EMPTY)
                return true;
        }

        for (int i = 0; i < (int)color_row.size(); i++)
        {
            if (color_row[i].bg_color != ANSI_DEFAULT_BACKGROUND)
                return true;
        }
    }

    return false;
}

bool AnsiCanvas::IsValidRow(const AnsiRow &cell_row, const AnsiColor *p_color_row, int color_row_size)
{
    std::vector<AnsiColor> color_row;
    if (p_color_row != NULL && color_row_size > 0)
        color_row.assign(p_color_row, p_color_row + color_row_size);

    return IsValidRow(cell_row, color_row);
}

void AnsiCanvas::InitAnsiCanvas(const std::vector<AnsiRow> &cell_rows, const std::vector<std::vector<AnsiColor> > &color_rows)
{
    int read_row_count = ch_Min((int)cell_rows.size(), (int)color_rows.size());

    m_width = MIN_CANVAS_WIDTH;
    m_height = ch_Max(read_row_count,  MIN_CANVAS_HEIGHT);

    _NEW_PTRS(mp_colors, AnsiColor, m_width * m_height);
    _NEW_PTRS(mp_old_colors, AnsiColor, m_width * m_height);
    _NEW_PTRS(mp_small_square_mask, BYTE, m_width * m_height * 2);

    m_cells.resize(m_height);
    for (int y = 0; y < read_row_count; y++)
    {
        const AnsiRow &src_cell_row = cell_rows[y];
        const std::vector<AnsiColor> &src_color_row = color_rows[y];

        AnsiRow &dst_cell_row = m_cells[y];
        AnsiColor *p_dst_color_row = mp_colors + y * m_width;

        dst_cell_row.clear();

        int cell_offset = 0;
        int x_offset = 0;
        while (true)
        {
            if (cell_offset >= (int)src_cell_row.size())
                break;

            const AnsiCell &src_cell = src_cell_row[cell_offset];
            int char_count = src_cell.GetCharCount();

            if (x_offset + char_count > ch_Min((int)src_color_row.size(), m_width))
                break;

            dst_cell_row.push_back(src_cell);
            cell_offset++;
            for (int i = 0; i < char_count; i++)
            {
                p_dst_color_row[x_offset] = src_color_row[x_offset];
                x_offset++;
            }
        }

        for (int x = x_offset; x < m_width; x++)
        {
            dst_cell_row.push_back(AnsiCell(EMPTY));
            p_dst_color_row[x] = AnsiColor();
        }
    }

    for (int y = read_row_count; y < m_height; y++)
    {
        m_cells[y].resize(m_width);
        AnsiColor *p_color_row = mp_colors + y * m_width;
        for (int x = 0; x < m_width; x++)
        {
            m_cells[y][x] = AnsiCell(EMPTY);
            p_color_row[x] = AnsiColor();
        }
    }
}

HySize AnsiCanvas::GetDisplayUnitSize()
{
    return hySize(MIN_CANVAS_WIDTH, MIN_CANVAS_HEIGHT); 
}

void AnsiCanvas::GetDisplayOffset(int &x_offset, int &y_offset)
{
    x_offset = m_display_x_offset;
    y_offset = m_display_y_offset;
}

bool AnsiCanvas::MoveDisplayOffset(int x_shift, int y_shift, bool &is_canvas_changed)
{
    ChAutoLock auto_lock(&m_data_lock);

    is_canvas_changed = false;

    // x movement is not implemented yet.
    int new_x_offset = m_display_x_offset;
    int new_y_offset = ch_Max(m_display_y_offset + y_shift, 0);

    if (m_display_x_offset == new_x_offset && m_display_y_offset == new_y_offset)
        return false;

    m_display_x_offset = new_x_offset;
    m_display_y_offset = new_y_offset;

    int min_height = m_display_y_offset + MIN_CANVAS_HEIGHT;
    if (min_height > m_height)
    {
        AnsiColor *p_new_colors = new AnsiColor[m_width * min_height];
        memcpy(p_new_colors, mp_colors, sizeof(AnsiColor) * m_width * m_height);
        _DELETE_PTRS(mp_colors);
        mp_colors = p_new_colors;

        for (int y = m_height; y < min_height; y++)
        {
            m_cells.push_back(AnsiRow());
            AnsiRow &new_row = m_cells.back();
            new_row.resize(m_width);
            AnsiColor *p_row_color = mp_colors + y * m_width;

            for (int x = 0; x < m_width; x++)
            {
                new_row[x] = AnsiCell(EMPTY);
                p_row_color[x] = AnsiColor();
            }
        }

        int old_canvas_height = m_height;
        m_height = min_height;

        _NEW_PTRS(mp_small_square_mask, BYTE, m_width * m_height * 2);

        ActionInfo action_info(AC_EDIT_EXTEND_LINE);
        action_info.old_canvas_height = old_canvas_height;
        RecordAction(action_info);

        is_canvas_changed = true;
    }

    return true;
}

void AnsiCanvas::ResetDisplayOffset()
{
    m_display_x_offset = 0;
    m_display_y_offset = 0;
}

void AnsiCanvas::AddColorCommand(FILE *p_file, const AnsiColor &curr_color, const AnsiColor &old_color)
{
    bool is_bright = IsBrightColor(curr_color.fg_color);
    bool is_change_brightbess = (is_bright != IsBrightColor(old_color.fg_color));

    char fg_color_char = ColorChar(curr_color.fg_color);
    char bg_color_char = ColorChar(curr_color.bg_color);
    char old_fg_color_char = ColorChar(old_color.fg_color);
    char old_bg_color_char = ColorChar(old_color.bg_color);
    bool is_change_fg_color = (fg_color_char != old_fg_color_char);
    bool is_change_bg_color = (bg_color_char != old_bg_color_char);
    bool is_change_color = is_change_fg_color || is_change_bg_color;

    // "0" will reset the foreground/background color to white/black
    bool is_reset_color = (is_change_brightbess && is_bright == false);
    if (is_reset_color)
    {
        if (fg_color_char != '7')
            is_change_fg_color = true;
        if (bg_color_char != '0')
            is_change_bg_color = true;
    }

    bool is_default_color = (is_bright == false && fg_color_char == '7' && bg_color_char == '0');

    StartAnsiCommand(p_file);

    std::string format;

    bool is_need_comma = false;

    if (is_default_color)
    {
        // empty format
    }
    else
    {
        if (is_change_brightbess)
        {
            if (is_bright)
                format = "1";
            else
                format = "0";

            is_need_comma = true;
        }

        if (is_change_fg_color)
        {
            if (is_need_comma)
            {
                format += ";";
                is_need_comma = false;
            }

            format += "3";
            format += fg_color_char;

            is_need_comma = true;
        }

        if (is_change_bg_color)
        {
            if (is_need_comma)
            {
                format += ";";
                is_need_comma = false;
            }

            format += "4";
            format += bg_color_char;
        }
    }

    if (format.length() > 0)
        fwrite(format.c_str(), sizeof(char), format.length(), p_file);

    EndAnsiCommand(p_file);
}

void AnsiCanvas::ReadAnsiCommand(const char *&p_line, AnsiColor &curr_color)
{
    const int max_length = 10; // *[1;37;40m

    int end_offset = 2;
    for (int i = 2; i < max_length; i++)
    {
        if (p_line[i] == 0x6d)
        {
            end_offset = i;
            break;
        }
    }

    char content[16];
    int content_length = end_offset - 2;
    for (int i = 0; i < content_length; i++)
        content[i] = p_line[i + 2];

    content[content_length] = '\0';

    if (content_length == 0)
    {
        curr_color = AnsiColor();
    } 
    else
    {
        int offset = 0;
        while (offset < content_length)
        {
            int split_offset = content_length;
            for (int i = offset; i < content_length; i++)
            {
                if (content[i] == ';')
                {
                    split_offset = i;
                    break;
                }
            }

            int segment_size = split_offset - offset;
            if (segment_size > 0)
            {
                char command_char = content[offset];
                if (command_char == '0')
                {
                    curr_color.fg_color = ToNormalColor(curr_color.fg_color);
                }
                else if (command_char == '1')
                {
                    curr_color.fg_color = ToBrightColor(curr_color.fg_color);
                }
                else if (command_char == '3')
                {
                    int new_fg_color = ANSI_DEFAULT_FOREGROUND;
                    if (segment_size > 1)
                    {
                        int color_label = ColorLabel(content[offset + 1]);
                        new_fg_color = ColorValue(color_label, IsBrightColor(curr_color.fg_color));
                    }
                    
                    curr_color.fg_color = new_fg_color;
                }
                else if (command_char == '4')
                {
                    int new_bg_color = ANSI_DEFAULT_BACKGROUND;
                    if (segment_size > 1)
                    {
                        int color_label = ColorLabel(content[offset + 1]);
                        new_bg_color = ColorValue(color_label, false);
                    }

                    curr_color.bg_color = new_bg_color;
                }
                else if (command_char == '7')
                {
                    bool is_bright = IsBrightColor(curr_color.fg_color);
                    int fg_label = ColorLabel(ColorChar(curr_color.fg_color));
                    int bg_label = ColorLabel(ColorChar(curr_color.bg_color));

                    curr_color.fg_color = ColorValue(bg_label, is_bright);
                    curr_color.bg_color = ColorValue(fg_label, false);
                }
            }

            offset = split_offset + 1;
        }
    }

    p_line += (end_offset + 1);
}

void AnsiCanvas::LineDataToAnsiRow(const char *p_ansi_string, AnsiRow &row)
{
    // Assume p_ansi_string is NULL-terminated.

    std::wstring unicode_string = MyAnsiToUnicode(p_ansi_string);
    int string_length = unicode_string.length();

    row.clear();
    if (string_length == 0)
        return;

    int offset = 0;
    for (int i = 0; i < string_length; i++)
    {
        int valid_range = ch_Min(m_width - offset, 2);
        if (valid_range <= 0)
            break;

        AnsiCell cell = CharToAnsiCell(unicode_string.at(i), valid_range);

        offset += cell.GetCharCount();
        row.push_back(cell);
    }
}

bool AnsiCanvas::SearchCellInfo(const AnsiRow &row, int x, int &cell_offset, AnsiCharLocation &char_location)
{
    int x_search = 0;
    for (int i = 0; i < (int)row.size(); i++)
    {
        int chars = row[i].GetCharCount();

        if (x_search == x)
        {
            cell_offset = i;
            if (chars == 1)
                char_location = SINGLE_CHAR;
            else
                char_location = DOUBLE_CHAR_LEFT;

            return true;
        }
        else if (x_search == x - 1 && chars == 2)
        {
            cell_offset = i;
            char_location = DOUBLE_CHAR_RIGHT;

            return true;
        }

        x_search += chars;
    }

    return false;
}

bool AnsiCanvas::GetCharLocation(const AnsiRow &row, int x, AnsiCharLocation &char_location)
{
    // Used when the caller doesn't need cell offset.

    int cell_offset;
    return SearchCellInfo(row, x, cell_offset, char_location);
}

bool AnsiCanvas::GetXYOffset(int x, int y, int &x_offset, int &y_offset)
{
    y_offset = m_display_y_offset + FloorDivision(y, m_block_size);
    if (y_offset < 0 || y_offset >= m_height)
        return false;

    x_offset = m_display_x_offset + FloorDivision(x, m_half_block_size);
    if (x_offset < 0 || x_offset >= m_width)
        return false;

    return true;
}

bool AnsiCanvas::GetBlockXYOffset(int x, int y, int &x_offset, int &y_offset)
{
    if (GetXYOffset(x, y, x_offset, y_offset) == false)
        return false;

    // Adjust the x offset to be best fit the location.
    int shift = x - ToCoordX(x_offset);
    if (shift <= m_half_block_size / 2)
        x_offset--;
    x_offset = FitInRange(x_offset, 0, m_width - 2);

    return true;
}

void AnsiCanvas::GetLineStepParameters(const HyPoint &start_pt, const HyPoint &end_pt, 
                                       float &step_x, float &step_y, int &step_count)
{
    const float safe_ratio = 0.4f;
    float max_unit = (float)m_block_size * safe_ratio;

    float start_x = (float)start_pt.x;
    float start_y = (float)start_pt.y;
    float move_x = (float)end_pt.x - start_x;
    float move_y = (float)end_pt.y - start_y;

    float abs_move_x = ch_Abs(move_x);
    float abs_move_y = ch_Abs(move_y);
    if (abs_move_x >= abs_move_y)
    {
        step_count = ch_Round(floorf(abs_move_x / max_unit));
        if (step_count > 0)
        {
            step_x = (move_x > 0.0f ? max_unit : -max_unit);
            step_y = move_y * (step_x / move_x);
        }
    }
    else
    {
        step_count = ch_Round(floorf(abs_move_y / max_unit));
        if (step_count > 0)
        {
            step_y = (move_y > 0.0f ? max_unit : -max_unit);
            step_x = move_x * (step_y / move_y);
        }
    }
}

bool AnsiCanvas::SetCanvasColor(int x, int y, const AnsiColor &color, HyRect &change_rect)
{
    int x_offset, y_offset;
    if (GetXYOffset(x, y, x_offset, y_offset) == false)
        return false;

    int cell_offset;
    AnsiCharLocation location;
    bool is_success = SearchCellInfo(m_cells[y_offset], x_offset, cell_offset, location);
    if (is_success == false)
        return false;

    AnsiCell &cell = m_cells[y_offset][cell_offset];
    bool is_foreground_only = (cell.type != GENERAL_CHAR);

    if (is_foreground_only)
    {
        // Set the foreground color to partial region according to the coordinate and cell tpe.

        int new_color = color.fg_color;

        int x_shift = x - ToCoordX(x_offset);
        int y_shift = y - ToCoordY(y_offset);
        SetSingleColorInCell(x_offset, y_offset,
                             cell, color.fg_color,
                             x_shift, y_shift, location);
    }
    else
    {
        // Set the foreground/background color at this location.
        CanvasColorAt(x_offset, y_offset) = color;
    }

    change_rect.x = ToCoordX(x_offset);
    change_rect.y = ToCoordY(y_offset);
    change_rect.width = m_half_block_size;
    change_rect.height = m_block_size;

    return true;
}

void AnsiCanvas::DrawSpaces(const HyPoint &start_pt, const HyPoint &end_pt,
                            int color, bool is_continuous, HyRect &change_rect, bool &is_action_list_changed)
{
    change_rect = hyRect(0, 0, 0, 0);
    is_action_list_changed = false;

    float step_x = 0.0f;
    float step_y = 0.0f;
    int step_count = 0;
    GetLineStepParameters(start_pt, end_pt, step_x, step_y, step_count);

    int x_offset = 0;
    int y_offset = 0;

    bool is_valid = true;
    float curr_x = (float)start_pt.x;
    float curr_y = (float)start_pt.y;
    HyRect temp_rect;
    for (int i = 0; i <= step_count; i++)
    {
        is_valid = GetXYOffset(ch_Round(curr_x), ch_Round(curr_y), x_offset, y_offset);
        if (is_valid)
        {
            WriteSpace(x_offset, y_offset, color, temp_rect);
            change_rect = UnionRect(change_rect, temp_rect);
        }

        curr_x += step_x;
        curr_y += step_y;
    }

    is_valid = GetXYOffset(end_pt.x, end_pt.y, x_offset, y_offset);
    if (is_valid)
    {
        WriteSpace(x_offset, y_offset, color, temp_rect);
        change_rect = UnionRect(change_rect, temp_rect);
    }

    ActionInfo action_info(AC_EDIT_DRAW_SPACE, change_rect);
    action_info.is_continuous = is_continuous;
    is_action_list_changed = RecordAction(action_info);
}

void AnsiCanvas::DrawLargeBrush(const HyPoint &start_pt, const HyPoint &end_pt,
                                int brush_level, int brush_shape,
                                int color, bool is_continuous, HyRect &change_rect, bool &is_action_list_changed)
{
    change_rect = hyRect(0, 0, 0, 0);
    is_action_list_changed = false;

    float step_x = 0.0f;
    float step_y = 0.0f;
    int step_count = 0;
    GetLineStepParameters(start_pt, end_pt, step_x, step_y, step_count);

    ZeroMemory(mp_small_square_mask, m_width * m_height * 2);

    HyRect unbounded_offset_range;
    HyRect offset_range;
    int x_offset_range, y_offset_range_2x;
    HyImage *p_brush_mask = NULL;

    float curr_x = (float)start_pt.x;
    float curr_y = (float)start_pt.y;
    for (int i = 0; i <= step_count; i++)
    {
        GetLargeBrushOffsetRange(ch_Round(curr_x), ch_Round(curr_y),
                                 brush_level, brush_shape,
                                 offset_range, unbounded_offset_range);

        x_offset_range = offset_range.width;
        y_offset_range_2x = offset_range.height;
        p_brush_mask = hyCreateImage(hySize(x_offset_range, y_offset_range_2x), HY_DEPTH_8U, 1);
        if (GetBrushMask(offset_range, unbounded_offset_range, brush_shape, p_brush_mask))
        {
            for (int y = 0; y < y_offset_range_2x; y++)
            {
                const BYTE *p_brush_scan = p_brush_mask->imageData + y * p_brush_mask->widthStep;
                BYTE *p_mask_scan = mp_small_square_mask + (offset_range.y + y) * m_width;

                for (int x = 0; x < x_offset_range; x++)
                {
                    if (p_brush_scan[x] == 255)
                        p_mask_scan[offset_range.x + x] = 255;
                }
            }
        }

        hyReleaseImage(&p_brush_mask);

        curr_x += step_x;
        curr_y += step_y;
    }

    GetLargeBrushOffsetRange(end_pt.x, end_pt.y,
                             brush_level, brush_shape,
                             offset_range, unbounded_offset_range);

    x_offset_range = offset_range.width;
    y_offset_range_2x = offset_range.height;
    p_brush_mask = hyCreateImage(hySize(x_offset_range, y_offset_range_2x), HY_DEPTH_8U, 1);
    if (GetBrushMask(offset_range, unbounded_offset_range, brush_shape, p_brush_mask))
    {
        for (int y = 0; y < y_offset_range_2x; y++)
        {
            const BYTE *p_brush_scan = p_brush_mask->imageData + y * p_brush_mask->widthStep;
            BYTE *p_mask_scan = mp_small_square_mask + (offset_range.y + y) * m_width;

            for (int x = 0; x < x_offset_range; x++)
            {
                if (p_brush_scan[x] == 255)
                    p_mask_scan[offset_range.x + x] = 255;
            }
        }
    }

    hyReleaseImage(&p_brush_mask);

    HyRect temp_rect;
    for (int line_offset = 0; line_offset < m_height; line_offset++)
    {
        const BYTE *p_mask_scan1 = mp_small_square_mask + (line_offset * 2) * m_width;
        const BYTE *p_mask_scan2 = p_mask_scan1 + m_width;

        int x_offset_min = m_width;
        int x_offset_max = -1;
        for (int x_offset = 0; x_offset < m_width; x_offset++)
        {
            if (p_mask_scan1[x_offset] == 255 || p_mask_scan2[x_offset] == 255)
            {
                x_offset_min = ch_Min(x_offset_min, x_offset);
                x_offset_max = ch_Max(x_offset_max, x_offset);
            }
        }

        if (x_offset_min > x_offset_max)
            continue;

        WriteSmallSquaresAtLine(mp_small_square_mask, line_offset,
                                x_offset_min, x_offset_max, ToNormalColor(color), temp_rect);

        change_rect = UnionRect(change_rect, temp_rect);
    }

    ActionInfo action_info(AC_EDIT_DRAW_LARGE_BRUSH, change_rect);
    action_info.is_continuous = is_continuous;
    is_action_list_changed = RecordAction(action_info);
}

void AnsiCanvas::WriteSmallSquaresAtLine(const BYTE *p_mask, int y_offset,
                                         int x_offset_min, int x_offset_max,
                                         int color, HyRect &change_rect)
{
    change_rect = hyRect(0, 0, 0, 0);

    const BYTE *p_mask_scan1 = p_mask + (y_offset * 2) * m_width;
    const BYTE *p_mask_scan2 = p_mask_scan1 + m_width;

    ChAutoPtr<bool> is_can_write_spaces(m_width);
    for (int x = 0; x < m_width; x++)
        is_can_write_spaces[x] = (p_mask_scan1[x] == 255 && p_mask_scan2[x] == 255);

    const AnsiRow &row = m_cells[y_offset];

    AnsiCharLocation char_location;
    int space_color;

    // Determine the placing of outmost full-size block.
    bool is_left_inner_ok = false;
    bool is_left_outer_ok = true;
    if (x_offset_min < m_width - 1)
    {
        GetCharLocation(row, x_offset_min, char_location);
        if (char_location == DOUBLE_CHAR_RIGHT)
        {
            is_left_inner_ok = false;
            is_left_outer_ok = true;

            if (GetPureSpaceColor(x_offset_min - 1, y_offset, space_color))
                if (IsNormalColor(space_color))
                    is_left_inner_ok = true;
        }
        else
        {
            is_left_inner_ok = true;
            is_left_outer_ok = false;

            int cell_offset;
            if (SearchCellInfo(row, x_offset_min - 1, cell_offset, char_location))
                if (row[cell_offset].type == EMPTY)
                    is_left_outer_ok = true;
        }
    }

    bool is_right_inner_ok = false;
    bool is_right_outer_ok = true;
    if (x_offset_max > 0)
    {
        GetCharLocation(row, x_offset_max, char_location);
        if (char_location == DOUBLE_CHAR_LEFT)
        {
            is_right_inner_ok = false;
            is_right_outer_ok = true;

            if (GetPureSpaceColor(x_offset_max + 1, y_offset, space_color))
                if (IsNormalColor(space_color))
                    is_right_inner_ok = true;
        }
        else
        {
            is_right_inner_ok = true;
            is_right_outer_ok = false;

            int cell_offset;
            if (SearchCellInfo(row, x_offset_max + 1, cell_offset, char_location))
                if (row[cell_offset].type == EMPTY)
                    is_right_outer_ok = true;
        }
    }

    int left_block_start = x_offset_min;
    bool is_can_left_move_out = false;
    if (is_left_inner_ok)
        is_can_left_move_out = is_left_outer_ok;
    else
        left_block_start--;

    int right_block_start = x_offset_max - 1;
    bool is_can_right_move_out = false;
    if (is_right_inner_ok)
        is_can_right_move_out = is_right_outer_ok;
    else
        right_block_start++;

    // Determine a list of blocks/spaces to fill the inner contents.
    std::vector<int> cell_steps;
    
    if (left_block_start > right_block_start)
    {
        // This happens only when x_offset_min = x_offset_max.

        if (is_can_write_spaces[x_offset_min])
        {
            cell_steps.push_back(x_offset_min);
            cell_steps.push_back(x_offset_min + 1);
        }
        else
        {
            int valid_start;
            if (is_can_right_move_out)
            {
                valid_start = left_block_start;
            }
            else if (is_can_left_move_out)
            {
                valid_start = right_block_start;
            }
            else
            {
                if (x_offset_min == m_width - 1)
                    valid_start = right_block_start;
                else
                    valid_start = left_block_start;
            }

            cell_steps.push_back(valid_start);
            cell_steps.push_back(valid_start + 2);
        }
    }
    else if ((right_block_start - left_block_start) % 2 == 0)
    {
        // Simple case. Put the cell steps evenly.

        for (int x = left_block_start; x <= right_block_start + 2; x += 2)
            cell_steps.push_back(x);
    }
    else
    {
        // 1. Try to insert a space according to drawing contents.
        // 2. If fail, try to move left or right outward.

        int put_space_offset = -1;
        for (int x = left_block_start; x <= right_block_start + 1; x += 2)
        {
            if (is_can_write_spaces[x])
            {
                put_space_offset = x;
                break;
            }
        }

        if (put_space_offset != -1)
        {
            for (int x = left_block_start; x <= put_space_offset; x += 2)
                cell_steps.push_back(x);

            for (int x = put_space_offset + 1; x <= right_block_start + 2; x += 2)
                cell_steps.push_back(x);
        }
        else
        {
            if (is_can_right_move_out)
            {
                right_block_start++;
            }
            else if (is_can_left_move_out)
            {
                left_block_start--;
            }
            else
            {
                // Force move one offset if possible.
                if (right_block_start == x_offset_max && x_offset_max > 0)
                {
                    right_block_start = x_offset_max - 1;
                }
                else if (right_block_start == x_offset_max - 1 && x_offset_max < m_width - 1)
                {
                    right_block_start = x_offset_max;
                }
                else if (left_block_start == x_offset_min && x_offset_min > 0)
                {
                    left_block_start = x_offset_min - 1;
                }
                else if (left_block_start == x_offset_min - 1 && x_offset_min < m_width - 1)
                {
                    left_block_start = x_offset_min;
                }
                else
                {
                    // Extreme case. Cannot move at all and the write range is still odd.
                    // Decrease the right offset by one to discard the rightmost result.
                    // (This case should not happen under normal brush design.)
                    right_block_start--;
                }
            }

            for (int x = left_block_start; x <= right_block_start + 2; x += 2)
                cell_steps.push_back(x);
        }       
    }

    int cell_count = (int)cell_steps.size() - 1;
    if (cell_count <= 0)
        return;

    std::vector<AnsiCell> cells;
    std::vector<AnsiColor> colors;

    for (int i = 0; i < cell_count; i++)
    {
        int x_start = cell_steps[i];
        int char_count = cell_steps[i + 1] - x_start;

        if (char_count == 1)
        {
            cells.push_back(AnsiCell(EMPTY));
            colors.push_back(AnsiColor(ANSI_DEFAULT_FOREGROUND, color));
        }
        else
        {
            cells.push_back(AnsiCell(HORI_BLOCK, 4));

            int square_color;

            AnsiColor left_color(color, color);
            if (p_mask_scan1[x_start] == 0)
                if (GetSmallSquareColor(x_start, y_offset * 2, square_color))
                    left_color.bg_color = ToNormalColor(square_color);
            if (p_mask_scan2[x_start] == 0)
                if (GetSmallSquareColor(x_start, y_offset * 2 + 1, square_color))
                    left_color.fg_color = square_color;

            colors.push_back(left_color);

            AnsiColor right_color(color, color);
            if (p_mask_scan1[x_start + 1] == 0)
                if (GetSmallSquareColor(x_start + 1, y_offset * 2, square_color))
                    right_color.bg_color = ToNormalColor(square_color);
            if (p_mask_scan2[x_start + 1] == 0)
                if (GetSmallSquareColor(x_start + 1, y_offset * 2 + 1, square_color))
                    right_color.fg_color = square_color;
            colors.push_back(right_color);
        }
    }
    
    HyRect write_rect;
    WriteRegion(cell_steps[0], y_offset, cells, colors, write_rect, true);

    change_rect = UnionRect(change_rect, write_rect);
}

void AnsiCanvas::InsertSpace(const HyPoint &point, int color, HyRect &change_rect)
{
    ChAutoLock auto_lock(&m_data_lock);

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(point.x, point.y, x_offset, y_offset) == false)
        return;

    AnsiRow &row = m_cells[y_offset];

    int cell_offset;
    AnsiCharLocation location;
    bool is_success = SearchCellInfo(row, x_offset, cell_offset, location);
    if (is_success == false)
        return;

    AnsiCell space_cell(EMPTY);

    int redraw_offset = 0;
    
    AnsiColor *p_color_row = mp_colors + y_offset * m_width;

    if (location == DOUBLE_CHAR_RIGHT)
    {
        // Break a block by a new space.

        int left_space_color, right_space_color;
        if (GetPureSpaceColor(p_color_row[x_offset - 1], row[cell_offset], false, left_space_color) == false)
            left_space_color = p_color_row[x_offset - 1].bg_color;
        if (GetPureSpaceColor(p_color_row[x_offset], row[cell_offset], true, right_space_color) == false)
            right_space_color = p_color_row[x_offset].bg_color;

        for (int x = m_width - 1; x >= x_offset + 2; x--)
            p_color_row[x] = p_color_row[x - 1];

        p_color_row[x_offset - 1] = SpaceColor(left_space_color);
        p_color_row[x_offset] = SpaceColor(color);
        p_color_row[x_offset + 1] = SpaceColor(right_space_color);

        row[cell_offset] = space_cell;
        row.insert(row.begin() + cell_offset, space_cell);
        row.insert(row.begin() + cell_offset, space_cell);

        redraw_offset = -1;
    }
    else
    {
        // Insert a new space without breaking any block.

        for (int x = m_width - 1; x >= x_offset + 1; x--)
            p_color_row[x] = p_color_row[x - 1];

        p_color_row[x_offset] = SpaceColor(color);

        row.insert(row.begin() + cell_offset, space_cell);
    }

    // Truncate the last space in the row.
    // If it is a block, set the left part with appropriate color.
    AnsiCell &last_cell = row.back();
    if (last_cell.IsDoubleChar())
    {
        int left_space_color;
        if (GetPureSpaceColor(p_color_row[m_width - 1], last_cell, false, left_space_color) == false)
            left_space_color = p_color_row[m_width - 1].bg_color;

        last_cell = space_cell;
        p_color_row[m_width - 1] = SpaceColor(left_space_color);
    }
    else
    {
        row.pop_back();
    }

    // Set change_rect to the line region starting from the modified location.
    // It may be larger than the display size. The caller should handle it.
    change_rect.x = ToCoordX(x_offset + redraw_offset);
    change_rect.y = ToCoordY(y_offset);
    change_rect.width = (m_width - m_display_x_offset) * m_half_block_size - change_rect.x;
    change_rect.height = m_block_size;

    RecordAction(ActionInfo(AC_EDIT_INSERT_DELETE_SPACE, change_rect));
}

void AnsiCanvas::DeleteSpace(const HyPoint &point, HyRect &change_rect)
{
    ChAutoLock auto_lock(&m_data_lock);

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(point.x, point.y, x_offset, y_offset) == false)
        return;

    AnsiRow &row = m_cells[y_offset];

    int cell_offset;
    AnsiCharLocation location;
    bool is_success = SearchCellInfo(row, x_offset, cell_offset, location);
    if (is_success == false)
        return;

    AnsiCell space_cell(EMPTY);

    int redraw_offset = 0;
    
    AnsiColor *p_color_row = mp_colors + y_offset * m_width;

    if (location == DOUBLE_CHAR_LEFT)
    {
        // Replace the right half block by a space.

        int right_space_color;
        if (GetPureSpaceColor(p_color_row[x_offset + 1], row[cell_offset], true, right_space_color) == false)
            right_space_color = p_color_row[x_offset + 1].bg_color;

        for (int x = x_offset + 1; x < m_width - 1; x++)
            p_color_row[x] = p_color_row[x + 1];

        p_color_row[x_offset] = SpaceColor(right_space_color);

        row[cell_offset] = space_cell;
    }
    else if (location == DOUBLE_CHAR_RIGHT)
    {
        // Replace the left half block by a space.

        int left_space_color;
        if (GetPureSpaceColor(p_color_row[x_offset - 1], row[cell_offset], false, left_space_color) == false)
            left_space_color = p_color_row[x_offset - 1].bg_color;

        for (int x = x_offset; x < m_width - 1; x++)
            p_color_row[x] = p_color_row[x + 1];

        p_color_row[x_offset - 1] = SpaceColor(left_space_color);

        row[cell_offset] = space_cell;

        redraw_offset = -1;
    }
    else
    {
        // Just delete the current space.

        for (int x = x_offset; x < m_width - 1; x++)
            p_color_row[x] = p_color_row[x + 1];

        row.erase(row.begin() + cell_offset);
    }

    // Add a space at the right end of this row.
    // Set its color according to its left neighbor.
    int new_space_color;
    if (GetPureSpaceColor(m_width - 2, y_offset, new_space_color) == false)
        new_space_color = p_color_row[m_width - 2].bg_color;

    row.push_back(space_cell);
    p_color_row[m_width - 1] = SpaceColor(new_space_color);

    // Set change_rect to the line region starting from the modified location.
    // It may be larger than the display size. The caller should handle it.
    change_rect.x = ToCoordX(x_offset + redraw_offset);
    change_rect.y = ToCoordY(y_offset);
    change_rect.width = (m_width - m_display_x_offset) * m_half_block_size - change_rect.x;
    change_rect.height = m_block_size;

    ActionInfo action_info(AC_EDIT_INSERT_DELETE_SPACE, change_rect);
    action_info.is_alternated_edit = true;
    RecordAction(action_info);
}

void AnsiCanvas::DrawBlock(const HyPoint &point, const AnsiCell &cell, const AnsiColor &color, HyRect &change_rect)
{
    ChAutoLock auto_lock(&m_data_lock);

    if (cell.IsDoubleChar() == false)
        return;

    int x_offset = 0, y_offset = 0;
    if (GetBlockXYOffset(point.x, point.y, x_offset, y_offset) == false)
        return;

    std::vector<AnsiCell> cells;
    cells.push_back(cell);

    std::vector<AnsiColor> colors;
    colors.push_back(color);
    colors.push_back(color);

    WriteRegion(x_offset, y_offset, cells, colors, change_rect, false, false);

    RecordAction(ActionInfo(AC_EDIT_DRAW_BLOCK, change_rect));
}

void AnsiCanvas::WriteBlock(const HyPoint &point, const AnsiCell &cell,
                            const AnsiColor &left_color, const AnsiColor &right_color,
                            HyRect &redraw_rect, bool is_convert_to_spaces/* = true*/)
{
    int x_offset = 0, y_offset = 0;
    if (GetBlockXYOffset(point.x, point.y, x_offset, y_offset) == false)
        return;

    return WriteBlock(x_offset, y_offset, cell, left_color, right_color, redraw_rect, is_convert_to_spaces);
}

void AnsiCanvas::WriteBlock(int x_offset, int y_offset, const AnsiCell &cell,
                            const AnsiColor &left_color, const AnsiColor &right_color,
                            HyRect &redraw_rect, bool is_convert_to_spaces/* = true*/)
{
    if (cell.IsDoubleChar() == false)
        return;

    std::vector<AnsiColor> write_colors;
    write_colors.push_back(left_color);
    write_colors.push_back(right_color);

    if (is_convert_to_spaces)
    {
        int space_colors[2];
        bool is_write_spaces = CheckIfWriteSpaces(write_colors, cell, space_colors);
        if (is_write_spaces)
        {
            AnsiCell write_cell(EMPTY);
            std::vector<AnsiColor> write_color(1);

            redraw_rect = hyRect(0, 0, 0, 0);
            for (int c = 0; c < cell.GetCharCount(); c++)
            {
                write_color[0].fg_color = ANSI_DEFAULT_FOREGROUND;
                write_color[0].bg_color = space_colors[c];

                HyRect temp_rect;
                WriteAnsiData(x_offset + c, y_offset, write_color, write_cell, temp_rect);

                redraw_rect = UnionRect(redraw_rect, temp_rect);
            }

            return;
        }
    }

    WriteAnsiData(x_offset, y_offset, write_colors, cell, redraw_rect);
}

void AnsiCanvas::MergeBlockBeforeEditing(const HyRect &block_rect, HyRect &change_rect)
{
    HyPoint block_center(block_rect.x + block_rect.width / 2,
                         block_rect.y + block_rect.height / 2);

    MergeBlock(block_center, change_rect, false);
}

bool AnsiCanvas::DrawSmallSquare(const HyPoint &start_point, const HyPoint &end_point,
                                 int color, bool is_continuous, HyRect &change_rect, bool &is_action_list_changed)
{
    change_rect = hyRect(0, 0, 0, 0);
    is_action_list_changed = false;

    float step_x = 0.0f;
    float step_y = 0.0f;
    int step_count = 0;
    GetLineStepParameters(start_point, end_point, step_x, step_y, step_count);

    HyRect temp_rect;
    bool is_success = false;
   
    float curr_x = (float)start_point.x;
    float curr_y = (float)start_point.y;
    for (int i = 0; i <= step_count; i++)
    {
        HyPoint sample_point(ch_Round(curr_x), ch_Round(curr_y));
        if (DrawSingleSmallSquare(sample_point, color, temp_rect))
        {
            is_success = true;
            change_rect = UnionRect(change_rect, temp_rect);
        }

        curr_x += step_x;
        curr_y += step_y;
    }

    if (DrawSingleSmallSquare(end_point, color, temp_rect))
    {
        is_success = true;
        change_rect = UnionRect(change_rect, temp_rect);
    }

    if (is_success)
    {
        ActionInfo action_info(AC_EDIT_DRAW_SMALL_SQUARE, change_rect);
        action_info.is_continuous = is_continuous;
        is_action_list_changed = RecordAction(action_info);
    }

    return is_success;
}

bool AnsiCanvas::DrawSingleSmallSquare(const HyPoint &point, int color, HyRect &change_rect)
{
    AC_EditInfo edit_info;
    SearchDrawSmallSquareInfo(point.x, point.y, edit_info);
    if (edit_info.is_valid == false)
        return false;

    HyRect write_rect = edit_info.covered_rects[0];

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(write_rect.x, write_rect.y, x_offset, y_offset) == false)
        return false;

    bool is_leftward = (point.x - write_rect.x >= m_half_block_size);
    bool is_bottom_half = (point.y - write_rect.y >= m_half_block_size);

    int curr_x_offset = x_offset;
    int neighbor_x_offset = x_offset + 1;
    if (is_leftward)
        ch_Swap(curr_x_offset, neighbor_x_offset);

    int curr_cell_offset, neighbor_cell_offset;
    AnsiCharLocation curr_location, neighbor_location;
    const AnsiRow &row = m_cells[y_offset];
    bool is_success = SearchCellInfo(row, curr_x_offset, curr_cell_offset, curr_location) &&
                      SearchCellInfo(row, neighbor_x_offset, neighbor_cell_offset, neighbor_location);
    if (is_success == false)
        return false;

    AnsiColor curr_color = CanvasColorAt(curr_x_offset, y_offset);
    AnsiColor neighbor_color = CanvasColorAt(neighbor_x_offset, y_offset);

    // Set the foreground or background color to the given color.
    // If the current location has single space color, Set the other color to it.
    int curr_space_color;
    bool is_curr_color_single = GetPureSpaceColor(curr_x_offset, y_offset, row[curr_cell_offset], 
                                                 (curr_location == DOUBLE_CHAR_RIGHT), curr_space_color);

    if (is_bottom_half)
    {
        curr_color.fg_color = color;
        if (is_curr_color_single)
            curr_color.bg_color = ToNormalColor(curr_space_color);
    }
    else
    {
        curr_color.bg_color = ToNormalColor(color);
        if (is_curr_color_single)
            curr_color.fg_color = curr_space_color;
    }


    // If the neighbor location has single space color, Set foreground/background color to it.
    // Otherwise, keep its original foreground/background color.
    int neighbor_space_color;
    bool is_neighbor_color_single = GetPureSpaceColor(neighbor_x_offset, y_offset, row[neighbor_cell_offset], 
                                                      (neighbor_location == DOUBLE_CHAR_RIGHT), neighbor_space_color);
    if (is_neighbor_color_single)
    {
        neighbor_color.fg_color = neighbor_space_color;
        neighbor_color.bg_color = ToNormalColor(neighbor_space_color);
    }

    AnsiCell write_cell(HORI_BLOCK, 4);
    AnsiColor left_write_color, right_write_color;
    if (is_leftward)
    {
        left_write_color = neighbor_color;
        right_write_color = curr_color;
    }
    else
    {
        left_write_color = curr_color;
        right_write_color = neighbor_color;
    }

    WriteRegion(x_offset, y_offset, write_cell,
                left_write_color, right_write_color, change_rect);

    return true;
}

bool AnsiCanvas::ChangeColor(const HyPoint &start_pt, const HyPoint &end_pt, 
                             const AnsiColor &color, bool is_continuous, HyRect &change_rect, bool &is_action_list_changed)
{
    change_rect = hyRect(0, 0, 0, 0);
    is_action_list_changed = false;

    float step_x = 0.0f;
    float step_y = 0.0f;
    int step_count = 0;
    GetLineStepParameters(start_pt, end_pt, step_x, step_y, step_count);

    int x_offset = 0;
    int y_offset = 0;
    bool is_success = true;

    bool is_valid = true;
    float curr_x = (float)start_pt.x;
    float curr_y = (float)start_pt.y;
    HyRect temp_rect;
    for (int i = 0; i <= step_count; i++)
    {
        is_valid = SetCanvasColor(ch_Round(curr_x), ch_Round(curr_y), color, temp_rect);
        if (is_valid)
            change_rect = UnionRect(change_rect, temp_rect);

        curr_x += step_x;
        curr_y += step_y;
    }

    is_valid = SetCanvasColor(end_pt.x, end_pt.y, color, temp_rect);
    if (is_valid)
        change_rect = UnionRect(change_rect, temp_rect);

    ActionInfo action_info(AC_EDIT_CHANGE_COLOR, change_rect);
    action_info.is_continuous = is_continuous;
    is_action_list_changed = RecordAction(action_info);

    return true;
}

bool AnsiCanvas::ChangeColorArea(const HyPoint &seed, const AnsiColor &color, HyRect &change_rect)
{
    change_rect = hyRect(0, 0, 0, 0);

    std::vector<ColorAreaUnit> all_changed_units;
    if (GetAllChangedColorAreaUnits(seed.x, seed.y, all_changed_units) == false)
        return false;

    int new_color = ToNormalColor(color.fg_color);

    for (int i = 0; i < (int)all_changed_units.size(); i++)
    {
        const ColorAreaUnit &changed_unit = all_changed_units[i];
        if (changed_unit.IsValid() == false)
            continue;

        AnsiColor &color = CanvasColorAt(changed_unit.x_offset, changed_unit.y_offset);
        if (changed_unit.is_foreground_valid)
            color.fg_color = new_color;
        if (changed_unit.is_background_valid)
            color.bg_color = new_color;

        HyRect temp_rect(ToCoordX(changed_unit.x_offset),
                         ToCoordY(changed_unit.y_offset),
                         m_half_block_size, m_block_size);
        change_rect = UnionRect(change_rect, temp_rect);
    }

    ActionInfo action_info(AC_EDIT_CHANGE_COLOR, change_rect);
    action_info.is_alternated_edit = true;
    RecordAction(action_info);

    return true;
}

bool AnsiCanvas::GetAllChangedColorAreaUnits(int seed_x, int seed_y, std::vector<ColorAreaUnit> &all_changed_units)
{
    all_changed_units.clear();

    ColorAreaUnit seed_unit;
    if (GetColorAreaSeedUnit(seed_x, seed_y, seed_unit) == false)
        return false;

    AnsiColor old_color = CanvasColorAt(seed_unit.x_offset, seed_unit.y_offset);
    int target_color = old_color.fg_color;
    if (seed_unit.is_background_valid)
        target_color = old_color.bg_color;

    ChAutoPtr<BYTE> visit_mask(m_width * m_height);
    ZeroMemory(visit_mask, m_width * m_height);

    std::stack<ColorAreaUnit> search_units;
    search_units.push(seed_unit);

    while (search_units.empty() == false)
    {
        ColorAreaUnit unit = search_units.top();
        search_units.pop();

        const int x_offset = unit.x_offset;
        const int y_offset = unit.y_offset;

        all_changed_units.push_back(unit);
        visit_mask[y_offset * m_width + x_offset] = 255;

        ColorAreaUnit left_unit;
        if (GetColorAreaSearchUnit(x_offset - 1, y_offset,
                                   visit_mask, target_color,
                                   RIGHT_SIDE, unit.boundary.left, left_unit))
            search_units.push(left_unit);

        ColorAreaUnit top_unit;
        if (GetColorAreaSearchUnit(x_offset, y_offset - 1,
                                   visit_mask, target_color,
                                   BOTTOM_SIDE, unit.boundary.top, top_unit))
            search_units.push(top_unit);

        ColorAreaUnit right_unit;
        if (GetColorAreaSearchUnit(x_offset + 1, y_offset,
                                   visit_mask, target_color,
                                   LEFT_SIDE, unit.boundary.right, right_unit))
            search_units.push(right_unit);

        ColorAreaUnit bottom_unit;
        if (GetColorAreaSearchUnit(x_offset, y_offset + 1,
                                   visit_mask, target_color,
                                   TOP_SIDE, unit.boundary.bottom, bottom_unit))
            search_units.push(bottom_unit);
    }

    return true;
}

bool AnsiCanvas::GetColorAreaSeedUnit(int seed_x, int seed_y, ColorAreaUnit &seed_unit)
{
    int x_offset, y_offset;
    if (GetXYOffset(seed_x, seed_y, x_offset, y_offset) == false)
        return false;

    int cell_offset;
    AnsiCharLocation char_location;
    bool is_success = SearchCellInfo(m_cells[y_offset], x_offset, cell_offset, char_location);
    if (is_success == false)
        return false;

    seed_unit.x_offset = x_offset;
    seed_unit.y_offset = y_offset;
    seed_unit.cell = m_cells[y_offset][cell_offset];
    seed_unit.char_location = char_location;

    seed_unit.SetInvalid();

    const AnsiCell &cell = CanvasCellAt(cell_offset, y_offset);
    const AnsiColor &color = CanvasColorAt(x_offset, y_offset);

    // Find boundary according to shape and seed point.
    int x_shift = seed_x - ToCoordX(x_offset);
    int y_shift = seed_y - ToCoordY(y_offset);

    bool is_full_range = false;
    if (cell.type == EMPTY)
    {
        seed_unit.is_background_valid = true;
        is_full_range = true;
    }
    else if (cell.type == HORI_BLOCK)
    {
        if (cell.label == 0)
        {
            seed_unit.is_background_valid = true;
            is_full_range = true;
        }
        else if (cell.label == 8)
        {
            seed_unit.is_foreground_valid = true;
            is_full_range = true;
        }
        else
        {
            int y_bound = GetHoriBorder(cell.label);
            if (y_shift >= y_bound)
                seed_unit.is_foreground_valid = true;
            else
                seed_unit.is_background_valid = true;
        }
    }
    else if (cell.type == VERT_BLOCK)
    {
        if ((char_location == DOUBLE_CHAR_LEFT && cell.label == 0) ||
            (char_location == DOUBLE_CHAR_RIGHT && cell.label <= 4))
        {
            seed_unit.is_background_valid = true;
            is_full_range = true;
        }
        else if ((char_location == DOUBLE_CHAR_LEFT && cell.label >= 4) ||
                 (char_location == DOUBLE_CHAR_RIGHT && cell.label == 8))
        {
            seed_unit.is_foreground_valid = true;
            is_full_range = true;
        }
        else
        {
            int x_bound = GetVertBorder(cell.label);
            if (char_location == DOUBLE_CHAR_RIGHT)
            x_bound -= m_half_block_size;

            if (x_shift < x_bound)
                seed_unit.is_foreground_valid = true;
            else
                seed_unit.is_background_valid = true;
        }
    }
    else if (cell.type == TRIANGLE)
    {
        if (IsForegroundTriangle(x_shift, y_shift, char_location, cell.label))
            seed_unit.is_foreground_valid = true;
        else
            seed_unit.is_background_valid = true;
    }
    else if (cell.type == REGULAR_TRIANGLE)
    {
        if (IsForegroundRegularTriangle(x_shift, y_shift, char_location, cell.label))
            seed_unit.is_foreground_valid = true;
        else
            seed_unit.is_background_valid = true;
    }
    else if (cell.type == GENERAL_CHAR)
    {
        seed_unit.is_background_valid = true;
    }

    if (is_full_range == false && color.fg_color == color.bg_color)
    {
        // Same color in two split regions. Set both colors to valid.

        seed_unit.is_foreground_valid = true;
        seed_unit.is_background_valid = true;
    }

    seed_unit.boundary = GetColorAreaBoundary(cell, char_location,
                                              seed_unit.is_foreground_valid,
                                              seed_unit.is_background_valid);

    return seed_unit.IsValid();
}

bool AnsiCanvas::GetColorAreaSearchUnit(int x_offset, int y_offset,
                                        const BYTE *p_visit_mask, int target_color,
                                        ColorAreaBoundarySide boundary_type,
                                        const ColorAreaBoundarySegment &boundary_segment, ColorAreaUnit &unit)
{
    if (x_offset < 0 || x_offset >= m_width)
        return false;
    if (y_offset < 0 || y_offset >= m_height)
        return false;

    if (p_visit_mask[y_offset * m_width + x_offset] != 0)
        return false;
    if (boundary_segment.IsValid() == false)
        return false;

    int cell_offset;
    AnsiCharLocation char_location;
    bool is_success = SearchCellInfo(m_cells[y_offset], x_offset, cell_offset, char_location);
    if (is_success == false)
        return false;

    unit.x_offset = x_offset;
    unit.y_offset = y_offset;
    unit.cell = m_cells[y_offset][cell_offset];
    unit.char_location = char_location;
    unit.SetInvalid();

    const AnsiCell &cell = CanvasCellAt(cell_offset, y_offset);
    const AnsiColor &color = CanvasColorAt(x_offset, y_offset);

    ColorAreaBoundary foreground_boundary = GetColorAreaBoundary(cell, char_location, true, false);
    ColorAreaBoundary background_boundary = GetColorAreaBoundary(cell, char_location, false, true);
    bool is_foreground_valid = foreground_boundary.IsValid();
    bool is_background_valid = background_boundary.IsValid();

    // Special case for GENERAL_CHAR:
    // The foreground boundary is invalid, the background boundary is full range.
    // If both foreground/background are the target color, set both to valid.

    bool is_select_both = (target_color == color.fg_color && target_color == color.bg_color) &&
                          ((is_foreground_valid && is_background_valid) || (cell.type == GENERAL_CHAR));
    if (is_select_both)
    {
        unit.is_foreground_valid = true;
        unit.is_background_valid = true;
        unit.boundary = GetColorAreaBoundary(cell, char_location, true, true);
    }
    else if (is_foreground_valid && target_color == color.fg_color)
    {
        unit.is_foreground_valid = true;
        unit.boundary = foreground_boundary;

    }
    else if (is_background_valid && target_color == color.bg_color)
    {
        unit.is_background_valid = true;
        unit.boundary = background_boundary;
    }

    if (unit.IsValid())
    {
        // Check if the boundary overlaps.

        ColorAreaBoundarySegment check_segment;
        if (boundary_type == LEFT_SIDE)
            check_segment = unit.boundary.left;
        else if (boundary_type == TOP_SIDE)
            check_segment = unit.boundary.top;
        else if (boundary_type == RIGHT_SIDE)
            check_segment = unit.boundary.right;
        else if (boundary_type == BOTTOM_SIDE)
            check_segment = unit.boundary.bottom;

        if (check_segment.IsOverlap(boundary_segment) == false)
            unit.SetInvalid();
    }

    return unit.IsValid();
}

AnsiCanvas::ColorAreaBoundary AnsiCanvas::GetColorAreaBoundary(const AnsiCell &cell, AnsiCharLocation char_location,
                                                               bool is_foreground_valid, bool is_background_valid)
{
    ColorAreaBoundary boundary;

    boundary.SetInvalid();

    if (is_foreground_valid == true && is_background_valid == true)
    {
        boundary.SetAllValid(m_half_block_size, m_block_size);
    }
    else if (is_foreground_valid == false && is_background_valid == false)
    {
        // keep invalid
    }
    else
    {
        // Only foreground or background is valid.
        if (cell.type == EMPTY)
        {
            if (is_background_valid)
                boundary.SetAllValid(m_half_block_size, m_block_size);
        }
        else if (cell.type == HORI_BLOCK)
        {
            if (cell.label == 0)
            {
                if (is_background_valid)
                    boundary.SetAllValid(m_half_block_size, m_block_size);
            }
            else if (cell.label == 8)
            {
                if (is_foreground_valid)
                    boundary.SetAllValid(m_half_block_size, m_block_size);
            }
            else
            {
                int y_bound = GetHoriBorder(cell.label);
                if (is_foreground_valid)
                {
                    boundary.left.SetRange(y_bound, m_block_size);
                    boundary.right.SetRange(y_bound, m_block_size);
                    boundary.bottom.SetRange(0, m_half_block_size);
                }
                else
                {
                    boundary.left.SetRange(0, y_bound);
                    boundary.right.SetRange(0, y_bound);
                    boundary.top.SetRange(0, m_half_block_size);
                }
            }
        }
        else if (cell.type == VERT_BLOCK)
        {
            if ((char_location == DOUBLE_CHAR_LEFT && cell.label == 0) ||
                (char_location == DOUBLE_CHAR_RIGHT && cell.label <= 4))
            {
                if (is_background_valid)
                    boundary.SetAllValid(m_half_block_size, m_block_size);
            }
            else if ((char_location == DOUBLE_CHAR_LEFT && cell.label >= 4) ||
                     (char_location == DOUBLE_CHAR_RIGHT && cell.label == 8))
            {
                if (is_foreground_valid)
                    boundary.SetAllValid(m_half_block_size, m_block_size);
            }
            else
            {
                int x_bound = GetVertBorder(cell.label);
                if (char_location == DOUBLE_CHAR_RIGHT)
                    x_bound -= m_half_block_size;

                if (is_foreground_valid)
                {
                    boundary.left.SetRange(0, m_block_size);
                    boundary.top.SetRange(0, x_bound);
                    boundary.bottom.SetRange(0, x_bound);
                }
                else
                {
                    boundary.right.SetRange(0, m_block_size);
                    boundary.top.SetRange(x_bound, m_half_block_size);
                    boundary.bottom.SetRange(x_bound, m_half_block_size);
                }
            }
        }
        else if (cell.type == TRIANGLE)
        {
            if ((cell.label == 0 && is_foreground_valid) || (cell.label == 3 && is_background_valid))
            {
                if (char_location == DOUBLE_CHAR_RIGHT)
                {
                    boundary.left.SetRange(0, m_half_block_size);
                    boundary.top.SetRange(0, m_half_block_size);
                }
                else
                {
                    boundary.left.SetRange(0, m_block_size);
                    boundary.top.SetRange(0, m_half_block_size);
                    boundary.right.SetRange(0, m_half_block_size);
                }
            }
            else if ((cell.label == 0 && is_background_valid) || (cell.label == 3 && is_foreground_valid))
            {
                if (char_location == DOUBLE_CHAR_RIGHT)
                {
                    boundary.left.SetRange(m_half_block_size, m_block_size);
                    boundary.right.SetRange(0, m_block_size);
                    boundary.bottom.SetRange(0, m_half_block_size);
                }
                else
                {
                    boundary.right.SetRange(m_half_block_size, m_block_size);
                    boundary.bottom.SetRange(0, m_half_block_size);
                }
            }
            else if ((cell.label == 1 && is_foreground_valid) || (cell.label == 2 && is_background_valid))
            {
                if (char_location == DOUBLE_CHAR_RIGHT)
                {
                    boundary.left.SetRange(0, m_half_block_size);
                    boundary.top.SetRange(0, m_half_block_size);
                    boundary.right.SetRange(0, m_block_size);
                }
                else
                {
                    boundary.top.SetRange(0, m_half_block_size);
                    boundary.right.SetRange(0, m_half_block_size);
                }
            }
            else if ((cell.label == 1 && is_background_valid) || (cell.label == 2 && is_foreground_valid))
            {
                if (char_location == DOUBLE_CHAR_RIGHT)
                {
                    boundary.left.SetRange(m_half_block_size, m_block_size);
                    boundary.bottom.SetRange(0, m_half_block_size);
                }
                else
                {
                    boundary.left.SetRange(0, m_block_size);
                    boundary.right.SetRange(m_half_block_size, m_block_size);
                    boundary.bottom.SetRange(0, m_half_block_size);
                }
            }
        }
        else if (cell.type == REGULAR_TRIANGLE)
        {
            if (char_location == DOUBLE_CHAR_RIGHT)
            {
                if (cell.label == 0)
                {
                    if (is_foreground_valid)
                    {
                        boundary.left.SetRange(0, m_block_size);
                        boundary.bottom.SetRange(0, m_half_block_size);
                    }
                    else
                    {
                        boundary.right.SetRange(0, m_block_size);
                        boundary.top.SetRange(0, m_half_block_size);
                    }
                }
                else if (cell.label == 1)
                {
                    if (is_foreground_valid)
                    {
                        boundary.left.SetRange(0, m_block_size);
                        boundary.top.SetRange(0, m_half_block_size);
                    }
                    else
                    {
                        boundary.right.SetRange(0, m_block_size);
                        boundary.bottom.SetRange(0, m_half_block_size);
                    }
                }
            }
            else
            {
                if (cell.label == 0)
                {
                    if (is_foreground_valid)
                    {
                        boundary.right.SetRange(0, m_block_size);
                        boundary.bottom.SetRange(0, m_half_block_size);
                    }
                    else
                    {
                        boundary.left.SetRange(0, m_block_size);
                        boundary.top.SetRange(0, m_half_block_size);
                    }
                }
                else if (cell.label == 1)
                {
                    if (is_foreground_valid)
                    {
                        boundary.right.SetRange(0, m_block_size);
                        boundary.top.SetRange(0, m_half_block_size);
                    }
                    else
                    {
                        boundary.left.SetRange(0, m_block_size);
                        boundary.bottom.SetRange(0, m_half_block_size);
                    }
                }
            }
        }
        else if (cell.type == GENERAL_CHAR)
        {
            if (is_background_valid)
                boundary.SetAllValid(m_half_block_size, m_block_size);
        }
    }

    return boundary;
}

void AnsiCanvas::FindChangeMask(const HyPoint &seed, HyRect &change_roi, std::vector<BYTE> &change_mask)
{
    change_roi = hyRect(0, 0, 0, 0);
    change_mask.clear();

    std::vector<ColorAreaUnit> all_changed_units;
    if (GetAllChangedColorAreaUnits(seed.x, seed.y, all_changed_units) == false)
        return;

    const int unit_count = (int)all_changed_units.size();

    int min_x_offset = m_width;
    int min_y_offset = m_height;
    int max_x_offset = -1;
    int max_y_offset = -1;

    for (int i = 0; i < unit_count; i++)
    {
        int x_offset = all_changed_units[i].x_offset;
        int y_offset = all_changed_units[i].y_offset;

        min_x_offset = ch_Min(min_x_offset, x_offset);
        min_y_offset = ch_Min(min_y_offset, y_offset);
        max_x_offset = ch_Max(max_x_offset, x_offset);
        max_y_offset = ch_Max(max_y_offset, y_offset);
    }

    const int display_offset_width = MIN_CANVAS_WIDTH;
    const int display_offset_height = MIN_CANVAS_HEIGHT;

    min_x_offset = ch_Max(min_x_offset, m_display_x_offset);
    min_y_offset = ch_Max(min_y_offset, m_display_y_offset);
    max_x_offset = ch_Min(max_x_offset, m_display_x_offset + display_offset_width - 1);
    max_y_offset = ch_Min(max_y_offset, m_display_y_offset + display_offset_height - 1);

    if (max_x_offset < min_x_offset || max_y_offset < min_y_offset)
        return;

    const int valid_offset_width = max_x_offset - min_x_offset + 1;
    const int valid_offset_height = max_y_offset - min_y_offset + 1;

    int mask_width = valid_offset_width * m_half_block_size;
    int mask_height = valid_offset_height * m_block_size;

    change_mask.resize(mask_width * mask_height);
    BYTE *p_mask_data = &change_mask[0];
    memset(p_mask_data, 0, mask_width * mask_height);

    // we can only "draw" color contents now, so we need a 3-channel buffer.
    const int space_stride = m_half_block_size * 3;
    ChAutoPtr<BYTE> space_buffer(space_stride * m_block_size);

    // Draw the target regions in bright-white cell.
    // For background region, use bright-color background for this task.

    for (int i = 0; i < unit_count; i++)
    {
        const ColorAreaUnit &unit = all_changed_units[i];
        
        // relative offset
        int x_offset = unit.x_offset - min_x_offset;
        int y_offset = unit.y_offset - min_y_offset;

        if (x_offset < 0 || x_offset >= valid_offset_width ||
            y_offset < 0 || y_offset >= valid_offset_height)
            continue;

        BYTE *p_mask_space_start = p_mask_data + (y_offset * m_block_size) * mask_width + (x_offset * m_half_block_size);

        if (unit.cell.type == GENERAL_CHAR || 
            (unit.is_foreground_valid && unit.is_background_valid))
        {
            // Fill the whole space unit.
            ippiSet_8u_C1R(255, p_mask_space_start, mask_width, ippiSize(m_half_block_size, m_block_size));
        }
        else
        {
            // Either is_foreground_valid or is_background_valid is true.
            // Use DrawHalfBlock() to get the content.

            AnsiColor draw_color(ANSI_WHITE_BRIGHT, ANSI_BLACK);
            if (unit.is_background_valid)
                ch_Swap(draw_color.fg_color, draw_color.bg_color);

            m_ansi_template.DrawHalfBlock(space_buffer, space_stride, 3,
                                          draw_color, unit.cell, (unit.char_location == DOUBLE_CHAR_RIGHT));

            for (int y = 0; y < m_block_size; y++)
            {
                const BYTE *p_color_scan = (const BYTE *)space_buffer + y * space_stride;
                BYTE *p_mask_scan = p_mask_space_start + y * mask_width;

                for (int x = 0; x < m_half_block_size; x++)
                    p_mask_scan[x] = p_color_scan[x * 3];
            }
        }
    }

    change_roi.x = (min_x_offset - m_display_x_offset) * m_half_block_size;
    change_roi.y = (min_y_offset - m_display_y_offset) * m_block_size;
    change_roi.width = mask_width;
    change_roi.height = mask_height;
}

bool AnsiCanvas::MergeBlockAction(const HyPoint &point, HyRect &change_rect, bool is_force_merge)
{
    bool is_success = MergeBlock(point, change_rect, is_force_merge);
    if (is_success == false)
        return false;

    ActionInfo action_info(AC_EDIT_MERGE_BLOCK, change_rect);
    action_info.is_alternated_edit = is_force_merge;
    RecordAction(action_info);

    return true;
}

bool AnsiCanvas::MergeBlock(const HyPoint &point, HyRect &redraw_rect, bool is_force_merge)
{
    AC_EditInfo info;
    SearchMergeBlockInfo(point.x, point.y, info, is_force_merge);

    if (info.is_valid == false)
        return false;

    WriteBlock(point, info.cell, info.color, info.color2, redraw_rect);

    for (int i = 0; i < (int)info.additional_merge_data.size(); i++)
    {
        const AC_AdditionalMergeData &merge_data = info.additional_merge_data[i];

        HyPoint block_center;
        block_center.x = merge_data.merge_rect.x + m_half_block_size;
        block_center.y = merge_data.merge_rect.y + m_half_block_size;

        HyRect temp_rect;
        WriteBlock(block_center, merge_data.cell, merge_data.left_color, merge_data.right_color, temp_rect);

        redraw_rect = UnionRect(redraw_rect, temp_rect);
    }

    return true;
}

bool AnsiCanvas::AddText(const HyPoint &point, LPCTSTR text_string, int text_color, HyRect &change_rect)
{
    if (text_string == NULL)
        return false;

    int string_length = _tcslen(text_string);
    if (string_length == 0)
        return false;

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(point.x, point.y, x_offset, y_offset) == false)
        return false;

    // Collect the valid characters/colors, and write them by WriteRegion().
    int x_offset_scan = x_offset;
    std::vector<AnsiCell> write_cells;
    std::vector<AnsiColor> write_colors;

    for (int i = 0; i < string_length; i++)
    {
        AnsiCell cell = CharToAnsiCell(text_string[i]);
        int char_count = cell.GetCharCount();
        if (char_count > m_width - x_offset_scan)
            break;

        // Use the given color as foreground, and preserve the original background color.
        // If they are the same, auto-change the text color to black or (non-bright) white.
        for (int c = 0; c < char_count; c++)
        {
            AnsiColor old_color = CanvasColorAt(x_offset_scan + c, y_offset);
            AnsiColor color(text_color, old_color.bg_color);
            if (color.fg_color == color.bg_color)
            {
                color.fg_color = ANSI_WHITE;
                if (color.bg_color == ANSI_WHITE || color.bg_color == ANSI_YELLOW)
                    color.fg_color = ANSI_BLACK;
            }

            write_colors.push_back(color);
        }

        write_cells.push_back(cell);

        x_offset_scan += char_count;
    }

    WriteRegion(x_offset, y_offset, write_cells, write_colors, change_rect, false, false);

    RecordAction(ActionInfo(AC_EDIT_ADD_TEXT, change_rect));

    return true;
}

std::vector<AnsiColor> AnsiCanvas::InvalidColor()
{
    return std::vector<AnsiColor>();
}

void AnsiCanvas::WriteAnsiData(int x_offset, int y_offset,
                               const AnsiColor &color, const AnsiCell &cell, HyRect &change_rect)
{
    std::vector<AnsiColor> color_list;
    color_list.push_back(color);
    WriteAnsiData(x_offset, y_offset, color_list, cell, change_rect);
}

bool AnsiCanvas::CheckIfWriteSpaces(const std::vector<AnsiColor> &color,
                                    const AnsiCell &cell, int space_colors[2])
{
    if (color.empty())
        return false;

    if (cell.IsDoubleChar() == false)
        return false;


    AnsiColor left_color = color[0];
    int left_space_color;
    if (GetPureSpaceColor(left_color, cell, false, left_space_color) == false)
        return false;
    if (IsBrightColor(left_space_color))
        return false;

    AnsiColor right_color = left_color;
    if ((int)color.size() >= 2)
        right_color = color[1];
    int right_space_color;
    if (GetPureSpaceColor(right_color, cell, true, right_space_color) == false)
        return false;
    if (IsBrightColor(right_space_color))
        return false;

    space_colors[0] = left_space_color;
    space_colors[1] = right_space_color;

    return true;
}

void AnsiCanvas::WriteAnsiData(int x_offset, int y_offset,
                               const std::vector<AnsiColor> &color,
                               const AnsiCell &cell, HyRect &change_rect)
{
    change_rect = hyRect(0, 0, 0, 0);

    AnsiWriteInfo write_info;
    if (FindWriteInfo(x_offset, y_offset, cell.IsDoubleChar(), write_info) == false)
        return;

    if (cell.IsDoubleChar())
        WriteDoubleCharData(color, cell, write_info, change_rect);
    else
        WriteSingleCharData(color, cell, write_info, change_rect);
}

void AnsiCanvas::WriteDoubleCharData(const std::vector<AnsiColor> &color, const AnsiCell &cell, const AnsiWriteInfo &info, HyRect &change_rect)
{
    const int line_y = info.y_offset;
    const int color_x = info.color_offset;
    const int cell_x = info.cell_offset;
    const AnsiCharLocation location = info.char_location;

    int rect_x = ToCoordX(color_x);
    int rect_y = ToCoordY(line_y);
    change_rect.y = rect_y;
    change_rect.height = m_block_size;

    if (color.size() >= 2)
    {
        CanvasColorAt(color_x, line_y) = color[0];
        CanvasColorAt(color_x + 1, line_y) = color[1];
    }
    else if (color.size() == 1)
    {
        CanvasColorAt(color_x, line_y) = color[0];
        CanvasColorAt(color_x + 1, line_y) = color[0];
    }

    AnsiRow &row = m_cells[line_y];
    const AnsiCell old_cell = row[cell_x];

    bool is_have_right_cell = (cell_x < (int)row.size() - 1);
    AnsiCell old_right_cell;
    if (is_have_right_cell)
        old_right_cell = row[cell_x + 1];

    if (location == DOUBLE_CHAR_LEFT)
    {
        row[cell_x] = cell;

        change_rect.x = rect_x;
        change_rect.width = m_block_size;
    }
    else
    {
        if (is_have_right_cell && old_right_cell.IsDoubleChar())
        {
            if (location == SINGLE_CHAR)
            {
                SetSpaceColor(color_x + 2, line_y, old_right_cell, true);

                row[cell_x] = cell;
                row[cell_x + 1] = AnsiCell(EMPTY);

                change_rect.x = rect_x;
                change_rect.width = m_block_size + m_half_block_size;
            }
            else
            {
                SetSpaceColor(color_x - 1, line_y, old_cell, false);
                SetSpaceColor(color_x + 2, line_y, old_right_cell, true);

                row[cell_x] = AnsiCell(EMPTY);
                row[cell_x + 1] = cell;
                row.insert(row.begin() + cell_x + 2, AnsiCell(EMPTY));

                change_rect.x = rect_x - m_half_block_size;
                change_rect.width = m_block_size * 2;
            }
        }
        else
        {
            if (location == SINGLE_CHAR)
            {
                row.erase(row.begin() + cell_x);
                row[cell_x] = cell;

                change_rect.x = rect_x;
                change_rect.width = m_block_size;
            }
            else
            {
                SetSpaceColor(color_x - 1, line_y, old_cell, false);

                row[cell_x] = AnsiCell(EMPTY);
                row[cell_x + 1] = cell;

                change_rect.x = rect_x - m_half_block_size;
                change_rect.width = m_block_size + m_half_block_size;
            }
        }
    }
}

void AnsiCanvas::WriteSingleCharData(const std::vector<AnsiColor> &color, const AnsiCell &cell, const AnsiWriteInfo &info, HyRect &change_rect)
{
    const int line_y = info.y_offset;
    const int color_x = info.color_offset;
    const int cell_x = info.cell_offset;
    const AnsiCharLocation location = info.char_location;

    int rect_x = ToCoordX(color_x);
    int rect_y = ToCoordY(line_y);
    change_rect.y = rect_y;
    change_rect.height = m_block_size;

    if (color.size() >= 1)
        CanvasColorAt(info.color_offset, info.y_offset) = color[0];

    AnsiRow &row = m_cells[line_y];
    const AnsiCell old_cell = row[cell_x];

    if (location == SINGLE_CHAR)
    {   
        row[cell_x] = cell;

        change_rect.x = rect_x;
        change_rect.width = m_half_block_size;
    }
    else if (location == DOUBLE_CHAR_LEFT)
    {
        SetSpaceColor(color_x + 1, line_y, old_cell, true);

        row[cell_x] = cell;
        row.insert(row.begin() + cell_x + 1, AnsiCell(EMPTY));

        change_rect.x = rect_x;
        change_rect.width = m_block_size;
    }
    else if (location == DOUBLE_CHAR_RIGHT)
    {
        SetSpaceColor(color_x - 1, line_y, old_cell, false);

        row[cell_x] = AnsiCell(EMPTY);
        row.insert(row.begin() + cell_x + 1, cell);

        change_rect.x = rect_x - m_half_block_size;
        change_rect.width = m_block_size;
    }
}

void AnsiCanvas::GetSpaceColor(const AnsiColor &color, const AnsiCell &cell,
                               bool is_right_half, int &space_color, bool &is_pure_color)
{
    // Always return a color.
    // Set is_pure_color to notify if the space region has pure color.

    // Default: NOT pure color, use background color.
    // Set is_pure_color = true and/or use foreground color when matching some criteria.
    is_pure_color = false;
    space_color = color.bg_color;

    if (color.fg_color == color.bg_color)
    {
        is_pure_color = true;
    }
    else if (cell.type == EMPTY)
    {
        is_pure_color = true;
    }
    else if (cell.type == HORI_BLOCK)
    {
        is_pure_color = (cell.label == 0 || cell.label == 8);
        if (cell.label > 4)
            space_color = color.fg_color;
    }
    else if (cell.type == VERT_BLOCK)
    {
        if (is_right_half == false)
        {
            is_pure_color = (cell.label == 0) || (cell.label >= 4);
            if (cell.label >= 3)
                space_color = color.fg_color;
        }
        else
        {
            is_pure_color = (cell.label <= 4) || (cell.label == 8);
            if (cell.label >= 7)
                space_color = color.fg_color;
        }
    }
    else if (cell.type == TRIANGLE)
    {
        if (cell.label == 0)
        {
            if (is_right_half == false)
                space_color = color.fg_color;
        }
        else if (cell.label == 1)
        {
            if (is_right_half == true)
                space_color = color.fg_color;
        }
        else if (cell.label == 2)
        {
            if (is_right_half == false)
                space_color = color.fg_color;
        }
        else if (cell.label == 3)
        {
            if (is_right_half == true)
                space_color = color.fg_color;
        }
    }
    else if (cell.type == REGULAR_TRIANGLE)
    {
        space_color = color.bg_color;
    }
}

bool AnsiCanvas::GetSpaceColor(int x_offset, int y_offset, int &space_color, bool &is_pure_color)
{
    // return fail if (x_offset, y_offset) is valid.

    if (x_offset < 0 || x_offset >= m_width)
        return false;
    if (y_offset < 0 || y_offset >= m_height)
        return false;

    const AnsiRow &row = m_cells[y_offset];

    int cell_offset;
    AnsiCharLocation char_location;
    if (SearchCellInfo(row, x_offset, cell_offset, char_location) == false)
        return false;

    GetSpaceColor(CanvasColorAt(x_offset, y_offset), row[cell_offset], 
                  (char_location == DOUBLE_CHAR_RIGHT), space_color, is_pure_color);

    return true;
}

bool AnsiCanvas::GetSpaceColor(int x_offset, int y_offset, int &space_color)
{
    // Used when the caller doesn't care if it is pure color.

    bool is_pure_color = false;
    return GetSpaceColor(x_offset, y_offset, space_color, is_pure_color);
}

bool AnsiCanvas::GetPureSpaceColor(const AnsiColor &color, const AnsiCell &cell, bool is_right_half, int &space_color)
{
    // Return false if the region is not pure color.

    bool is_pure_color = false;
    int test_space_color;
    GetSpaceColor(color, cell, is_right_half, test_space_color, is_pure_color);

    if (is_pure_color)
        space_color = test_space_color;

    return is_pure_color;
}

bool AnsiCanvas::GetPureSpaceColor(int x_offset, int y_offset, const AnsiCell &cell, bool is_right_half, int &space_color)
{
    // Return false if (x_offset, y_offset) is invalid, or the region is not pure color.

    if (y_offset < 0 || y_offset >= m_height)
        return false;
    if (x_offset < 0 || x_offset >= m_width)
        return false;

    return GetPureSpaceColor(CanvasColorAt(x_offset, y_offset), cell, is_right_half, space_color);
}

bool AnsiCanvas::GetPureSpaceColor(int x_offset, int y_offset, int &space_color)
{
    // Return false if (x_offset, y_offset) is invalid, or the region is not pure color.

    if (y_offset < 0 || y_offset >= m_height)
        return false;
    if (x_offset < 0 || x_offset >= m_width)
        return false;

    int cell_offset;
    AnsiCharLocation char_location;
    bool is_success = SearchCellInfo(m_cells[y_offset], x_offset, cell_offset, char_location);
    if (is_success == false)
        return false;

    return GetPureSpaceColor(CanvasColorAt(x_offset, y_offset),
                             m_cells[y_offset][cell_offset],
                             (char_location == DOUBLE_CHAR_RIGHT), space_color);
}

void AnsiCanvas::SetSpaceColor(int x_offset, int y_offset, const AnsiCell &cell, bool is_right_half)
{
    // Get the current color at (x_offset, offset),
    // then use GetSpaceColor() to find the best-matched color.
    // Set the background color (for space) at (x_offset, offset).

    if (y_offset < 0 || y_offset >= m_height)
        return;
    if (x_offset < 0 || x_offset >= m_width)
        return;

    int space_color;
    bool is_pure_color = false; // not used
    GetSpaceColor(CanvasColorAt(x_offset, y_offset), cell, is_right_half, space_color, is_pure_color);

    CanvasColorAt(x_offset, y_offset).bg_color = ToNormalColor(space_color);
}


bool AnsiCanvas::GetSmallSquareColor(int x_offset, int y_offset_2x, int &square_color, bool &is_pure_color)
{
    // Always return a color if (x_offset, y_offset_2x) is valid.
    // Set is_pure_color to notify if the square region has pure color.

    if (x_offset < 0 || x_offset >= m_width)
        return false;
    if (y_offset_2x < 0 || y_offset_2x >= m_height * 2)
        return false;

    bool is_lower = (y_offset_2x % 2 == 1);
    int y_offset = y_offset_2x / 2;
    const AnsiRow &row = m_cells[y_offset];

    int cell_offset;
    AnsiCharLocation cell_location;
    if (SearchCellInfo(row, x_offset, cell_offset, cell_location) == false)
        return false;

    const AnsiCell &cell = row[cell_offset];
    const AnsiColor &color = CanvasColorAt(x_offset, y_offset);

    // Default: NOT pure color, use background color.
    // Set is_pure_color = true and/or use foreground color when matching some criteria.
    is_pure_color = false;
    square_color = color.bg_color;

    if (color.fg_color == color.bg_color)
    {
        is_pure_color = true;
    }
    else if (cell.type == EMPTY)
    {
        is_pure_color = true;
    }
    else if (cell.type == HORI_BLOCK)
    {
        if (is_lower)
        {
            is_pure_color = (cell.label == 0) || (cell.label >= 4);
            if (cell.label >= 3)
                square_color = color.fg_color;
        }
        else
        {
            is_pure_color = (cell.label <= 4) || (cell.label == 8);
            if (cell.label >= 7)
                square_color = color.fg_color;
        }
    }
    else if (cell.type == VERT_BLOCK)
    {
        if (cell_location == DOUBLE_CHAR_LEFT)
        {
            is_pure_color = (cell.label == 0) || (cell.label >= 4);
            if (cell.label >= 3)
                square_color = color.fg_color;
        }
        else
        {
            is_pure_color = (cell.label <= 4) || (cell.label == 8);
            if (cell.label >= 7)
                square_color = color.fg_color;
        }
    }
    else if (cell.type == TRIANGLE)
    {
        if (cell.label == 0)
        {
            is_pure_color = (cell_location == DOUBLE_CHAR_LEFT && is_lower == false) ||
                            (cell_location == DOUBLE_CHAR_RIGHT && is_lower == true);
            if (cell_location == DOUBLE_CHAR_LEFT && is_lower == false)
                square_color = color.fg_color;
        }
        else if (cell.label == 1)
        {
            is_pure_color = (cell_location == DOUBLE_CHAR_LEFT && is_lower == true) ||
                            (cell_location == DOUBLE_CHAR_RIGHT && is_lower == false);
            if (cell_location == DOUBLE_CHAR_RIGHT && is_lower == false)
                square_color = color.fg_color;
        }
        else if (cell.label == 2)
        {
            is_pure_color = (cell_location == DOUBLE_CHAR_LEFT && is_lower == true) ||
                            (cell_location == DOUBLE_CHAR_RIGHT && is_lower == false);
            if (cell_location == DOUBLE_CHAR_LEFT && is_lower == true)
                square_color = color.fg_color;
        }
        else if (cell.label == 3)
        {
            is_pure_color = (cell_location == DOUBLE_CHAR_LEFT && is_lower == false) ||
                            (cell_location == DOUBLE_CHAR_RIGHT && is_lower == true);
            if (cell_location == DOUBLE_CHAR_RIGHT && is_lower == true)
                square_color = color.fg_color;
        }
    }
    else if (cell.type == REGULAR_TRIANGLE)
    {
        if (cell.label == 0)
        {
            if (is_lower == true)
                square_color = color.fg_color;
        }
        else if (cell.label == 1)
        {
            if (is_lower == false)
                square_color = color.fg_color;
        }
    }

    return true;
}

bool AnsiCanvas::GetSmallSquareColor(int x_offset, int y_offset_2x, int &square_color)
{
    // Used when the caller doesn't care if it is pure color.

    bool is_pure_color = false;
    return GetSmallSquareColor(x_offset, y_offset_2x, square_color, is_pure_color);
}

bool AnsiCanvas::FindWriteInfo(int x_offset, int y_offset, bool is_double_char, AnsiWriteInfo &info)
{
    if (y_offset < 0 || y_offset >= m_height)
        return false;
    if (x_offset < 0 || x_offset >= m_width)
        return false;
    if (is_double_char && x_offset == m_width - 1)
        return false;

    info.y_offset = y_offset;
    info.color_offset = x_offset;

    const AnsiRow &row = m_cells[y_offset];

    int offset;
    AnsiCharLocation location;
    bool is_success = SearchCellInfo(row, x_offset, offset, location);
    if (is_success)
    {
        info.cell_offset = offset;
        info.char_location = location;
    }

    return is_success;
}

void AnsiCanvas::GetEditInfo(int x, int y, AC_EditMode mode,
                             AC_EditInfo &edit_info, const AC_EditOption &option)
{
    if (mode == AC_EDIT_VIEW)
    {
        SearchViewInfo(x, y, edit_info);
    }
    else if (mode == AC_EDIT_DRAW_SPACE)
    {
        SearchDrawSpaceInfo(x, y, edit_info);
    }
    else if (mode == AC_EDIT_DRAW_BLOCK)
    {
        SearchDrawBlockInfo(x, y, edit_info);
    }
    else if (mode == AC_EDIT_DRAW_SMALL_SQUARE)
    {
        SearchDrawSmallSquareInfo(x, y, edit_info);
    }
    else if (mode == AC_EDIT_CHANGE_COLOR)
    {
        SearchChangeColorInfo(x, y, edit_info, option.is_change_color_area_mode);
    }
    else if (mode == AC_EDIT_MERGE_BLOCK)
    {
        SearchMergeBlockInfo(x, y, edit_info, option.is_alternated_edit);
    }
    else if (mode == AC_EDIT_INSERT_DELETE_SPACE)
    {
        SearchInsertDeleteSpaceInfo(x, y, edit_info);
    }
    else if (mode == AC_EDIT_DRAW_LARGE_BRUSH)
    {
        SearchDrawLargeBrushInfo(x, y, option.brush_size, option.brush_shape, edit_info);
    }
    else if (mode == AC_EDIT_ADD_TEXT)
    {
        SearchAddTextInfo(x, y, edit_info);
    }
    else if (mode == AC_EDIT_REFINE_BOUNDARY ||
             mode == AC_EDIT_REFINE_BOUNDARY_HALF ||
             mode == AC_EDIT_REFINE_BOUNDARY_TRIANGLE ||
             mode == AC_EDIT_REFINE_BOUNDARY_REGULAR_TRIANGLE)
    {
        RefineBoundaryInfo info;
        SearchRefineBoundaryInfo(x, y, mode, info);

        ConvertToEditInfo(info, edit_info);
    }
}

bool AnsiCanvas::GetColorAtLocation(int x, int y, AnsiColor &color)
{
    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(x, y, x_offset, y_offset) == false)
        return false;

    color = CanvasColorAt(x_offset, y_offset);

    return true;
}

bool AnsiCanvas::GetChangeableColorAtLocation(int x, int y, AnsiColor &color)
{
    // Get color for "Change Color" feature.
    // For general-character cell, get the foreground/background colors.
    // For special-shaped cell, get the single color according to the shape.

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(x, y, x_offset, y_offset) == false)
        return false;

    int cell_offset;
    AnsiCharLocation location;
    if (SearchCellInfo(m_cells[y_offset], x_offset, cell_offset, location) == false)
        return false;

    int x_shift = x - ToCoordX(x_offset);
    int y_shift = y - ToCoordY(y_offset);
    const AnsiCell &cell = m_cells[y_offset][cell_offset];
    if (cell.type == GENERAL_CHAR)
    {
        color = mp_colors[y_offset * m_width + x_offset];
    }
    else
    {
        color.fg_color =  GetSingleColorInCell(CanvasColorAt(x_offset, y_offset), 
                                               cell, x_shift, y_shift, location);
        color.bg_color = ToNormalColor(color.fg_color);
    }

    return true;
}

int AnsiCanvas::GetSingleColorInCell(const AnsiColor &color, const AnsiCell &cell, 
                                     int x_shift, int y_shift, AnsiCharLocation location)
{
    if (color.fg_color == color.bg_color)
        return color.fg_color;

    if (cell.type == HORI_BLOCK)
    {
        int y_bound = m_block_size - ch_Round((float)m_block_size * cell.label / 8);
        if (y_shift >= y_bound)
            return color.fg_color;
        else
            return color.bg_color;
    }
    else if (cell.type == VERT_BLOCK)
    {
        int x_bound = ch_Round((float)m_block_size * cell.label / 8);
        if (location == DOUBLE_CHAR_RIGHT)
            x_bound -= m_half_block_size;

        if (x_shift >= x_bound)
            return color.bg_color;
        else
            return color.fg_color;
    }
    else if (cell.type == TRIANGLE)
    {
        bool is_foreground = IsForegroundTriangle(x_shift, y_shift, location, cell.label);

        if (is_foreground)
            return color.fg_color;
        else
            return color.bg_color;
    }
    else if (cell.type == REGULAR_TRIANGLE)
    {
        bool is_foreground = IsForegroundRegularTriangle(x_shift, y_shift, location, cell.label);

        if (is_foreground)
            return color.fg_color;
        else
            return color.bg_color;
    }
    else
    {
        // treat as EMPTY cell
        return color.bg_color;
    }
}

void AnsiCanvas::SetSingleColorInCell(int x_offset, int y_offset,
                                      AnsiCell &cell, int new_color,
                                      int x_shift, int y_shift, AnsiCharLocation location)
{
    if (x_offset < 0 || x_offset >= m_width)
        return;
    if (y_offset < 0 || y_offset >= m_height)
        return;

    AnsiColor &color = CanvasColorAt(x_offset, y_offset);

    bool is_set_foreground = true;
    if (cell.type == HORI_BLOCK)
    {
        int y_bound = GetHoriBorder(cell.label);
        is_set_foreground = (y_shift >= y_bound);
    }
    else if (cell.type == VERT_BLOCK)
    {
        int x_bound = GetVertBorder(cell.label);
        if (location == DOUBLE_CHAR_RIGHT)
            x_bound -= m_half_block_size;

        is_set_foreground = (x_shift < x_bound);
    }
    else if (cell.type == TRIANGLE)
    {
        is_set_foreground = IsForegroundTriangle(x_shift,  y_shift, location, cell.label);
    }
    else if (cell.type == REGULAR_TRIANGLE)
    {
        is_set_foreground = IsForegroundRegularTriangle(x_shift,  y_shift, location, cell.label);
    }
    else
    {
        // treat as EMPTY cell
        is_set_foreground = false;
    }

    if (cell.type == TRIANGLE && is_set_foreground == false && IsBrightColor(new_color))
    {
        // For triangle cell, we flip the cell to set bright color to the target region.
        // But the original foreground color need to be normal color.

        int other_x_offset = -1;
        if (location == DOUBLE_CHAR_LEFT)
            other_x_offset = x_offset + 1;
        else if (location == DOUBLE_CHAR_RIGHT)
            other_x_offset = x_offset - 1;

        if (other_x_offset >= 0 && other_x_offset < m_width)
        {
            AnsiColor &other_color = CanvasColorAt(other_x_offset, y_offset); 

            if (IsNormalColor(color.fg_color) && IsNormalColor(other_color.fg_color))
            {
                cell.label = 3 - cell.label; // become opposite triangle
                ch_Swap(color.fg_color, color.bg_color);
                ch_Swap(other_color.fg_color, other_color.bg_color);

                is_set_foreground = true;
            }
        }
    }

    if (is_set_foreground)
        color.fg_color = new_color;
    else
        color.bg_color = ToNormalColor(new_color);
}

void AnsiCanvas::SearchViewInfo(int x, int y, AC_EditInfo &info)
{
    info.is_valid = false;

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(x, y, x_offset, y_offset) == false)
        return;

    int cell_offset;
    AnsiCharLocation location;
    bool is_success = SearchCellInfo(m_cells[y_offset], x_offset, cell_offset, location);
    if (is_success == false)
        return;

    info.is_valid = true;
    info.mode = AC_EDIT_VIEW;

    info.target_rect.x = ToCoordX(x_offset);
    info.target_rect.y = ToCoordY(y_offset);
    info.target_rect.width = m_half_block_size;
    info.target_rect.height = m_block_size;

    info.cell = m_cells[y_offset][cell_offset];
    info.color = CanvasColorAt(x_offset, y_offset);

    if (location == DOUBLE_CHAR_LEFT)
    {
        info.covered_rects.resize(1);
        info.covered_rects[0].x = info.target_rect.x;
        info.covered_rects[0].y = info.target_rect.y;
        info.covered_rects[0].width = m_block_size;
        info.covered_rects[0].height = m_block_size;
    }
    else if (location == DOUBLE_CHAR_RIGHT)
    {
        info.covered_rects.resize(1);
        info.covered_rects[0].x = info.target_rect.x - m_half_block_size;
        info.covered_rects[0].y = info.target_rect.y;
        info.covered_rects[0].width = m_block_size;
        info.covered_rects[0].height = m_block_size;
    }
    else
    {
        info.covered_rects.clear();
    }
}

void AnsiCanvas::SearchDrawSpaceInfo(int x, int y, AC_EditInfo &info)
{
    info.is_valid = false;

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(x, y, x_offset, y_offset) == false)
        return;

    AnsiCharLocation location;
    bool is_success = GetCharLocation(m_cells[y_offset], x_offset, location);
    if (is_success == false)
        return;

    info.is_valid = true;
    info.mode = AC_EDIT_DRAW_SPACE;
    info.target_rect.x = ToCoordX(x_offset);
    info.target_rect.y = ToCoordY(y_offset);
    info.target_rect.width = m_half_block_size;
    info.target_rect.height = m_block_size;

    if (location == SINGLE_CHAR)
    {
        info.covered_rects.clear();
    }
    else
    {
        info.covered_rects.resize(1);

        HyRect &rect1 = info.covered_rects[0];
        rect1.width = m_block_size;
        rect1.height = m_block_size;

        if (location == DOUBLE_CHAR_LEFT)
        {
            rect1.x = info.target_rect.x;
            rect1.y = info.target_rect.y;
        }
        else
        {
            rect1.x = info.target_rect.x - m_half_block_size;
            rect1.y = info.target_rect.y;
        }
    }
}

void AnsiCanvas::SearchDrawBlockInfo(int x, int y, AC_EditInfo &info)
{
    info.is_valid = false;

    int x_offset = 0, y_offset = 0;
    if (GetBlockXYOffset(x, y, x_offset, y_offset) == false)
        return;

    int cell_offset;
    AnsiCharLocation location;
    const AnsiRow &row = m_cells[y_offset];
    bool is_success = SearchCellInfo(row, x_offset, cell_offset, location);
    if (is_success == false)
        return;

    info.is_valid = true;
    info.mode = AC_EDIT_DRAW_BLOCK;

    HyRect this_rect;
    this_rect.x = ToCoordX(x_offset);
    this_rect.y = ToCoordY(y_offset);
    this_rect.width = m_block_size;
    this_rect.height = m_block_size;
    info.target_rect = this_rect;

    if (location == DOUBLE_CHAR_LEFT)
    {
        info.covered_rects.clear();
    }
    else
    {
        info.covered_rects.resize(2);

        HyRect &rect1 = info.covered_rects[0];
        HyRect &rect2 = info.covered_rects[1];
        rect1.y = rect2.y = this_rect.y;
        rect1.height = rect2.height = m_block_size;

        if (location == SINGLE_CHAR)
        {
            rect1.x = this_rect.x;
            rect1.width = m_half_block_size;
        }
        else
        {
            rect1.x = this_rect.x - m_half_block_size;
            rect1.width = m_block_size;
        }

        rect2.x = this_rect.x + m_half_block_size;
        const AnsiCell &right_cell = row[cell_offset + 1];
        if (right_cell.IsDoubleChar())
            rect2.width = m_block_size;
        else
            rect2.width = m_half_block_size;
    }   
}

void AnsiCanvas::SearchDrawSmallSquareInfo(int x, int y, AC_EditInfo &info)
{
    info.is_valid = false;

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(x, y, x_offset, y_offset) == false)
        return;

    bool is_left_valid = IsCanDrawSmallSquare(x_offset - 1, y_offset);
    bool is_right_valid = IsCanDrawSmallSquare(x_offset, y_offset);

    // If both sides are valid or invalid, use the location to determine offset.
    // (Force draw when both sides are invalid.)
    if (is_left_valid == is_right_valid)
    {
        int x_shift = x - ToCoordX(x_offset);
        if (x_shift < m_half_block_size / 2)
        {
            is_left_valid = true;
            is_right_valid = false;
        }
        else
        {
            is_left_valid = false;
            is_right_valid = true;
        }
    }

    info.is_valid = true;
    info.mode = AC_EDIT_DRAW_SMALL_SQUARE;

    int base_x = ToCoordX(x_offset);
    int base_y = ToCoordY(y_offset);
    int square_x = base_x;
    int square_y = base_y;
    if (y - base_y >= m_half_block_size)
        square_y += m_half_block_size;

    HyRect this_rect;
    this_rect.x = square_x;
    this_rect.y = square_y;
    this_rect.width = m_half_block_size;
    this_rect.height = m_half_block_size;
    info.target_rect = this_rect;

    HyRect left_block_rect(base_x - m_half_block_size, base_y, m_block_size, m_block_size);
    HyRect right_block_rect(base_x, base_y, m_block_size, m_block_size);

    info.covered_rects.resize(1);
    HyRect &cover_rect = info.covered_rects[0];
    if (is_left_valid)
        cover_rect = left_block_rect;
    else
        cover_rect = right_block_rect;
}

bool AnsiCanvas::IsCanDrawSmallSquare(int x_offset, int y_offset)
{
    // Check the block at ([x_offset, x_offset+1], y_offset).

    if (x_offset < 0 || x_offset >= m_width - 1)
        return false;
    if (y_offset < 0 || y_offset >= m_height)
        return false;

    const AnsiRow &row = m_cells[y_offset];

    int left_cell_offset, right_cell_offset;
    AnsiCharLocation left_location, right_location;
    if (SearchCellInfo(row, x_offset, left_cell_offset, left_location) == false ||
        SearchCellInfo(row, x_offset + 1, right_cell_offset, right_location) == false)
        return false;

    if (left_location == DOUBLE_CHAR_LEFT)
        return true;
    if (left_location == SINGLE_CHAR && right_location == SINGLE_CHAR)
        return true;

    bool is_valid = false;

    AC_EditInfo merge_info;
    SearchMergeBlockInfoAtOffset(x_offset, y_offset, merge_info, false);
    if (merge_info.is_valid)
    {
        const AnsiCell &left_cell = row[left_cell_offset];
        const AnsiCell &right_cell = row[right_cell_offset];

        int left_space_color, right_space_color;
        bool is_left_single_color = GetPureSpaceColor(x_offset, y_offset, left_space_color);
        bool is_right_single_color = GetPureSpaceColor(x_offset + 1, y_offset, right_space_color);

        bool is_left_valid = (is_left_single_color && IsNormalColor(left_space_color)) ||
                             (left_cell.type == HORI_BLOCK && left_cell.label == 4);
        bool is_right_valid = (is_right_single_color && IsNormalColor(right_space_color)) ||
                              (right_cell.type == HORI_BLOCK && right_cell.label == 4);

        is_valid = is_left_valid && is_right_valid;
    }

    return is_valid;
}

void AnsiCanvas::SearchChangeColorInfo(int x, int y, AC_EditInfo &info, bool is_area_mode)
{
    info.is_valid = false;

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(x, y, x_offset, y_offset) == false)
        return;

    int cell_offset;
    AnsiCharLocation location;
    bool is_success = SearchCellInfo(m_cells[y_offset], x_offset, cell_offset, location);
    if (is_success == false)
        return;

    info.is_valid = true;
    info.mode = AC_EDIT_CHANGE_COLOR;
    info.target_rect.x = ToCoordX(x_offset);
    info.target_rect.y = ToCoordY(y_offset);
    info.target_rect.width = m_half_block_size;
    info.target_rect.height = m_block_size;

    if (location == SINGLE_CHAR)
    {
        info.covered_rects.clear();
    }
    else
    {
        info.covered_rects.resize(1);

        HyRect &rect1 = info.covered_rects[0];
        rect1.width = m_block_size;
        rect1.height = m_block_size;

        if (location == DOUBLE_CHAR_LEFT)
        {
            rect1.x = info.target_rect.x;
            rect1.y = info.target_rect.y;
        }
        else
        {
            rect1.x = info.target_rect.x - m_half_block_size;
            rect1.y = info.target_rect.y;
        }
    }

    int x_shift = x - ToCoordX(x_offset);
    int y_shift = y - ToCoordY(y_offset);
    FindTargetRegion(info.target_rect, info.target_region,
                     m_cells[y_offset][cell_offset], x_shift, y_shift, location);

    if (is_area_mode)
    {
        FindChangeMask(hyPoint(x, y), info.change_mask_roi, info.change_mask_buffer);
    }
}

void AnsiCanvas::SearchMergeBlockInfo(int x, int y, AC_EditInfo &info, bool is_force_merge)
{
    info.is_valid = false;

    int x_offset = 0, y_offset = 0;
    if (GetBlockXYOffset(x, y, x_offset, y_offset) == false)
        return;

    return SearchMergeBlockInfoAtOffset(x_offset, y_offset, info, is_force_merge);
}

void AnsiCanvas::SearchMergeBlockInfoAtOffset(int x_offset, int y_offset, AC_EditInfo &info, bool is_force_merge)
{
    AC_MergeBlockType merge_type = SearchMergeOneBlockInfoAtOffset(x_offset, y_offset, info);
    if (merge_type == CANNOT_MERGE || merge_type == SAFE_MERGE)
        return; // no need to check other blocks

    std::vector<AC_EditInfo> left_merge_infos;
    bool is_left_valid = (merge_type == FORCE_MERGE_RIGHT_INVALID);
    if (is_left_valid == false)
    {
        int search_offset = x_offset - 2;
        while (search_offset >= 0)
        {
            AC_EditInfo search_info;
            AC_MergeBlockType search_type = SearchMergeOneBlockInfoAtOffset(search_offset, y_offset, search_info);
            if (search_type == CANNOT_MERGE)
                break;

            left_merge_infos.push_back(search_info);

            if (search_type == SAFE_MERGE || search_type == FORCE_MERGE_RIGHT_INVALID)
            {
                is_left_valid = true;
                break;
            }

            search_offset -= 2;
        }
    }

    std::vector<AC_EditInfo> right_merge_infos;
    bool is_right_valid = (merge_type == FORCE_MERGE_LEFT_INVALID);
    if (is_right_valid == false)
    {
        int search_offset = x_offset + 2;
        while (search_offset < m_width)
        {
            AC_EditInfo search_info;
            AC_MergeBlockType search_type = SearchMergeOneBlockInfoAtOffset(search_offset, y_offset, search_info);
            if (search_type == CANNOT_MERGE)
                break;

            right_merge_infos.push_back(search_info);

            if (search_type == SAFE_MERGE || search_type == FORCE_MERGE_LEFT_INVALID)
            {
                is_right_valid = true;
                break;
            }

            search_offset += 2;
        }
    }

    if (is_left_valid == false || is_right_valid == false)
    {
        // Only allow force merge in this case.
        if (is_force_merge == false)
            info.is_valid = false;

        return;
    }

    info.additional_merge_data.clear();

    for (int i = 0; i < (int)left_merge_infos.size(); i++)
    {
        const AC_EditInfo &merge_info = left_merge_infos[i];

        AC_AdditionalMergeData merge_data;
        merge_data.merge_rect = merge_info.target_rect;
        merge_data.cell = merge_info.cell;
        merge_data.left_color = merge_info.color;
        merge_data.right_color = merge_info.color2;

        info.additional_merge_data.push_back(merge_data);

        info.covered_rects.insert(info.covered_rects.end(),
                                  merge_info.covered_rects.begin(),
                                  merge_info.covered_rects.end());
    }

    for (int i = 0; i < (int)right_merge_infos.size(); i++)
    {
        const AC_EditInfo &merge_info = right_merge_infos[i];

        AC_AdditionalMergeData merge_data;
        merge_data.merge_rect = merge_info.target_rect;
        merge_data.cell = merge_info.cell;
        merge_data.left_color = merge_info.color;
        merge_data.right_color = merge_info.color2;

        info.additional_merge_data.push_back(merge_data);

        info.covered_rects.insert(info.covered_rects.end(),
                                  merge_info.covered_rects.begin(),
                                  merge_info.covered_rects.end());
    }

    // Remove redundant covered rectangles.
    int covered_rect_count = (int)info.covered_rects.size();
    for (int i = covered_rect_count - 1; i >= 0; i--)
    {
        const HyRect &rect = info.covered_rects[i];

        for (int j = 0; j < i; j++)
        {
            if (rect == info.covered_rects[j])
            {
                info.covered_rects.erase(info.covered_rects.begin() + i);
                break;
            }
        }   
    }
}

AC_MergeBlockType AnsiCanvas::SearchMergeOneBlockInfoAtOffset(int x_offset, int y_offset, AC_EditInfo &info)
{
    info.is_valid = false;
    info.additional_merge_data.clear();

    int left_cell_offset, right_cell_offset;
    AnsiCharLocation left_location, right_location;
    const AnsiRow &row = m_cells[y_offset];
    bool is_success = SearchCellInfo(row, x_offset, left_cell_offset, left_location) &&
                      SearchCellInfo(row, x_offset + 1, right_cell_offset, right_location);
    if (is_success == false)
        return CANNOT_MERGE;

    if (left_location == DOUBLE_CHAR_LEFT)
        return CANNOT_MERGE; // already a block

    // left/right cells must be different ones
    const AnsiCell &left_cell = row[left_cell_offset];
    const AnsiCell &right_cell = row[right_cell_offset];
    
    int space_color;

    bool is_left_neighbor_valid = true;
    if (left_location == DOUBLE_CHAR_RIGHT)
    {
        bool is_single_color = GetPureSpaceColor(x_offset - 1, y_offset, left_cell, false, space_color);
        is_left_neighbor_valid = is_single_color && IsNormalColor(space_color);
    }

    bool is_right_neighbor_valid = true;
    if (right_location == DOUBLE_CHAR_LEFT)
    {
        bool is_single_color = GetPureSpaceColor(x_offset + 2, y_offset, right_cell, true, space_color);
        is_right_neighbor_valid = is_single_color && IsNormalColor(space_color);
    }

    AnsiCell merged_cell;
    AnsiColor merged_colors[2];
    if (IsCanMergeBlock(x_offset, y_offset, 
                        left_cell, left_location, 
                        right_cell, right_location,
                        merged_cell, merged_colors) == false)
        return CANNOT_MERGE;

    info.is_valid = true;
    info.mode = AC_EDIT_MERGE_BLOCK;
    info.cell = merged_cell;
    info.color = merged_colors[0];
    info.color2 = merged_colors[1];

    HyRect this_rect;
    this_rect.x = ToCoordX(x_offset);
    this_rect.y = ToCoordY(y_offset);
    this_rect.width = m_block_size;
    this_rect.height = m_block_size;
    info.target_rect = this_rect;

    info.covered_rects.resize(2);

    HyRect &rect1 = info.covered_rects[0];
    HyRect &rect2 = info.covered_rects[1];
    rect1.y = rect2.y = this_rect.y;
    rect1.height = rect2.height = m_block_size;

    if (left_location == SINGLE_CHAR)
    {
        rect1.x = this_rect.x;
        rect1.width = m_half_block_size;
    }
    else
    {
        rect1.x = this_rect.x - m_half_block_size;
        rect1.width = m_block_size;
    }

    rect2.x = this_rect.x + m_half_block_size;
    if (right_cell.IsDoubleChar())
        rect2.width = m_block_size;
    else
        rect2.width = m_half_block_size;

    if (is_left_neighbor_valid && is_right_neighbor_valid)
        return SAFE_MERGE;
    else if (is_left_neighbor_valid)
        return FORCE_MERGE_RIGHT_INVALID;
    else if (is_right_neighbor_valid)
        return FORCE_MERGE_LEFT_INVALID;
    else
        return FORCE_MERGE_BOTH_INVALID;
}

void AnsiCanvas::SearchInsertDeleteSpaceInfo(int x, int y, AC_EditInfo &info)
{
    info.is_valid = false;

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(x, y, x_offset, y_offset) == false)
        return;

    AnsiCharLocation location;
    bool is_success = GetCharLocation(m_cells[y_offset], x_offset, location);
    if (is_success == false)
        return;

    info.is_valid = true;
    info.mode = AC_EDIT_INSERT_DELETE_SPACE;
    info.target_rect.x = ToCoordX(x_offset);
    info.target_rect.y = ToCoordY(y_offset);
    info.target_rect.width = m_half_block_size;
    info.target_rect.height = m_block_size;

    if (location == SINGLE_CHAR)
    {
        info.covered_rects.clear();
    }
    else
    {
        info.covered_rects.resize(1);

        HyRect &rect1 = info.covered_rects[0];
        rect1.width = m_block_size;
        rect1.height = m_block_size;

        if (location == DOUBLE_CHAR_LEFT)
        {
            rect1.x = info.target_rect.x;
            rect1.y = info.target_rect.y;
        }
        else
        {
            rect1.x = info.target_rect.x - m_half_block_size;
            rect1.y = info.target_rect.y;
        }
    }
}

void AnsiCanvas::FindTargetRegion(const HyRect &space_rect, std::vector<HyPoint> &target_region,
                                  const AnsiCell &cell, int x_shift, int y_shift, AnsiCharLocation location)
{
    if (cell.type == HORI_BLOCK)
    {
        int y_bound = m_block_size - ch_Round((float)m_block_size * cell.label / 8);
        bool is_upper_part = (y_shift < y_bound);

        HyRect target_rect = space_rect;
        if (is_upper_part)
        {
            target_rect.height = y_bound;
        }
        else
        {
            target_rect.y = space_rect.y + y_bound;
            target_rect.height = m_block_size - y_bound;
        }

        SetTargetRegionToRect(target_region, target_rect);
    }
    else if (cell.type == VERT_BLOCK)
    {
        int x_bound = ch_Round((float)m_block_size * cell.label / 8);
        if (location == DOUBLE_CHAR_RIGHT)
            x_bound -= m_half_block_size;

        bool is_left_part = (x_shift < x_bound);

        HyRect target_rect = space_rect;
        x_bound = FitInRange(x_bound, 0, m_half_block_size);
        if (is_left_part)
        {
            target_rect.width = x_bound;
        }
        else
        {
            target_rect.x = space_rect.x + x_bound;
            target_rect.width = m_half_block_size - x_bound;
        }

        SetTargetRegionToRect(target_region, target_rect);
    }
    else if (cell.type == TRIANGLE)
    {
        // dx, dy are relative to the top-left corner of full-width character.
        int dx = x_shift;
        int dy = y_shift;
        if (location == DOUBLE_CHAR_RIGHT)
            dx += m_half_block_size;

        // 0 1
        // 2 3
        // 4 5
        HyPoint space_points[6] = 
        {
            hyPoint(space_rect.x, space_rect.y),
            hyPoint(space_rect.Right(), space_rect.y),
            hyPoint(space_rect.x, space_rect.y + m_half_block_size),
            hyPoint(space_rect.Right(), space_rect.y + m_half_block_size),
            hyPoint(space_rect.x, space_rect.Bottom()),
            hyPoint(space_rect.Right(), space_rect.Bottom()),
        };

        if (cell.label == 0)
        {
            bool is_foreground = (dx + dy < m_block_size);
            if (location == DOUBLE_CHAR_RIGHT)
            {
                if (is_foreground)
                {
                    target_region.resize(3);
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[1];
                    target_region[2] = space_points[2];
                }
                else
                {
                    target_region.resize(4);
                    target_region[0] = space_points[2];
                    target_region[1] = space_points[1];
                    target_region[2] = space_points[5];
                    target_region[3] = space_points[4];
                }
            }
            else
            {
                if (is_foreground)
                {
                    target_region.resize(4);
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[1];
                    target_region[2] = space_points[3];
                    target_region[3] = space_points[4];
                }
                else
                {
                    target_region.resize(3);
                    target_region[0] = space_points[3];
                    target_region[1] = space_points[5];
                    target_region[2] = space_points[4];
                }
            }
        }
        else if (cell.label == 1)
        {
            bool is_foreground = (dx > dy);
            if (location == DOUBLE_CHAR_RIGHT)
            {
                if (is_foreground)
                {
                    target_region.resize(4);
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[1];
                    target_region[2] = space_points[5];
                    target_region[3] = space_points[2];
                }
                else
                {
                    target_region.resize(3);
                    target_region[0] = space_points[2];
                    target_region[1] = space_points[5];
                    target_region[2] = space_points[4];
                }
            }
            else
            {
                if (is_foreground)
                {
                    target_region.resize(3);
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[1];
                    target_region[2] = space_points[3];
                }
                else
                {
                    target_region.resize(4);
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[3];
                    target_region[2] = space_points[5];
                    target_region[3] = space_points[4];
                }
            }
        }
        else if (cell.label == 2)
        {
            bool is_foreground = (dx < dy);
            if (location == DOUBLE_CHAR_RIGHT)
            {
                if (is_foreground)
                {
                    target_region.resize(3);
                    target_region[0] = space_points[2];
                    target_region[1] = space_points[5];
                    target_region[2] = space_points[4];
                }
                else
                {
                    target_region.resize(4);
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[1];
                    target_region[2] = space_points[5];
                    target_region[3] = space_points[2];
                }
            }
            else
            {
                if (is_foreground)
                {
                    target_region.resize(4);
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[3];
                    target_region[2] = space_points[5];
                    target_region[3] = space_points[4];
                }
                else
                {
                    target_region.resize(3);
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[1];
                    target_region[2] = space_points[3];
                }
            }
        }
        else if (cell.label == 3)
        {
            bool is_foreground = (dx + dy > m_block_size);
            if (location == DOUBLE_CHAR_RIGHT)
            {
                if (is_foreground)
                {
                    target_region.resize(4);
                    target_region[0] = space_points[2];
                    target_region[1] = space_points[1];
                    target_region[2] = space_points[5];
                    target_region[3] = space_points[4];
                }
                else
                {
                    target_region.resize(3);
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[1];
                    target_region[2] = space_points[2];
                }
            }
            else
            {
                if (is_foreground)
                {
                    target_region.resize(3);
                    target_region[0] = space_points[3];
                    target_region[1] = space_points[5];
                    target_region[2] = space_points[4];
                }
                else
                {
                    target_region.resize(4);
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[1];
                    target_region[2] = space_points[3];
                    target_region[3] = space_points[4];
                }
            }
        }
    }
    else if (cell.type == REGULAR_TRIANGLE)
    {
        // 0 1
        // 
        // 2 3
        HyPoint space_points[4] = 
        {
            hyPoint(space_rect.x, space_rect.y),
            hyPoint(space_rect.Right(), space_rect.y),
            hyPoint(space_rect.x, space_rect.Bottom()),
            hyPoint(space_rect.Right(), space_rect.Bottom()),
        };

        bool is_foreground = IsForegroundRegularTriangle(x_shift, y_shift, location, cell.label);
        if (cell.label == 0) // upward triangle
        {
            target_region.resize(3);

            if (location == DOUBLE_CHAR_RIGHT)
            {
                if (is_foreground)
                {
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[3];
                    target_region[2] = space_points[2];
                }
                else
                {
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[1];
                    target_region[2] = space_points[3];
                }
            }
            else
            {
                if (is_foreground)
                {
                    target_region[0] = space_points[1];
                    target_region[1] = space_points[3];
                    target_region[2] = space_points[2];
                }
                else
                {
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[1];
                    target_region[2] = space_points[2];
                }
            }
        }
        else if (cell.label == 1) // downward triangle
        {
            target_region.resize(3);

            if (location == DOUBLE_CHAR_RIGHT)
            {
                if (is_foreground)
                {
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[1];
                    target_region[2] = space_points[2];
                }
                else
                {
                    target_region[0] = space_points[1];
                    target_region[1] = space_points[3];
                    target_region[2] = space_points[2];
                }
            }
            else
            {
                if (is_foreground)
                {
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[1];
                    target_region[2] = space_points[3];
                }
                else
                {
                    target_region[0] = space_points[0];
                    target_region[1] = space_points[3];
                    target_region[2] = space_points[2];
                }
            }
        }
    }
    else
    {
        // For either EMPTY or GENERAL_CHAR, the region is whole space.

        SetTargetRegionToRect(target_region, space_rect);
    }
}

void AnsiCanvas::SetTargetRegionToRect(std::vector<HyPoint> &target_region, const HyRect &rect)
{
    target_region.resize(4);
    target_region[0] = hyPoint(rect.x, rect.y);
    target_region[1] = hyPoint(rect.x + rect.width, rect.y);
    target_region[2] = hyPoint(rect.x + rect.width, rect.y + rect.height);
    target_region[3] = hyPoint(rect.x, rect.y + rect.height);
}

bool AnsiCanvas::IsCanMergeBlock(int x_offset, int y_offset, 
                                 const AnsiCell &left_cell, AnsiCharLocation left_location, 
                                 const AnsiCell &right_cell, AnsiCharLocation right_location,
                                 AnsiCell &merged_cell, AnsiColor merged_colors[2])
{
    // Check if a block can be merged at (x_offset, y_offset) without changing the contents.
    // Output merged_cell amd merged_colors if merging is valid.

    const AnsiColor &left_color = CanvasColorAt(x_offset, y_offset);
    const AnsiColor &right_color = CanvasColorAt(x_offset + 1, y_offset);

    int left_space_color, right_space_color;
    bool is_left_single_color = GetPureSpaceColor(x_offset, y_offset, left_cell, (left_location == DOUBLE_CHAR_RIGHT), left_space_color);
    bool is_right_single_color = GetPureSpaceColor(x_offset + 1, y_offset, right_cell, (right_location == DOUBLE_CHAR_RIGHT), right_space_color);

    if (is_left_single_color && is_right_single_color)
    {
        merged_cell = AnsiCell(HORI_BLOCK, 8);
        merged_colors[0] = AnsiColor(left_space_color);
        merged_colors[1] = AnsiColor(right_space_color);

        return true;
    }
    else if (is_left_single_color)
    {
        if (IsNormalColor(left_space_color) && right_cell.type == HORI_BLOCK)
        {
            merged_cell = AnsiCell(HORI_BLOCK, right_cell.label);
            merged_colors[0] = AnsiColor(left_space_color, left_space_color);
            merged_colors[1] = right_color;

            return true;
        }
        else if (right_cell.type == VERT_BLOCK)
        {
            merged_cell = AnsiCell(VERT_BLOCK, ch_Min(right_cell.label + 4, 8));
            merged_colors[0] = AnsiColor(left_space_color, right_color.bg_color);
            merged_colors[1] = right_color;

            return true;
        }
    }
    else if (is_right_single_color)
    {
        if (IsNormalColor(right_space_color) && left_cell.type == HORI_BLOCK)
        {
            merged_cell = AnsiCell(HORI_BLOCK, left_cell.label);
            merged_colors[0] = left_color;
            merged_colors[1] = AnsiColor(right_space_color, right_space_color);

            return true;
        }
        else if (IsNormalColor(right_space_color) && left_cell.type == VERT_BLOCK)
        {
            merged_cell = AnsiCell(VERT_BLOCK, ch_Max(left_cell.label - 4, 0));
            merged_colors[0] = left_color;
            merged_colors[1] = AnsiColor(left_color.fg_color, right_space_color);

            return true;
        }
    }
    else
    {
        if (left_cell.type == HORI_BLOCK && right_cell.type == HORI_BLOCK && left_cell.label == right_cell.label)
        {
            merged_cell = AnsiCell(HORI_BLOCK, left_cell.label);
            merged_colors[0] = left_color;
            merged_colors[1] = right_color;

            return true;
        }
    }    

    return false;
}

void AnsiCanvas::SearchDrawLargeBrushInfo(int x, int y, int brush_level, int brush_shape, AC_EditInfo &info)
{
    info.is_valid = false;
    info.target_region.clear();

    HyRect offset_range;
    HyRect unbounded_offset_range;
    GetLargeBrushOffsetRange(x, y, brush_level, brush_shape, offset_range, unbounded_offset_range);

    int x_offset_range = offset_range.width;
    int y_offset_range_2x = offset_range.height;
    if (x_offset_range <= 0 || y_offset_range_2x <= 0)
        return;

    int start_x_offset = offset_range.x;
    int end_x_offset = offset_range.Right();
    int start_y_offset_2x = offset_range.y;
    int end_y_offset_2x = offset_range.Bottom();

    HyImage *p_brush_mask = hyCreateImage(hySize(x_offset_range, y_offset_range_2x), HY_DEPTH_8U, 1);

    std::vector<HyPoint> brush_region;
    bool is_valid = GetBrushMask(offset_range, unbounded_offset_range, brush_shape, p_brush_mask);
    if (is_valid)
        is_valid = MakeBrushTargetRegion(offset_range, p_brush_mask, brush_region);

    hyReleaseImage(&p_brush_mask);

    if (is_valid == false)
        return;

    info.is_valid = true;
    info.mode = AC_EDIT_DRAW_LARGE_BRUSH;
    info.target_region = brush_region;
}

void AnsiCanvas::GetLargeBrushOffsetRange(int x, int y, int brush_level, int brush_shape,
                                          HyRect &offset_range, HyRect &unbounded_offset_range)
{
    int test_x = x - (m_block_size * (brush_level - 1) / 4);
    int test_y = y - (m_block_size * (brush_level - 1) / 4);

    // For some special brushes, the draw region is biased on even-number size.
    // Shift the test coordinates to compensate the bias.
    int bias_x = 0;
    int bias_y = 0;
    bool is_even_size = (brush_level % 2 == 0);
    if (is_even_size)
    {
        if (brush_shape == ANSI_BRUSH_HORI_LINE)
            bias_y -= m_block_size / 4;
        else if (brush_shape == ANSI_BRUSH_VERT_LINE)
            bias_x -= m_block_size / 4;
    }

    test_x += bias_x;
    test_y += bias_y;

    int start_x_offset = m_display_x_offset + FloorDivision(test_x, m_half_block_size);
    int start_y_offset = m_display_y_offset * 2 + FloorDivision(test_y, m_half_block_size);
    int end_x_offset = start_x_offset + brush_level;
    int end_y_offset = start_y_offset + brush_level;

    unbounded_offset_range.x = start_x_offset;
    unbounded_offset_range.y = start_y_offset;
    unbounded_offset_range.width = end_x_offset - start_x_offset;
    unbounded_offset_range.height = end_y_offset - start_y_offset;

    start_x_offset = ch_Max(start_x_offset, 0);
    start_y_offset = ch_Max(start_y_offset, 0);
    end_x_offset = ch_Min(end_x_offset, m_width);
    end_y_offset = ch_Min(end_y_offset, m_height * 2);

    offset_range.x = start_x_offset;
    offset_range.y = start_y_offset;
    offset_range.width = end_x_offset - start_x_offset;
    offset_range.height = end_y_offset - start_y_offset;
}

bool AnsiCanvas::GetBrushMask(const HyRect &offset_range,
                              const HyRect &unbounded_offset_range,
                              int brush_shape, HyImage *p_brush_mask)
{
    if (p_brush_mask == NULL)
        return false;

    int valid_width = offset_range.width;
    int valid_height = offset_range.height;
    if (p_brush_mask->width != valid_width || p_brush_mask->height != valid_height)
        return false;

    HY_ZEROIMAGE(p_brush_mask);

    if (brush_shape != ANSI_BRUSH_SQUARE)
    {
        int bound_shift_x = offset_range.x - unbounded_offset_range.x;
        int bound_shift_y = offset_range.y - unbounded_offset_range.y;

        // Compute circle radius by integer coordinates which have 2x resolution than the offset.
        // (Float coordinates may cause unexpected rounding error.)
        const int radius = unbounded_offset_range.width - 1;
        const int center_x_coord = radius;
        const int center_y_coord = radius;

        if (brush_shape == ANSI_BRUSH_CIRCLE)
        {
            const int distance2_threshold = radius * (radius + 1);

            for (int y = 0; y < valid_height; y++)
            {
                const int dy = (y + bound_shift_y) * 2 - center_y_coord;
                BYTE *p_mask_scan = p_brush_mask->imageData + y * p_brush_mask->widthStep;

                for (int x = 0; x < valid_width; x++)
                {
                    const int dx = (x + bound_shift_x) * 2 - center_x_coord;

                    if (dx * dx + dy * dy <= distance2_threshold)
                        p_mask_scan[x] = 255;
                }
            }
        }
        else if (brush_shape == ANSI_BRUSH_DIAGONAL)
        {
            const int distance_threshold = radius + 1;

            for (int y = 0; y < valid_height; y++)
            {
                const int abs_dy = ch_Abs((y + bound_shift_y) * 2 - center_y_coord);
                BYTE *p_mask_scan = p_brush_mask->imageData + y * p_brush_mask->widthStep;

                for (int x = 0; x < valid_width; x++)
                {
                    const int abs_dx = ch_Abs((x + bound_shift_x) * 2 - center_x_coord);

                    if (abs_dx + abs_dy <= distance_threshold)
                        p_mask_scan[x] = 255;
                }
            }
        }
        else if (brush_shape == ANSI_BRUSH_HORI_LINE)
        {
            int target_y = unbounded_offset_range.y + unbounded_offset_range.height / 2;
            if (target_y >= offset_range.y && target_y < offset_range.Bottom())
            {
                int valid_y = target_y - offset_range.y;
                for (int x = 0; x < valid_width; x++)
                    p_brush_mask->imageData[valid_y * p_brush_mask->widthStep + x] = 255;
            }
        }
        else if (brush_shape == ANSI_BRUSH_VERT_LINE)
        {
            int target_x = unbounded_offset_range.x + unbounded_offset_range.width / 2;
            if (target_x >= offset_range.x && target_x < offset_range.Right())
            {
                int valid_x = target_x - offset_range.x;
                for (int y = 0; y < valid_height; y++)
                    p_brush_mask->imageData[y * p_brush_mask->widthStep + valid_x] = 255;
            }
        }
    }
    else
    {
        // Full rectangle. Just fill the mask.

        memset(p_brush_mask->imageData, 255, p_brush_mask->widthStep * p_brush_mask->height);
    }

    bool is_valid = false;
    for (int y = 0; y < valid_height; y++)
    {
        const BYTE *p_mask_scan = p_brush_mask->imageData + y * p_brush_mask->widthStep;

        for (int x = 0; x < valid_width; x++)
        {
            if (p_mask_scan[x] == 255)
            {
                is_valid = true;
                break;
            }
        }
        
        if (is_valid)
            break;
    }

    return is_valid;
}

bool AnsiCanvas::MakeBrushTargetRegion(const HyRect &offset_range,
                                       HyImage *p_brush_mask, std::vector<HyPoint> &target_region)
{
    target_region.clear();

    const int valid_width = offset_range.width;
    const int valid_height = offset_range.height;

    const int pad_width = valid_width + 2;
    const int pad_height = valid_height + 2;

    HyPoint start_point;
    bool is_valid = false;
    for (int y = 0; y < valid_height; y++)
    {
        const BYTE *p_mask_scan = p_brush_mask->imageData + y * p_brush_mask->widthStep;

        for (int x = 0; x < valid_width; x++)
        {
            if (p_mask_scan[x] == 255)
            {
                start_point = hyPoint(x, y);
                is_valid = true;
                break;
            }
        }
        
        if (is_valid)
            break;
    }

    if (is_valid == false)
        return false;

    HyImage *p_pad_mask = hyCreateImage(hySize(pad_width, pad_height), HY_DEPTH_8U, 1);
    HY_ZEROIMAGE(p_pad_mask);
    ippiCopy_8u_C1R(p_brush_mask->imageData, p_brush_mask->widthStep,
                    p_pad_mask->imageData + p_pad_mask->widthStep + 1, p_pad_mask->widthStep,
                    ippiSize(valid_width, valid_height));

    enum SearchDirection
    {
        LEFT = 0,
        UP,
        RIGHT,
        DOWN,
        UNDEFINED,
    };

    HyPoint curr_corner(start_point.x + 1, start_point.y + 1);
    SearchDirection curr_direction = RIGHT;

    std::vector<HyPoint> corner_list;

    const BYTE *p_scan_data = p_pad_mask->imageData;
    const int scan_stride = p_pad_mask->widthStep;

    bool is_error = false;
    while (true)
    {
        HyPoint next_corner;
        SearchDirection next_direction = UNDEFINED;

        if (curr_direction == LEFT)
        {
            for (int x = curr_corner.x - 2; x >= 0; x--)
            {
                const BYTE *p_pixel = p_scan_data + curr_corner.y * scan_stride + x;
                if (p_pixel[0] == 255)
                    next_direction = DOWN;
                else if (p_pixel[-scan_stride] == 0)
                    next_direction = UP;

                if (next_direction != UNDEFINED)
                {
                    next_corner.x = x + 1;
                    next_corner.y = curr_corner.y;
                    break;
                }
            }
        }
        else if (curr_direction == UP)
        {
            for (int y = curr_corner.y - 2; y >= 0; y--)
            {
                const BYTE *p_pixel = p_scan_data + y * scan_stride + curr_corner.x;
                if (p_pixel[-1] == 255)
                    next_direction = LEFT;
                else if (p_pixel[0] == 0)
                    next_direction = RIGHT;

                if (next_direction != UNDEFINED)
                {
                    next_corner.x = curr_corner.x;
                    next_corner.y = y + 1;
                    break;
                }
            }
        }
        else if (curr_direction == RIGHT)
        {
            for (int x = curr_corner.x + 1; x < pad_width; x++)
            {
                const BYTE *p_pixel = p_scan_data + curr_corner.y * scan_stride + x;
                if (p_pixel[-scan_stride] == 255)
                    next_direction = UP;
                else if (p_pixel[0] == 0)
                    next_direction = DOWN;

                if (next_direction != UNDEFINED)
                {
                    next_corner.x = x;
                    next_corner.y = curr_corner.y;
                    break;
                }
            }
        }
        else if (curr_direction == DOWN)
        {
            for (int y = curr_corner.y + 1; y < pad_height; y++)
            {
                const BYTE *p_pixel = p_scan_data + y * scan_stride + curr_corner.x;
                if (p_pixel[0] == 255)
                    next_direction = RIGHT;
                else if (p_pixel[-1] == 0)
                    next_direction = LEFT;

                if (next_direction != UNDEFINED)
                {
                    next_corner.x = curr_corner.x;
                    next_corner.y = y;
                    break;
                }
            }
        }

        if (next_direction == UNDEFINED)
        {
            is_error = true;
            break;
        }

        corner_list.push_back(curr_corner);

        if (next_corner == corner_list[0])
            break;

        curr_corner = next_corner;
        curr_direction = next_direction;
    }

    hyReleaseImage(&p_pad_mask);

    if (is_error)
        return false;

    for (int i = 0; i < (int)corner_list.size(); i++)
    {
        int x_offset = offset_range.x + corner_list[i].x - 1;
        int y_offset_2x = offset_range.y + corner_list[i].y - 1;

        HyPoint region_point;
        region_point.x = ToCoordX(x_offset);
        region_point.y = ToCoordY_2x(y_offset_2x);

        target_region.push_back(region_point);
    }

    return true;
}

void AnsiCanvas::SearchAddTextInfo(int x, int y, AC_EditInfo &info)
{
    info.is_valid = false;

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(x, y, x_offset, y_offset) == false)
        return;

    AnsiCharLocation location;
    bool is_success = GetCharLocation(m_cells[y_offset], x_offset, location);
    if (is_success == false)
        return;

    const int propmt_amount = 4; // in half-width characters
    int prompt_width = ch_Min(propmt_amount, m_width - x_offset) * m_half_block_size;

    info.is_valid = true;
    info.mode = AC_EDIT_ADD_TEXT;
    info.target_rect.x = ToCoordX(x_offset);
    info.target_rect.y = ToCoordY(y_offset);
    info.target_rect.width = prompt_width;
    info.target_rect.height = m_block_size;

    info.covered_rects.clear();
    if (location == DOUBLE_CHAR_RIGHT)
    {
        info.covered_rects.resize(1);

        HyRect &rect1 = info.covered_rects[0];
        rect1.x = info.target_rect.x - m_half_block_size;
        rect1.y = info.target_rect.y;
        rect1.width = m_block_size;
        rect1.height = m_block_size;
    }
}


void AnsiCanvas::SearchRefineBoundaryInfo(int x, int y, AC_EditMode mode, RefineBoundaryInfo &info)
{
    info.is_valid = false;

    if (mode == AC_EDIT_REFINE_BOUNDARY)
    {
        RefineBoundaryInfo hori_info;
        SearchHorizontalRefineBoundaryInfo(x, y, hori_info);

        RefineBoundaryInfo vert_info;
        SearchVerticalRefineBoundaryInfo(x, y, vert_info);

        if (hori_info.is_valid && vert_info.is_valid)
        {
            const int small_distance_th = ch_Min(m_block_size / 8, 2);

            int hori_block_center_x = ToCoordX(hori_info.x_offset) + m_half_block_size;
            int x_to_center_distance = x - hori_block_center_x;

            if (x_to_center_distance >= m_half_block_size / 2 ||
                x_to_center_distance < -m_half_block_size / 2)
            {
                // Favor vertical edit if the horizonal edit block is far.

                if (vert_info.edit_distance <= ch_Max(hori_info.edit_distance, small_distance_th))
                    info = vert_info;
                else
                    info = hori_info;
            }
            else
            {
                // Favor horizontal edit block.

                if (hori_info.edit_distance <= ch_Max(vert_info.edit_distance, small_distance_th))
                    info = hori_info;
                else
                    info = vert_info;
            }
        }
        else if (hori_info.is_valid)
        {
            info = hori_info;
        }
        else if (vert_info.is_valid)
        {
            info = vert_info;
        }
    }
    else if (mode == AC_EDIT_REFINE_BOUNDARY_HALF)
    {
        RefineBoundaryInfo half_info;
        SearchHorizontalHalfRefineBoundaryInfo(x, y, half_info);
        if (half_info.is_valid)
            info = half_info;
    }
    else if (mode == AC_EDIT_REFINE_BOUNDARY_TRIANGLE)
    {
        SearchTriangleRefineBoundaryInfo(x, y, info);
    }
    else if (mode == AC_EDIT_REFINE_BOUNDARY_REGULAR_TRIANGLE)
    {
        SearchRegularTriangleRefineBoundaryInfo(x, y, info);
    }
}

void AnsiCanvas::SearchHorizontalRefineBoundaryInfo(int x, int y, RefineBoundaryInfo &info)
{
    info.is_valid = false;

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(x, y, x_offset, y_offset) == false)
        return;

    int level1, level2;
    AnsiColor left_color1, left_color2, right_color1, right_color2;
    int distance1 = GetHorizontalEditDistance(y, x_offset - 1, y_offset, level1, left_color1, right_color1);
    int distance2 = GetHorizontalEditDistance(y, x_offset, y_offset, level2, left_color2, right_color2);

    if (distance1 >= 0 && distance2 >= 0)
    {
        bool is_edit_left = false;
        if (distance1 < distance2)
        {
            is_edit_left = true;
        }
        else if (distance1 == distance2)
        {
            int shift = x - ToCoordX(x_offset);
            is_edit_left = (shift < m_half_block_size / 2);
        }

        if (is_edit_left)
            SetHorizontalRefineBoundaryInfo(x_offset - 1, y_offset, level1, left_color1, right_color1, distance1, info);
        else
            SetHorizontalRefineBoundaryInfo(x_offset, y_offset, level2, left_color2, right_color2, distance2, info);
    }
    else if (distance1 >= 0)
    {
        SetHorizontalRefineBoundaryInfo(x_offset - 1, y_offset, level1, left_color1, right_color1, distance1, info);
    }
    else if (distance2 >= 0)
    {
        SetHorizontalRefineBoundaryInfo(x_offset, y_offset, level2, left_color2, right_color2, distance2, info);
    }
}

int AnsiCanvas::GetHorizontalEditDistance(int coordinate, int x, int y, int &level, 
                                          AnsiColor &left_color, AnsiColor &right_color)
{
    // Return edit distance to the given y coordinate, or -1 if not editable.

    if (y < 0 || y >= m_height)
        return -1;
    if (x < 0 || x >= m_width - 1)
        return -1;

    const AnsiRow &row = m_cells[y];

    int offset;
    AnsiCharLocation location;
    bool is_success = SearchCellInfo(row, x, offset, location);
    if (is_success == false)
        return -1;

    AnsiColor left_canvas_color = CanvasColorAt(x, y);
    AnsiColor right_canvas_color = CanvasColorAt(x + 1, y);

    int color1, color2;

    bool is_editable = false;

    AC_EditInfo merge_info;
    SearchMergeBlockInfoAtOffset(x, y, merge_info, false);
    bool is_block_valid = (location == DOUBLE_CHAR_LEFT) || merge_info.is_valid;

    // Find special rules based on contents, i.e. half or full block has single color.
    int left_space_color, right_space_color;
    bool is_two_spaces = GetPureSpaceColor(x, y, left_space_color) && GetPureSpaceColor(x + 1, y, right_space_color);
    if (is_two_spaces)
        is_two_spaces = (left_space_color != right_space_color) && is_block_valid;

    if (is_two_spaces)
    {
        is_editable = IsColorSpacesHorizontalEditable(coordinate, x, y,
                                                      left_space_color, right_space_color,
                                                      level, left_color, right_color);
    }

    if (is_editable == false)
    {
        // Determine editing by cell itself.

        if (is_block_valid)
        {
            AnsiCell block_cell = row[offset];
            AnsiColor block_left_color = left_canvas_color;
            AnsiColor block_right_color = right_canvas_color;
            if (merge_info.is_valid)
            {
                block_cell = merge_info.cell;
                block_left_color = merge_info.color;
                block_right_color = merge_info.color2;
            }

            int block_color;
            bool is_single_block_color = GetSingleBlockColor(block_cell, block_left_color, block_right_color, block_color);
            if (is_single_block_color)
            {
                is_editable = IsColorBlockHorizontalEditable(coordinate, x, y, block_color,
                                                             level, left_color, right_color);
            }
            else if (block_cell.type == HORI_BLOCK)
            {
                // At here, if the block's label = 8, it is a horizontal block with different L/R color.
                // For this case, we determine horizontal edit by IsColorSpacesHorizontalEditable().
                // If this block doesn't pass the case, do not allow editing.

                if (block_cell.label < 8)
                {
                    is_editable = true;
                    level = block_cell.label;
                    left_color = block_left_color;
                    right_color = block_right_color;
                }
            }
        }
        else if (location == SINGLE_CHAR)
        {
            const AnsiCell &cell = row[offset];
            const AnsiCell &right_cell = row[offset + 1];

            if (cell.type == EMPTY)
            {
                if (right_cell.type == EMPTY)
                {
                    color1 = left_canvas_color.bg_color;
                    color2 = right_canvas_color.bg_color;
                    if (color1 == color2)
                    {
                        is_editable = IsColorBlockHorizontalEditable(coordinate, x, y, color1,
                                                                     level, left_color, right_color);
                    }
                }
                else if (right_cell.IsDoubleChar())
                {
                    int block_color;
                    bool is_same_color = GetSingleBlockColor(right_cell, right_canvas_color, CanvasColorAt(x + 2, y), block_color);
                    if (is_same_color)
                    {
                        color1 = left_canvas_color.bg_color;
                        if (color1 == block_color)
                        {
                            is_editable = IsColorBlockHorizontalEditable(coordinate, x, y, color1,
                                                                         level, left_color, right_color);
                        }
                    }
                }
            }
        }
    }    

    if (is_editable)
    {
        int location = ToCoordY(y + 1) - ch_Round((float)m_block_size * level / 8.0f);
        return ch_Abs(location - coordinate);
    }
    else
    {
        return -1;
    }
}

bool AnsiCanvas::IsColorBlockHorizontalEditable(int coordinate, int x, int y, int block_color,
                                                int &level, AnsiColor &left_color, AnsiColor &right_color)
{
    int boundary_color1;
    bool is_success = GetBottomBoundaryColor(x, x + 1, y - 1, boundary_color1);
    bool is_top_editable = (is_success && boundary_color1 != block_color && IsBrightColor(boundary_color1) == false);

    int boundary_color2;
    is_success = GetTopBoundaryColor(x, x + 1, y + 1, boundary_color2);
    bool is_bottom_editable = (is_success && boundary_color2 != block_color && IsBrightColor(block_color) == false);

    if (is_top_editable && is_bottom_editable)
    {
        if (coordinate >= ToCoordY(y) + m_half_block_size)
            is_top_editable = false;
        else
            is_bottom_editable = false;
    }

    if (is_top_editable)
    {
        level = 8;
        left_color = AnsiColor(block_color, boundary_color1);
        right_color = left_color;
        return true;
    }
    else if (is_bottom_editable)
    {
        level = 0;
        left_color = AnsiColor(boundary_color2, block_color);
        right_color = left_color;
        return true;
    }

    return false;
}

bool AnsiCanvas::IsColorSpacesHorizontalEditable(int coordinate, int x, int y, int left_color, int right_color,
                                                 int &level, AnsiColor &left_edit_color, AnsiColor &right_edit_color)
{
    int top_left_color, top_right_color;
    bool is_top_editable = GetBottomBoundaryColor(x, y - 1, top_left_color) &&
                           GetBottomBoundaryColor(x + 1, y - 1, top_right_color);
    if (is_top_editable)
        is_top_editable = (left_color != top_left_color || right_color != top_right_color) &&
                          IsNormalColor(top_left_color) && IsNormalColor(top_right_color);

    int bottom_left_color, bottom_right_color;
    bool is_bottom_editable = GetTopBoundaryColor(x, y + 1, bottom_left_color) &&
                              GetTopBoundaryColor(x + 1, y + 1, bottom_right_color);
    if (is_bottom_editable)
        is_bottom_editable = (left_color != bottom_left_color || right_color != bottom_right_color) &&
                              IsNormalColor(left_color) && IsNormalColor(right_color);

    if (is_top_editable && is_bottom_editable)
    {
        if (coordinate >= ToCoordY(y) + m_half_block_size)
            is_top_editable = false;
        else
            is_bottom_editable = false;
    }

    if (is_top_editable)
    {
        level = 8;
        left_edit_color = AnsiColor(left_color, top_left_color);
        right_edit_color = AnsiColor(right_color, top_right_color);
        return true;
    }
    else if (is_bottom_editable)
    {
        level = 0;
        left_edit_color = AnsiColor(bottom_left_color, left_color);
        right_edit_color = AnsiColor(bottom_right_color, right_color);
        return true;
    }

    return false;
}

void AnsiCanvas::SetHorizontalRefineBoundaryInfo(int x_offset, int y_offset, int level, 
                                                 const AnsiColor &left_color, const AnsiColor &right_color,
                                                 int edit_distance, RefineBoundaryInfo &info)
{
    info.x_offset = x_offset;
    info.y_offset = y_offset;
    info.type = REFINE_HORIZONTAL_BOUNDARY;
    info.level = level;
    info.left_color = left_color;
    info.right_color = right_color;
    info.edit_distance = edit_distance;

    info.is_valid = true;
}

void AnsiCanvas::SearchVerticalRefineBoundaryInfo(int x, int y, RefineBoundaryInfo &info)
{
    info.is_valid = false;

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(x, y, x_offset, y_offset) == false)
        return;

    int level1, level2;
    AnsiColor left_color1, left_color2, right_color1, right_color2;
    int distance1 = GetVerticalEditDistance(x, x_offset - 1, y_offset, level1, left_color1, right_color1);
    int distance2 = GetVerticalEditDistance(x, x_offset, y_offset, level2, left_color2, right_color2);

    if (distance1 >= 0 && distance2 >= 0)
    {
        int shift = x - ToCoordX(x_offset);
        if (shift < m_half_block_size / 2)
            SetVerticalRefineBoundaryInfo(x_offset - 1, y_offset, level1, left_color1, right_color1, distance1, info);
        else
            SetVerticalRefineBoundaryInfo(x_offset, y_offset, level2, left_color2, right_color2, distance2, info);
    }
    else if (distance1 >= 0)
    {
        SetVerticalRefineBoundaryInfo(x_offset - 1, y_offset, level1, left_color1, right_color1, distance1, info);
    }
    else if (distance2 >= 0)
    {
        SetVerticalRefineBoundaryInfo(x_offset, y_offset, level2, left_color2, right_color2, distance2, info);
    }
}

int AnsiCanvas::GetVerticalEditDistance(int coordinate, int x, int y, int &level, 
                                        AnsiColor &left_color, AnsiColor &right_color)
{
    // Return edit distance to the given x coordinate, or -1 if not editable.

    if (y < 0 || y >= m_height)
        return -1;
    if (x < 0 || x >= m_width - 1)
        return -1;

    const AnsiRow &row = m_cells[y];

    int offset;
    AnsiCharLocation location;
    bool is_success = SearchCellInfo(row, x, offset, location);
    if (is_success == false)
        return -1;

    const AnsiCell &cell = row[offset];
    const int row_size = (int)row.size();
    AnsiColor left_canvas_color = CanvasColorAt(x, y);
    AnsiColor right_canvas_color = CanvasColorAt(x + 1, y);

    bool is_editable = false;

    AC_EditInfo merge_info;
    SearchMergeBlockInfoAtOffset(x, y, merge_info, false);
    bool is_block_valid = (location == DOUBLE_CHAR_LEFT) || merge_info.is_valid;

    bool is_split_color = false;
    int split_color1, split_color2;

    if (is_block_valid)
    {
        AnsiCell block_cell = cell;
        AnsiColor block_left_color = left_canvas_color;
        AnsiColor block_right_color = right_canvas_color;
        if (merge_info.is_valid)
        {
            block_cell = merge_info.cell;
            block_left_color = merge_info.color;
            block_right_color = merge_info.color2;
        }

        int block_color;
        bool is_same_color = GetSingleBlockColor(block_cell, block_left_color, block_right_color, block_color);
        if (is_same_color)
        {
            is_editable = IsColorBlockVerticalEditable(coordinate, x, y, block_color,
                                                       level, left_color, right_color);
        }
        else
        {
            is_split_color = GetSplitBlockColor(block_cell, block_left_color, block_right_color, split_color1, split_color2);
            if (is_split_color)
                is_split_color = IsNormalColor(split_color2);

            if (is_split_color == false)
            {
                if (block_cell.type == VERT_BLOCK)
                {
                    is_editable = true;
                    level = block_cell.label;
                    left_color = block_left_color;
                    right_color = block_right_color;
                }
            }
        }
    }
    else if (location == SINGLE_CHAR)
    {
        if (cell.type == EMPTY && row[offset + 1].type == EMPTY)
        {
            int color1 = left_canvas_color.bg_color;
            int color2 = right_canvas_color.bg_color;

            if (color1 == color2)
            {
                is_editable = IsColorBlockVerticalEditable(coordinate, x, y, color1,
                                                           level, left_color, right_color);
            }
            else
            {
                is_split_color = true;
                split_color1 = color1;
                split_color2 = color2;
            }
        }
    }

    if (is_split_color)
    {
        is_editable = true;
        level = 4;
        left_color = AnsiColor(split_color1, split_color2);
        right_color = left_color;

        AnsiCharLocation temp_location;

        int x_shift = coordinate - ToCoordX(x);
        if (m_block_size - x_shift < x_shift - m_half_block_size)
        {
            int boundary_color;
            bool is_valid_color = GetLeftBoundaryColor(x + 2, y, boundary_color);
            if (is_valid_color)
                is_valid_color = IsNormalColor(boundary_color) && (boundary_color != split_color2);

            bool is_valid_cell = (location == DOUBLE_CHAR_LEFT);
            if (is_valid_cell == false)
            {
                if (GetCharLocation(row, x + 2, temp_location))
                    is_valid_cell = (temp_location == DOUBLE_CHAR_LEFT);
            }

            if (is_valid_color && is_valid_cell)
            {
                level = 8;
                left_color = AnsiColor(split_color1, ANSI_DEFAULT_BACKGROUND);
                right_color = AnsiColor(split_color2, boundary_color);
            }
        }
        else if (x_shift < m_half_block_size - x_shift)
        {
            int boundary_color;
            bool is_valid_color = GetRightBoundaryColor(x - 1, y, boundary_color);
            if (is_valid_color)
                is_valid_color = IsNormalColor(split_color1) && (boundary_color != split_color1);

            bool is_valid_cell = (location == DOUBLE_CHAR_LEFT);
            if (is_valid_cell == false)
            {
                if (GetCharLocation(row, x - 1, temp_location))
                    is_valid_cell = (temp_location == DOUBLE_CHAR_RIGHT);
            }

            if (is_valid_color && is_valid_cell)
            {
                level = 0;
                left_color = AnsiColor(boundary_color, split_color1);
                right_color = AnsiColor(ANSI_DEFAULT_FOREGROUND, split_color2);
            }
        }
    }

    if (is_editable)
    {
        int location = ToCoordX(x) + ch_Round((float)m_block_size * level / 8.0f);
        return ch_Abs(location - coordinate);
    }
    else
    {
        return -1;
    }
}

bool AnsiCanvas::IsColorBlockVerticalEditable(int coordinate, int x, int y, int block_color,
                                              int &level, AnsiColor &left_color, AnsiColor &right_color)
{
    int boundary_color1;
    bool is_success = GetRightBoundaryColor(x - 1, y, boundary_color1);
    bool is_left_editable = (is_success && boundary_color1 != block_color && IsBrightColor(block_color) == false);

    int boundary_color2;
    is_success = GetLeftBoundaryColor(x + 2, y, boundary_color2);
    bool is_right_editable = (is_success && boundary_color2 != block_color && IsBrightColor(boundary_color2) == false);

    if (is_left_editable && is_right_editable)
    {
        if (coordinate >= ToCoordX(x + 1))
            is_left_editable = false;
        else
            is_right_editable = false;
    }

    if (is_left_editable)
    {
        level = 0;
        left_color = AnsiColor(boundary_color1, block_color);
        right_color = left_color;
        return true;
    }
    else if (is_right_editable)
    {
        level = 8;
        left_color = AnsiColor(block_color, boundary_color2);
        right_color = left_color;
        return true;
    }

    return false;
}

void AnsiCanvas::SetVerticalRefineBoundaryInfo(int x_offset, int y_offset, int level, 
                                               const AnsiColor &left_color, const AnsiColor &right_color, 
                                               int edit_distance, RefineBoundaryInfo &info)
{
    info.x_offset = x_offset;
    info.y_offset = y_offset;
    info.type = REFINE_VERTICAL_BOUNDARY;
    info.level = level;
    info.left_color = left_color;
    info.right_color = right_color;
    info.edit_distance = edit_distance;

    info.is_valid = true;
}

void AnsiCanvas::SearchHorizontalHalfRefineBoundaryInfo(int x, int y, RefineBoundaryInfo &info)
{
    info.is_valid = false;

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(x, y, x_offset, y_offset) == false)
        return;

    int level1[2], level2[2];
    AnsiColor color1[2], color2[2];
    int distance1 = GetHorizontalHalfEditDistance(y, x_offset - 1, y_offset, false, level1, color1);
    int distance2 = GetHorizontalHalfEditDistance(y, x_offset, y_offset, true, level2, color2);

    if (distance1 >= 0 && distance2 >= 0)
    {
        int shift = x - ToCoordX(x_offset);
        if (shift < m_half_block_size / 2)
            SetHorizontalHalfRefineBoundaryInfo(x_offset - 1, y_offset, false, level1, color1, distance1, info);
        else
            SetHorizontalHalfRefineBoundaryInfo(x_offset, y_offset, true, level2, color2, distance2, info);
    }
    else if (distance1 >= 0)
    {
        SetHorizontalHalfRefineBoundaryInfo(x_offset - 1, y_offset, false, level1, color1, distance1, info);
    }
    else if (distance2 >= 0)
    {
        SetHorizontalHalfRefineBoundaryInfo(x_offset, y_offset, true, level2, color2, distance2, info);
    }
}

int AnsiCanvas::GetHorizontalHalfEditDistance(int coordinate, int x, int y,
                                              bool is_left_half, int level[2], AnsiColor color[2])
{
    // Return edit distance to the given y coordinate, or -1 if not editable.

    if (y < 0 || y >= m_height)
        return -1;
    if (x < 0 || x >= m_width - 1)
        return -1;

    const AnsiRow &row = m_cells[y];

    int offset;
    AnsiCharLocation location;
    bool is_success = SearchCellInfo(row, x, offset, location);
    if (is_success == false)
        return -1;

    const AnsiCell &cell = row[offset];
    AnsiColor left_canvas_color = CanvasColorAt(x, y);
    AnsiColor right_canvas_color = CanvasColorAt(x + 1, y);

    AnsiColor left_color = left_canvas_color;
    AnsiColor right_color = right_canvas_color;
    bool is_editable = false;

    AC_EditInfo merge_info;
    SearchMergeBlockInfoAtOffset(x, y, merge_info, false);
    bool is_block_valid = (location == DOUBLE_CHAR_LEFT) || merge_info.is_valid;

    if (is_block_valid)
    {
        AnsiCell block_cell = cell;
        AnsiColor block_left_color = left_color;
        AnsiColor block_right_color = right_color;
        if (merge_info.is_valid)
        {
            block_cell = merge_info.cell;
            block_left_color = merge_info.color;
            block_right_color = merge_info.color2;
        }

        int left_space_color, right_space_color;
        bool is_left_single_color = GetPureSpaceColor(block_left_color, block_cell, false, left_space_color);
        bool is_right_single_color = GetPureSpaceColor(block_right_color, block_cell, true, right_space_color);
        if (is_left_single_color && is_right_single_color)
        {
            int boundary_color;

            // Check as horizontal block with label = 8.
            AnsiColor edit_top_color[2];
            int edit_top_level[2];

            block_cell.type = HORI_BLOCK;
            block_cell.label = 8;
            block_left_color.fg_color = left_space_color;
            block_right_color.fg_color = right_space_color;

            bool is_edit_top_valid = false;
            if (is_left_half)
            {
                is_edit_top_valid = GetBottomBoundaryColor(x, y - 1, boundary_color);
                if (is_edit_top_valid)
                    is_edit_top_valid = IsNormalColor(boundary_color);

                if (is_edit_top_valid)
                    block_left_color.bg_color = boundary_color;
            }
            else
            {
                is_edit_top_valid = GetBottomBoundaryColor(x + 1, y - 1, boundary_color);
                if (is_edit_top_valid)
                    is_edit_top_valid = IsNormalColor(boundary_color);

                if (is_edit_top_valid)
                    block_right_color.bg_color = boundary_color;
            }

            if (is_edit_top_valid)
            {
                is_edit_top_valid = GetHalfEditLevels(x, y, coordinate,
                                                      block_left_color, block_right_color,
                                                      is_left_half, block_cell.label, edit_top_level);
            }

            if (is_edit_top_valid)
            {
                edit_top_color[0] = block_left_color;
                edit_top_color[1] = block_right_color;
            }

            // Check as horizontal block with label = 0.
            AnsiColor edit_bottom_color[2];
            int edit_bottom_level[2];

            bool is_edit_bottom_valid = false;
            if (IsNormalColor(left_space_color) && IsNormalColor(right_space_color))
            {
                block_cell.type = HORI_BLOCK;
                block_cell.label = 0;
                block_left_color.bg_color = left_space_color;
                block_right_color.bg_color = right_space_color;

                if (is_left_half)
                {
                    is_edit_bottom_valid = GetTopBoundaryColor(x, y + 1, boundary_color);
                    if (is_edit_bottom_valid)
                        block_left_color.fg_color = boundary_color;
                }
                else
                {
                    is_edit_bottom_valid = GetTopBoundaryColor(x + 1, y + 1, boundary_color);
                    if (is_edit_bottom_valid)
                        block_right_color.fg_color = boundary_color;
                }

                if (is_edit_bottom_valid)
                {
                    is_edit_bottom_valid = GetHalfEditLevels(x, y, coordinate,
                                                             block_left_color, block_right_color,
                                                             is_left_half, block_cell.label, edit_bottom_level);
                }

                if (is_edit_bottom_valid)
                {
                    edit_bottom_color[0] = block_left_color;
                    edit_bottom_color[1] = block_right_color;
                }
            }

            // If both are valid, pick on according to mouse coordinate.
            is_editable = (is_edit_top_valid || is_edit_bottom_valid);
            if (is_editable)
            {
                if (is_edit_top_valid && is_edit_bottom_valid)
                {
                    int y_distance = coordinate - ToCoordY(y);
                    if (y_distance < m_half_block_size)
                        is_edit_bottom_valid = false;
                    else
                        is_edit_top_valid = false;
                }

                if (is_edit_top_valid)
                {
                    level[0] = edit_top_level[0];
                    level[1] = edit_top_level[1];
                    color[0] = edit_top_color[0];
                    color[1] = edit_top_color[1];
                }
                else
                {
                    level[0] = edit_bottom_level[0];
                    level[1] = edit_bottom_level[1];
                    color[0] = edit_bottom_color[0];
                    color[1] = edit_bottom_color[1];
                }
            }
        }
        else if (block_cell.type == HORI_BLOCK)
        {
            int boundary_color;
            bool is_valid = true;
            if (block_cell.label == 0)
            {
                if (is_left_half)
                {
                    is_valid = GetTopBoundaryColor(x, y + 1, boundary_color);
                    if (is_valid)
                        block_left_color.fg_color = boundary_color;
                }
                else
                {
                    is_valid = GetTopBoundaryColor(x + 1, y + 1, boundary_color);
                    if (is_valid)
                        block_right_color.fg_color = boundary_color;
                }
            }
            else if (block_cell.label == 8)
            {
                if (is_left_half)
                {
                    is_valid = GetBottomBoundaryColor(x, y - 1, boundary_color);
                    if (is_valid)
                        is_valid = IsNormalColor(boundary_color);

                    if (is_valid)
                        block_left_color.bg_color = boundary_color;
                }
                else
                {
                    is_valid = GetBottomBoundaryColor(x + 1, y - 1, boundary_color);
                    if (is_valid)
                        is_valid = IsNormalColor(boundary_color);

                    if (is_valid)
                        block_right_color.bg_color = boundary_color;
                }
            }

            if (is_valid)
            {
                is_valid = GetHalfEditLevels(x, y, coordinate,
                                             block_left_color, block_right_color,
                                             is_left_half, block_cell.label, level);
            }

            if (is_valid)
            {
                is_editable = true;
                color[0] = block_left_color;
                color[1] = block_right_color;
            }
        }
    }
    else if (location == DOUBLE_CHAR_RIGHT)
    {
        const AnsiCell &right_cell = row[offset + 1];

        int base_level = 0;
        bool is_valid = false;

        if (cell.type == HORI_BLOCK)
        {
            int left2_color;
            is_valid = GetPureSpaceColor(x - 1, y, cell, false, left2_color);
            if (is_valid)
                is_valid = IsBrightColor(left2_color) == false;
        }

        if (is_valid)
        {
            is_valid = false;
            if (right_cell.type == EMPTY)
            {
                right_color.fg_color = right_canvas_color.bg_color;
                right_color.bg_color = right_canvas_color.bg_color;
                base_level = cell.label;
                is_valid = true;
            }
            else if (right_cell.type == HORI_BLOCK)
            {
                int right2_color;
                is_valid = GetPureSpaceColor(x + 2, y, right_cell, true, right2_color);
                if (is_valid)
                    is_valid = IsBrightColor(right2_color) == false;

                if (is_valid)
                {
                    int left_level = cell.label;
                    int right_level = right_cell.label;

                    is_valid = false;
                    if (left_level == 0)
                    {
                        left_color.fg_color = left_canvas_color.bg_color;
                        left_color.bg_color = left_canvas_color.bg_color;
                        base_level = right_level;
                        is_valid = true;
                    }
                    else if (left_level == 8 && IsBrightColor(left_canvas_color.fg_color) == false)
                    {
                        left_color.fg_color = left_canvas_color.fg_color;
                        left_color.bg_color = left_canvas_color.fg_color;
                        base_level = right_level;
                        is_valid = true;
                    }
                    else if (right_level == 0)
                    {
                        right_color.fg_color = right_canvas_color.bg_color;
                        right_color.bg_color = right_canvas_color.bg_color;
                        base_level = left_level;
                        is_valid = true;
                    }
                    else if (right_level == 8 && IsBrightColor(right_canvas_color.fg_color) == false)
                    {
                        right_color.fg_color = right_canvas_color.fg_color;
                        right_color.bg_color = right_canvas_color.fg_color;
                        base_level = left_level;
                        is_valid = true;
                    }
                }
            }
        }

        if (is_valid)
        {
            is_valid = GetHalfEditLevels(x, y, coordinate,
                                         left_color, right_color,
                                         is_left_half, base_level, level);
        }

        if (is_valid)
        {
            is_editable = true;
            color[0] = left_color;
            color[1] = right_color;
        }
    }
    else if (location == SINGLE_CHAR)
    {
        int left_space_color;

        int base_level = 0;
        bool is_valid = false;
        if (cell.type == EMPTY)
        {
            left_space_color = left_canvas_color.bg_color;
            const AnsiCell &right_cell = row[offset + 1];
            if (right_cell.type == EMPTY)
            {
                int right_space_color = right_canvas_color.bg_color;
                is_valid = IsHalfEditValid(x, y, coordinate, is_left_half,
                                           left_space_color, right_space_color,
                                           left_color, right_color, base_level);
            }
            else if (right_cell.type == HORI_BLOCK)
            {
                int right2_color;
                is_valid = GetPureSpaceColor(x + 2, y, right_cell, true, right2_color);
                if (is_valid)
                    is_valid = IsBrightColor(right2_color) == false;

                if (is_valid)
                {
                    left_color.fg_color = left_space_color;
                    left_color.bg_color = left_space_color;
                    base_level = right_cell.label;
                }
            }
        }

        if (is_valid)
        {
            is_valid = GetHalfEditLevels(x, y, coordinate,
                                         left_color, right_color,
                                         is_left_half, base_level, level);
        }

        if (is_valid)
        {
            is_editable = true;
            color[0] = left_color;
            color[1] = right_color;
        }
    }

    if (is_editable)
    {
        int moving_level = (is_left_half ? level[0] : level[1]);

        int location = ToCoordY(y + 1) - ch_Round((float)m_block_size * moving_level / 8.0f);
        return ch_Abs(location - coordinate);
    }
    else
    {
        return -1;
    }
}

bool AnsiCanvas::IsHalfEditValid(int x, int y, int y_coordinate, bool is_left_half,
                                 int left_space_color, int right_space_color,
                                 AnsiColor &left_color, AnsiColor &right_color, int &base_level)
{
    int y_distance = y_coordinate - ToCoordY(y);
    int top_boundary_color, bottom_boundary_color;

    bool is_edit_top_valid = false;
    bool is_edit_bottom_valid = false;
    if (is_left_half)
    {
        is_edit_top_valid = GetBottomBoundaryColor(x, y - 1, top_boundary_color);
        if (is_edit_top_valid)
            is_edit_top_valid = (left_space_color != top_boundary_color && IsBrightColor(top_boundary_color) == false);

        is_edit_bottom_valid = GetTopBoundaryColor(x, y + 1, bottom_boundary_color);
        if (is_edit_bottom_valid)
            is_edit_bottom_valid = (left_space_color != bottom_boundary_color);

        if (is_edit_top_valid && is_edit_bottom_valid)
        {
            if (y_distance < m_half_block_size)
                is_edit_bottom_valid = false;
            else
                is_edit_top_valid = false;
        }

        if (is_edit_top_valid)
        {
            left_color.fg_color = left_space_color;
            left_color.bg_color = top_boundary_color;
            right_color.fg_color = right_space_color;
            right_color.bg_color = right_space_color;
            base_level = 8;
        }
        else if (is_edit_bottom_valid)
        {
            left_color.fg_color = bottom_boundary_color;
            left_color.bg_color = left_space_color;
            right_color.fg_color = right_space_color;
            right_color.bg_color = right_space_color;
            base_level = 0;
        }
    }
    else
    {
        is_edit_top_valid = GetBottomBoundaryColor(x + 1, y - 1, top_boundary_color);
        if (is_edit_top_valid)
            is_edit_top_valid = (right_space_color != top_boundary_color && IsBrightColor(top_boundary_color) == false);

        is_edit_bottom_valid = GetTopBoundaryColor(x + 1, y + 1, bottom_boundary_color);
        if (is_edit_bottom_valid)
            is_edit_bottom_valid = (right_space_color != bottom_boundary_color);

        if (is_edit_top_valid && is_edit_bottom_valid)
        {
            if (y_distance < m_half_block_size)
                is_edit_bottom_valid = false;
            else
                is_edit_top_valid = false;
        }

        if (is_edit_top_valid)
        {
            left_color.fg_color = left_space_color;
            left_color.bg_color = left_space_color;
            right_color.fg_color = right_space_color;
            right_color.bg_color = top_boundary_color;
            base_level = 8;
        }
        else if (is_edit_bottom_valid)
        {
            left_color.fg_color = left_space_color;
            left_color.bg_color = left_space_color;
            right_color.fg_color = bottom_boundary_color;
            right_color.bg_color = right_space_color;
            base_level = 0;
        }
    }

    return (is_edit_top_valid || is_edit_bottom_valid);
}

bool AnsiCanvas::GetHalfEditLevels(int x, int y, int y_coordinate,
                                   AnsiColor &left_color, AnsiColor &right_color,
                                   bool is_left_half, int base_level, int level[2])
{
    // left_color / right_color are input parameters,
    // but they may need to be changed under some level combinations.

    AnsiColor &fix_color = (is_left_half ? right_color : left_color);
    AnsiColor &edit_color = (is_left_half ? left_color : right_color);
    int &fix_level = (is_left_half ? level[1] : level[0]);
    int &edit_level = (is_left_half ? level[0] : level[1]);

    bool is_single_fix_color = (fix_color.fg_color == fix_color.bg_color) ||
                               (base_level == 0) || (base_level == 8);
    bool is_single_edit_color = (edit_color.fg_color == edit_color.bg_color) ||
                                (base_level == 0) || (base_level == 8);

    bool is_valid = false;
    if (is_single_fix_color)
    {
        int single_color = fix_color.fg_color;
        if (base_level == 0)
            single_color = fix_color.bg_color;

        if (edit_color.fg_color != edit_color.bg_color)
        {
            if (edit_color.fg_color == single_color)
            {
                if (base_level == 0)
                    fix_color.fg_color = fix_color.bg_color;

                fix_level = 8;
                edit_level = base_level;
                is_valid = true;
            }
            else if (edit_color.bg_color == single_color)
            {
                if (base_level == 8)
                {
                    if (IsBrightColor(fix_color.fg_color))
                        return false;

                    fix_color.bg_color = fix_color.fg_color;
                }

                fix_level = 0;
                edit_level = base_level;
                is_valid = true;
            }
        }
    }
    else
    {
        if (is_single_edit_color)
        {
            int single_color = edit_color.fg_color;
            if (base_level == 0)
                single_color = edit_color.bg_color;

            int edit_x = (is_left_half ? x : x + 1);

            int top_color, bottom_color;
            bool is_top_editable = GetBottomBoundaryColor(edit_x, y - 1, top_color);
            bool is_bottom_editable = GetTopBoundaryColor(edit_x, y + 1, bottom_color);

            if (is_top_editable)
                is_top_editable = IsNormalColor(top_color) && (top_color != single_color);
            if (is_bottom_editable)
                is_bottom_editable = IsNormalColor(single_color) && (single_color != bottom_color);

            if (is_top_editable && is_bottom_editable)
            {
                int y_distance = y_coordinate - ToCoordY(y);
                if (y_distance < m_half_block_size)
                    is_bottom_editable = false;
                else
                    is_top_editable = false;
            }

            if (is_top_editable)
            {
                edit_color = AnsiColor(single_color, top_color);
                fix_level = base_level;
                edit_level = 8;
                is_valid = true;
            }
            else if (is_bottom_editable)
            {
                edit_color = AnsiColor(bottom_color, single_color);
                fix_level = base_level;
                edit_level = 0;
                is_valid = true;
            }
        }
        else
        {
            fix_level = base_level;
            edit_level = base_level;
            is_valid = true;
        }
    }

    if (is_valid)
    {
        // Exclude invalid cases with bright color.

        if (IsBrightColor(fix_color.fg_color))
        {
            if (fix_level == 8)
            {
                return false;
            }
            else if (fix_level > 0)
            {
                if (edit_level == 8 && IsBrightColor(edit_color.fg_color))
                    return false;
            }
        }

        return true;
    }

    return false;
}

void AnsiCanvas::SetHorizontalHalfRefineBoundaryInfo(int x_offset, int y_offset, bool is_left_half,
                                                     const int level[2], const AnsiColor color[2],
                                                     int edit_distance, RefineBoundaryInfo &info)
{
    info.x_offset = x_offset;
    info.y_offset = y_offset;
    info.type = REFINE_HORIZONTAL_HALF_BOUNDARY;
    info.level = level[0];
    info.right_level = level[1];
    info.left_color = color[0];
    info.right_color = color[1];
    info.initial_left_color = color[0];
    info.initial_right_color = color[1];
    info.is_left_half = is_left_half;
    info.edit_distance = edit_distance;

    info.is_valid = true;
}

void AnsiCanvas::SearchTriangleRefineBoundaryInfo(int x, int y, RefineBoundaryInfo &info)
{
    info.is_valid = false;

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(x, y, x_offset, y_offset) == false)
        return;

    int shape1, shape2;
    int level1, level2;
    RefineTriangleType type1, type2;
    AnsiColor left_color1, left_color2, right_color1, right_color2;

    bool is_valid1 = GetTriangleRefineStatus(x, y, x_offset - 1, y_offset, shape1, level1, type1, left_color1, right_color1);
    bool is_valid2 = GetTriangleRefineStatus(x, y, x_offset, y_offset, shape2, level2, type2, left_color2, right_color2);

    if (is_valid1 && is_valid2)
    {
        int shift = x - ToCoordX(x_offset);

        if (shift < m_half_block_size / 2)
            is_valid2 = false;
        else
            is_valid1 = false;
    }
    
    if (is_valid1)
    {
        SetTriangleRefineBoundaryInfo(x_offset - 1, y_offset, shape1, level1, type1, left_color1, right_color1, info);
    }
    else if (is_valid2)
    {
        SetTriangleRefineBoundaryInfo(x_offset, y_offset, shape2, level2, type2, left_color2, right_color2, info);
    }
}

bool AnsiCanvas::GetTriangleRefineStatus(int x_coord, int y_coord, int x_offset, int y_offset,
                                         int &shape, int &level, RefineTriangleType &type,
                                         AnsiColor &left_color, AnsiColor &right_color)
{
    // Find refine boundary status for normal (right) triangle.
    // The "shape" is the cell type of triangle.
    // To reduce the complexity, we only use shape 0 and 1.
    // The triangle label 2/3 are treated as shape 1/0 correspondingly.
    // The "level" in [0, 2] is defined for each refine case.

    // A subtype RefineTriangleType defines the exact boundary shape.
    // REFINE_TRIANGLE_FULL_BLOCK is the simple, full-block case.
    // REFINE_TRIANGLE_LEFT_HALF and REFINE_TRIANGLE_RIGHT_HALF are used for complicated half-width color and boundary.
    // For each combination, the boundary to be refined is:
    //
    //                 FULL_BLOCK          LEFT_HALF           RIGHT_HALF
    //             +-------0-------+   +---0---+-------+   +-------+---0---+
    //             |             / |   |       |       |   |       |     / | 
    //             |           /   |   |      1|2      |   |      0|   1   |
    //             |         /     |   |       |       |   |       | /     |
    //   shape 0   0       1       2   0       +       |   |       +       2
    // (label 0/3) |     /         |   |     / |       |   |       |       |
    //             |   /           |   |   1   |2      |   |      0|1      |
    //             | /             |   | /     |       |   |       |       |
    //             +-------2-------+   +---2---+-------+   +-------+---2---+
    //
    //             +-------0-------+   +---0---+-------+   +-------+---0---+
    //             | \             |   | \     |       |   |       |       | 
    //             |   \           |   |   1   |0      |   |      2|1      |
    //             |     \         |   |     \ |       |   |       |       | 
    //   shape 1   2       1       0   2       +       |   |       +       0
    // (label 1/2) |         \     |   |       |       |   |       | \     |
    //             |           \   |   |      1|0      |   |      2|   1   |
    //             |             \ |   |       |       |   |       |     \ |
    //             +-------2-------+   +---2---+-------+   +-------+---2---+

    if (y_offset < 0 || y_offset >= m_height)
        return false;
    if (x_offset < 0 || x_offset >= m_width - 1)
        return false;

    const AnsiRow &row = m_cells[y_offset];

    int cell_offset;
    AnsiCharLocation char_location;
    bool is_success = SearchCellInfo(row, x_offset, cell_offset, char_location);
    if (is_success == false)
        return false;

    const AnsiCell &cell = row[cell_offset];
    AnsiColor left_canvas_color = CanvasColorAt(x_offset, y_offset);
    AnsiColor right_canvas_color = CanvasColorAt(x_offset + 1, y_offset);

    AC_EditInfo merge_info;
    SearchMergeBlockInfoAtOffset(x_offset, y_offset, merge_info, false);
    bool is_block_valid = (char_location == DOUBLE_CHAR_LEFT) || merge_info.is_valid;
    if (is_block_valid == false)
        return false;

    AnsiCell src_cell = cell;
    AnsiColor src_left_color = left_canvas_color;
    AnsiColor src_right_color = right_canvas_color;
    if (merge_info.is_valid)
    {
        src_cell = merge_info.cell;
        src_left_color = merge_info.color;
        src_right_color = merge_info.color2;
    }

    int left_space_color, right_space_color;
    bool is_left_pure_color = GetPureSpaceColor(x_offset, y_offset, left_space_color);
    bool is_right_pure_color = GetPureSpaceColor(x_offset + 1, y_offset, right_space_color);

    int boundary_colors[6] = {0};
    bool is_boundary_valid_array[6] = {false};
    GetOutsideBlockBoundaryColors(x_offset, y_offset, boundary_colors, is_boundary_valid_array);

    int block_dx = x_coord - ToCoordX(x_offset);
    int block_dy = y_coord - ToCoordY(y_offset);

    // Find possible refinement for shape 0 and shape 1.
    // Allow the color to have bright background for this refinement.
    // When writing cells, we can use triangle label 2/3 to flip the colors.
    // (But still don't allow both foreground / background to be bright.)
    AnsiColor shape0_left_color, shape0_right_color;
    RefineTriangleType shape0_type;
    int shape0_level = GetShape0TriangleRefineLevel(block_dx, block_dy, src_cell, 
                                                    src_left_color, src_right_color,
                                                    is_left_pure_color, left_space_color,
                                                    is_right_pure_color, right_space_color,
                                                    is_boundary_valid_array, boundary_colors,
                                                    shape0_left_color, shape0_right_color, shape0_type);
    
    AnsiColor shape1_left_color, shape1_right_color;
    RefineTriangleType shape1_type;
    int shape1_level = GetShape1TriangleRefineLevel(block_dx, block_dy, src_cell, 
                                                    src_left_color, src_right_color,
                                                    is_left_pure_color, left_space_color,
                                                    is_right_pure_color, right_space_color,
                                                    is_boundary_valid_array, boundary_colors,
                                                    shape1_left_color, shape1_right_color, shape1_type);

    if (shape0_level >= 0 && shape1_level >= 0)
    {
        // If both cases are valid, the levels must be 0 or 2.
        // Choose one case according to current coordinates and boundary location.

        // For each case in shape0, the possible case in shape1 is limited.
        // List only the possible combinations.
        if (shape0_level == 0)
        {
            if (shape0_type == REFINE_TRIANGLE_FULL_BLOCK)
            {
                if (shape1_level == 0)
                {
                    // shape1_type: FULL_BLOCK

                    if (block_dx < m_half_block_size)
                        shape1_level = -1;
                    else
                        shape0_level = -1;
                }
                else if (shape1_level == 2)
                {
                    // shape1_type: FULL_BLOCK or LEFT_HALF

                    if (block_dy < m_half_block_size)
                        shape1_level = -1;
                    else
                        shape0_level = -1;
                }
            }
            else if (shape0_type == REFINE_TRIANGLE_LEFT_HALF)
            {
                if (shape1_level == 0)
                {
                    // shape1_type: RIGHT_HALF

                    if (block_dx < m_half_block_size)
                        shape1_level = -1;
                    else
                        shape0_level = -1;
                }
                else if (shape1_level == 2)
                {
                    // shape1_type: FULL_BLOCK or LEFT_HALF

                    if (block_dy < m_half_block_size)
                        shape1_level = -1;
                    else
                        shape0_level = -1;
                }
            }
            else if (shape0_type == REFINE_TRIANGLE_RIGHT_HALF)
            {
                if (shape1_level == 0)
                {
                    // shape1_type: LEFT_HALF

                    if (block_dx < m_half_block_size)
                        shape0_level = -1;
                    else
                        shape1_level = -1;
                }
                else if (shape1_level == 2)
                {
                    // shape1_type: RIGHT_HALF

                    if (block_dy < m_half_block_size)
                        shape1_level = -1;
                    else
                        shape0_level = -1;
                }
            }
        }
        else if (shape0_level == 2)
        {
            if (shape0_type == REFINE_TRIANGLE_FULL_BLOCK)
            {
                if (shape1_level == 0)
                {
                    // shape1_type: FULL_BLOCK or RIGHT_HALF

                    if (block_dy < m_half_block_size)
                        shape0_level = -1;
                    else
                        shape1_level = -1;
                }
                else if (shape1_level == 2)
                {
                    // shape1_type: FULL_BLOCK

                    if (block_dx < m_half_block_size)
                        shape0_level = -1;
                    else
                        shape1_level = -1;
                }
            }
            else if (shape0_type == REFINE_TRIANGLE_LEFT_HALF)
            {
                if (shape1_level == 0)
                {
                    // shape1_type: LEFT_HALF

                    if (block_dy < m_half_block_size)
                        shape0_level = -1;
                    else
                        shape1_level = -1;
                }
                else if (shape1_level == 2)
                {
                    // shape1_type: RIGHT_HALF

                    if (block_dx < m_half_block_size)
                        shape1_level = -1;
                    else
                        shape0_level = -1;
                }
            }
            else if (shape0_type == REFINE_TRIANGLE_RIGHT_HALF)
            {
                if (shape1_level == 0)
                {
                    // shape1_type: FULL_BLOCK or RIGHT_HALF

                    if (block_dy < m_half_block_size)
                        shape0_level = -1;
                    else
                        shape1_level = -1;
                }
                else if (shape1_level == 2)
                {
                    // shape1_type: LEFT_HALF

                    if (block_dx < m_half_block_size)
                        shape0_level = -1;
                    else
                        shape1_level = -1;
                }
            }
        }
    }

    if (shape0_level >= 0)
    {
        shape = 0;
        level = shape0_level;
        type = shape0_type;
        left_color = shape0_left_color;
        right_color = shape0_right_color;

        return true;
    }
    else if (shape1_level >= 0)
    {
        shape = 1;
        level = shape1_level;
        type = shape1_type;
        left_color = shape1_left_color;
        right_color = shape1_right_color;

        return true;
    }

    return false;
}

int AnsiCanvas::GetShape0TriangleRefineLevel(int block_dx, int block_dy, const AnsiCell &src_cell,
                                             const AnsiColor &src_left_color, const AnsiColor &src_right_color, 
                                             bool is_left_pure_color, int left_space_color,
                                             bool is_right_pure_color, int right_space_color,
                                             const bool is_boundary_valid_array[6], const int boundary_colors[6],
                                             AnsiColor &left_color, AnsiColor &right_color, RefineTriangleType &type)
{
    // Check refinement for triangle shape 0 (label 0 or 3).
    // The generated color can have bright background, but not both bright foreground/background.
    // (Use triangle label 3 to write bright background color.)

    const bool is_target_triangle = (src_cell.type == TRIANGLE) &&
                                    (src_cell.label == 0 || src_cell.label == 3);
    if (is_target_triangle == false && (is_left_pure_color == false || is_right_pure_color == false))
        return -1;

    bool is_level1_valid = false;
    AnsiColor level1_left_color;
    AnsiColor level1_right_color;
    RefineTriangleType level1_type;

    if (is_target_triangle && (is_left_pure_color == false || is_right_pure_color == false))
    {
        AnsiColor curr_left_color = src_left_color;
        AnsiColor curr_right_color = src_right_color;
        if (src_cell.label == 3)
        {
            curr_left_color = curr_left_color.FlipColor();
            curr_right_color = curr_right_color.FlipColor();
        }

        if (is_left_pure_color == false && is_right_pure_color == false)
        {
            if (curr_left_color == curr_right_color)
            {
                level1_left_color = curr_left_color;
                level1_right_color = curr_right_color;
                level1_type = REFINE_TRIANGLE_FULL_BLOCK;
                is_level1_valid = true;
            }
        }
        else if (is_left_pure_color == false)
        {
            // Right space is pure color. Check for REFINE_TRIANGLE_LEFT_HALF.
            if (curr_left_color.bg_color == right_space_color)
            {
                level1_left_color = curr_left_color;
                level1_right_color = curr_right_color;
                level1_type = REFINE_TRIANGLE_LEFT_HALF;
                is_level1_valid = true;
            }
        }
        else if (is_right_pure_color == false)
        {
            // Left space is pure color. Check for REFINE_TRIANGLE_RIGHT_HALF.
            if (left_space_color == curr_right_color.fg_color)
            {
                level1_left_color = curr_left_color;
                level1_right_color = curr_right_color;
                level1_type = REFINE_TRIANGLE_RIGHT_HALF;
                is_level1_valid = true;
            }
        }
    }

    if (is_level1_valid)
    {
        left_color = level1_left_color;
        right_color = level1_right_color;
        type = level1_type;
        return 1;
    }

    bool is_level0_valid = false;
    bool is_level2_valid = false;
    AnsiColor level0_left_color, level0_right_color;
    AnsiColor level2_left_color, level2_right_color;
    RefineTriangleType level0_type;
    RefineTriangleType level2_type;

    bool is_block_pure_color = (is_left_pure_color && is_right_pure_color && left_space_color == right_space_color);
    
    if (is_block_pure_color)
    {
        int block_color = left_space_color;

        if (is_boundary_valid_array[0] &&
            is_boundary_valid_array[1] &&
            boundary_colors[0] == boundary_colors[1] &&
            boundary_colors[0] != block_color &&
            IsAllBrightColor(boundary_colors[0], block_color) == false)
        {
            int target_fg_color = boundary_colors[0];
            int target_bg_color = block_color;

            level0_left_color.fg_color = target_fg_color;
            level0_left_color.bg_color = target_bg_color;

            if (is_boundary_valid_array[2] && boundary_colors[2] == target_fg_color)
            {
                level0_right_color = level0_left_color;
                level0_type = REFINE_TRIANGLE_FULL_BLOCK;
                is_level0_valid = true;
            }
            else if (IsNormalColor(target_bg_color))
            {
                level0_right_color.fg_color = target_bg_color;
                level0_right_color.bg_color = target_bg_color;
                level0_type = REFINE_TRIANGLE_LEFT_HALF;
                is_level0_valid = true;
            }
        }

        if (is_boundary_valid_array[3] &&
            is_boundary_valid_array[4] &&
            boundary_colors[3] == boundary_colors[4] &&
            boundary_colors[3] != block_color &&
            IsAllBrightColor(boundary_colors[3], block_color) == false)
        {
            int target_fg_color = block_color;
            int target_bg_color = boundary_colors[3];

            level2_right_color.fg_color = target_fg_color;
            level2_right_color.bg_color = target_bg_color;

            if (is_boundary_valid_array[5] && boundary_colors[5] == target_bg_color)
            {
                level2_left_color = level2_right_color;
                level2_type = REFINE_TRIANGLE_FULL_BLOCK;
                is_level2_valid = true;
            }
            else if (IsNormalColor(target_fg_color))
            {
                level2_left_color.fg_color = target_fg_color;
                level2_left_color.bg_color = target_fg_color;
                level2_type = REFINE_TRIANGLE_RIGHT_HALF;
                is_level2_valid = true;
            }
        }
    }
    else if (is_left_pure_color && is_right_pure_color)
    {
        if (is_boundary_valid_array[2] &&
            boundary_colors[2] == left_space_color &&
            IsNormalColor(left_space_color))
        {
            level0_left_color.fg_color = left_space_color;
            level0_left_color.bg_color = left_space_color;
            level0_right_color.fg_color = left_space_color;
            level0_right_color.bg_color = right_space_color;
            level0_type = REFINE_TRIANGLE_RIGHT_HALF;
            is_level0_valid = true;
        }

        if (is_boundary_valid_array[5] &&
            boundary_colors[5] == right_space_color &&
            IsNormalColor(right_space_color))
        {
            level2_left_color.fg_color = left_space_color;
            level2_left_color.bg_color = right_space_color;
            level2_right_color.fg_color = right_space_color;
            level2_right_color.bg_color = right_space_color;
            level2_type = REFINE_TRIANGLE_LEFT_HALF;
            is_level2_valid = true;
        }
    }

    if (is_level0_valid && is_level2_valid)
    {
        if (level0_type == REFINE_TRIANGLE_FULL_BLOCK || level2_type == REFINE_TRIANGLE_FULL_BLOCK)
        {
            if (IsForegroundTriangle(block_dx, block_dy, DOUBLE_CHAR_LEFT, 0))
                is_level2_valid = false;
            else
                is_level0_valid = false;
        }
        else
        {
            if (block_dy < m_half_block_size)
                is_level2_valid = false;
            else
                is_level0_valid = false;
        }
    }

    if (is_level0_valid)
    {
        left_color = level0_left_color;
        right_color = level0_right_color;
        type = level0_type;
        return 0;
    }
    else if (is_level2_valid)
    {
        left_color = level2_left_color;
        right_color = level2_right_color;
        type = level2_type;
        return 2;
    } 
    else
    {
        return -1;
    }
}

int AnsiCanvas::GetShape1TriangleRefineLevel(int block_dx, int block_dy, const AnsiCell &src_cell,
                                             const AnsiColor &src_left_color, const AnsiColor &src_right_color, 
                                             bool is_left_pure_color, int left_space_color,
                                             bool is_right_pure_color, int right_space_color,
                                             const bool is_boundary_valid_array[6], const int boundary_colors[6],
                                             AnsiColor &left_color, AnsiColor &right_color, RefineTriangleType &type)
{
    // Check refinement for triangle shape 1 (label 1 or 2).
    // The generated color can have bright background, but not both bright foreground/background.
    // (Use triangle label 2 to write bright background color.)

    const bool is_target_triangle = (src_cell.type == TRIANGLE) &&
                                    (src_cell.label == 1 || src_cell.label == 2);
    if (is_target_triangle == false && (is_left_pure_color == false || is_right_pure_color == false))
        return -1;

    bool is_level1_valid = false;
    AnsiColor level1_left_color;
    AnsiColor level1_right_color;
    RefineTriangleType level1_type;

    if (is_target_triangle && (is_left_pure_color == false || is_right_pure_color == false))
    {
        AnsiColor curr_left_color = src_left_color;
        AnsiColor curr_right_color = src_right_color;
        if (src_cell.label == 2)
        {
            curr_left_color = curr_left_color.FlipColor();
            curr_right_color = curr_right_color.FlipColor();
        }

        if (is_left_pure_color == false && is_right_pure_color == false)
        {
            if (curr_left_color == curr_right_color)
            {
                is_level1_valid = true;
                level1_left_color = curr_left_color;
                level1_right_color = curr_right_color;
                level1_type = REFINE_TRIANGLE_FULL_BLOCK;
            }
        }
        else if (is_left_pure_color == false)
        {
            // Right space is pure color. Check for REFINE_TRIANGLE_LEFT_HALF.
            if (curr_left_color.fg_color == right_space_color)
            {
                is_level1_valid = true;
                level1_left_color = curr_left_color;
                level1_right_color = curr_right_color;
                level1_type = REFINE_TRIANGLE_LEFT_HALF;
            }
        }
        else if (is_right_pure_color == false)
        {
            // Left space is pure color. Check for REFINE_TRIANGLE_RIGHT_HALF.
            if (left_space_color == curr_right_color.bg_color)
            {
                is_level1_valid = true;
                level1_left_color = curr_left_color;
                level1_right_color = curr_right_color;
                level1_type = REFINE_TRIANGLE_RIGHT_HALF;
            }
        }
    }

    if (is_level1_valid)
    {
        left_color = level1_left_color;
        right_color = level1_right_color;
        type = level1_type;
        return 1;
    }

    bool is_level0_valid = false;
    bool is_level2_valid = false;
    AnsiColor level0_left_color, level0_right_color;
    AnsiColor level2_left_color, level2_right_color;
    RefineTriangleType level0_type;
    RefineTriangleType level2_type;

    bool is_block_pure_color = (is_left_pure_color && is_right_pure_color && left_space_color == right_space_color);
    
    if (is_block_pure_color)
    {
        int block_color = left_space_color;

        if (is_boundary_valid_array[2] &&
            is_boundary_valid_array[3] &&
            boundary_colors[2] == boundary_colors[3] &&
            boundary_colors[2] != block_color &&
            IsAllBrightColor(boundary_colors[2], block_color) == false)
        {
            int target_fg_color = boundary_colors[2];
            int target_bg_color = block_color;

            level0_right_color.fg_color = target_fg_color;
            level0_right_color.bg_color = target_bg_color;

            if (is_boundary_valid_array[1] && boundary_colors[1] == target_fg_color)
            {
                level0_left_color = level0_right_color;
                level0_type = REFINE_TRIANGLE_FULL_BLOCK;
                is_level0_valid = true;
            }
            else if (IsNormalColor(target_bg_color))
            {
                level0_left_color.fg_color = target_bg_color;
                level0_left_color.bg_color = target_bg_color;
                level0_type = REFINE_TRIANGLE_RIGHT_HALF;
                is_level0_valid = true;
            }
        }

        if (is_boundary_valid_array[0] &&
            is_boundary_valid_array[5] &&
            boundary_colors[0] == boundary_colors[5] &&
            boundary_colors[0] != block_color &&
            IsAllBrightColor(boundary_colors[0], block_color) == false)
        {
            int target_fg_color = block_color;
            int target_bg_color = boundary_colors[0];

            level2_left_color.fg_color = target_fg_color;
            level2_left_color.bg_color = target_bg_color;

            if (is_boundary_valid_array[4] && boundary_colors[4] == target_bg_color)
            {
                level2_right_color = level2_left_color;
                level2_type = REFINE_TRIANGLE_FULL_BLOCK;
                is_level2_valid = true;
            }
            else if (IsNormalColor(target_fg_color))
            {
                level2_right_color.fg_color = target_fg_color;
                level2_right_color.bg_color = target_fg_color;
                level2_type = REFINE_TRIANGLE_LEFT_HALF;
                is_level2_valid = true;
            }
        }
    }
    else if (is_left_pure_color && is_right_pure_color)
    {
        if (is_boundary_valid_array[1] &&
            boundary_colors[1] == right_space_color &&
            IsNormalColor(right_space_color))
        {
            level0_left_color.fg_color = right_space_color;
            level0_left_color.bg_color = left_space_color;
            level0_right_color.fg_color = right_space_color;
            level0_right_color.bg_color = right_space_color;
            level0_type = REFINE_TRIANGLE_LEFT_HALF;
            is_level0_valid = true;
        }

        if (is_boundary_valid_array[4] &&
            boundary_colors[4] == left_space_color &&
            IsNormalColor(left_space_color))
        {
            level2_left_color.fg_color = left_space_color;
            level2_left_color.bg_color = left_space_color;
            level2_right_color.fg_color = right_space_color;
            level2_right_color.bg_color = left_space_color;
            level2_type = REFINE_TRIANGLE_RIGHT_HALF;
            is_level2_valid = true;
        }
    }

    if (is_level0_valid && is_level2_valid)
    {
        if (level0_type == REFINE_TRIANGLE_FULL_BLOCK || level2_type == REFINE_TRIANGLE_FULL_BLOCK)
        {
            if (IsForegroundTriangle(block_dx, block_dy, DOUBLE_CHAR_LEFT, 1))
                is_level2_valid = false;
            else
                is_level0_valid = false;
        }
        else
        {
            if (block_dy < m_half_block_size)
                is_level2_valid = false;
            else
                is_level0_valid = false;
        }
    }

    if (is_level0_valid)
    {
        left_color = level0_left_color;
        right_color = level0_right_color;
        type = level0_type;
        return 0;
    }
    else if (is_level2_valid)
    {
        left_color = level2_left_color;
        right_color = level2_right_color;
        type = level2_type;
        return 2;
    } 
    else
    {
        return -1;
    }
}

void AnsiCanvas::SetTriangleRefineBoundaryInfo(int x_offset, int y_offset,
                                               int shape, int level, RefineTriangleType refine_type,
                                               const AnsiColor &left_color, const AnsiColor &right_color,
                                               RefineBoundaryInfo &info)
{
    info.x_offset = x_offset;
    info.y_offset = y_offset;
    info.type = REFINE_TRIANGLE_BOUNDARY;
    info.shape = shape;
    info.level = level;
    info.refine_triangle_type = refine_type;
    info.left_color = left_color;
    info.right_color = right_color;

    info.is_valid = true;
}

void AnsiCanvas::SearchRegularTriangleRefineBoundaryInfo(int x, int y, RefineBoundaryInfo &info)
{
    info.is_valid = false;

    int x_offset = 0, y_offset = 0;
    if (GetXYOffset(x, y, x_offset, y_offset) == false)
        return;

    int shape1, shape2;
    int level1, level2;
    AnsiColor left_color1, left_color2, right_color1, right_color2;

    bool is_valid1 = GetRegularTriangleRefineStatus(x, y, x_offset - 1, y_offset, shape1, level1, left_color1, right_color1);
    bool is_valid2 = GetRegularTriangleRefineStatus(x, y, x_offset, y_offset, shape2, level2, left_color2, right_color2);

    if (is_valid1 && is_valid2)
    {
        int shift = x - ToCoordX(x_offset);

        if (shift < m_half_block_size / 2)
            is_valid2 = false;
        else
            is_valid1 = false;
    }
    
    if (is_valid1)
    {
        SetRegularTriangleRefineBoundaryInfo(x_offset - 1, y_offset, shape1, level1, left_color1, right_color1, info);
    }
    else if (is_valid2)
    {
        SetRegularTriangleRefineBoundaryInfo(x_offset, y_offset, shape2, level2, left_color2, right_color2, info);
    }
}

bool AnsiCanvas::GetRegularTriangleRefineStatus(int x_coord, int y_coord, int x_offset, int y_offset,
                                                int &shape, int &level, AnsiColor &left_color, AnsiColor &right_color)
{
    // Find refine boundary status for regular triangle.
    // The "shape" is the cell type of regular triangle: 0 for upward, 1 for downward.
    // The "level" in [0, 5] is defined for each refine case.
    // For shape 0 / shape 1, the boundary to be refines is:
    //      +--0--+--5--+       +--2--+--3--+
    //      |    /|\    |       |\    |    /|
    //      |   / | \   |       | \   |   / |
    //      0  1 2|3 4  5       0  1 2|3 4  5
    //      | /   |   \ |       |   \ | /   |
    //      |/    |    \|       |    \|/    |
    //      +--2--+--3--+       +--0--+--5--+
    //         shape 0             shape 1

    if (y_offset < 0 || y_offset >= m_height)
        return false;
    if (x_offset < 0 || x_offset >= m_width - 1)
        return false;

    const AnsiRow &row = m_cells[y_offset];

    int cell_offset;
    AnsiCharLocation char_location;
    bool is_success = SearchCellInfo(row, x_offset, cell_offset, char_location);
    if (is_success == false)
        return false;

    const AnsiCell &cell = row[cell_offset];
    AnsiColor left_canvas_color = CanvasColorAt(x_offset, y_offset);
    AnsiColor right_canvas_color = CanvasColorAt(x_offset + 1, y_offset);

    AC_EditInfo merge_info;
    SearchMergeBlockInfoAtOffset(x_offset, y_offset, merge_info, false);
    bool is_block_valid = (char_location == DOUBLE_CHAR_LEFT) || merge_info.is_valid;
    if (is_block_valid == false)
        return false;

    AnsiCell src_cell = cell;
    AnsiColor src_left_color = left_canvas_color;
    AnsiColor src_right_color = right_canvas_color;
    if (merge_info.is_valid)
    {
        src_cell = merge_info.cell;
        src_left_color = merge_info.color;
        src_right_color = merge_info.color2;
    }

    int left_space_color, right_space_color;
    bool is_left_pure_color = GetPureSpaceColor(x_offset, y_offset, left_space_color);
    bool is_right_pure_color = GetPureSpaceColor(x_offset + 1, y_offset, right_space_color);

    int left_boundary_colors[4] = {0};
    int right_boundary_colors[4] = {0};
    bool is_left_boundary_valid_array[4] = {false};
    bool is_right_boundary_valid_array[4] = {false};
    GetOutsideBoundaryColors(x_offset, y_offset, left_boundary_colors, is_left_boundary_valid_array);
    GetOutsideBoundaryColors(x_offset + 1, y_offset, right_boundary_colors, is_right_boundary_valid_array);

    int block_dx = x_coord - ToCoordX(x_offset);
    int block_dy = y_coord - ToCoordY(y_offset);

    AnsiColor left_color_for_shape0, right_color_for_shape0;
    int level_for_shape0 = GetShape0RegularTriangleRefineLevel(block_dx, block_dy, src_cell, 
                                                               src_left_color, src_right_color,
                                                               is_left_pure_color, left_space_color,
                                                               is_right_pure_color, right_space_color,
                                                               is_left_boundary_valid_array, left_boundary_colors,
                                                               is_right_boundary_valid_array, right_boundary_colors,
                                                               left_color_for_shape0, right_color_for_shape0);

    AnsiColor left_color_for_shape1, right_color_for_shape1;
    int level_for_shape1 = GetShape1RegularTriangleRefineLevel(block_dx, block_dy, src_cell, 
                                                               src_left_color, src_right_color,
                                                               is_left_pure_color, left_space_color,
                                                               is_right_pure_color, right_space_color,
                                                               is_left_boundary_valid_array, left_boundary_colors,
                                                               is_right_boundary_valid_array, right_boundary_colors,
                                                               left_color_for_shape1, right_color_for_shape1);

    if (level_for_shape0 >= 0 && level_for_shape1 >= 0)
    {
        if (block_dy < m_half_block_size)
            level_for_shape0 = -1;
        else
            level_for_shape1 = -1;
    }

    if (level_for_shape0 >= 0)
    {
        shape = 0;
        level = level_for_shape0;
        left_color = left_color_for_shape0;
        right_color = right_color_for_shape0;

        return true;
    }
    else if (level_for_shape1 >= 0)
    {
        shape = 1;
        level = level_for_shape1;
        left_color = left_color_for_shape1;
        right_color = right_color_for_shape1;

        return true;
    }

    return false;
}

int AnsiCanvas::GetShape0RegularTriangleRefineLevel(int block_dx, int block_dy, const AnsiCell &src_cell,
                                                    const AnsiColor &src_left_color, const AnsiColor &src_right_color, 
                                                    bool is_left_pure_color, int left_space_color,
                                                    bool is_right_pure_color, int right_space_color,
                                                    const bool is_left_boundary_valid_array[4], const int left_boundary_colors[4],
                                                    const bool is_right_boundary_valid_array[4], const int right_boundary_colors[4],
                                                    AnsiColor &left_color, AnsiColor &right_color)
{
    const bool is_target_triangle = (src_cell == AnsiCell(REGULAR_TRIANGLE, 0));
    if (is_target_triangle == false && (is_left_pure_color == false || is_right_pure_color == false))
        return -1;
    
    int left_level = -1;
    int right_level = -1;
    AnsiColor left_edit_color;
    AnsiColor right_edit_color;

    if (is_target_triangle && (is_left_pure_color == false))
    {
        left_level = 1;
        left_edit_color = src_left_color;
    }
    else if (is_left_pure_color && IsNormalColor(right_space_color))
    {
        bool is_level0_valid = false;
        AnsiColor level0_color;
        if (is_left_boundary_valid_array[0] &&
            is_left_boundary_valid_array[1] &&
            left_boundary_colors[0] == left_boundary_colors[1] &&
            left_boundary_colors[0] != left_space_color &&
            IsNormalColor(left_boundary_colors[0]))
        {
            is_level0_valid = true;
            level0_color.fg_color = left_space_color;
            level0_color.bg_color = left_boundary_colors[0];
        }

        bool is_level2_valid = false;
        AnsiColor level2_color;
        if (is_left_boundary_valid_array[2] &&
            is_left_boundary_valid_array[3] &&
            left_boundary_colors[2] == left_boundary_colors[3] &&
            left_boundary_colors[2] != left_space_color &&
            IsNormalColor(left_space_color))
        {
            is_level2_valid = true;
            level2_color.fg_color = left_boundary_colors[2];
            level2_color.bg_color = left_space_color;
        }

        if (is_level0_valid && is_level2_valid)
        {
            if (IsForegroundRegularTriangle(block_dx, block_dy, DOUBLE_CHAR_LEFT, 0))
                is_level0_valid = false;
            else
                is_level2_valid = false;
        }

        if (is_level0_valid)
        {
            left_level = 0;
            left_edit_color = level0_color;
        }
        else if (is_level2_valid)
        {
            left_level = 2;
            left_edit_color = level2_color;
        }                       
    }

    if (is_target_triangle && (is_right_pure_color == false))
    {
        right_level = 4;
        right_edit_color = src_right_color;
    }
    else if (is_right_pure_color && IsNormalColor(left_space_color))
    {
        bool is_level3_valid = false;
        AnsiColor level3_color;
        if (is_right_boundary_valid_array[0] &&
            is_right_boundary_valid_array[3] &&
            right_boundary_colors[0] == right_boundary_colors[3] &&
            right_boundary_colors[0] != right_space_color &&
            IsNormalColor(right_space_color))
        {
            is_level3_valid = true;
            level3_color.fg_color = right_boundary_colors[0];
            level3_color.bg_color = right_space_color;
        }

        bool is_level5_valid = false;
        AnsiColor level5_color;
        if (is_right_boundary_valid_array[1] &&
            is_right_boundary_valid_array[2] &&
            right_boundary_colors[1] == right_boundary_colors[2] &&
            right_boundary_colors[1] != right_space_color &&
            IsNormalColor(right_boundary_colors[1]))
        {
            is_level5_valid = true;
            level5_color.fg_color = right_space_color;
            level5_color.bg_color = right_boundary_colors[1];
        }

        if (is_level3_valid && is_level5_valid)
        {
            if (IsForegroundRegularTriangle(block_dx - m_half_block_size, block_dy, DOUBLE_CHAR_RIGHT, 0))
                is_level5_valid = false;
            else
                is_level3_valid = false;
        }

        if (is_level3_valid)
        {
            right_level = 3;
            right_edit_color = level3_color;
        }
        else if (is_level5_valid)
        {
            right_level = 5;
            right_edit_color = level5_color;
        }                       
    }

    if (left_level >= 0 && right_level >= 0)
    {
        if (block_dx < m_half_block_size)
            right_level = -1;
        else
            left_level = -1;
    }

    if (left_level >= 0)
    {
        left_color = left_edit_color;
        if (is_target_triangle)
        {
            right_color = src_right_color;
        }
        else
        {
            right_color.fg_color = right_space_color;
            right_color.bg_color = right_space_color;
        }

        return left_level;
    }
    else if (right_level >= 0)
    {
        right_color = right_edit_color;
        if (is_target_triangle)
        {
            left_color = src_left_color;
        }
        else
        {
            left_color.fg_color = left_space_color;
            left_color.bg_color = left_space_color;
        }

        return right_level;
    }
    else
    {
        return -1;
    }
}

int AnsiCanvas::GetShape1RegularTriangleRefineLevel(int block_dx, int block_dy, const AnsiCell &src_cell,
                                                    const AnsiColor &src_left_color, const AnsiColor &src_right_color, 
                                                    bool is_left_pure_color, int left_space_color,
                                                    bool is_right_pure_color, int right_space_color,
                                                    const bool is_left_boundary_valid_array[4], const int left_boundary_colors[4],
                                                    const bool is_right_boundary_valid_array[4], const int right_boundary_colors[4],
                                                    AnsiColor &left_color, AnsiColor &right_color)
{
    const bool is_target_triangle = (src_cell == AnsiCell(REGULAR_TRIANGLE, 1));
    if (is_target_triangle == false && (is_left_pure_color == false || is_right_pure_color == false))
        return -1;
    
    int left_level = -1;
    int right_level = -1;
    AnsiColor left_edit_color;
    AnsiColor right_edit_color;

    if (is_target_triangle && (is_left_pure_color == false))
    {
        left_level = 1;
        left_edit_color = src_left_color;
    }
    else if (is_left_pure_color && IsNormalColor(right_space_color))
    {
        bool is_level0_valid = false;
        AnsiColor level0_color;
        if (is_left_boundary_valid_array[0] &&
            is_left_boundary_valid_array[3] &&
            left_boundary_colors[0] == left_boundary_colors[3] &&
            left_boundary_colors[0] != left_space_color &&
            IsNormalColor(left_boundary_colors[0]))
        {
            is_level0_valid = true;
            level0_color.fg_color = left_space_color;
            level0_color.bg_color = left_boundary_colors[0];
        }

        bool is_level2_valid = false;
        AnsiColor level2_color;
        if (is_left_boundary_valid_array[1] &&
            is_left_boundary_valid_array[2] &&
            left_boundary_colors[1] == left_boundary_colors[2] &&
            left_boundary_colors[1] != left_space_color &&
            IsNormalColor(left_space_color))
        {
            is_level2_valid = true;
            level2_color.fg_color = left_boundary_colors[1];
            level2_color.bg_color = left_space_color;
        }

        if (is_level0_valid && is_level2_valid)
        {
            if (IsForegroundRegularTriangle(block_dx, block_dy, DOUBLE_CHAR_LEFT, 1))
                is_level0_valid = false;
            else
                is_level2_valid = false;
        }

        if (is_level0_valid)
        {
            left_level = 0;
            left_edit_color = level0_color;
        }
        else if (is_level2_valid)
        {
            left_level = 2;
            left_edit_color = level2_color;
        }                       
    }

    if (is_target_triangle && (is_right_pure_color == false))
    {
        right_level = 4;
        right_edit_color = src_right_color;
    }
    else if (is_right_pure_color && IsNormalColor(left_space_color))
    {
        bool is_level3_valid = false;
        AnsiColor level3_color;
        if (is_right_boundary_valid_array[0] &&
            is_right_boundary_valid_array[1] &&
            right_boundary_colors[0] == right_boundary_colors[1] &&
            right_boundary_colors[0] != right_space_color &&
            IsNormalColor(right_space_color))
        {
            is_level3_valid = true;
            level3_color.fg_color = right_boundary_colors[0];
            level3_color.bg_color = right_space_color;
        }

        bool is_level5_valid = false;
        AnsiColor level5_color;
        if (is_right_boundary_valid_array[2] &&
            is_right_boundary_valid_array[3] &&
            right_boundary_colors[2] == right_boundary_colors[3] &&
            right_boundary_colors[2] != right_space_color &&
            IsNormalColor(right_boundary_colors[2]))
        {
            is_level5_valid = true;
            level5_color.fg_color = right_space_color;
            level5_color.bg_color = right_boundary_colors[2];
        }

        if (is_level3_valid && is_level5_valid)
        {
            if (IsForegroundRegularTriangle(block_dx - m_half_block_size, block_dy, DOUBLE_CHAR_RIGHT, 1))
                is_level5_valid = false;
            else
                is_level3_valid = false;
        }

        if (is_level3_valid)
        {
            right_level = 3;
            right_edit_color = level3_color;
        }
        else if (is_level5_valid)
        {
            right_level = 5;
            right_edit_color = level5_color;
        }                       
    }

    if (left_level >= 0 && right_level >= 0)
    {
        if (block_dx < m_half_block_size)
            right_level = -1;
        else
            left_level = -1;
    }

    if (left_level >= 0)
    {
        left_color = left_edit_color;
        if (is_target_triangle)
        {
            right_color = src_right_color;
        }
        else
        {
            right_color.fg_color = right_space_color;
            right_color.bg_color = right_space_color;
        }

        return left_level;
    }
    else if (right_level >= 0)
    {
        right_color = right_edit_color;
        if (is_target_triangle)
        {
            left_color = src_left_color;
        }
        else
        {
            left_color.fg_color = left_space_color;
            left_color.bg_color = left_space_color;
        }

        return right_level;
    }
    else
    {
        return -1;
    }
}

void AnsiCanvas::SetRegularTriangleRefineBoundaryInfo(int x_offset, int y_offset, int shape, int level, 
                                                      const AnsiColor &left_color, const AnsiColor &right_color,
                                                      RefineBoundaryInfo &info)
{
    info.x_offset = x_offset;
    info.y_offset = y_offset;
    info.type = REFINE_REGULAR_TRIANGLE_BOUNDARY;
    info.shape = shape;
    info.level = level;
    info.left_color = left_color;
    info.right_color = right_color;
    info.initial_left_color = left_color;
    info.initial_right_color = right_color;

    info.is_valid = true;
}

void AnsiCanvas::ConvertToEditInfo(const RefineBoundaryInfo &info, AC_EditInfo &edit_info)
{
    edit_info.is_valid = info.is_valid;

    if (info.is_valid)
    {
        int base_x = ToCoordX(info.x_offset);
        int base_y = ToCoordY(info.y_offset);
        edit_info.target_rect.x = base_x;
        edit_info.target_rect.y = base_y;
        edit_info.target_rect.width = m_block_size;
        edit_info.target_rect.height = m_block_size;

        HyPoint &line_start = edit_info.line_start;
        HyPoint &line_end = edit_info.line_end;
        HyPoint &line_start2 = edit_info.second_line_start;
        HyPoint &line_end2 = edit_info.second_line_end;

        if (info.type == REFINE_HORIZONTAL_BOUNDARY)
        {
            edit_info.mode = AC_EDIT_REFINE_BOUNDARY;

            line_start.x = base_x;
            line_start.y = base_y + ch_Round((float)m_block_size * (8 - info.level) / 8.0f);
            line_end.x = edit_info.line_start.x + m_block_size;
            line_end.y = edit_info.line_start.y;
        }
        else if (info.type == REFINE_VERTICAL_BOUNDARY)
        {
            edit_info.mode = AC_EDIT_REFINE_BOUNDARY;

            line_start.x = base_x + ch_Round((float)m_block_size * info.level / 8.0f);
            line_start.y = base_y;
            line_end.x = edit_info.line_start.x;
            line_end.y = edit_info.line_start.y + m_block_size;
        }
        else if (info.type == REFINE_HORIZONTAL_HALF_BOUNDARY)
        {
            edit_info.mode = AC_EDIT_REFINE_BOUNDARY_HALF;

            line_start.x = base_x;
            line_start.y = base_y + ch_Round((float)m_block_size * (8 - info.level) / 8.0f);
            line_end.x = edit_info.line_start.x + m_half_block_size;
            line_end.y = edit_info.line_start.y;

            edit_info.is_have_second_line = true;
            line_start2.x = base_x + m_half_block_size;
            line_start2.y = base_y + ch_Round((float)m_block_size * (8 - info.right_level) / 8.0f);
            line_end2.x = line_start2.x + m_half_block_size;
            line_end2.y = line_start2.y;

            edit_info.is_left_half = info.is_left_half;
        }
        else if (info.type == REFINE_TRIANGLE_BOUNDARY)
        {
            edit_info.mode = AC_EDIT_REFINE_BOUNDARY_TRIANGLE;

            SetRefineTriangleEditLine(base_x, base_y, info.shape, info.level, info.refine_triangle_type,
                                      line_start, line_end, line_start2, line_end2, edit_info.is_have_second_line);
        }
        else if (info.type == REFINE_REGULAR_TRIANGLE_BOUNDARY)
        {
            edit_info.mode = AC_EDIT_REFINE_BOUNDARY_REGULAR_TRIANGLE;

            SetRefineRegularTriangleEditLine(base_x, base_y, info.shape, info.level,
                                             line_start, line_end, line_start2, line_end2, edit_info.is_have_second_line);
        }
    }
}

void AnsiCanvas::SetRefineTriangleEditLine(int base_x, int base_y,
                                           int shape, int level, RefineTriangleType type,
                                           HyPoint &line_start, HyPoint &line_end,
                                           HyPoint &line_start2, HyPoint &line_end2,
                                           bool &is_have_second_line)
{
    HyPoint top_left(base_x, base_y);
    HyPoint top_middle(base_x + m_half_block_size, base_y);
    HyPoint top_right(base_x + m_block_size, base_y);
    HyPoint block_center(base_x + m_half_block_size, base_y + m_half_block_size);
    HyPoint bottom_left(base_x, base_y + m_block_size);
    HyPoint bottom_middle(base_x + m_half_block_size, base_y + m_block_size);
    HyPoint bottom_right(base_x + m_block_size, base_y + m_block_size);

    if (level == 1)
    {
        if (type == REFINE_TRIANGLE_FULL_BLOCK)
        {
            if (shape == 0)
            {
                line_start = top_right;
                line_end = bottom_left;
            }
            else
            {
                line_start = top_left;
                line_end = bottom_right;
            }

            is_have_second_line = false;
        }
        else
        {
            if (shape == 0)
            {
                if (type == REFINE_TRIANGLE_LEFT_HALF)
                {
                    line_start = top_middle;
                    line_end = block_center;
                    line_start2 = block_center;
                    line_end2 = bottom_left;
                }
                else
                {
                    line_start = top_right;
                    line_end = block_center;
                    line_start2 = block_center;
                    line_end2 = bottom_middle;
                }
            }
            else
            {
                if (type == REFINE_TRIANGLE_LEFT_HALF)
                {
                    line_start = top_left;
                    line_end = block_center;
                    line_start2 = block_center;
                    line_end2 = bottom_middle;
                }
                else
                {
                    line_start = top_middle;
                    line_end = block_center;
                    line_start2 = block_center;
                    line_end2 = bottom_right;
                }
            }

            is_have_second_line = true;
        }
    }
    else
    {
        if (shape == 0 && level == 0)
        {
            if (type == REFINE_TRIANGLE_FULL_BLOCK)
            {
                line_start = top_left;
                line_end = bottom_left;
                line_start2 = top_left;
                line_end2 = top_right;
            }
            else if (type == REFINE_TRIANGLE_LEFT_HALF)
            {
                line_start = top_left;
                line_end = bottom_left;
                line_start2 = top_left;
                line_end2 = top_middle;
            }
            else if (type == REFINE_TRIANGLE_RIGHT_HALF)
            {
                line_start = top_middle;
                line_end = bottom_middle;
                line_start2 = top_middle;
                line_end2 = top_right;
            }
        }
        else if (shape == 0 && level == 2)
        {
            if (type == REFINE_TRIANGLE_FULL_BLOCK)
            {
                line_start = top_right;
                line_end = bottom_right;
                line_start2 = bottom_left;
                line_end2 = bottom_right;
            }
            else if (type == REFINE_TRIANGLE_LEFT_HALF)
            {
                line_start = top_middle;
                line_end = bottom_middle;
                line_start2 = bottom_left;
                line_end2 = bottom_middle;
            }
            else if (type == REFINE_TRIANGLE_RIGHT_HALF)
            {
                line_start = top_right;
                line_end = bottom_right;
                line_start2 = bottom_middle;
                line_end2 = bottom_right;
            }
        }
        else if (shape == 1 && level == 0)
        {
            if (type == REFINE_TRIANGLE_FULL_BLOCK)
            {
                line_start = top_right;
                line_end = bottom_right;
                line_start2 = top_left;
                line_end2 = top_right;
            }
            else if (type == REFINE_TRIANGLE_LEFT_HALF)
            {
                line_start = top_middle;
                line_end = bottom_middle;
                line_start2 = top_left;
                line_end2 = top_middle;
            }
            else if (type == REFINE_TRIANGLE_RIGHT_HALF)
            {
                line_start = top_right;
                line_end = bottom_right;
                line_start2 = top_middle;
                line_end2 = top_right;
            }
        }
        else if (shape == 1 && level == 2)
        {
            if (type == REFINE_TRIANGLE_FULL_BLOCK)
            {
                line_start = top_left;
                line_end = bottom_left;
                line_start2 = bottom_left;
                line_end2 = bottom_right;
            }
            else if (type == REFINE_TRIANGLE_LEFT_HALF)
            {
                line_start = top_left;
                line_end = bottom_left;
                line_start2 = bottom_left;
                line_end2 = bottom_middle;
            }
            else if (type == REFINE_TRIANGLE_RIGHT_HALF)
            {
                line_start = top_middle;
                line_end = bottom_middle;
                line_start2 = bottom_middle;
                line_end2 = bottom_right;
            }
        }

        is_have_second_line = true;
    }
}

void AnsiCanvas::SetRefineRegularTriangleEditLine(int base_x, int base_y,
                                                  int shape, int level,
                                                  HyPoint &line_start, HyPoint &line_end,
                                                  HyPoint &line_start2, HyPoint &line_end2,
                                                  bool &is_have_second_line)
{
    if (level == 0)
    {
        line_start.x = base_x;
        line_start.y = base_y;
        line_end.x = base_x;
        line_end.y = base_y + m_block_size;

        line_start2.x = base_x;
        line_end2.x = base_x + m_half_block_size;
        if (shape == 0)
        {
            line_start2.y = base_y;
            line_end2.y = base_y;
        }
        else
        {
            line_start2.y = base_y + m_block_size;
            line_end2.y = base_y + m_block_size;
        }

        is_have_second_line = true;
    }
    else if (level == 1)
    {
        line_start.y = base_y;
        line_end.y = base_y + m_block_size;
        if (shape == 0)
        {
            line_start.x = base_x + m_half_block_size;
            line_end.x = base_x;
        }
        else
        {
            line_start.x = base_x;
            line_end.x = base_x + m_half_block_size;
        }

        is_have_second_line = false;
    }
    else if (level == 2)
    {
        line_start.x = base_x + m_half_block_size;
        line_start.y = base_y;
        line_end.x = base_x + m_half_block_size;
        line_end.y = base_y + m_block_size;

        line_start2.x = base_x;
        line_end2.x = base_x + m_half_block_size;
        if (shape == 0)
        {
            line_start2.y = base_y + m_block_size;
            line_end2.y = base_y + m_block_size;
        }
        else
        {
            line_start2.y = base_y;
            line_end2.y = base_y;
        }

        is_have_second_line = true;
    }
    else if (level == 3)
    {
        line_start.x = base_x + m_half_block_size;
        line_start.y = base_y;
        line_end.x = base_x + m_half_block_size;
        line_end.y = base_y + m_block_size;

        line_start2.x = base_x + m_half_block_size;
        line_end2.x = base_x + m_block_size;
        if (shape == 0)
        {
            line_start2.y = base_y + m_block_size;
            line_end2.y = base_y + m_block_size;
        }
        else
        {
            line_start2.y = base_y;
            line_end2.y = base_y;
        }

        is_have_second_line = true;
    }
    else if (level == 4)
    {
        line_start.y = base_y;
        line_end.y = base_y + m_block_size;
        if (shape == 0)
        {
            line_start.x = base_x + m_half_block_size;
            line_end.x = base_x + m_block_size;
        }
        else
        {
            line_start.x = base_x + m_block_size;
            line_end.x = base_x + m_half_block_size;
        }

        is_have_second_line = false;
    }
    else if (level == 5)
    {
        line_start.x = base_x + m_block_size;
        line_start.y = base_y;
        line_end.x = base_x + m_block_size;
        line_end.y = base_y + m_block_size;

        line_start2.x = base_x + m_half_block_size;
        line_end2.x = base_x + m_block_size;
        if (shape == 0)
        {
            line_start2.y = base_y;
            line_end2.y = base_y;
        }
        else
        {
            line_start2.y = base_y + m_block_size;
            line_end2.y = base_y + m_block_size;
        }

        is_have_second_line = true;
    }
}

void AnsiCanvas::ChangeNeighborBlockIfNeeded(const RefineBoundaryInfo &info, AC_RefineBoundaryOrientation orientation)
{
    if (info.is_valid == false || orientation == REFINE_FROM_UNDEFINED)
        return;

    int x_offset = info.x_offset;
    int y_offset = info.y_offset;

    if (info.type == REFINE_HORIZONTAL_BOUNDARY)
    {
        if (orientation == REFINE_FROM_BOTTOM)
            y_offset++;
        else if (orientation == REFINE_FROM_TOP)
            y_offset--;
        else
            return;
    }
    else if (info.type == REFINE_VERTICAL_BOUNDARY)
    {
        if (orientation == REFINE_FROM_LEFT)
            x_offset -= 2;
        else if (orientation == REFINE_FROM_RIGHT)
            x_offset += 2;
        else
            return;
    }

    if (x_offset < 0 && x_offset >= m_width - 1)
        return;
    if (y_offset < 0 && y_offset >= m_height)
        return;

    AnsiRow &row = m_cells[y_offset];

    int cell_offset;
    AnsiCharLocation location;
    bool is_success = SearchCellInfo(row, x_offset, cell_offset, location);
    if (is_success == false)
        return;

    if (location != DOUBLE_CHAR_LEFT)
        return;

    AnsiCell &current_cell = row[cell_offset];

    int left_space_color, right_space_color;
    if (GetPureSpaceColor(x_offset, y_offset, current_cell, false, left_space_color) == false ||
        GetPureSpaceColor(x_offset + 1, y_offset, current_cell, true, right_space_color) == false)
        return;

    if (IsBrightColor(left_space_color) || IsBrightColor(right_space_color))
        return;

    current_cell = AnsiCell(EMPTY);
    row.insert(row.begin() + cell_offset, AnsiCell(EMPTY));

    CanvasColorAt(x_offset, y_offset) = AnsiColor(ANSI_DEFAULT_FOREGROUND, left_space_color);
    CanvasColorAt(x_offset + 1, y_offset) = AnsiColor(ANSI_DEFAULT_FOREGROUND, right_space_color);
}

bool AnsiCanvas::StartRefineBoundary(int x, int y, AC_EditMode mode, AC_EditInfo &edit_info, HyRect &change_rect)
{
    SearchRefineBoundaryInfo(x, y, mode, m_refine_boundary_info);
    if (m_refine_boundary_info.is_valid == false)
        return false;

    // Change neighbor block only if the edit starts from block boundary.
    AC_RefineBoundaryOrientation orientation = REFINE_FROM_UNDEFINED;
    if (m_refine_boundary_info.type == REFINE_HORIZONTAL_BOUNDARY)
    {
        if (m_refine_boundary_info.level == 0)
            orientation = REFINE_FROM_BOTTOM;
        else if (m_refine_boundary_info.level == 8)
            orientation = REFINE_FROM_TOP;
    }
    else if (m_refine_boundary_info.type == REFINE_VERTICAL_BOUNDARY)
    {
        if (m_refine_boundary_info.level == 0)
            orientation = REFINE_FROM_LEFT;
        else if (m_refine_boundary_info.level == 8)
            orientation = REFINE_FROM_RIGHT;
    }

    ChangeNeighborBlockIfNeeded(m_refine_boundary_info, orientation);

    UpdateDataForRefineBoundary(x, y, change_rect, true);
    ConvertToEditInfo(m_refine_boundary_info, edit_info);

    RecordAction(ActionInfo(mode, change_rect));

    return true;
}

void AnsiCanvas::ContinueRefineBoundary(int x, int y, AC_EditInfo &edit_info,
                                        HyRect &change_rect, bool &is_action_list_changed)
{
    change_rect = hyRect(0, 0, 0, 0);
    is_action_list_changed = false;

    if (m_refine_boundary_info.is_valid == false)
        return;

    UpdateDataForRefineBoundary(x, y, change_rect, false);
    ConvertToEditInfo(m_refine_boundary_info, edit_info);

    AC_EditMode edit_mode = AC_EDIT_REFINE_BOUNDARY;
    if (m_refine_boundary_info.type == REFINE_HORIZONTAL_HALF_BOUNDARY)
        edit_mode = AC_EDIT_REFINE_BOUNDARY_HALF;
    else if (m_refine_boundary_info.type == REFINE_TRIANGLE_BOUNDARY)
        edit_mode = AC_EDIT_REFINE_BOUNDARY_TRIANGLE;
    else if (m_refine_boundary_info.type == REFINE_REGULAR_TRIANGLE_BOUNDARY)
        edit_mode = AC_EDIT_REFINE_BOUNDARY_REGULAR_TRIANGLE;

    ActionInfo action_info(edit_mode, change_rect);
    action_info.is_continuous = true;
    is_action_list_changed = RecordAction(action_info);
}

void AnsiCanvas::EndRefineBoundary()
{
    m_refine_boundary_info.is_valid = false;
}

void AnsiCanvas::UpdateDataForRefineBoundary(int x, int y, HyRect &change_rect, bool is_start_refine)
{
    const bool is_enable_move = (is_start_refine == false);
    const bool is_need_merge = is_start_refine;

    RefineBoundaryInfo &info = m_refine_boundary_info;

    HyRect merge_change_rect = hyRect(0, 0, 0, 0);
    if (is_need_merge)
    {
        HyRect block_rect;
        block_rect.x = ToCoordX(info.x_offset);
        block_rect.y = ToCoordY(info.y_offset);
        block_rect.width = m_block_size;
        block_rect.height = m_block_size;

        MergeBlockBeforeEditing(block_rect, merge_change_rect);
    }

    const int move_to_neighbor_th = 1; // in pixel
    
    const AnsiColor &left_color = info.left_color;
    const AnsiColor &right_color = info.right_color;

    bool is_move_location = false;
    int base_color, neighbor_color;

    AnsiColor left_write_color;
    AnsiColor right_write_color;

    HyRect normal_change_rect = hyRect(0, 0, 0, 0);
    HyRect additional_change_rect = hyRect(0, 0, 0, 0);

    if (info.type == REFINE_HORIZONTAL_BOUNDARY)
    {
        int distance = y - ToCoordY(info.y_offset);

        if (is_enable_move)
        {
            int top_distance_th = -move_to_neighbor_th;
            int bottom_distance_th = (m_block_size - 1) + move_to_neighbor_th;

            if (distance < top_distance_th)
            {
                if (left_color.fg_color == right_color.fg_color)
                {
                    base_color = left_color.fg_color;

                    is_move_location = IsValidToMoveForRefineBoundary(info.x_offset, info.y_offset - 1, neighbor_color, false);
                    if (is_move_location)
                    {
                        is_move_location = (base_color != neighbor_color) &&
                                           (left_color.bg_color == neighbor_color) &&
                                           (right_color.bg_color == neighbor_color);
                    }

                    if (is_move_location)
                    {
                        left_write_color = AnsiColor(base_color, ANSI_DEFAULT_BACKGROUND);
                        right_write_color = AnsiColor(base_color, ANSI_DEFAULT_BACKGROUND);
                        WriteBlock(info.x_offset, info.y_offset, AnsiCell(HORI_BLOCK, 8),
                                   left_write_color, right_write_color, additional_change_rect);

                        info.y_offset--;
                        info.left_color = AnsiColor(base_color, neighbor_color);
                        info.right_color = AnsiColor(base_color, neighbor_color);

                        distance += m_block_size;
                    }
                }
            }
            else if (distance > bottom_distance_th)
            {
                if (left_color.bg_color == right_color.bg_color)
                {
                    base_color = left_color.bg_color;

                    is_move_location = IsValidToMoveForRefineBoundary(info.x_offset, info.y_offset + 1, neighbor_color, true);
                    if (is_move_location)
                    {
                        is_move_location = (base_color != neighbor_color) &&
                                           (left_color.fg_color == neighbor_color) &&
                                           (right_color.fg_color == neighbor_color);
                    }

                    if (is_move_location)
                    {
                        left_write_color = AnsiColor(ANSI_DEFAULT_FOREGROUND, base_color);
                        right_write_color = AnsiColor(ANSI_DEFAULT_FOREGROUND, base_color);
                        WriteBlock(info.x_offset, info.y_offset, AnsiCell(HORI_BLOCK, 0),
                                   left_write_color, right_write_color, additional_change_rect);

                        info.y_offset++;
                        info.left_color = AnsiColor(neighbor_color, base_color);
                        info.right_color = AnsiColor(neighbor_color, base_color);

                        distance -= m_block_size;
                    }
                }
            }
        }

        int level = 8 - ch_Round((float)(distance * 8) / m_block_size);
        level = FitInRange(level, 0, 8);
        info.level = level;

        AnsiCell cell(HORI_BLOCK, level);
        left_write_color = info.left_color;
        right_write_color = info.right_color;
        WriteBlock(info.x_offset, info.y_offset, cell, left_write_color, right_write_color, normal_change_rect);
    }
    else if (info.type == REFINE_VERTICAL_BOUNDARY)
    {
        int distance = x - ToCoordX(info.x_offset);

        if (is_enable_move)
        {
            int left_distance_th = -move_to_neighbor_th;
            int right_distance_th = (m_block_size - 1) + move_to_neighbor_th;

            if (distance < left_distance_th)
            {
                base_color = left_color.bg_color;

                is_move_location = IsValidToMoveForRefineBoundary(info.x_offset - 2, info.y_offset, neighbor_color, true);
                if (is_move_location)
                {
                    is_move_location = (base_color != neighbor_color) &&
                                       (left_color.fg_color == neighbor_color);
                }

                if (is_move_location)
                {
                    left_write_color = AnsiColor(ANSI_DEFAULT_FOREGROUND, base_color);
                    right_write_color = right_color;
                    WriteBlock(info.x_offset, info.y_offset, AnsiCell(VERT_BLOCK, 0),
                               left_write_color, right_write_color, additional_change_rect);

                    info.x_offset -= 2;
                    info.left_color = AnsiColor(neighbor_color, base_color);
                    info.right_color = AnsiColor(neighbor_color, base_color);

                    distance += m_block_size;
                }
            }
            else if (distance > right_distance_th)
            {
                base_color = right_color.fg_color;

                is_move_location = IsValidToMoveForRefineBoundary(info.x_offset + 2, info.y_offset, neighbor_color, false);
                if (is_move_location)
                {
                    is_move_location = (base_color != neighbor_color) &&
                                       (right_color.bg_color == neighbor_color);
                }

                if (is_move_location)
                {
                    left_write_color = left_color;
                    right_write_color = AnsiColor(base_color, ANSI_DEFAULT_BACKGROUND);
                    WriteBlock(info.x_offset, info.y_offset, AnsiCell(VERT_BLOCK, 8),
                               left_write_color, right_write_color, additional_change_rect);

                    info.x_offset += 2;
                    info.left_color = AnsiColor(base_color, neighbor_color);
                    info.right_color = AnsiColor(base_color, neighbor_color);

                    distance -= m_block_size;
                }
            }
        }

        int level = ch_Round((float)(distance * 8) / m_block_size);
        level = FitInRange(level, 0, 8);
        info.level = level;

        AnsiCell cell(VERT_BLOCK, level);
        left_write_color = info.left_color;
        right_write_color = info.right_color;
        WriteBlock(info.x_offset, info.y_offset, cell, left_write_color, right_write_color, normal_change_rect);
    }
    else if (info.type == REFINE_HORIZONTAL_HALF_BOUNDARY)
    {
        int level = 0;
        UpdateRefineHalfBoundaryInfo(x, y, info, level);
        
        AnsiCell cell(HORI_BLOCK, level);
        left_write_color = info.left_color;
        right_write_color = info.right_color;
        WriteBlock(info.x_offset, info.y_offset, cell, left_write_color, right_write_color, normal_change_rect);
    }
    else if (info.type == REFINE_TRIANGLE_BOUNDARY)
    {
        AnsiCell write_cell;
        UpdateRefineTriangleInfo(x, y, info, write_cell, left_write_color, right_write_color);

        WriteBlock(info.x_offset, info.y_offset, write_cell, left_write_color, right_write_color, normal_change_rect);
    }
    else if (info.type == REFINE_REGULAR_TRIANGLE_BOUNDARY)
    {
        AnsiCell write_cell;
        UpdateRefineRegularTriangleInfo(x, y, info, write_cell, left_write_color, right_write_color);

        WriteBlock(info.x_offset, info.y_offset, write_cell, left_write_color, right_write_color, normal_change_rect);
    }

    change_rect = UnionRect(normal_change_rect, additional_change_rect);
    change_rect = UnionRect(change_rect, merge_change_rect);
}

bool AnsiCanvas::IsValidToMoveForRefineBoundary(int x_offset, int y_offset, int &neighbor_color, bool is_accept_bright_color)
{
    if (x_offset < 0 || x_offset >= m_width - 1)
        return false;
    if (y_offset < 0 || y_offset >= m_height)
        return false;

    const AnsiRow &row = m_cells[y_offset];

    int cell_offset;
    AnsiCharLocation location;
    if (SearchCellInfo(row, x_offset, cell_offset, location) == false)
        return false;

    bool is_valid = false;
    int left_space_color, right_space_color;
    if (location == DOUBLE_CHAR_LEFT)
    {
        const AnsiCell &block_cell = row[cell_offset];
        bool is_left_single_color = GetPureSpaceColor(x_offset, y_offset, block_cell, false, left_space_color);
        bool is_right_single_color = GetPureSpaceColor(x_offset + 1, y_offset, block_cell, true, right_space_color);

        is_valid = (is_left_single_color && is_right_single_color);
    }
    else if (row[cell_offset].type == EMPTY && row[cell_offset + 1].type == EMPTY)
    {
        left_space_color = CanvasColorAt(x_offset, y_offset).bg_color;
        right_space_color = CanvasColorAt(x_offset + 1, y_offset).bg_color;

        is_valid = true;
    }

    if (is_valid)
    {
        is_valid = (left_space_color == right_space_color) &&
                   (is_accept_bright_color || IsNormalColor(left_space_color));
    }

    if (is_valid)
        neighbor_color = left_space_color;

    return is_valid;
}

void AnsiCanvas::UpdateRefineHalfBoundaryInfo(int x, int y, RefineBoundaryInfo &info, int &level)
{
    const bool is_left = info.is_left_half;

    AnsiColor &fix_color = (is_left ? info.right_color : info.left_color);
    const int &fix_level = (is_left ? info.right_level : info.level);
    AnsiColor &edit_color = (is_left ? info.left_color : info.right_color);
    int &edit_level = (is_left ? info.level : info.right_level);

    AnsiColor &initial_edit_color = (is_left ? info.initial_left_color : info.initial_right_color);

    bool is_single_fix_color = (fix_color.fg_color == fix_color.bg_color) ||
                               (fix_level == 0) || (fix_level == 8);

    if (is_single_fix_color)
    {
        int distance = y - ToCoordY(info.y_offset);
        edit_level = 8 - ch_Round((float)(distance * 8) / m_block_size);
        edit_level = FitInRange(edit_level, 0, 8);

        if (fix_level == 0)
        {
            fix_color.fg_color = fix_color.bg_color;
        }
        else if (fix_level == 8)
        {
            fix_color.bg_color = fix_color.fg_color;
        }

        level = edit_level;
    }
    else
    {
        edit_level = GetRefineHorizontalHalfLevel(y, info.y_offset, initial_edit_color, fix_level);

        if (edit_level == 0)
        {
            edit_color.fg_color = initial_edit_color.bg_color;
            edit_color.bg_color = initial_edit_color.bg_color;
        }
        else if (edit_level == 8)
        {
            edit_color.fg_color = initial_edit_color.fg_color;
            edit_color.bg_color = initial_edit_color.fg_color;
        }
        else
        {
            edit_color = initial_edit_color;
        }

        level = fix_level;
    }
}

int AnsiCanvas::GetRefineHorizontalHalfLevel(int y_coord, int y_offset, const AnsiColor &fix_color, int fix_level)
{
    int top_coord = ToCoordY(y_offset);
    int bottom_coord = ToCoordY(y_offset + 1);
    int base_coord = top_coord + ch_Round((float)m_block_size * (8 - fix_level) / 8.0f);

    int dist_to_top = ch_Abs(y_coord - top_coord);
    int dist_to_bottom = ch_Abs(y_coord - bottom_coord);
    int dist_to_base = ch_Abs(y_coord - base_coord);

    if (IsBrightColor(fix_color.fg_color))
        dist_to_top = INT_MAX; // cannot move to top

    if (dist_to_base <= dist_to_top && dist_to_base <= dist_to_bottom)
        return fix_level;
    else if (dist_to_top < dist_to_bottom)
        return 8;
    else
        return 0;
}

int AnsiCanvas::GetTriangleLocation(int dx, int dy, int width, int height, bool is_backslash)
{
    // Consider a rectangle from (0, 0) to (width, height),
    // and a line (width, 0) - (0, height) or (0, 0) - (width, height) or according to is_backslash.
    // Return -1, 0, or 1 if the given location is close to the left boundary, line, or right boundary.
    // (Return -1 or 1 if the location is outside of the rectangle.)

    if (width <= 0 || height <= 0)
        return 0;

    // Use half-value coordinate for symmetry.
    float test_x = dx + 0.5f;
    float test_y = dy + 0.5f;

    float line_a, line_b, line_c;
    line_a = 1.0f;
    if (is_backslash)
    {
        line_b = -(float)width / height;
        line_c = 0.0f;
    }
    else
    {
        line_b = (float)width / height;
        line_c = -(float)width;
    }

    bool is_left = (line_a * test_x + line_b * test_y + line_c < 0.0f);
    int location = (is_left ? -1 : 1);

    if (test_x > 0.0f && test_y > 0.0f && test_x < width && test_x < height)
    {
        // Project (test_x, test_y) to the line on both horizontal / vertical directions.
        float project_x = -(line_b * test_y + line_c) / line_a;
        float project_y = -(line_a * test_x + line_c) / line_b;

        float dist_x_to_line = ch_Abs(project_x - test_x);
        float dist_y_to_line = ch_Abs(project_y - test_y);
        float dist_x_to_side = ch_Min(test_x, width - test_x);
        float dist_y_to_side = ch_Min(test_y, height - test_y);

        if (dist_x_to_line < dist_x_to_side && dist_y_to_line < dist_y_to_side)
            location = 0;
    }

    return location;
}

void AnsiCanvas::UpdateRefineTriangleInfo(int x, int y, RefineBoundaryInfo &info, 
                                          AnsiCell &write_cell, AnsiColor &left_write_color, AnsiColor &right_write_color)
{
    write_cell.type = TRIANGLE;
    write_cell.label = info.shape;

    int dx = x - ToCoordX(info.x_offset);
    int dy = y - ToCoordY(info.y_offset);

    const AnsiColor &src_left_color = info.left_color;
    const AnsiColor &src_right_color = info.right_color;
    left_write_color = src_left_color;
    right_write_color = src_right_color;

    bool is_backslash = (info.shape == 1);
    int location = GetTriangleLocation(dx, dy, m_block_size, m_block_size, is_backslash);

    // The location from GetTriangleLocation() is Left/Right based.
    // Check the triangle shape to determine the correct level.
    if ((location < 0 && info.shape == 0) || (location > 0 && info.shape == 1))
    {
        left_write_color.fg_color = src_left_color.bg_color;
        right_write_color.fg_color = src_right_color.bg_color;
        info.level = 0;
    }
    else if ((location > 0 && info.shape == 0) || (location < 0 && info.shape == 1))
    {
        left_write_color.bg_color = src_left_color.fg_color;
        right_write_color.bg_color = src_right_color.fg_color;
        info.level = 2;
    }
    else
    {
        info.level = 1;
    }

    // In this refine mode, the color may have bright background.
    // Perform the location check to determine the shape first,
    // then change the write cell/color to satisfy the color requirements.
    if (IsBrightColor(left_write_color.bg_color) || IsBrightColor(right_write_color.bg_color))
    {
        // Either left/right (or both) color may have bright background,
        // but it is impossible to have both bright foreground/background.
        // (This case is prevented in the searching of boundary refinement.)
        // So we can just flip the color in whole block.

        if (info.level == 1)
        {
            // Flip the color and triangle label.

            if (info.shape == 0)
                write_cell.label = 3;
            else
                write_cell.label = 2;

            left_write_color = left_write_color.FlipColor();
            right_write_color = right_write_color.FlipColor();
        }
        else
        {
            // Write a full-width block width the desired color.

            int left_space_color = left_write_color.bg_color;
            int right_space_color = right_write_color.bg_color;
            if (info.level == 2)
            {
                left_space_color = left_write_color.fg_color;
                right_space_color = right_write_color.fg_color;
            }

            write_cell.type = HORI_BLOCK;
            write_cell.label = 8;
            left_write_color = AnsiColor(left_space_color);
            right_write_color = AnsiColor(right_space_color);
        }
    }
}

void AnsiCanvas::UpdateRefineRegularTriangleInfo(int x, int y, RefineBoundaryInfo &info, 
                                                 AnsiCell &write_cell, AnsiColor &left_write_color, AnsiColor &right_write_color)
{
    write_cell.type = REGULAR_TRIANGLE;
    write_cell.label = info.shape;

    const bool is_left_side = (info.level < 3);

    int dx = x - ToCoordX(info.x_offset);
    int dy = y - ToCoordY(info.y_offset);
    if (is_left_side == false)
        dx -= m_half_block_size;

    bool is_backslash = (info.shape == 0 && is_left_side == false) ||
                        (info.shape == 1 && is_left_side == true);

    const AnsiColor &src_left_color = info.left_color;
    const AnsiColor &src_right_color = info.right_color;
    left_write_color = src_left_color;
    right_write_color = src_right_color;

    int location = GetTriangleLocation(dx, dy, m_half_block_size, m_block_size, is_backslash);

    if (is_left_side)
    {
        if (location < 0 && IsBrightColor(src_left_color.fg_color))
            location = 0;

        if (location < 0)
        {
            left_write_color.bg_color = src_left_color.fg_color;
            info.level = 0;
        }
        else if (location > 0)
        {
            left_write_color.fg_color = src_left_color.bg_color;
            info.level = 2;
        }
        else
        {
            info.level = 1;
        }
    }
    else
    {
        if (location > 0 && IsBrightColor(src_right_color.fg_color))
            location = 0;

        if (location < 0)
        {
            right_write_color.fg_color = src_right_color.bg_color;
            info.level = 3;
        }
        else if (location > 0)
        {
            right_write_color.bg_color = src_right_color.fg_color;
            info.level = 5;
        }
        else
        {
            info.level = 4;
        }
    }
}

void AnsiCanvas::WriteSpace(int x_offset, int y_offset, int space_color, HyRect &redraw_rect)
{
    AnsiCell write_cell(EMPTY);
    AnsiColor write_color(ANSI_DEFAULT_FOREGROUND, ToNormalColor(space_color));

    std::vector<AnsiCell> cells;
    cells.push_back(write_cell);

    std::vector<AnsiColor> colors;
    colors.push_back(write_color);

    return WriteRegion(x_offset, y_offset, cells, colors, redraw_rect, false);
}

void AnsiCanvas::WriteSpaces(int x_offset_min, int x_offset_max, int y_offset, int space_color, HyRect &redraw_rect)
{
    AnsiCell write_cell(EMPTY);
    AnsiColor write_color(ANSI_DEFAULT_FOREGROUND, ToNormalColor(space_color));

    std::vector<AnsiCell> cells;
    std::vector<AnsiColor> colors;

    for (int x = x_offset_min; x <= x_offset_max; x++)
    {
        cells.push_back(write_cell);
        colors.push_back(write_color);
    }

    return WriteRegion(x_offset_min, y_offset, cells, colors, redraw_rect, false);
}

void AnsiCanvas::WriteRegion(int x_offset, int y_offset, const AnsiCell &cell,
                             const AnsiColor &left_color, const AnsiColor &right_color,
                             HyRect &redraw_rect, bool is_convert_to_spaces/* = true*/)
{
    std::vector<AnsiCell> cells;
    cells.push_back(cell);

    std::vector<AnsiColor> colors;
    colors.push_back(left_color);
    colors.push_back(right_color);

    return WriteRegion(x_offset, y_offset, cells, colors, redraw_rect, false, is_convert_to_spaces);
}

void AnsiCanvas::WriteRegion(int x_offset, int y_offset,
                             const std::vector<AnsiCell> &cells,
                             const std::vector<AnsiColor> &colors,
                             HyRect &redraw_rect, bool is_handle_small_square,
                             bool is_convert_to_spaces/* = true*/)
{
    redraw_rect = HyRect(0, 0, 0, 0);

    if (y_offset < 0 || y_offset >= m_height)
        return;

    int cell_count = (int)cells.size();
    int color_count = (int)colors.size();
    if (cell_count == 0 || color_count == 0)
        return;

    std::vector<AnsiCell> valid_cells;
    std::vector<AnsiColor> valid_colors;
    int valid_x_offset_min = -1;
    int valid_x_offset_max = -1;

    int char_offset = 0;
    for (int i = 0; i < cell_count; i++)
    {
        int char_count = cells[i].GetCharCount();
        int color_offset_min = char_offset;
        int color_offset_max = color_offset_min + char_count - 1;
        int x_offset_min = x_offset + color_offset_min;
        int x_offset_max = x_offset + color_offset_max;

        char_offset += char_count;

        if (color_offset_max >= color_count)
            continue;
        if (x_offset_min < 0 || x_offset_min >= m_width)
            continue;
        if (x_offset_max < 0 || x_offset_max >= m_width)
            continue;

        valid_cells.push_back(cells[i]);
        for (int c = color_offset_min; c <= color_offset_max; c++)
            valid_colors.push_back(colors[c]);

        if (valid_x_offset_min == -1)
            valid_x_offset_min = x_offset_min;

        valid_x_offset_max = x_offset_max;
    }

    if (valid_x_offset_min == -1 || valid_x_offset_max == -1)
        return;

    // Rearrange (merge or convert to spaces) neighbor blocks if able before writing.
    // (If impossible, let the write action destroy the contents.)
    AnsiRow &row = m_cells[y_offset];
    AnsiCharLocation left_char_location, right_char_location;

    bool is_handle_left = false;
    if (GetCharLocation(row, valid_x_offset_min, left_char_location))
        is_handle_left = (left_char_location == DOUBLE_CHAR_RIGHT);

    bool is_handle_right = false;
    if (GetCharLocation(row, valid_x_offset_max, right_char_location))
        is_handle_right = (right_char_location == DOUBLE_CHAR_LEFT);

    if (is_handle_left)
    {
        AnsiColor &color = CanvasColorAt(valid_x_offset_min, y_offset);
        color.fg_color = color.bg_color;

        int space_color;
        bool is_convert_space = false;
        if (GetPureSpaceColor(valid_x_offset_min - 1, y_offset, space_color))
            is_convert_space = IsNormalColor(space_color);

        HyRect temp_rect;
        if (is_convert_space)
        {
            WriteAnsiData(valid_x_offset_min - 1, y_offset,
                          AnsiColor(ANSI_DEFAULT_FOREGROUND, space_color),
                          AnsiCell(EMPTY), temp_rect);
        }
        else
        {
            HyPoint block_center;
            block_center.x = ToCoordX(valid_x_offset_min - 1);
            block_center.y = ToCoordY(y_offset) + m_half_block_size;
            bool is_merge_success = MergeBlock(block_center, temp_rect, false);

            if (is_merge_success == false && is_handle_small_square)
            {
                // Try to merge nearby half-horizontal blocks until impossible,
                // then destroy the contents at the first location that cannot merge.
                // This behavior is only used for large brush with small-square resolution.

                int start_cell_offset;
                AnsiCharLocation start_char_location;
                SearchCellInfo(row, valid_x_offset_min - 1, start_cell_offset, start_char_location);

                int start_x_offset = valid_x_offset_min - 1;
                int end_x_offset = start_x_offset;
                int end_cell_offset = start_cell_offset;

                // Find continuous half-horizontal blocks. we cannot merge other combinations.
                if (row[start_cell_offset] == AnsiCell(HORI_BLOCK, 4))
                {
                    for (int cell_x = start_cell_offset - 1; cell_x >= 0; cell_x--)
                    {
                        if (row[cell_x] != AnsiCell(HORI_BLOCK, 4))
                            break;

                        end_x_offset -= 2;
                        end_cell_offset--;
                    }
                }

                // Rearrange continuous half-horizontal blocks:
                // 1. Get the space color at end_x_offset.
                // 2. Change the cell at start_cell_offset to space.
                // 3. Insert another space at end_cell_offset.
                // 4. Change the space color at end_x_offset.
                // (Also works when start_x_offset == end_x_offset, i.e. only one half-horizontal block.)

                int space_color = CanvasColorAt(end_x_offset, y_offset).bg_color;
                if (GetPureSpaceColor(end_x_offset, y_offset, space_color))
                    space_color = IsNormalColor(space_color);

                row[start_cell_offset] = AnsiCell(EMPTY);
                row.insert(row.begin() + end_cell_offset, AnsiCell(EMPTY));
                CanvasColorAt(end_x_offset, y_offset).bg_color = space_color;

                temp_rect.x = ToCoordX(end_x_offset);
                temp_rect.y = ToCoordY(y_offset);
                temp_rect.width = (start_x_offset - end_x_offset + 1) * m_half_block_size;
                temp_rect.height = m_block_size;
            }
        }

        redraw_rect = UnionRect(redraw_rect, temp_rect);
    }

    if (is_handle_right)
    {
        AnsiColor &color = CanvasColorAt(valid_x_offset_max, y_offset);
        color.fg_color = color.bg_color;

        int space_color;
        bool is_convert_space = false;
        if (GetPureSpaceColor(valid_x_offset_max + 1, y_offset, space_color))
            is_convert_space = IsNormalColor(space_color);

        HyRect temp_rect;
        if (is_convert_space)
        {
            WriteAnsiData(valid_x_offset_max + 1, y_offset,
                          AnsiColor(ANSI_DEFAULT_FOREGROUND, space_color),
                          AnsiCell(EMPTY), temp_rect);
        }
        else
        {
            HyPoint block_center;
            block_center.x = ToCoordX(valid_x_offset_max + 2);
            block_center.y = ToCoordY(y_offset) + m_half_block_size;
            bool is_merge_success = MergeBlock(block_center, temp_rect, false);

            if (is_merge_success == false && is_handle_small_square)
            {
                // See the comments at the left part. The follow algorithm is symmetric to it.

                int start_cell_offset;
                AnsiCharLocation start_char_location;
                SearchCellInfo(row, valid_x_offset_max + 1, start_cell_offset, start_char_location);

                int start_x_offset = valid_x_offset_max + 1;
                int end_x_offset = start_x_offset;
                int end_cell_offset = start_cell_offset;

                if (row[start_cell_offset] == AnsiCell(HORI_BLOCK, 4))
                {
                    for (int cell_x = start_cell_offset + 1; cell_x < (int)row.size(); cell_x++)
                    {
                        if (row[cell_x] != AnsiCell(HORI_BLOCK, 4))
                            break;

                        end_x_offset += 2;
                        end_cell_offset++;
                    }
                }

                int space_color = CanvasColorAt(end_x_offset, y_offset).bg_color;
                if (GetPureSpaceColor(end_x_offset, y_offset, space_color))
                    space_color = IsNormalColor(space_color);

                row[end_cell_offset] = AnsiCell(EMPTY);
                row.insert(row.begin() + start_cell_offset, AnsiCell(EMPTY));
                CanvasColorAt(end_x_offset, y_offset).bg_color = space_color;

                temp_rect.x = ToCoordX(start_x_offset);
                temp_rect.y = ToCoordY(y_offset);
                temp_rect.width = (end_x_offset - start_x_offset + 1) * m_half_block_size;
                temp_rect.height = m_block_size;

            }
        }

        redraw_rect = UnionRect(redraw_rect, temp_rect);
    }

    int valid_char_offset = 0;
    for (int i = 0; i < (int)valid_cells.size(); i++)
    {
        const AnsiCell &write_cell = valid_cells[i];
        int char_count = write_cell.GetCharCount();

        int write_x_offset = valid_x_offset_min + valid_char_offset;

        HyRect temp_rect;
        if (char_count == 2)
        {
            WriteBlock(write_x_offset, y_offset, write_cell,
                       valid_colors[valid_char_offset],
                       valid_colors[valid_char_offset + 1],
                       temp_rect, is_convert_to_spaces);
        }
        else
        {
            WriteAnsiData(write_x_offset, y_offset,
                          valid_colors[valid_char_offset],
                          write_cell, temp_rect);
        }

        redraw_rect = UnionRect(redraw_rect, temp_rect);

        valid_char_offset += char_count;
    }
}

void AnsiCanvas::ClearActionHistory()
{
    m_action_history.clear();
    m_current_action_index = 0;

    if (mp_colors != NULL && mp_old_colors != NULL)
        memcpy(mp_old_colors, mp_colors, sizeof(AnsiColor) * m_width * m_height);

    m_old_cells = m_cells;
}

void AnsiCanvas::DumpHistory()
{
    const int action_count = (int)m_action_history.size();
    ch_dprintf(_T("step: %d / %d"), m_current_action_index, action_count);

    for (int i = 0; i < action_count; i++)
    {
        const ActionUnit &unit = m_action_history[i];

        ch_dprintf(_T("action step %d:"), i);
        ch_dprintf(_T(" - type %d, ROI (%d, %d, %d, %d)"), unit.type,
            unit.roi.x, unit.roi.y, unit.roi.width, unit.roi.height);
        ch_dprintf(_T(" - color data %d/%d, cell data %d/%d"),
            unit.old_colors.size(), unit.new_colors.size(),
            unit.old_cells.size(), unit.new_cells.size());
    }
}

bool AnsiCanvas::RecordAction(const ActionInfo &action_info)
{
    HyRect current_roi;
    if (action_info.type == AC_EDIT_EXTEND_LINE)
    {
        current_roi.x = 0;
        current_roi.y = action_info.old_canvas_height;
        current_roi.width = m_width;
        current_roi.height = m_height - action_info.old_canvas_height;
    }
    else if (action_info.type == AC_EDIT_CLEAR_CANVAS)
    {
        current_roi.x = 0;
        current_roi.y = 0;
        current_roi.width = m_width;
        current_roi.height = m_height;
    }
    else
    {
        current_roi.x = m_display_x_offset + FloorDivision(action_info.change_rect.x, m_half_block_size);
        current_roi.y = m_display_y_offset + FloorDivision(action_info.change_rect.y, m_block_size);
        current_roi.width = action_info.change_rect.width / m_half_block_size;
        current_roi.height = action_info.change_rect.height / m_block_size;
        current_roi = hyIntersectRect(current_roi, hyRect(0, 0, m_width, m_height));
    }

    if (current_roi.width < 0 || current_roi.height < 0)
        return false;

    bool is_action_list_changed = false;

    const bool is_continuous = IsContinuousAction(action_info);

    // Remove actions with index >= m_current_action_index. (They should be discarded.)
    if ((int)m_action_history.size() > m_current_action_index)
        m_action_history.erase(m_action_history.begin() + m_current_action_index, m_action_history.end());

    // Record current action.
    HyRect action_roi = current_roi;
    if (is_continuous)
    {
        const ActionUnit &old_action = m_action_history[m_current_action_index - 1];
        PrepareOldCanvasDataForContinuousAction(old_action, mp_old_colors, m_old_cells);
        action_roi = UnionRect(action_roi, old_action.roi);
    }
    else
    {
        m_action_history.push_back(ActionUnit());
        is_action_list_changed = true;
    }

    m_current_action_index = (int)m_action_history.size();

    ActionUnit &action_unit = m_action_history.back();
    action_unit.type = action_info.type;
    action_unit.is_alternated_edit = action_info.is_alternated_edit;
    action_unit.roi = action_roi;

    const int y_start = action_roi.y;
    const int y_range = action_roi.height;
    const int y_end = y_start + y_range;

    if (action_info.type == AC_EDIT_EXTEND_LINE)
    {
        action_unit.old_colors.resize(0);
        action_unit.new_colors.resize(m_width * y_range);
        action_unit.old_cells.resize(0);
        action_unit.new_cells.resize(y_range);

        for (int y_offset = y_start; y_offset < y_end; y_offset++)
        {
            const int local_y_offset = y_offset - y_start;

            const AnsiRow &new_row = m_cells[y_offset];
            action_unit.new_cells[local_y_offset] = new_row;

            const AnsiColor *p_new_color_scan = mp_colors + y_offset * m_width;
            memcpy(&(action_unit.new_colors[local_y_offset * m_width]), p_new_color_scan, sizeof(AnsiColor) * m_width);
        }

        // Update old cells and colors. The color array needs to be reallocated.
        for (int y_offset = y_start; y_offset < y_start + y_range; y_offset++)
            m_old_cells.push_back(m_cells[y_offset]);

        _NEW_PTRS(mp_old_colors, AnsiColor, m_width * m_height);
        memcpy(mp_old_colors, mp_colors, sizeof(AnsiColor) * m_width * m_height);
    }
    else
    {
        const int x_start = action_roi.x;
        const int x_range = action_roi.width;

        action_unit.old_colors.resize(x_range * y_range);
        action_unit.new_colors.resize(x_range * y_range);
        action_unit.old_cells.resize(y_range);
        action_unit.new_cells.resize(y_range);

        for (int y_offset = y_start; y_offset < y_end; y_offset++)
        {
            const int local_y_offset = y_offset - y_start;

            const AnsiRow &old_row = m_old_cells[y_offset];
            const AnsiRow &new_row = m_cells[y_offset];
            action_unit.old_cells[local_y_offset] = old_row;
            action_unit.new_cells[local_y_offset] = new_row;

            AnsiColor *p_old_color_scan = mp_old_colors + y_offset * m_width + x_start;
            const AnsiColor *p_new_color_scan = mp_colors + y_offset * m_width + x_start;

            memcpy(&(action_unit.old_colors[local_y_offset * x_range]), p_old_color_scan, sizeof(AnsiColor) * x_range);
            memcpy(&(action_unit.new_colors[local_y_offset * x_range]), p_new_color_scan, sizeof(AnsiColor) * x_range);
        }

        UpdateOldCanvasData(action_roi);
    }

    // Remove oldest action if the history size reaches the limit.
    int remove_count = m_current_action_index - MAX_ACTION_HISTORY_SIZE;
    if (remove_count > 0)
    {
        m_action_history.erase(m_action_history.begin(), m_action_history.begin() + remove_count);
        m_current_action_index -= remove_count;
        is_action_list_changed = true;
    }

    return is_action_list_changed;
}

void AnsiCanvas::PrepareOldCanvasDataForContinuousAction(const ActionUnit &old_action,
                                                         AnsiColor *p_old_colors, 
                                                         std::vector<AnsiRow> &old_cells)
{
    // Restore p_old_colors and old_cells to the status BEFORE the given action.

    const int x_start = old_action.roi.x;
    const int y_start = old_action.roi.y;
    const int x_range = old_action.roi.width;
    const int y_range = old_action.roi.height;
    const int y_end = y_start + y_range;

    for (int y_offset = y_start; y_offset < y_end; y_offset++)
    {
        const int local_y_offset = y_offset - y_start;

        old_cells[y_offset] = old_action.old_cells[local_y_offset];
        memcpy(p_old_colors + y_offset * m_width + x_start,
               &(old_action.old_colors[local_y_offset * x_range]), sizeof(AnsiColor) * x_range);
    }
}

void AnsiCanvas::UpdateOldCanvasData(const HyRect &offset_roi)
{
    int x_start = offset_roi.x;
    int y_start = offset_roi.y;
    int x_range = offset_roi.width;
    int y_range = offset_roi.height;

    for (int y_offset = y_start; y_offset < y_start + y_range; y_offset++)
    {
        memcpy(mp_old_colors + y_offset * m_width + x_start,
                mp_colors + y_offset * m_width + x_start, sizeof(AnsiColor) * x_range);

        m_old_cells[y_offset] = m_cells[y_offset];
    }
}

std_tstring AnsiCanvas::GetActionName(const ActionUnit &unit)
{
    if (unit.type == AC_EDIT_DRAW_SPACE)
        return std_tstring(_T("填入半形空白"));
    else if (unit.type == AC_EDIT_DRAW_BLOCK)
        return std_tstring(_T("填入全形字元"));
    else if (unit.type == AC_EDIT_CHANGE_COLOR)
        return std_tstring(unit.is_alternated_edit ? _T("改變範圍顏色"): _T("改變顏色"));
    else if (unit.type == AC_EDIT_REFINE_BOUNDARY)
        return std_tstring(_T("邊緣調整(一般)"));
    else if (unit.type == AC_EDIT_REFINE_BOUNDARY_HALF)
        return std_tstring(_T("邊緣調整(部分)"));
    else if (unit.type == AC_EDIT_REFINE_BOUNDARY_TRIANGLE)
        return std_tstring(_T("邊緣調整(三角)"));
    else if (unit.type == AC_EDIT_REFINE_BOUNDARY_REGULAR_TRIANGLE)
        return std_tstring(_T("邊緣調整(正三角)"));
    else if (unit.type == AC_EDIT_DRAW_SMALL_SQUARE)
        return std_tstring(_T("填入小正方格"));
    else if (unit.type == AC_EDIT_MERGE_BLOCK)
        return std_tstring(_T("合併全形方格"));
    else if (unit.type == AC_EDIT_INSERT_DELETE_SPACE)
        return std_tstring(unit.is_alternated_edit ? _T("刪除半形空白"): _T("插入半形空白"));
    else if (unit.type == AC_EDIT_DRAW_LARGE_BRUSH)
        return std_tstring(_T("大型筆刷"));
    else if (unit.type == AC_EDIT_ADD_TEXT)
        return std_tstring(_T("填入文字"));
    else if (unit.type == AC_EDIT_EXTEND_LINE)
        return std_tstring(_T("延伸底端範圍"));
    else if (unit.type == AC_EDIT_CLEAR_CANVAS)
        return std_tstring(_T("清除畫板"));
    else
        return std_tstring(_T("檢視內容"));
}

int AnsiCanvas::GetTotalActionCount()
{
    ChAutoLock auto_lock(&m_data_lock);

    return (int)m_action_history.size();
}

int AnsiCanvas::GetCurrentActionIndex()
{
    ChAutoLock auto_lock(&m_data_lock);

    return m_current_action_index;
}
    
std::vector<std_tstring> AnsiCanvas::GetActionNameList()
{
    ChAutoLock auto_lock(&m_data_lock);

    std::vector<std_tstring> name_list;

    for (int i = 0; i < (int)m_action_history.size(); i++)
        name_list.push_back(GetActionName(m_action_history[i])); 

    return name_list;
}

bool AnsiCanvas::SetActionIndex(int new_index, HyRect &change_rect, bool &is_display_offset_moved)
{
    // Bound new_index to valid range and set currentaction index to it.
    // Return false if the action is unchanged.

    ChAutoLock auto_lock(&m_data_lock);

    change_rect = hyRect(0, 0, 0, 0);
    is_display_offset_moved = false;

    const int action_count = (int)m_action_history.size();

    int valid_new_index = FitInRange(new_index, 0, action_count);
    if (valid_new_index == m_current_action_index)
        return false;

    // At action index N, we need to do all actions with index < N, but not the actions with index >= N.
    // (The action index starts from 0. If N = action count, all actions will be done.)

    HyRect union_offset_roi(0, 0, 0, 0);
    HyRect temp_offset_roi(0, 0, 0, 0);

    if (valid_new_index < m_current_action_index)
    {
        const int undo_count = m_current_action_index - valid_new_index;
        for (int i = 0; i < undo_count; i++)
        {
            UndoOneAction(temp_offset_roi);
            union_offset_roi = UnionRect(union_offset_roi, temp_offset_roi);
        }
    }
    else
    {
        const int redo_count = valid_new_index - m_current_action_index;
        for (int i = 0; i < redo_count; i++)
        {
            RedoOneAction(temp_offset_roi);
            union_offset_roi = UnionRect(union_offset_roi, temp_offset_roi);
        }
    }

    change_rect.x = ToCoordX(union_offset_roi.x);
    change_rect.y = ToCoordY(union_offset_roi.y);
    change_rect.width = union_offset_roi.width * m_half_block_size;
    change_rect.height = union_offset_roi.height * m_block_size;

    // Check if we need to move the display offset.
    // This happens when some action changed the canvas size,
    // and the old display offset becomes out-of-bound.
    int valid_display_x_offset = FitInRange(m_display_x_offset, 0, m_width - MIN_CANVAS_WIDTH);
    int valid_display_y_offset = FitInRange(m_display_y_offset, 0, m_height - MIN_CANVAS_HEIGHT);

    is_display_offset_moved = (valid_display_x_offset != m_display_x_offset ||
                               valid_display_y_offset != m_display_y_offset);
    if (is_display_offset_moved)
    {
        m_display_x_offset = valid_display_x_offset;
        m_display_y_offset = valid_display_y_offset;
    }

    return true;
}

bool AnsiCanvas::UndoOneAction(HyRect &offset_roi)
{
    offset_roi = hyRect(0, 0, 0, 0);

    if (m_current_action_index == 0)
        return false;

    const ActionUnit &action = m_action_history[m_current_action_index - 1];
    offset_roi = action.roi;

    const int x_start = offset_roi.x;
    const int y_start = offset_roi.y;
    const int x_range = offset_roi.width;
    const int y_range = offset_roi.height;
    const int y_end = y_start + y_range;

    if (action.type == AC_EDIT_EXTEND_LINE)
    {
        // Remove rows at [y_start, y_end). The canvas size is changed.

        m_old_cells.erase(m_old_cells.begin() + y_start, m_old_cells.begin() + y_end);
        m_cells.erase(m_cells.begin() + y_start, m_cells.begin() + y_end);

        m_height -= y_range;
        _NEW_PTRS(mp_old_colors, AnsiColor, m_width * m_height);
        for (int y_offset = 0; y_offset < y_start; y_offset++)
        {
            memcpy(mp_old_colors + y_offset * m_width,
                   mp_colors + y_offset * m_width, sizeof(AnsiColor) * m_width);
        }
        for (int y_offset = y_start; y_offset < m_height; y_offset++)
        {
            memcpy(mp_old_colors + y_offset * m_width,
                   mp_colors + (y_offset + y_range) * m_width, sizeof(AnsiColor) * m_width);
        }
            
        _NEW_PTRS(mp_colors, AnsiColor, m_width * m_height);
        memcpy(mp_colors, mp_old_colors, sizeof(AnsiColor) * m_width * m_height);
    }
    else
    {
        for (int y_offset = y_start; y_offset < y_end; y_offset++)
        {
            const int local_y = y_offset - y_start;

            const AnsiRow &old_row = action.old_cells[local_y];
            m_old_cells[y_offset] = old_row;
            m_cells[y_offset] = old_row;

            const AnsiColor *p_old_color_scan = &(action.old_colors[local_y * x_range]);
            memcpy(mp_old_colors + y_offset * m_width + x_start, p_old_color_scan, sizeof(AnsiColor) * x_range);
            memcpy(mp_colors + y_offset * m_width + x_start, p_old_color_scan, sizeof(AnsiColor) * x_range);
        }
    }

    m_current_action_index--;

    return true;
}

bool AnsiCanvas::RedoOneAction(HyRect &offset_roi)
{
    offset_roi = hyRect(0, 0, 0, 0);

    if (m_current_action_index == (int)m_action_history.size())
        return false;

    const ActionUnit &action = m_action_history[m_current_action_index];
    offset_roi = action.roi;

    const int x_start = offset_roi.x;
    const int y_start = offset_roi.y;
    const int x_range = offset_roi.width;
    const int y_range = offset_roi.height;
    const int y_end = y_start + y_range;

    if (action.type == AC_EDIT_EXTEND_LINE)
    {
        // Insert rows at [y_start, y_end). The canvas size is changed.
        // The inserted data come from the action's "new" data.

        m_old_cells.insert(m_old_cells.begin() + y_start, action.new_cells.begin(), action.new_cells.end());
        m_cells.insert(m_cells.begin() + y_start, action.new_cells.begin(), action.new_cells.end());

        m_height += y_range;
        _NEW_PTRS(mp_old_colors, AnsiColor, m_width * m_height);
        for (int y_offset = 0; y_offset < y_start; y_offset++)
        {
            memcpy(mp_old_colors + y_offset * m_width,
                   mp_colors + y_offset * m_width, sizeof(AnsiColor) * m_width);
        }
        for (int y_offset = y_start; y_offset < y_end; y_offset++)
        {
            memcpy(mp_old_colors + y_offset * m_width,
                   &(action.new_colors[(y_offset - y_start) * m_width]), sizeof(AnsiColor) * m_width);
        }
        for (int y_offset = y_end; y_offset < m_height; y_offset++)
        {
            memcpy(mp_old_colors + y_offset * m_width,
                   mp_colors + (y_offset - y_range) * m_width, sizeof(AnsiColor) * m_width);
        }
            
        _NEW_PTRS(mp_colors, AnsiColor, m_width * m_height);
        memcpy(mp_colors, mp_old_colors, sizeof(AnsiColor) * m_width * m_height);
    }
    else
    {
        for (int y_offset = y_start; y_offset < y_end; y_offset++)
        {
            const int local_y = y_offset - y_start;

            const AnsiRow &new_row = action.new_cells[local_y];
            m_old_cells[y_offset] = new_row;
            m_cells[y_offset] = new_row;

            if (x_range > 0)
            {
                const AnsiColor *p_new_color_scan = &(action.new_colors[local_y * x_range]);
                memcpy(mp_old_colors + y_offset * m_width + x_start, p_new_color_scan, sizeof(AnsiColor) * x_range);
                memcpy(mp_colors + y_offset * m_width + x_start, p_new_color_scan, sizeof(AnsiColor) * x_range);
            }
        }
    }

    m_current_action_index++;

    return true;
}

 bool AnsiCanvas::IsContinuousAction(const ActionInfo &action_info)
 {
     if (action_info.is_continuous == false)
         return false;

     // Double-check if the action type matches the previous action.
     int previous_action_index = m_current_action_index - 1;
     if (previous_action_index >= 0 && previous_action_index < (int)m_action_history.size())
     {
         if (action_info.type == m_action_history[previous_action_index].type)
             return true;
     }

     return false;
 }