#include "stdafx.h"
#include "ImageCraftShare.h"
#include "ImageCraft.h"
#include "AnsiCanvas.h"
#include "use_ipp.h"

#define APP_MIN_BLOCK_SIZE      16
#define APP_DEFAULT_BLOCK_SIZE  24
#define APP_MAX_BLOCK_SIZE      32

int GetMinBlockSize()
{
    return FitInRange(APP_MIN_BLOCK_SIZE, AnsiCanvas::m_min_block_size, AnsiCanvas::m_max_block_size);
}

int GetMaxBlockSize()
{
    return FitInRange(APP_MAX_BLOCK_SIZE, AnsiCanvas::m_min_block_size, AnsiCanvas::m_max_block_size);
}

int GetDefaultBlockSize()
{
    return APP_DEFAULT_BLOCK_SIZE;
}

int GetValidBlockSize(int block_size)
{
    int valid_block_size = ALIGN(block_size, 2);
    valid_block_size = FitInRange(valid_block_size, APP_MIN_BLOCK_SIZE, APP_MAX_BLOCK_SIZE);

    return valid_block_size;
}

std_tstring GetTempSaveAnsiPath()
{
    std_tstring path = theApp.m_document_folder;
    path += _T("/tempsave.ans");

    return path;
}

std_tstring GetAutoSaveProjectPath()
{
    std_tstring path = theApp.m_document_folder;
    path += _T("/autosave.aproj");

    return path;
}

std_tstring ToHexString(const std::string &ansi_string)
{
    std_tstring hex_string;

    _TCHAR buffer[16];
    for (int i = 0; i < (int)ansi_string.size(); i++)
    {
        _stprintf(buffer, _T("0x%02x "), reinterpret_cast<const BYTE &>(ansi_string.at(i)));
        hex_string += buffer;
    }

    return hex_string;
}

std_tstring ToHexString(const std::wstring &unicode_string)
{
    std_tstring hex_string;

    _TCHAR buffer[16];
    for (int i = 0; i < (int)unicode_string.size(); i++)
    {
        _stprintf(buffer, _T("0x%04x "), unicode_string.at(i));
        hex_string += buffer;
    }

    return hex_string;
}

Big5Codec g_big5_codec;

static bool InitDefaultBig5Codec()
{
    g_big5_codec.LoadTable(g_big5_UAO_250_table);

    return true;
}

static bool g_dummy = InitDefaultBig5Codec();

std::wstring MyAnsiToUnicode(const char *p_ansi_string)
{
    return g_big5_codec.AnsiToUnicode(p_ansi_string);
}

std::string MyUnicodeToAnsi(const wchar_t *p_unicode_string)
{
    return g_big5_codec.UnicodeToAnsi(p_unicode_string);
}

int GetStringHalfWidthCount(LPCWSTR string)
{
    // U+10000 or later character uses 2 wide chars.
    // We have to convert the string to get the actual size.
    // (Under current setup, the size of ANSI data equals to the text range in half-width.)
    if (string == NULL)
        return 0;

    std::string ansi_string = MyUnicodeToAnsi(string);
    return (int)ansi_string.size();
}

std::vector<UnicodeTextInfo> AnalyzeUnicodeString(const wchar_t *p_unicode_string)
{
    return g_big5_codec.AnalyzeUnicodeString(p_unicode_string);
}

std::vector<UnicodeTextInfo> AnalyzeUnicodeString(const std::wstring &unicode_string)
{
    return ::AnalyzeUnicodeString(unicode_string.c_str());
}

std::wstring GetValidStringForANSIFile(const std::vector<UnicodeTextInfo> &string_info,
                                       std::vector<std::wstring> &invalid_text_list)
{
    std::wstring valid_string;

    invalid_text_list.clear();

    for (int i = 0; i < (int)string_info.size(); i++)
    {
        const UnicodeTextInfo &text_info = string_info[i];

        if (text_info.is_big5_valid)
        {
            valid_string += text_info.unicode_text;
        }
        else
        {
            // Currently, all invalid unicode value becomes two spaces.
            valid_string += L"  ";

            bool is_recorded = false;
            for (int j = 0; j < (int)invalid_text_list.size(); j++)
            {
                if (text_info.unicode_text.compare(invalid_text_list[j]) == 0)
                {
                    is_recorded = true;
                    break;
                }
            }

            if (is_recorded == false)
                invalid_text_list.push_back(text_info.unicode_text);
        }
    }

    return valid_string;
}

