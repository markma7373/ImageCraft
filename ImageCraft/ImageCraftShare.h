#pragma once

#include "use_hylib.h"
#include "use_ipp.h"
#include "Big5Codec.h"

#define MIN_IMAGE_SCALE_RATIO  0.25f
#define MAX_IMAGE_SCALE_RATIO  4.00f

enum IC_DrawMode
{
    IC_VIEW_ANSI = 0,
    IC_DRAW_SPACES,
    IC_CHANGE_COLOR,
    IC_REFINE_BOUNDARY,
    IC_DRAW_BLOCK,
    IC_DRAW_SMALL_SQUARE,
    IC_MERGE_BLOCK,
    IC_INSERT_DELETE_SPACE,
    IC_DRAW_LARGE_BRUSH,
    IC_ADD_TEXT,
};

inline bool IsColorDrawMode(int mode)
{
    return (mode == IC_DRAW_SPACES) ||
           (mode == IC_DRAW_BLOCK) ||
           (mode == IC_CHANGE_COLOR) ||
           (mode == IC_DRAW_SMALL_SQUARE) ||
           (mode == IC_INSERT_DELETE_SPACE) ||
           (mode == IC_DRAW_LARGE_BRUSH) ||
           (mode == IC_ADD_TEXT);
}

enum IC_SelectBlockMode
{
    IC_SELECT_BLOCK_GEOMETRIC = 0,
    IC_SELECT_BLOCK_SIGN_MARK,
    IC_SELECT_BLOCK_MATH,
    IC_SELECT_BLOCK_BRACELET,
    IC_SELECT_BLOCK_ARROW_PUNCTUATION_OTHERS,
    IC_SELECT_BLOCK_TABLE_LINE1,
    IC_SELECT_BLOCK_TABLE_LINE2,
    IC_SELECT_BLOCK_GREEK_UPPER,
    IC_SELECT_BLOCK_GREEK_LOWER,
    IC_SELECT_BLOCK_JAPANESE_HIRAGANA1,
    IC_SELECT_BLOCK_JAPANESE_HIRAGANA2,
    IC_SELECT_BLOCK_JAPANESE_HIRAGANA3,
    IC_SELECT_BLOCK_JAPANESE_KATAKANA1,
    IC_SELECT_BLOCK_JAPANESE_KATAKANA2,
    IC_SELECT_BLOCK_JAPANESE_KATAKANA3,
    IC_SELECT_BLOCK_DRAWING_TEXT,
    IC_SELECT_BLOCK_MODE_AMOUNT,
};

inline std_tstring FullSizeDigit(int value)
{
    int digit = FitInRange(value, 0, 9);
    if (digit == 0)
        return std_tstring(_T("¢¯"));
    else if (digit == 1)
        return std_tstring(_T("¢°"));
    else if (digit == 2)
        return std_tstring(_T("¢±"));
    else if (digit == 3)
        return std_tstring(_T("¢²"));
    else if (digit == 4)
        return std_tstring(_T("¢³"));
    else if (digit == 5)
        return std_tstring(_T("¢´"));
    else if (digit == 6)
        return std_tstring(_T("¢µ"));
    else if (digit == 7)
        return std_tstring(_T("¢¶"));
    else if (digit == 8)
        return std_tstring(_T("¢·"));
    else if (digit == 9)
        return std_tstring(_T("¢¸"));
    else
        return std_tstring(_T("N/A"));
}

inline std_tstring FullSizeValue(int value)
{
    std_tstring value_string = _T("");

    int positive_value = ch_Abs(value);
    if (value < 0)
        value_string += _T("¡Ð");

    std::vector<int> digits;

    while (positive_value > 0)
    {
        digits.push_back(positive_value % 10);
        positive_value /= 10;
    }

    if (digits.empty())
        digits.push_back(0);

    for (int i = (int)digits.size() - 1; i >= 0; i--)
        value_string += FullSizeDigit(digits[i]);

    return value_string;
}

inline bool IsDigit(char value)
{
    return (value >= '0' && value <= '9');
}

