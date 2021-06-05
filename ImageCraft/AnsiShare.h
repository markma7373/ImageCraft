#pragma once

#include "use_hylib.h"
#include "ImageCraftShare.h"

#define ANSI_COLOR_COUNT    8
#define ANSI_ALL_COLOR_COUNT    (ANSI_COLOR_COUNT * 2)

#define ANSI_BLACK          HY_RGB(0x00, 0x00, 0x00)
#define ANSI_RED            HY_RGB(0x80, 0x00, 0x00)
#define ANSI_GREEN          HY_RGB(0x00, 0x80, 0x00)
#define ANSI_BLUE           HY_RGB(0x00, 0x00, 0x80)
#define ANSI_YELLOW         HY_RGB(0x80, 0x80, 0x00)
#define ANSI_CYAN           HY_RGB(0x00, 0x80, 0x80)
#define ANSI_PURPLE         HY_RGB(0x80, 0x00, 0x80)
#define ANSI_WHITE          HY_RGB(0xc0, 0xc0, 0xc0)

#define ANSI_BLACK_BRIGHT   HY_RGB(0x80, 0x80, 0x80)
#define ANSI_RED_BRIGHT     HY_RGB(0xff, 0x00, 0x00)
#define ANSI_GREEN_BRIGHT   HY_RGB(0x00, 0xff, 0x00)
#define ANSI_BLUE_BRIGHT    HY_RGB(0x00, 0x00, 0xff)
#define ANSI_YELLOW_BRIGHT  HY_RGB(0xff, 0xff, 0x00)
#define ANSI_CYAN_BRIGHT    HY_RGB(0x00, 0xff, 0xff)
#define ANSI_PURPLE_BRIGHT  HY_RGB(0xff, 0x00, 0xff)
#define ANSI_WHITE_BRIGHT   HY_RGB(0xff, 0xff, 0xff)

#define ANSI_DEFAULT_FOREGROUND     ANSI_WHITE
#define ANSI_DEFAULT_BACKGROUND     ANSI_BLACK

enum ANSI_BrushShape
{
    ANSI_BRUSH_SQUARE = 0,
    ANSI_BRUSH_CIRCLE,
    ANSI_BRUSH_DIAGONAL,
    ANSI_BRUSH_HORI_LINE,
    ANSI_BRUSH_VERT_LINE,
    ANSI_BRUSH_SHAPE_AMOUNT,
};

#define MIN_LARGE_BRUSH_SIZE    2
#define MAX_LARGE_BRUSH_SIZE    46

namespace AnsiShare
{
    extern const int g_normal_color_list[ANSI_COLOR_COUNT];
    extern const int g_bright_color_list[ANSI_COLOR_COUNT];
    extern const int g_all_color_list[ANSI_ALL_COLOR_COUNT];
    extern const _TCHAR *g_normal_color_string_list[ANSI_COLOR_COUNT];
    extern const _TCHAR *g_bright_color_string_list[ANSI_COLOR_COUNT];

    LPCTSTR ColorToString(int color);

    int ToNormalColor(int color);
    int ToBrightColor(int color);

    __forceinline bool IsBrightColor(int color)
    {
        return (color == ANSI_BLACK_BRIGHT ||
                color == ANSI_RED_BRIGHT ||
                color == ANSI_GREEN_BRIGHT ||
                color == ANSI_BLUE_BRIGHT ||
                color == ANSI_YELLOW_BRIGHT ||
                color == ANSI_CYAN_BRIGHT ||
                color == ANSI_PURPLE_BRIGHT ||
                color == ANSI_WHITE_BRIGHT);
    }

    __forceinline bool IsNormalColor(int color)
    {
        return (color == ANSI_BLACK ||
                color == ANSI_RED ||
                color == ANSI_GREEN ||
                color == ANSI_BLUE ||
                color == ANSI_YELLOW ||
                color == ANSI_CYAN ||
                color == ANSI_PURPLE ||
                color == ANSI_WHITE);
    }

    _forceinline bool IsAllBrightColor(int color1, int color2)
    {
        return IsBrightColor(color1) && IsBrightColor(color2);
    }

    __forceinline char ColorChar(int color)
    {
        if (color == ANSI_BLACK || color == ANSI_BLACK_BRIGHT)
            return '0';
        else if (color == ANSI_RED || color == ANSI_RED_BRIGHT)
            return '1';
        else if (color == ANSI_GREEN || color == ANSI_GREEN_BRIGHT)
            return '2';
        else if (color == ANSI_YELLOW || color == ANSI_YELLOW_BRIGHT)
            return '3';
        else if (color == ANSI_BLUE || color == ANSI_BLUE_BRIGHT)
            return '4';
        else if (color == ANSI_PURPLE || color == ANSI_PURPLE_BRIGHT)
            return '5';
        else if (color == ANSI_CYAN || color == ANSI_CYAN_BRIGHT)
            return '6';
        else if (color == ANSI_WHITE || color == ANSI_WHITE_BRIGHT)
            return '7';
        else
            return '0';
    }

