#include "stdafx.h"
#include "BinaryFileShare.h"


////////////////////////////////////////////////////////////
//  BinaryFileConverter
BinaryFileConverter::BinaryFileConverter()
{
    mp_buffer = NULL;
    m_buffer_size = 0;
}

BinaryFileConverter::~BinaryFileConverter()
{
    Close();
}

bool BinaryFileConverter::Open(LPCTSTR file_path)
{
    if (file_path == NULL)
        return false;

#ifndef _WINRT_METRO
    if (PathFileExists(file_path) == false)
        return false;
#endif

    FILE *p_file = _tfopen(file_path, _T("rb"));
    if (p_file == NULL)
        return false;

    // seek to end to get the file size
    fseek(p_file, 0, SEEK_END);
    m_buffer_size = (int)ftell(p_file);
    fseek(p_file, 0, SEEK_SET); // seek back to start

    _NEW_PTRS(mp_buffer, BYTE, m_buffer_size);

    int read_count = (int)fread(mp_buffer, sizeof(BYTE), (size_t)m_buffer_size, p_file);
    _MYASSERT(read_count == m_buffer_size);

    fclose(p_file);

    return true;
}

void BinaryFileConverter::Close()
{
    _DELETE_PTRS(mp_buffer);
    m_buffer_size = 0;
}

bool BinaryFileConverter::DumpArray(LPCTSTR text_file_path, LPCTSTR array_name/* = NULL*/)
{
    // Dump the loaded data as a BYTE array in C-code format.

    if (text_file_path == NULL)
        return false;

    if (mp_buffer == NULL || m_buffer_size <= 0)
        return false; // data not loaded

    FILE *p_file = _tfopen(text_file_path, _T("w"));
    if (p_file == NULL)
        return false;

    std_tstring name = _T("array_name");
    if (array_name)
        name = array_name;

    const int value_per_line = 256;

#if defined(UNICODE) && !defined(UNIX_OS)
    std::string name_string = unicodeToAnsi(name.c_str());
#else
    std::string name_string = name;
#endif

    fprintf(p_file, "static const BYTE %s[%d] =\n", name_string.c_str(), m_buffer_size);
    fprintf(p_file, "{\n");

    const int line_count = (m_buffer_size + value_per_line - 1) / value_per_line;

    for (int i = 0; i < line_count; i++)
    {
        const int index_start = i * value_per_line;

        fprintf(p_file, "    ");

        int value_count = ch_Min(m_buffer_size - index_start, value_per_line);

        for (int j = 0; j < value_count; j++)
        {
            int index = index_start + j;
            fprintf(p_file, "%3d", mp_buffer[index]);
            if (index < m_buffer_size - 1)
                fprintf(p_file, ", ");
        }

        fprintf(p_file, "\n");
    }

    fprintf(p_file, "};\n");

    fclose(p_file);

    return true;
}

////////////////////////////////////////////////////////////
//  BinaryFileReader

BinaryFileReader::BinaryFileReader(void)
: m_is_buffer_empty(true)
{
    mp_file = NULL;
    m_float12_buffer = 0;
}

BinaryFileReader::~BinaryFileReader(void)
{
    CloseFile();
}

int BinaryFileReader::OpenFile(LPCTSTR file_path)
{
    mp_file = _tfopen(file_path, _T("rb"));

    return (mp_file != NULL);
}

void BinaryFileReader::CloseFile()
{
    if (mp_file == NULL)
        return;

    fclose(mp_file);
    mp_file = NULL;
}

int BinaryFileReader::ReadInt()
{
    if (mp_file == NULL)
        return 0;

    int int_value = 0;
    fread(&int_value, sizeof(int), 1, mp_file);

    return int_value;
}

bool BinaryFileReader::ReadBYTE(BYTE &byte_value)
{
    if (mp_file == NULL)
        return 0;

    int count = fread(&byte_value, sizeof(BYTE), 1, mp_file);

    return count == 1;
}

bool BinaryFileReader::ReadUShort(unsigned short &ushort_value)
{
    if (mp_file == NULL)
        return 0;

    int count = fread(&ushort_value, sizeof(unsigned short), 1, mp_file);

    return count == 1;
}

bool BinaryFileReader::ReadUInt(unsigned int &uint_value)
{
    if (mp_file == NULL)
        return 0;

    int count = fread(&uint_value, sizeof(unsigned int), 1, mp_file);

    return count == 1;
}

short BinaryFileReader::ReadShort()
{
    if (mp_file == NULL)
        return 0;

    short short_value = 0;
    fread(&short_value, sizeof(short), 1, mp_file);

    return short_value;
}

float BinaryFileReader::ReadFloat()
{
    if (mp_file == NULL)
        return 0.0f;

    float float_value = 0.0f;
    fread(&float_value, sizeof(float), 1, mp_file);

    return float_value;
}

half BinaryFileReader::ReadHalf()
{
    if (mp_file == NULL)
        return 0.0f;

    uint16_t read_value;
    fread(&read_value, sizeof(uint16_t), 1, mp_file);

    half half_value;
    half_value.set_value(read_value);

    return half_value;
}

BYTE* BinaryFileReader::ReadBytes(BYTE *p_byte_buffer, const int buffer_size)
{
    if (mp_file == NULL)
        return NULL;

    if (p_byte_buffer == NULL)
        return NULL;

    fread(p_byte_buffer, sizeof(BYTE), buffer_size, mp_file);

    return p_byte_buffer;
}

