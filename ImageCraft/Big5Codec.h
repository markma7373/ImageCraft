#pragma once

#include "Common.h"

#define BIG5_TABLE_SIZE     32768   // 0x8000 to 0xFFFF
#define UNICODE_TABLE_SIZE  65536   // 0x0000 to 0xFFFF

struct UnicodeTextInfo
{
    std::wstring unicode_text;
    std::string big5_text;
    bool is_big5_valid;

    UnicodeTextInfo()
    {
        unicode_text = L" ";
        big5_text = " ";
        is_big5_valid = true;
    }
};

class Big5Codec
{
public:
    Big5Codec();
    ~Big5Codec();

    void LoadTableFromFile(LPCTSTR path);
    void LoadTable(const UINT table[BIG5_TABLE_SIZE]);

    void WriteTableArray(LPCTSTR path, LPCTSTR table_name);

    std::wstring AnsiToUnicode(const char *p_ansi_string);
    std::string UnicodeToAnsi(const wchar_t *p_unicode_string);

    std::vector<UnicodeTextInfo> AnalyzeUnicodeString(const wchar_t *p_unicode_string);

private:
    UINT m_big5_table[BIG5_TABLE_SIZE];
    USHORT m_unicode_table[UNICODE_TABLE_SIZE];

    __forceinline bool AnsiToUnicodeData(const char *p_ansi_buffer, int offset, int valid_range,
                                         wchar_t unicode_data[2], int &wchar_count, int &data_range);
    __forceinline bool UnicodeToAnsiData(const wchar_t *p_unicode_buffer, int offset, int valid_range,
                                         char ansi_data[2], int &char_count, int &data_range);

    void MakeUnicodeTable();
};

extern UINT g_big5_UAO_250_table[BIG5_TABLE_SIZE];