inline bool IsAlphabet(char value)
{
    return (value >= 'A' && value <= 'Z') || (value >= 'a' && value <= 'z');
}

inline bool IsHexValue(char value)
{
    return IsDigit(value) || (value >= 'A' && value <= 'F') || (value >= 'a' && value <= 'f');
}

int GetMinBlockSize();
int GetMaxBlockSize();
int GetDefaultBlockSize();
int GetValidBlockSize(int block_size);

inline float GetSafeDegree(float dx, float dy)
{
    // Use atan2f() to get the degree of vector (dx, dy).

    float radian = atan2f(dy, dx);
    if (ch_IsValidNumber(radian) == false)
        radian = 0.0f;

    return (float)TO_DEGREE(radian);
}

inline float ValidDegree(float degree)
{
    // convert a degree of any real number to [-180, +180].

    float abs_degree = ch_Abs(degree);
    float degree_shift = floorf((abs_degree + 180.0f) / 360.0f) * 360.0f;
    float valid_degree = abs_degree - degree_shift;

    if (degree < 0.0f)
        valid_degree = -valid_degree;

    return valid_degree;
}

inline HyRect UnionRect(const HyRect &rect1, const HyRect &rect2)
{
    bool is_rect1_valid = (rect1.width > 0 && rect1.height > 0);
    bool is_rect2_valid = (rect2.width > 0 && rect2.height > 0);

    if (is_rect1_valid && is_rect2_valid)
        return hyUnionRect(rect1, rect2);
    else if (is_rect1_valid)
        return rect1;
    else if (is_rect2_valid)
        return rect2;
    else
        return hyRect(0, 0, 0, 0);
}

std_tstring GetTempSaveAnsiPath();
std_tstring GetAutoSaveProjectPath();

std_tstring ToHexString(const std::string &ansi_string);
std_tstring ToHexString(const std::wstring &unicode_string);

extern Big5Codec g_big5_codec;

std::wstring MyAnsiToUnicode(const char *p_ansi_string);
std::string MyUnicodeToAnsi(const wchar_t *p_unicode_string);

int GetStringHalfWidthCount(LPCWSTR string);
std::vector<UnicodeTextInfo> AnalyzeUnicodeString(const wchar_t *p_unicode_string);
std::vector<UnicodeTextInfo> AnalyzeUnicodeString(const std::wstring &unicode_string);

std::wstring GetValidStringForANSIFile(const std::vector<UnicodeTextInfo> &string_info,
                                       std::vector<std::wstring> &invalid_text_list);

struct OldFormatData
{
    int scaled_image_width;
    int scaled_image_height;

    OldFormatData()
    {
        scaled_image_width = 0;
        scaled_image_height = 0;
    }
};

inline void AffineMultiply(const float src_factors_1[6], const float src_factors_2[6], float dst_factors[6])
{
    // Multiply affine factors as 3x3 matrix with last row = [0, 0, 1].

    // [0] [1] [2]  [0] [1] [2]  
    // [3] [4] [5]  [3] [4] [5]
    //   0   0   1    0   0   1

    dst_factors[0] = src_factors_1[0] * src_factors_2[0] + src_factors_1[1] * src_factors_2[3];
    dst_factors[1] = src_factors_1[0] * src_factors_2[1] + src_factors_1[1] * src_factors_2[4];
    dst_factors[2] = src_factors_1[0] * src_factors_2[2] + src_factors_1[1] * src_factors_2[5] + src_factors_1[2];
    dst_factors[3] = src_factors_1[3] * src_factors_2[0] + src_factors_1[4] * src_factors_2[3];
    dst_factors[4] = src_factors_1[3] * src_factors_2[1] + src_factors_1[4] * src_factors_2[4];
    dst_factors[5] = src_factors_1[3] * src_factors_2[2] + src_factors_1[4] * src_factors_2[5] + src_factors_1[5];
}

HyImage *CreateRotatedImage(const HyImage *p_src_image, float rotate_degree, HyPoint2D32f &rotate_shift);