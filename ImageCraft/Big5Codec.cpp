#include "stdafx.h"
#include "Big5Codec.h"
#include "ImageCraftShare.h"

static const int BIG5_TABLE_OFFSET = 65536 - BIG5_TABLE_SIZE;
static const UINT INVALID_UNICODE_VALUE = 0xffffffff;
static const UINT INVALID_BIG5_VALUE = 0xffff;

Big5Codec::Big5Codec()
{
    for (int i = 0; i < BIG5_TABLE_SIZE; i++)
        m_big5_table[i] = INVALID_UNICODE_VALUE;
    for (int i = 0; i < UNICODE_TABLE_SIZE; i++)
        m_unicode_table[i] = INVALID_BIG5_VALUE;
}

Big5Codec::~Big5Codec()
{

}

__forceinline bool Big5Codec::AnsiToUnicodeData(const char *p_ansi_buffer, int offset, int valid_range,
                                                wchar_t unicode_data[2], int &wchar_count, int &data_range)
{
    // Assume valid_range >= 1. The caller should check it.

    bool is_valid = false;

    BYTE first_byte = reinterpret_cast<const BYTE &>(p_ansi_buffer[offset]);
    if (first_byte <= 0x7f)
    {
        unicode_data[0] = (wchar_t)first_byte;
        wchar_count = 1;
        data_range = 1;
        is_valid = true;
    }
    else
    {
        if (valid_range == 1)
        {
            unicode_data[0] = L' ';
            wchar_count = 1;
            data_range = 1;
            is_valid = false;
        }
        else
        {
            BYTE second_byte = reinterpret_cast<const BYTE &>(p_ansi_buffer[offset + 1]);
            int ansi_value = (first_byte << 8) + second_byte;

            UINT unicode_value = INVALID_UNICODE_VALUE;
            if (ansi_value >= BIG5_TABLE_OFFSET && ansi_value <= 0xFFFF)
                unicode_value = m_big5_table[ansi_value - BIG5_TABLE_OFFSET];

            if (unicode_value == INVALID_UNICODE_VALUE)
            {
                unicode_data[0] = L' ';
                unicode_data[1] = L' ';
                wchar_count = 2;
                data_range = 2;
                is_valid = false;
            }
            else
            {
                unicode_data[0] = (wchar_t)unicode_value;

                wchar_count = 1;
                data_range = 2;
                is_valid = true;
            }
        }
    }

    return is_valid;
}

__forceinline bool Big5Codec::UnicodeToAnsiData(const wchar_t *p_unicode_buffer, int offset, int valid_range,
                                                char ansi_data[2], int &char_count, int &data_range)
{
    // Assume valid_range >= 1. The caller should check it.

    // Do not convert multi-char Unicode (U+10000 or after),
    // but need to detect it and replaced by spaces.

    wchar_t unicode_value = p_unicode_buffer[offset];

    bool is_valid = false;
    if (unicode_value <= 0x7f)
    {
        ansi_data[0] = (char)unicode_value;
        char_count = 1;
        data_range = 1;
        is_valid = true;
    }
    else
    {
        bool is_surrogate_pair = false;
        if (valid_range >= 2)
        {
            wchar_t next_value = p_unicode_buffer[offset + 1];
            is_surrogate_pair = (unicode_value >= 0xD800 && unicode_value <= 0xDBFF) && (next_value >= 0xDC00 && next_value <= 0xDFFF);
        }

        if (is_surrogate_pair)
        {
            ansi_data[0] = ' ';
            ansi_data[1] = ' ';
            char_count = 2;
            data_range = 2;
            is_valid = false;
        }
        else
        {
            USHORT ansi_value = m_unicode_table[unicode_value];

            if (ansi_value == INVALID_BIG5_VALUE)
            {
                ansi_data[0] = ' ';
                ansi_data[1] = ' ';
                char_count = 2;
                data_range = 1;
                is_valid = false;
            }
            else
            {
                BYTE *p_byte_data = (BYTE *)ansi_data;
                p_byte_data[0] = (ansi_value >> 8);
                p_byte_data[1] = (ansi_value & 0xff);
                char_count = 2;
                data_range = 1;
                is_valid = true;
            }
        }
    }

    return is_valid;
}

void Big5Codec::LoadTableFromFile(LPCTSTR path)
{
    FILE *p_file = _tfopen(path, _T("r"));
    if (p_file == NULL)
    {
        ch_dprintf(_T("ERROR: Cannot load codec table at %s !!"), path);
        return;
    }

    const int max_read_line_size = 256;
    ChAutoPtr<char> line_buffer(max_read_line_size);

    std::vector<UINT> token_values;
    while (fgets(line_buffer, max_read_line_size, p_file))
    {
        token_values.clear();

        std::string line_string(line_buffer);
        int line_length = line_string.length();

        int find_offset = 0;
        while (true)
        {
            std::size_t found_location = line_string.find("0x", find_offset);
            if (found_location == std::string::npos)
                break;

            int value_start = found_location;
            int value_end = value_start + 2;
            for (int i = value_start + 2; i < line_length; i++)
            {
                char value = line_string.at(i);
                if (IsHexValue(value) == false)
                    break;

                value_end++;
            }

            std::string hex_token = line_string.substr(value_start, value_end - value_start);
            token_values.push_back((UINT)strtoul(hex_token.c_str(), NULL, 0));

            find_offset = value_end;
            if (find_offset >= line_length)
                break;
        }
        
        if ((int)token_values.size() < 2)
            continue;

        // Do not handle multiple characters.
        if ((int)token_values.size() >= 3)
            continue;
        
        // Do not handle Unicode values > 0xFFFF.
        UINT unicode_value = token_values[1];
        if (unicode_value > 0xFFFF)
            continue;

        UINT big5_value = token_values[0];
        if (big5_value < (UINT)BIG5_TABLE_OFFSET || big5_value > 0xFFFF)
            continue;

        int table_offset = (int)big5_value - BIG5_TABLE_OFFSET;
        m_big5_table[table_offset] = unicode_value;
    }

    fclose(p_file);

    MakeUnicodeTable();
}

