#include "stdafx.h"
#include "AnsiTemplate.h"

using namespace AnsiShare;

AnsiTemplate::AnsiTemplate()
{
    SetSize(16);
}

AnsiTemplate::~AnsiTemplate()
{

}

void AnsiTemplate::SetSize(int size)
{
    m_size = size;
    m_half_size = size / 2;

    for (int i = 0; i <= 8; i++)
    {
        BlendSpec &spec = m_blend_specs[i];

        float fg_real_size = (float)m_size * i / 8;
        int fg_size = (int)fg_real_size;

        if (fg_size == m_size)
        {
            spec.fg_size = m_size - 1;
            spec.bg_size = 0;
            spec.fg_weight = 256;
            spec.bg_weight = 0;
        }
        else
        {
            spec.fg_size = fg_size;
            spec.bg_size = m_size - 1 - fg_size;
            spec.fg_weight = ch_Round((fg_real_size - fg_size) * 256.0f);
            spec.bg_weight = 256 - spec.fg_weight;
        }
    }

    m_text_image_maker.SetFontSize(m_size);
}

static const int g_triangle_color_factor[8][4] = 
{
    {-1, -1,  2, -1}, {-1, -1,  1, -1}, { 1, -1,  0,  0}, { 1, -1,  1,  0},
    {-1,  1,  0,  0}, {-1,  1, -1,  0}, { 1,  1, -2,  1}, { 1,  1, -1,  1},
};

void AnsiTemplate::DrawHalfBlock(BYTE *p_image_data, int image_step, int channels, const AnsiColor &color, const AnsiCell &cell, bool is_right)
{
    BYTE bg_buffer[3] = {0};
    ExtractColor(color.bg_color, bg_buffer);

    if (cell.type == EMPTY)
    {
        for (int y = 0; y < m_size; y++)
        {
            BYTE *p_image_scan = p_image_data + y * image_step;

            for (int x = 0; x < m_half_size; x++)
                memcpy(p_image_scan + x * 3, bg_buffer, 3);
        }

        return;
    }

    BYTE fg_buffer[3] = {0};
    BYTE blend_buffer[3] = {0};
    ExtractColor(color.fg_color, fg_buffer);

    switch(cell.type)
    {
    case HORI_BLOCK:
        {
            // is_right is invariant for horizontal block

            const BlendSpec &spec = m_blend_specs[cell.label];
            int fg_height = spec.fg_size;
            int bg_height = spec.bg_size;

            for (int y = 0; y < bg_height; y++)
            {
                BYTE *p_row_start = p_image_data + y * image_step;

                for (int x = 0; x < m_half_size; x++)
                {
                    memcpy(p_row_start, bg_buffer, 3);
                    p_row_start += channels;
                }
            }

            for (int c = 0; c < 3; c++)
                blend_buffer[c] = (BYTE)((spec.fg_weight * fg_buffer[c] + spec.bg_weight * bg_buffer[c]) >> 8);

            BYTE *p_blend_row_start = p_image_data + bg_height * image_step;
            for (int x = 0; x < m_half_size; x++)
            {
                memcpy(p_blend_row_start, blend_buffer, 3);
                p_blend_row_start += channels;
            }
                
            for (int y = m_size - fg_height; y < m_size; y++)
            {
                BYTE *p_row_start = p_image_data + y * image_step;

                for (int x = 0; x < m_half_size; x++)
                {
                    memcpy(p_row_start, fg_buffer, 3);
                    p_row_start += channels;
                }
            }

            break;
        }

    case VERT_BLOCK:
        {
            const BlendSpec &spec = m_blend_specs[cell.label];
            int fg_width = spec.fg_size;
            int bg_width = spec.bg_size;

            for (int c = 0; c < 3; c++)
                blend_buffer[c] = (BYTE)((spec.fg_weight * fg_buffer[c] + spec.bg_weight * bg_buffer[c]) >> 8);

            ChAutoPtr<BYTE> row_buffer(m_half_size * channels);
            BYTE *p_buffer = (BYTE *)row_buffer;

            if (is_right)
            {
                if (fg_width > m_half_size)
                {
                    for (int x = 0; x < fg_width - m_half_size; x++)
                        memcpy(p_buffer + x * channels, fg_buffer, 3);
                }

                if (fg_width >= m_half_size)
                    memcpy(p_buffer + (fg_width - m_half_size) * channels, blend_buffer, 3);

                bg_width = ch_Min(bg_width, m_half_size);
                for (int x = m_half_size - bg_width; x < m_half_size; x++)
                    memcpy(p_buffer + x * channels, bg_buffer, 3);
            }
            else
            {
                fg_width = ch_Min(fg_width, m_half_size);
                for (int x = 0; x < fg_width; x++)
                    memcpy(p_buffer + x * channels, fg_buffer, 3);

                if (fg_width < m_half_size)
                    memcpy(p_buffer + fg_width * channels, blend_buffer, 3);

                if (fg_width + 1 < m_half_size)
                {
                    for (int x = fg_width + 1; x < m_half_size; x++)
                        memcpy(p_buffer + x * channels, bg_buffer, 3);
                }
            }

            for (int y = 0; y < m_size; y++)
                memcpy(p_image_data + y * image_step, p_buffer, m_half_size * channels);

            break;
        }

    case TRIANGLE:
        {
            int case_label = cell.label * 2 + (is_right ? 1 : 0);
            int fx = g_triangle_color_factor[case_label][0];
            int fy = g_triangle_color_factor[case_label][1];
            int fw = g_triangle_color_factor[case_label][2];
            int fc = g_triangle_color_factor[case_label][3];

            for (int c = 0; c < 3; c++)
                blend_buffer[c] = (BYTE)(((int)fg_buffer[c] + (int)bg_buffer[c]) >> 1);

            for (int y = 0; y < m_size; y++)
            {
                BYTE *p_row_start = p_image_data + y * image_step;

                for (int x = 0; x < m_half_size; x++)
                {
                    BYTE *p_pixel = p_row_start + x * channels;

                    int determinant = fx * x + fy * y + fw * m_half_size + fc;
                    if (determinant > 0)
                        memcpy(p_pixel, fg_buffer, 3);
                    else if (determinant < 0)
                        memcpy(p_pixel, bg_buffer, 3);
                    else
                        memcpy(p_pixel, blend_buffer, 3);
                }
            }

            break;
        }

    case REGULAR_TRIANGLE:
    case GENERAL_CHAR:
        {
            for (int y = 0; y < m_size; y++)
            {
                BYTE *p_image_scan = p_image_data + y * image_step;

                for (int x = 0; x < m_half_size; x++)
                    memcpy(p_image_scan + x * 3, bg_buffer, 3);
            }

            HyRect half_roi(0, 0, m_half_size, m_size);
            if (is_right)
                half_roi.x = m_half_size;

            wchar_t unicode_char = (wchar_t)cell.label;
            if (cell.type == REGULAR_TRIANGLE)
                unicode_char = (cell.label == 0 ? L'¡¶' : L'¡¿');

            m_text_image_maker.SetText(unicode_char);
            HyImage *p_text_image = m_text_image_maker.GetTextImage(color);
            HyRect valid_roi = hyIntersectRect(half_roi, p_text_image->roi);
            if (valid_roi.width > 0 && valid_roi.height > 0)
            {
                const BYTE *p_valid_text_start = p_text_image->get_pixels<BYTE>(valid_roi);

                int valid_draw_start_x = valid_roi.x;
                int valid_draw_start_y = valid_roi.y;
                if (is_right)
                    valid_draw_start_x -= m_half_size;

                BYTE *p_valid_draw_start = p_image_data + valid_draw_start_y * image_step + valid_draw_start_x * 3;
                for (int y = 0; y < valid_roi.height; y++)
                {
                    memcpy(p_valid_draw_start + y * image_step,
                           p_valid_text_start + y * p_text_image->widthStep,
                           valid_roi.width * 3);
                }
            }

            break;
        }

    default:
        break;
    }
}

