#include "stdafx.h"
#include "AnsiShare.h"
#include "TextImageMaker.h"

namespace AnsiShare
{
    const int g_normal_color_list[ANSI_COLOR_COUNT] =
    {
        ANSI_BLACK, ANSI_RED, ANSI_GREEN, ANSI_YELLOW, ANSI_BLUE, ANSI_PURPLE, ANSI_CYAN, ANSI_WHITE
    };

    const int g_bright_color_list[ANSI_COLOR_COUNT] =
    {
        ANSI_BLACK_BRIGHT, ANSI_RED_BRIGHT, ANSI_GREEN_BRIGHT, ANSI_YELLOW_BRIGHT, ANSI_BLUE_BRIGHT, ANSI_PURPLE_BRIGHT, ANSI_CYAN_BRIGHT, ANSI_WHITE_BRIGHT
    };

    const int g_all_color_list[ANSI_ALL_COLOR_COUNT] =
    {
        ANSI_BLACK, ANSI_RED, ANSI_GREEN, ANSI_YELLOW, ANSI_BLUE, ANSI_PURPLE, ANSI_CYAN, ANSI_WHITE,
        ANSI_BLACK_BRIGHT, ANSI_RED_BRIGHT, ANSI_GREEN_BRIGHT, ANSI_YELLOW_BRIGHT, ANSI_BLUE_BRIGHT, ANSI_PURPLE_BRIGHT, ANSI_CYAN_BRIGHT, ANSI_WHITE_BRIGHT
    };

    const _TCHAR *g_normal_color_string_list[ANSI_COLOR_COUNT] = 
    {
        _T("BLACK"),
        _T("RED"),
        _T("GREEN"),
        _T("YELLOW"),
        _T("BLUE"),
        _T("PURPLE"),
        _T("CYAN"),
        _T("WHITE"),
    };

    const _TCHAR *g_bright_color_string_list[ANSI_COLOR_COUNT] = 
    {
        _T("BLACK_BRIGHT"),
        _T("RED_BRIGHT"),
        _T("GREEN_BRIGHT"),
        _T("YELLOW_BRIGHT"),
        _T("BLUE_BRIGHT"),
        _T("PURPLE_BRIGHT"),
        _T("CYAN_BRIGHT"),
        _T("WHITE_BRIGHT"),
    };

    const _TCHAR *g_unknown_color_string = _T("UNKNOWN_COLOR");

    LPCTSTR ColorToString(int color)
    {
        for (int i = 0; i < ANSI_COLOR_COUNT; i++)
        {
            if (g_normal_color_list[i] == color)
                return g_normal_color_string_list[i];
            if (g_bright_color_list[i] == color)
                return g_bright_color_string_list[i];
        }

        return g_unknown_color_string;
    }

    int ToNormalColor(int color)
    {
        if (color == ANSI_BLACK || color == ANSI_BLACK_BRIGHT)
            return ANSI_BLACK;
        else if (color == ANSI_RED || color == ANSI_RED_BRIGHT)
            return ANSI_RED;
        else if (color == ANSI_GREEN || color == ANSI_GREEN_BRIGHT)
            return ANSI_GREEN;
        else if (color == ANSI_YELLOW || color == ANSI_YELLOW_BRIGHT)
            return ANSI_YELLOW;
        else if (color == ANSI_BLUE || color == ANSI_BLUE_BRIGHT)
            return ANSI_BLUE;
        else if (color == ANSI_PURPLE || color == ANSI_PURPLE_BRIGHT)
            return ANSI_PURPLE;
        else if (color == ANSI_CYAN || color == ANSI_CYAN_BRIGHT)
            return ANSI_CYAN;
        else if (color == ANSI_WHITE || color == ANSI_WHITE_BRIGHT)
            return ANSI_WHITE;
        else
            return ANSI_BLACK;
    }

    int ToBrightColor(int color)
    {
        if (color == ANSI_BLACK || color == ANSI_BLACK_BRIGHT)
            return ANSI_BLACK_BRIGHT;
        else if (color == ANSI_RED || color == ANSI_RED_BRIGHT)
            return ANSI_RED_BRIGHT;
        else if (color == ANSI_GREEN || color == ANSI_GREEN_BRIGHT)
            return ANSI_GREEN_BRIGHT;
        else if (color == ANSI_YELLOW || color == ANSI_YELLOW_BRIGHT)
            return ANSI_YELLOW_BRIGHT;
        else if (color == ANSI_BLUE || color == ANSI_BLUE_BRIGHT)
            return ANSI_BLUE_BRIGHT;
        else if (color == ANSI_PURPLE || color == ANSI_PURPLE_BRIGHT)
            return ANSI_PURPLE_BRIGHT;
        else if (color == ANSI_CYAN || color == ANSI_CYAN_BRIGHT)
            return ANSI_CYAN_BRIGHT;
        else if (color == ANSI_WHITE || color == ANSI_WHITE_BRIGHT)
            return ANSI_WHITE_BRIGHT;
        else
            return ANSI_WHITE_BRIGHT;
    }

    std_tstring AnsiColor::ToString() const
    {
        _TCHAR buffer[64];
        _stprintf(buffer, _T("%s / %s"), ColorToString(fg_color), ColorToString(bg_color));
        return std_tstring(buffer);
    }

    AnsiCell CharToAnsiCell(wchar_t wchar)
    {
        // Convert a wide char to AnsiCell. Don't consider if we have enough space.

        if (wchar >= 0x2581 && wchar <= 0x2588)
            return AnsiCell(HORI_BLOCK, (int)(wchar - 0x2580));
        else if (wchar >= 0x2589 && wchar <= 0x258f)
            return AnsiCell(VERT_BLOCK, (int)(0x2590 - wchar));
        else if (wchar == L'¢«')
            return AnsiCell(TRIANGLE, 0);
        else if (wchar == L'¢ª')
            return AnsiCell(TRIANGLE, 1);
        else if (wchar == L'¢©')
            return AnsiCell(TRIANGLE, 2);
        else if (wchar == L'¢¨')
            return AnsiCell(TRIANGLE, 3);
        else if (wchar == L'¡¶')
            return AnsiCell(REGULAR_TRIANGLE, 0);
        else if (wchar == L'¡¿')
            return AnsiCell(REGULAR_TRIANGLE, 1);
        else if (wchar == L' ')
            return AnsiCell(EMPTY);
        else
            return AnsiCell(GENERAL_CHAR, (int)wchar);
    }

    AnsiCell CharToAnsiCell(wchar_t wchar, int valid_range)
    {
        // Convert a wide char to AnsiCell.
        // If there is no enough range, truncate it to a valid content.

        AnsiCell cell = CharToAnsiCell(wchar);

        if (cell.IsDoubleChar() && valid_range == 1)
            cell = AnsiCell(EMPTY);

        return cell;
    }
}