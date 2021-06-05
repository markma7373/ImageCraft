#pragma once

#include "Common.h"
#include "TextImageMaker.h"

class AnsiTemplate
{
public:
    AnsiTemplate();
    ~AnsiTemplate();

    void SetSize(int size);
    int GetSize()
    {
        return m_size;
    }

    void DrawHalfBlock(BYTE *p_image_data, int image_step, int channels,
                       const AnsiShare::AnsiColor &color, const AnsiShare::AnsiCell &cell, bool is_right);
    void DrawHalfBlock(HyImage *p_image, int x, int y,
                       const AnsiShare::AnsiColor &color, const AnsiShare::AnsiCell &cell, bool is_right);
    void DrawBlock(HyImage *p_image, int x, int y,
                   const AnsiShare::AnsiColor &color, const AnsiShare::AnsiCell &cell);

    void GetStringImage(HyImage *p_string_image, LPCTSTR text_string);

private:
    struct BlendSpec
    {
        int fg_size;
        int bg_size;
        int fg_weight;
        int bg_weight;
    };

    int m_size;
    int m_half_size;
    BlendSpec m_blend_specs[9];

    TextImageMaker m_text_image_maker;

    __forceinline void ExtractColor(int color, BYTE buffer[3])
    {
        buffer[0] = (BYTE)color;
        buffer[1] = (BYTE)(color >> 8);
        buffer[2] = (BYTE)(color >> 16);
    }
};