void Big5Codec::LoadTable(const UINT table[BIG5_TABLE_SIZE])
{
    memcpy(m_big5_table, table, sizeof(UINT) * BIG5_TABLE_SIZE);

    MakeUnicodeTable();
}

void Big5Codec::MakeUnicodeTable()
{
    for (int i = 0; i < UNICODE_TABLE_SIZE; i++)
        m_unicode_table[i] = INVALID_BIG5_VALUE;

    int count = 0;
    for (int i = 0; i < BIG5_TABLE_SIZE; i++)
    {
        UINT unicode_value = m_big5_table[i];
        if (unicode_value == INVALID_UNICODE_VALUE)
            continue;
        if (unicode_value > 0xFFFF)
            continue;

        USHORT big5_value = (USHORT)(i + BIG5_TABLE_OFFSET);
        m_unicode_table[unicode_value] = big5_value;

        count++;
    }
}

void Big5Codec::WriteTableArray(LPCTSTR path, LPCTSTR table_name)
{
    FILE *p_file = _tfopen(path, _T("w"));
    if (p_file == NULL)
    {
        ch_dprintf(_T("ERROR: Cannot open file at %s !!"), path);
        return;
    }

    std::string ansi_table_name("big5_table");
    if (table_name)
        ansi_table_name = unicodeToAnsi(table_name);

    fprintf(p_file, "UINT %s[BIG5_TABLE_SIZE] = \n", ansi_table_name.c_str());
    fprintf(p_file, "{\n");

    const int value_per_line = 16;
    for (int i = 0; i < BIG5_TABLE_SIZE; i += value_per_line)
    {
        int i_end = ch_Min(i + value_per_line, BIG5_TABLE_SIZE);

        fprintf(p_file, "    ");

        for (int j = i; j < i_end; j++)
            fprintf(p_file, "0x%08x, ", m_big5_table[j]);

        fprintf(p_file, "\n");
    }

    fprintf(p_file, "};\n");

    fclose(p_file);
}

std::wstring Big5Codec::AnsiToUnicode(const char *p_ansi_string)
{
    if (p_ansi_string == NULL)
        return std::wstring();

    int ansi_length = strlen(p_ansi_string);

    int max_wchar_length = ansi_length + 1;
    wchar_t *p_unicode_buffer = new wchar_t[max_wchar_length];
    int unicode_offset = 0;

    wchar_t unicode_data[2] = {0};
    int wchar_count = 0;
    int data_range = 0;

    int ansi_offset = 0;
    while (ansi_offset < ansi_length)
    {
        AnsiToUnicodeData(p_ansi_string, ansi_offset,
                          ansi_length - ansi_offset,
                          unicode_data, wchar_count, data_range);

        for (int c = 0; c < wchar_count; c++)
            p_unicode_buffer[unicode_offset + c] = unicode_data[c];

        ansi_offset += data_range;
        unicode_offset += wchar_count;
    }

    p_unicode_buffer[unicode_offset] = L'\0';

    std::wstring unicode_string = p_unicode_buffer;
    delete [] p_unicode_buffer;

    return unicode_string;
}

std::string Big5Codec::UnicodeToAnsi(const wchar_t *p_unicode_string)
{
    if (p_unicode_string == NULL)
        return std::string();

    int unicode_length = wcslen(p_unicode_string);

    int max_char_length = unicode_length * 2 + 1;
    char *p_ansi_buffer = new char[max_char_length];
    int ansi_offset = 0;

    char ansi_data[2] = {0};
    int char_count = 0;
    int data_range = 0;

    int unicode_offset = 0;
    while (unicode_offset < unicode_length)
    {
        UnicodeToAnsiData(p_unicode_string, unicode_offset,
                          unicode_length - unicode_offset,
                          ansi_data, char_count, data_range);

        for (int c = 0; c < char_count; c++)
            p_ansi_buffer[ansi_offset + c] = ansi_data[c];

        unicode_offset += data_range;
        ansi_offset += char_count;
    }

    p_ansi_buffer[ansi_offset] = '\0';

    std::string ansi_string = p_ansi_buffer;
    delete [] p_ansi_buffer;

    return ansi_string;
}

std::vector<UnicodeTextInfo> Big5Codec::AnalyzeUnicodeString(const wchar_t *p_unicode_string)
{
    std::vector<UnicodeTextInfo> string_info;

    if (p_unicode_string == NULL)
        return string_info;

    int unicode_length = wcslen(p_unicode_string);

    char ansi_data[2] = {0};
    int char_count = 0;
    int data_range = 0;
    bool is_valid = false;

    int unicode_offset = 0;
    while (unicode_offset < unicode_length)
    {
        is_valid = UnicodeToAnsiData(p_unicode_string, unicode_offset,
                                     unicode_length - unicode_offset,
                                     ansi_data, char_count, data_range);
        
        UnicodeTextInfo text_info;
        text_info.unicode_text.assign(p_unicode_string + unicode_offset, data_range);
        text_info.big5_text.assign(ansi_data, char_count);
        text_info.is_big5_valid = is_valid;

        string_info.push_back(text_info);

        unicode_offset += data_range;
    }

    return string_info;
}