    __forceinline bool IsFullWidth(wchar_t character)
    {
        // For current support, all characters greater than 0x7F are full width.

        return (character > 0x7f);
    }

    enum CellType
    {
        EMPTY = 0,
        HORI_BLOCK, // label = 0, 1, 2, 3, 4, 5, 6, 7, 8 (full)
        VERT_BLOCK, // label = 0, 1, 2, 3, 4, 5, 6, 7, 8 (full)
        TRIANGLE,   // label = 0, 1, 2, 3 with the orthogonal corner at
                    //         top-left, top-right, bottom-left, bottom-right
        REGULAR_TRIANGLE, // label = 0 for upward, 1 for downward
        GENERAL_CHAR,
    };

    struct AnsiCell
    {
        static const int DEFAULT_LABEL = -1;

        CellType type;
        int label;

        AnsiCell()
        {
            Set(EMPTY, 0);
        }

        AnsiCell(CellType type, int label = DEFAULT_LABEL)
        {
            Set(type, label);
        }

        bool operator==(const AnsiCell &cell) const
        {
            return (type == cell.type && label == cell.label);
        }

        bool operator!=(const AnsiCell &cell) const
        {
            return (((*this) == cell) == false);
        }

        void Set(CellType type, int label)
        {
            this->type = type;

            if (label == DEFAULT_LABEL)
            {
                if (type == HORI_BLOCK || type == VERT_BLOCK)
                    label = 8;
                else
                    label = 0;
            }
            else
            {
                this->label = label;
            }
        }

        bool IsDoubleChar() const
        {
            if (type == EMPTY)
                return false;
            else if (type == GENERAL_CHAR)
                return IsFullWidth((wchar_t)label);
            else
                return true;
        }

        int GetCharCount() const
        {
            return (IsDoubleChar() ? 2 : 1);
        }

        void GetCharacter(BYTE character[2]) const
        {
            // For horizontal / vertical blocks with label = 0,
            // treat it as two space characters.

            if (type == HORI_BLOCK)
            {
                if (label == 0)
                {
                    character[0] = character[1] = ' ';
                }
                else
                {
                    character[0] = 0xa2;
                    character[1] = (BYTE)(0x61 + label);
                }
            }
            else if (type == VERT_BLOCK)
            {
                if (label == 0)
                {
                    character[0] = character[1] = ' ';
                }
                else
                {
                    character[0] = 0xa2;

                    if (label < 8)
                        character[1] = (BYTE)(0x69 + label);
                    else
                        character[1] = 0x69;
                }
            }
            else if (type == TRIANGLE)
            {
                character[0] = 0xa2;
                character[1] = (BYTE)(0xab - label);
            }
            else if (type == REGULAR_TRIANGLE)
            {
                character[0] = 0xa1;
                character[1] = (label == 0 ? 0xb6 : 0xbf);
            }
            else if (type == GENERAL_CHAR)
            {
                wchar_t unicode_string[2] = {(wchar_t)label, (wchar_t)0};
                std::string ansi_string = MyUnicodeToAnsi(unicode_string);

                if (ansi_string.length() == 0)
                {
                    character[0] = ' ';
                    character[1] = ' ';
                }
                else
                {
                    int char_count = ch_Min((int)ansi_string.length(), 2);
                    for (int c = 0; c < char_count; c++)
                        character[c] = ansi_string.at(c);
                }
            }
            else
            {
                character[0] = ' ';
            }
        }
    };

    struct AnsiColor
    {
        int fg_color;
        int bg_color;

        AnsiColor(int fg = ANSI_DEFAULT_FOREGROUND, int bg = ANSI_DEFAULT_BACKGROUND)
        {
            fg_color = fg;
            bg_color = bg;
        }

        bool operator==(const AnsiColor &color) const
        {
            return (fg_color == color.fg_color && bg_color == color.bg_color);
        }

        AnsiColor FlipColor() const
        {
            // Return a color with exchanged foreground / background.
            // This function DOES NOT check if the color is bright.
            // The caller should handle it, or in some algorithms, bright background color is allowed.
            return AnsiColor(bg_color, fg_color);
        }

        std_tstring ToString() const;
    };

    __forceinline AnsiColor SpaceColor(int color)
    {
        return AnsiColor(ANSI_DEFAULT_FOREGROUND, ToNormalColor(color));
    }

    template<typename T>
    bool AllEqual(const T &item1, const T &item2, const T &item3)
    {
        return (item1 == item2) && (item1 == item3);
    }

    AnsiCell CharToAnsiCell(wchar_t wchar);
    AnsiCell CharToAnsiCell(wchar_t wchar, int valid_range);
}