float12 BinaryFileReader::ReadFloat12()
{
    if (mp_file == NULL)
        return 0.0f;

    uint16_t read_12bit_value = 0;
    if (m_is_buffer_empty)
    {
        BYTE read_value[3];
        fread(read_value, 3, 1, mp_file);
        m_float12_buffer = 0;
        m_float12_buffer = m_float12_buffer | read_value[0];
        m_float12_buffer = m_float12_buffer | (read_value[1] << 8);
        m_float12_buffer = m_float12_buffer | (read_value[2] << 16);

        read_12bit_value = (uint16_t)(read_12bit_value | (m_float12_buffer >> 12));
        m_is_buffer_empty = false;
    }
    else
    {
        read_12bit_value = (uint16_t)(read_12bit_value | (m_float12_buffer & 0xFFFFFF));
        m_is_buffer_empty = true;
    }

    float12 float12_value;
    float12_value.set_value(read_12bit_value);

    return float12_value;
}

void BinaryFileReader::ClearFloat12Buffer()
{
    m_is_buffer_empty = true;
}

////////////////////////////////////////////////////////////
//  BinaryArrayReader

BinaryArrayReader::BinaryArrayReader(void)
{
    mp_array = NULL;
    m_position = -1; // < 0 means invalid
}

BinaryArrayReader::~BinaryArrayReader(void)
{
    CloseArray();
}

int BinaryArrayReader::OpenArray(const BYTE *p_array)
{
    if (p_array == NULL)
        return 0;

    mp_array = p_array;
    m_position = 0;

    return 1;
}

void BinaryArrayReader::CloseArray()
{
    mp_array = NULL;
    m_position = -1;
}

int BinaryArrayReader::ReadInt()
{
    if (mp_array == NULL || m_position < 0)
        return 0;

    int read_size = sizeof(int);

    int int_value = 0;
    memcpy(&int_value, mp_array + m_position, read_size);
    m_position += read_size;

    return int_value;
}

float BinaryArrayReader::ReadFloat()
{
    if (mp_array == NULL || m_position < 0)
        return 0.0f;

    int read_size = sizeof(float);

    float float_value = 0.0f;
    memcpy(&float_value, mp_array + m_position, read_size);
    m_position += read_size;

    return float_value;
}

half BinaryArrayReader::ReadHalf()
{
    if (mp_array == NULL || m_position < 0)
        return 0.0f;

    int read_size = sizeof(uint16_t);

    uint16_t read_value = 0;
    memcpy(&read_value, mp_array + m_position, read_size);
    m_position += read_size;

    half half_value;
    half_value.set_value(read_value);

    return half_value;
};

////////////////////////////////////////////////////////////
//  BinaryFileWriter

BinaryFileWriter::BinaryFileWriter(void)
: m_is_buffer_empty(true)
{
    mp_file = NULL;
}

BinaryFileWriter::~BinaryFileWriter(void)
{
    CloseFile();
}

int BinaryFileWriter::OpenFile(LPCTSTR file_path)
{
    mp_file = _tfopen(file_path, _T("wb"));

    return (mp_file != NULL);
}

void BinaryFileWriter::CloseFile()
{
    if (mp_file == NULL)
        return;

    fclose(mp_file);
    mp_file = NULL;
}

int BinaryFileWriter::WriteInt(const int int_value)
{
    if (mp_file == NULL)
        return 0;

    int ret = fwrite(&int_value, sizeof(int), 1, mp_file);

    return (ret == 1);
}

int BinaryFileWriter::WriteFloat(const float float_value)
{
    if (mp_file == NULL)
        return 0;

    int ret = fwrite(&float_value, sizeof(float), 1, mp_file);

    return (ret == 1);
}

int BinaryFileWriter::WriteHalf(const half half_value)
{
    if (mp_file == NULL)
        return 0;

    uint16_t write_value = half_value.get_value();
    int ret = fwrite(&write_value, sizeof(uint16_t), 1, mp_file);

    return (ret == 1);
}

int BinaryFileWriter::WriteBytes(const BYTE *p_byte_buffer, const int buffer_size)
{
    if (p_byte_buffer == NULL)
        return 0;

    if (mp_file == NULL)
        return 0;

    int ret = fwrite(p_byte_buffer, sizeof(BYTE), buffer_size, mp_file);

    return (ret == buffer_size);
}

int BinaryFileWriter::WriteFloat12(const float12 float12_value)
{
    if (mp_file == NULL)
        return 0;

    uint16_t write_value = float12_value.get_value();

    int ret = 1;
    if (m_is_buffer_empty)
    {
        m_float12_buffer = 0;
        m_float12_buffer = write_value;
        m_float12_buffer = m_float12_buffer << 12;
        m_is_buffer_empty = false;
    }
    else
    {
        int vvv = write_value;
        m_float12_buffer = m_float12_buffer | vvv;

        BYTE write_byte[3];
        write_byte[0] = (BYTE)(m_float12_buffer & 0xFF);
        write_byte[1] = (BYTE)((m_float12_buffer & 0xFF00) >> 8);
        write_byte[2] = (BYTE)((m_float12_buffer & 0xFF0000) >> 16);
        ret = fwrite(write_byte, 3, 1, mp_file);
        m_is_buffer_empty = true;
    }
    

    return (ret == 1);
}

void BinaryFileWriter::FlushFloat12()
{
    if (mp_file == NULL)
        return;

    if (m_is_buffer_empty)
        return;

    BYTE write_byte[3];
    write_byte[0] = (BYTE)(m_float12_buffer & 0xFF);
    write_byte[1] = (BYTE)((m_float12_buffer & 0xFF00) >> 8);
    write_byte[2] = (BYTE)((m_float12_buffer & 0xFF0000) >> 16);
    fwrite(write_byte, 3, 1, mp_file);
    m_is_buffer_empty = true;
}

int BinaryFileWriter::WriteShort(const short short_value)
{
    if (mp_file == NULL)
        return 0;

    int ret = fwrite(&short_value, sizeof(short), 1, mp_file);

    return (ret == 1);
}