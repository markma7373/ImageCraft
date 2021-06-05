#include "stdafx.h"
#include "TextImageMaker.h"
#include "AnsiShare.h"

using namespace AnsiShare;
using namespace Gdiplus;

TextImageMaker::TextImageMaker()
: mp_gray_text_image(NULL)
, mp_color_text_image(NULL)
, mp_bitmap_image(NULL)
, mp_bitmap(NULL)
, mp_graphics(NULL)
, mp_font(NULL)
, m_character(_T('\0'))
, m_is_full_width(false)
{
    mp_brush = new SolidBrush(Color(255, 255, 255, 255));
    mp_font_family = new FontFamily(_T("²Ó©úÅé"));

    SetFontSize(24);
}

TextImageMaker::~TextImageMaker()
{
    hyReleaseImage(&mp_gray_text_image);
    hyReleaseImage(&mp_color_text_image);
    hyReleaseImage(&mp_bitmap_image);
    _DELETE_PTR(mp_bitmap);
    _DELETE_PTR(mp_graphics);
    _DELETE_PTR(mp_brush);
    _DELETE_PTR(mp_font_family);
    _DELETE_PTR(mp_font);
}

void TextImageMaker::SetFontSize(int font_size)
{
    if (m_font_size == font_size)
        return;

    hyReleaseImage(&mp_gray_text_image);
    hyReleaseImage(&mp_color_text_image);
    hyReleaseImage(&mp_bitmap_image);
    _DELETE_PTR(mp_bitmap);
    _DELETE_PTR(mp_graphics);
    _DELETE_PTR(mp_font);

    m_font_size = font_size;

    mp_gray_text_image = hyCreateImage(hySize(m_font_size, m_font_size), HY_DEPTH_8U, 1);
    mp_color_text_image = hyCreateImage(hySize(m_font_size, m_font_size), HY_DEPTH_8U, 3);

    int buffer_size = m_font_size * 2;
    mp_bitmap_image = hyCreateImage(hySize(buffer_size, buffer_size), HY_DEPTH_8U, 3);

    mp_bitmap = new Bitmap(mp_bitmap_image->width, mp_bitmap_image->height, 
                           mp_bitmap_image->widthStep, PixelFormat24bppRGB, mp_bitmap_image->imageData);

    mp_graphics = Graphics::FromImage(mp_bitmap);

    mp_font = new Font(mp_font_family, (float)m_font_size, FontStyleRegular, UnitPixel);
}

void TextImageMaker::SetText(_TCHAR text)
{
    HY_ZEROIMAGE(mp_bitmap_image);

    _TCHAR text_string[2] = {text, _T('\0')};
    mp_graphics->DrawString(text_string, -1, mp_font, PointF(0.0f, 0.0f), mp_brush);

    CharacterRange char_range(0, 1);
    StringFormat str_format;
    str_format.SetMeasurableCharacterRanges(1, &char_range);

    RectF layout_rect(0.0f, 0.0f, (float)mp_bitmap->GetWidth(), (float)mp_bitmap->GetHeight());
    Region char_region;
    Status status = mp_graphics->MeasureCharacterRanges(text_string, -1, mp_font, layout_rect, &str_format, 1, &char_region);
    RectF region_bound;
    char_region.GetBounds(&region_bound, mp_graphics);

    HyRect bound_rect;
    bound_rect.x = ch_Round(region_bound.X);
    bound_rect.y = ch_Round(region_bound.Y);
    bound_rect.width = ch_Round(region_bound.Width) - 1;
    bound_rect.height = ch_Round(region_bound.Height);

    m_character = text;
    m_is_full_width = IsFullWidth(text);

    // For Unicode values U+0080 ~ U+00FF,
    // we need a special alignment to match the display of BBS:
    // The contents from DrawString() is expected as half-width size,
    // but the generated image is treated as full-width.
    int target_width = (m_is_full_width ? m_font_size : m_font_size / 2);
    int target_height = m_font_size;
    int expected_width = target_width;
    if (text >= 0x0080 && text <= 0x00FF)
        expected_width = m_font_size / 2;

    int bound_width = bound_rect.width;
    int bound_height = bound_rect.height;
    int x_shift = (expected_width - bound_rect.width + 1) / 2;
    int y_shift = (target_height - bound_rect.height + 1) / 2;
    HyRect roi_rect;
    roi_rect.x = bound_rect.x - x_shift;
    roi_rect.y = bound_rect.y - y_shift;
    roi_rect.width = expected_width;
    roi_rect.height = target_height;

    roi_rect.x = FitInRange(roi_rect.x, 0, mp_bitmap_image->width - expected_width);
    roi_rect.y = FitInRange(roi_rect.y, 0, mp_bitmap_image->height - target_height);

    HY_ZEROIMAGE(mp_gray_text_image);
    ippiColorToGray_8u_C3C1R(mp_bitmap_image->get_pixels<BYTE>(roi_rect.x, roi_rect.y), mp_bitmap_image->widthStep,
                             mp_gray_text_image->imageData, mp_gray_text_image->widthStep, ippiSize(expected_width, target_height), ipp_bgr_to_gray_coeff);

    // Set the ROI of mp_gray_text_image as its valid range.
    hySetImageROI(mp_gray_text_image, hyRect(0, 0, target_width, target_height));
}

HyImage *TextImageMaker::GetTextImage(const AnsiColor &color)
{
    HyRect text_roi = mp_gray_text_image->roi;
    hySetImageROI(mp_color_text_image, text_roi);

    BYTE fg_bgr[3] = 
    {
        HY_GetBValue(color.fg_color),
        HY_GetGValue(color.fg_color),
        HY_GetRValue(color.fg_color),
    };

    BYTE bg_bgr[3] = 
    {
        HY_GetBValue(color.bg_color),
        HY_GetGValue(color.bg_color),
        HY_GetRValue(color.bg_color),
    };

    for (int y = text_roi.y; y < text_roi.Bottom(); y++)
    {
        const BYTE *p_gray_scan = mp_gray_text_image->imageData + y * mp_gray_text_image->widthStep;
        BYTE *p_color_scan = mp_color_text_image->imageData + y * mp_color_text_image->widthStep;

        for (int x = text_roi.x; x < text_roi.Right(); x++)
        {
            BYTE *p_color_pixel = p_color_scan + x * 3;

            int fg_weight = (int)p_gray_scan[x];
            int bg_weight = 255 - fg_weight;
            for (int c = 0; c < 3; c++)
                p_color_pixel[c] = (BYTE)((fg_bgr[c] * fg_weight + bg_bgr[c] * bg_weight + 255) >> 8);
        }
    }

    return mp_color_text_image;
}