HyImage *CreateRotatedImage(const HyImage *p_src_image, float rotate_degree, HyPoint2D32f &rotate_shift)
{
    // Create a rotated image with proper size for all rotated coreners.
    // rotate_shift is a translation to map the transformed coordinates:
    // A point (x, y) will map to (x' + rotate_shift.x, y' + rotate_shift.y) in the rotated image,
    // where (x', y') is the rotated coordinate by using (0, 0) as rotate center.

    if (p_src_image == NULL)
        return NULL;

    const int channels = p_src_image->nChannels;

    // The following algorithm has problem if source width/height < 2.
    // Make a zero-padded image if needed.
    int src_width = p_src_image->width;
    int src_height = p_src_image->height;

    const HyImage *p_safe_src_image = p_src_image;
    HyImagePtr safe_src_image;
    if (src_width < 2 || src_height < 2)
    {
        int safe_width = ch_Max(src_width, 2);
        int safe_height = ch_Max(src_height, 2);
        safe_src_image.Alloc(hySize(safe_width, safe_height), HY_DEPTH_8U, channels);
        HY_ZEROIMAGE(safe_src_image.Content());

        for (int y = 0; y < src_height; y++)
        {
            memcpy(safe_src_image->imageData + y * safe_src_image->widthStep,
                   p_src_image->imageData + y * p_src_image->widthStep, src_width * channels);
        }

        p_safe_src_image = safe_src_image.Content();
        src_width = safe_width;
        src_height = safe_height;
    }

    float radian = (float)TO_RADIAN(rotate_degree);
    const float cos_theta = cosf(radian);
    const float sin_theta = sinf(radian);

    // Use half value coordinates to locate the center of corner pixels.
    HyPoint2D32f corners[4] = 
    {
        HyPoint2D32f(0.5f, 0.5f),
        HyPoint2D32f(src_width - 0.5f, 0.5f),
        HyPoint2D32f(0.5f, src_height - 0.5f),
        HyPoint2D32f(src_width - 0.5f, src_height - 0.5f),
    };

    float rotated_min_x = FLT_MAX;
    float rotated_min_y = FLT_MAX;
    float rotated_max_x = -FLT_MAX;
    float rotated_max_y = -FLT_MAX;

    HyPoint2D32f rotated_corners[4];
    for (int i = 0; i < 4; i++)
    {
        rotated_corners[i].x = cos_theta * corners[i].x - sin_theta * corners[i].y;
        rotated_corners[i].y = sin_theta * corners[i].x + cos_theta * corners[i].y;

        rotated_min_x = ch_Min(rotated_min_x, rotated_corners[i].x);
        rotated_min_y = ch_Min(rotated_min_y, rotated_corners[i].y);
        rotated_max_x = ch_Max(rotated_max_x, rotated_corners[i].x);
        rotated_max_y = ch_Max(rotated_max_y, rotated_corners[i].y);
    }

    float transformed_min_x = 0.5f;
    float transformed_min_y = 0.5f;
    rotate_shift.x = transformed_min_x - rotated_min_x;
    rotate_shift.y = transformed_min_y - rotated_min_y;

    float transformed_max_x = rotated_max_x + rotate_shift.x;
    float transformed_max_y = rotated_max_y + rotate_shift.y;

    const float back_translate_factors[6] = 
    {
        1.0f, 0.0f, -rotate_shift.x,
        0.0f, 1.0f, -rotate_shift.y,
    };
    const float back_rotate_factors[6] = 
    {
         cos_theta, sin_theta, 0.0f,
        -sin_theta, cos_theta, 0.0f,
    };
    float back_factors[6] = {0.0f};
    AffineMultiply(back_rotate_factors, back_translate_factors, back_factors);

    const int src_stride = p_safe_src_image->widthStep;

    int dst_width = ch_Round(transformed_max_x + 1.0f);
    int dst_height = ch_Round(transformed_max_y + 1.0f);
    HyImage *p_rotated_image = hyCreateImage(hySize(dst_width, dst_height), HY_DEPTH_8U, channels);
    HY_ZEROIMAGE(p_rotated_image);

    ChAutoPtr<float> src_x_factor_table(dst_width);
    ChAutoPtr<float> src_y_factor_table(dst_width);
    for (int x = 0; x < dst_width; x++)
    {
        const float dst_x = x + 0.5f;
        src_x_factor_table[x] = back_factors[0] * dst_x;
        src_y_factor_table[x] = back_factors[3] * dst_x;
    }

    const float src_x_max = (src_width - 0.5f);
    const float src_y_max = (src_height - 0.5f);
    const int floor_x_max = src_width - 2;
    const int floor_y_max = src_height - 2;

    for (int y = 0; y < dst_height; y++)
    {
        BYTE *p_dst_scan = p_rotated_image->imageData + y * p_rotated_image->widthStep;

        const float dst_y = y + 0.5f;
        const float src_x_coefficient = back_factors[1] * dst_y + back_factors[2] - 0.5f;
        const float src_y_coefficient = back_factors[4] * dst_y + back_factors[5] - 0.5f;

        for (int x = 0; x < dst_width; x++)
        {
            float src_x = src_x_factor_table[x] + src_x_coefficient;
            float src_y = src_y_factor_table[x] + src_y_coefficient;

            if (src_x < -0.5f || src_x > src_x_max ||
                src_y < -0.5f || src_y > src_y_max)
                continue;

            int floor_x = FitInRange((int)src_x, 0, floor_x_max);
            int floor_y = FitInRange((int)src_y, 0, floor_y_max);
            int weight_x = FitInRange(ch_Round((src_x - floor_x) * 256.0f), 0, 256);
            int weight_y = FitInRange(ch_Round((src_y - floor_y) * 256.0f), 0, 256);

            int weight1 = (256 - weight_x) * (256 - weight_y);
            int weight2 = weight_x * (256 - weight_y);
            int weight3 = (256 - weight_x) * weight_y;
            int weight4 = weight_x * weight_y;

            const BYTE *p_src_start_pixel = p_safe_src_image->get_pixels<BYTE>(floor_x, floor_y);
            BYTE *p_dst_pixel = p_dst_scan + x * channels;
            for (int c = 0; c < channels; c++)
            {
                int value = ((weight1 * p_src_start_pixel[c] +
                              weight2 * p_src_start_pixel[channels + c] +
                              weight3 * p_src_start_pixel[src_stride + c] +
                              weight4 * p_src_start_pixel[src_stride + channels + c]) >> 16);

                p_dst_pixel[c] = (BYTE)value;
            }
        }
    }

    return p_rotated_image;
}