#pragma once

#include "Common.h"
#include "use_half.h"
#ifndef UNIX_OS
#include "tchar.h"

#if defined(_UNICODE) || defined(UNICODE)
typedef std::wstring std_tstring;
#else
typedef std::string std_tstring;
#endif
#endif

// This class is used to convert any binary file to a BYTE array.
class BinaryFileConverter
{
public:
    BinaryFileConverter();
    ~BinaryFileConverter();

    bool Open(LPCTSTR file_path);
    void Close();

    bool DumpArray(LPCTSTR text_file_path, LPCTSTR array_name = NULL);

private:
    BYTE *mp_buffer;
    int m_buffer_size;
};

class BinaryFileReader
{
public:
    BinaryFileReader(void);
    ~BinaryFileReader(void);

    int OpenFile(LPCTSTR file_path);
    void CloseFile();

    int ReadInt();
    bool ReadUInt(unsigned int &uint_value);
    bool ReadBYTE(BYTE &byte_value);
    bool ReadUShort(unsigned short &ushort_value);
    short ReadShort();
    float ReadFloat();
    half ReadHalf();
    float12 ReadFloat12();
    void ClearFloat12Buffer();

    BYTE* ReadBytes(BYTE *p_byte_buffer, const int buffer_size);

private:
    FILE *mp_file;

    int m_float12_buffer;
    bool m_is_buffer_empty;
};

class BinaryArrayReader
{
public:
    BinaryArrayReader(void);
    ~BinaryArrayReader(void);

    int OpenArray(const BYTE *p_array);
    void CloseArray();

    int ReadInt();
    float ReadFloat();
    half ReadHalf();

private:
    const BYTE *mp_array;
    int m_position;
};

class BinaryFileWriter
{
public:
    BinaryFileWriter(void);
    ~BinaryFileWriter(void);

    int OpenFile(LPCTSTR file_path);
    void CloseFile();

    int WriteInt(const int int_value);
    int WriteFloat(const float float_value);
    int WriteHalf(const half half_value);
    int WriteFloat12(const float12 float12_value);
    void FlushFloat12();
    int WriteShort(const short short_value);

    int WriteBytes(const BYTE *p_byte_buffer, const int buffer_size);

private:
    FILE *mp_file;

    int m_float12_buffer;
    bool m_is_buffer_empty;
};