void AnsiTemplate::DrawHalfBlock(HyImage *p_image, int x, int y, const AnsiColor &color, const AnsiCell &cell, bool is_right)
{
    DrawHalfBlock(p_image->get_pixels<BYTE>(x, y), 
                    p_image->widthStep, p_image->nChannels, color, cell, is_right);
}

void AnsiTemplate::DrawBlock(HyImage *p_image, int x, int y, const AnsiColor &color, const AnsiCell &cell)
{
    DrawHalfBlock(p_image->get_pixels<BYTE>(x, y), 
                    p_image->widthStep, p_image->nChannels, color, cell, false);

    if (cell.IsDoubleChar())
    {
        DrawHalfBlock(p_image->get_pixels<BYTE>(x + m_half_size, y), 
                        p_image->widthStep, p_image->nChannels, color, cell, true);
    }
}

void AnsiTemplate::GetStringImage(HyImage *p_string_image, LPCTSTR text_string)
{
    if (p_string_image == NULL || text_string == NULL)
        return;

    HY_ZEROIMAGE(p_string_image);

    int string_length = _tcslen(text_string);
    if (string_length == 0)
        return;

    const int image_width = p_string_image->width;
    const int image_height = p_string_image->height;
    const HyRect image_rect(0, 0, image_width, image_height);

    int x_offset = 0;

    for (int i = 0; i < string_length; i++)
    {
        m_text_image_maker.SetText(text_string[i]);
        HyImage *p_text_image = m_text_image_maker.GetTextImage(AnsiColor(ANSI_WHITE_BRIGHT, ANSI_BLACK));
        int text_roi_x = p_text_image->roi.x;
        int text_roi_y = p_text_image->roi.y;
        int text_width = p_text_image->roi.width;
        int text_height = p_text_image->roi.height;

        HyRect text_rect;
        text_rect.x = x_offset;
        text_rect.y = 0;
        text_rect.width = text_width;
        text_rect.height = text_height;

        HyRect valid_rect = hyIntersectRect(text_rect, image_rect);
        if (valid_rect.width <= 0 || valid_rect.height <= 0)
            break;

        int valid_x_start = valid_rect.x - text_rect.x;
        int valid_y_start = valid_rect.y - text_rect.y;

        ippiCopy_8u_C3R(p_text_image->get_pixels<BYTE>(text_roi_x + valid_x_start, text_roi_y + valid_y_start), p_text_image->widthStep,
                        p_string_image->get_pixels<BYTE>(valid_rect.x, valid_rect.y), p_string_image->widthStep, ippiSize(valid_rect.width, valid_rect.height));


        x_offset += text_width;
    }
}