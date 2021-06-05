#pragma once

#include "Common.h"
#include "use_hylib.h"
#include "use_gdiplus.h"
#include "AnsiShare.h"

class TextImageMaker
{
public:
    TextImageMaker();
    ~TextImageMaker();

    void SetFontSize(int font_size);
    void SetText(_TCHAR text);

    HyImage *GetTextImage(const AnsiShare::AnsiColor &color);

private:
    int m_font_size;
    HyImage *mp_gray_text_image;
    HyImage *mp_color_text_image;
    HyImage *mp_bitmap_image;
    Gdiplus::Bitmap *mp_bitmap;
    Gdiplus::Graphics *mp_graphics;

    _TCHAR m_character;
    bool m_is_full_width;

    Gdiplus::SolidBrush *mp_brush;
    Gdiplus::FontFamily *mp_font_family;
    Gdiplus::Font *mp_font;
};
