#include "stdafx.h"
#include "AnsiProjectManager.h"
#include "ImageCraft.h"

const _TCHAR *g_field_label_titles[AP_FIELD_LABEL_AMOUNT] =
{
    _T("ImageFileData"),        // AP_IMAGE_FILE_DATA
    _T("ImageExtension"),       // AP_IMAGE_EXTENSION
    _T("AnsiFileData"),         // AP_ANSI_FILE_DATA
    _T("BlockSize"),            // AP_BLOCK_SIZE
    _T("ScaledImageWidth"),     // AP_SCALED_IMAGE_WIDTH,
    _T("ScaledImageHeight"),    // AP_SCALED_IMAGE_HEIGHT,
    _T("ImageScaleX"),          // AP_IMAGE_SCALE_X,
    _T("ImageScaleY"),          // AP_IMAGE_SCALE_Y,
    _T("ImageRotateDegree"),    // AP_IMAGE_ROTATE_DEGREE,
    _T("ImageRoiOffsetX"),      // AP_IMAGE_ROI_OFFSET_X,
    _T("ImageRoiOffsetY"),      // AP_IMAGE_ROI_OFFSET_Y,
    _T("ImageAlphaRatio"),      // AP_IMAGE_ALPHA_RATIO,
};

const _TCHAR g_project_temp_image_name[] = _T("/project_image");
const _TCHAR g_project_temp_ansi_name[] = _T("/project_ansi.ans");

AnsiProjectManager::AnsiProjectManager()
{

}

AnsiProjectManager::~AnsiProjectManager()
{

}

inline bool AnsiProjectManager::DataField::IsMatchTitle(int title_index) const
{
    return (title.compare(g_field_label_titles[title_index]) == 0);
}

void AnsiProjectManager::DataField::SetInteger(int value)
{
    int32_t data_value = (int32_t)value;
    SetData(&data_value, sizeof(int32_t));
}

int AnsiProjectManager::DataField::GetInteger() const
{
    int32_t value = 0;
    memcpy(&value, p_data, sizeof(int32_t));
    return (int)value;
}

void AnsiProjectManager::DataField::SetFloat(float value)
{
    SetData(&value, sizeof(float));
}

float AnsiProjectManager::DataField::GetFloat() const
{
    float value = 0.0f;
    memcpy(&value, p_data, sizeof(float));
    return value;
}

void AnsiProjectManager::DataField::SetString(const std_tstring &string)
{
    int length = string.length();
    ChAutoPtr<_TCHAR> string_buffer(length + 1);
    memcpy(string_buffer, string.c_str(), sizeof(_TCHAR) * (length + 1));

    SetData(string_buffer, sizeof(_TCHAR) * (length + 1));
}

std_tstring AnsiProjectManager::DataField::GetString() const
{
    std_tstring string = _T("");

    if (p_data != NULL && data_size >= sizeof(_TCHAR))
        string = (_TCHAR *)p_data;

    return string;
}

void AnsiProjectManager::DataField::SetBinaryFile(FILE *p_file)
{
    fseek(p_file, 0, SEEK_END);
    int buffer_size = (int)ftell(p_file);
    ChAutoPtr<BYTE> binary_buffer(buffer_size);

    fseek(p_file, 0, SEEK_SET);
    fread(binary_buffer, 1, buffer_size, p_file);

    SetData(binary_buffer, buffer_size);
}

bool AnsiProjectManager::BinaryInfo::GenerateFile()
{
    if (IsValid() == false)
        return false;

    FILE *p_file = _tfopen(path.c_str(), _T("wb"));
    if (p_file == NULL)
        return false;

    size_t write_size = fwrite((BYTE *)data, 1, size, p_file);
    if ((int)write_size != size)
        return false;

    fclose(p_file);

    return true;
}

bool AnsiProjectManager::SaveProject(const std_tstring &project_path,
                                     const std_tstring &image_path, const std_tstring &ansi_path)
{
    m_error_message.clear();

    FILE *p_project_file = _tfopen(project_path.c_str(), _T("wb"));
    if (p_project_file == NULL)
    {
        m_error_message = _T("無法寫入專案檔案");
        return false;
    }

    SaveImageToProject(p_project_file, image_path);
    SaveAnsiToProject(p_project_file, ansi_path);

    // Save current settings.
    WriteIntegerField(p_project_file, theApp.m_block_size, AP_BLOCK_SIZE);
    WriteFloatField(p_project_file, theApp.m_image_rotate_degree, AP_IMAGE_ROTATE_DEGREE);
    WriteFloatField(p_project_file, theApp.m_image_scale_x, AP_IMAGE_SCALE_X);
    WriteFloatField(p_project_file, theApp.m_image_scale_y, AP_IMAGE_SCALE_Y);
    WriteFloatField(p_project_file, theApp.m_image_roi_offset.x, AP_IMAGE_ROI_OFFSET_X);
    WriteFloatField(p_project_file, theApp.m_image_roi_offset.y, AP_IMAGE_ROI_OFFSET_Y);
    WriteIntegerField(p_project_file, theApp.m_image_alpha_ratio, AP_IMAGE_ALPHA_RATIO);

    fclose(p_project_file);

    return true;
}

void AnsiProjectManager::SaveImageToProject(FILE *p_project_file, const std_tstring &image_path)
{
    DataField image_data_field(g_field_label_titles[AP_IMAGE_FILE_DATA], BINARY);
    DataField image_ext_field(g_field_label_titles[AP_IMAGE_EXTENSION], STRING);

    FILE *p_image_file = _tfopen(image_path.c_str(), _T("rb"));
    if (p_image_file)
    {
        image_data_field.SetBinaryFile(p_image_file);

        std_tstring image_ext = ch_GetFileExtName(image_path.c_str());
        image_ext_field.SetString(image_ext);

        fclose(p_image_file);
    }

    WriteDataField(p_project_file, image_data_field);
    WriteDataField(p_project_file, image_ext_field);
}

void AnsiProjectManager::SaveAnsiToProject(FILE *p_project_file, const std_tstring &ansi_path)
{
    DataField ansi_data_field(g_field_label_titles[AP_ANSI_FILE_DATA], BINARY);

    FILE *p_ansi_file = _tfopen(ansi_path.c_str(), _T("rb"));
    if (p_ansi_file)
    {
        ansi_data_field.SetBinaryFile(p_ansi_file);

        fclose(p_ansi_file);
    }

    WriteDataField(p_project_file, ansi_data_field);
}

template<typename T>
bool AnsiProjectManager::ReadData(FILE *p_file, T *p_data, int count)
{
    if (p_file == NULL || p_data == NULL)
        return false;

    size_t read_count = fread(p_data, sizeof(T), count, p_file);
    return ((int)read_count == count);
}

void AnsiProjectManager::WriteInt(FILE *p_file, int value)
{
    int32_t write_value = (int32_t)value;
    fwrite(&write_value, sizeof(int32_t), 1, p_file);
}

bool AnsiProjectManager::ReadInt(FILE *p_file, int &value)
{
    int32_t read_value = 0;
    if (fread(&read_value, sizeof(int32_t), 1, p_file) != 1)
        return false;

    value = (int)read_value;
    return true;
}

void AnsiProjectManager::WriteIntegerField(FILE *p_file, int value, int label_index)
{
    DataField field(g_field_label_titles[label_index], INTEGER);
    field.SetInteger(value);
    WriteDataField(p_file, field);
}

void AnsiProjectManager::WriteFloatField(FILE *p_file, float value, int label_index)
{
    DataField field(g_field_label_titles[label_index], FLOAT);
    field.SetFloat(value);
    WriteDataField(p_file, field);
}

void AnsiProjectManager::WriteDataField(FILE *p_project_file, const DataField &field)
{
    fwrite(&datafield_label, sizeof(datafield_label), 1, p_project_file);

    int title_size = field.title.length() + 1;
    WriteInt(p_project_file, title_size);
    fwrite(field.title.c_str(), sizeof(_TCHAR), title_size, p_project_file);

    WriteInt(p_project_file, (int)field.type);
    WriteInt(p_project_file, field.data_size);

    if (field.data_size > 0)
        fwrite(field.p_data, 1, field.data_size, p_project_file);
}

bool AnsiProjectManager::ReadDataField(FILE *p_project_file, DataField &field, bool &is_eof)
{
    // Return true with is_eof = false if a field is read successfully,
    //          or with is_eof = true if there are no more data.
    // Return false if any other error occurs.

    is_eof = false;

    BYTE check_label = 0;
    if (fread(&check_label, sizeof(check_label), 1, p_project_file) != 1)
    {
        is_eof = true;
        return true;
    }

    if (check_label != datafield_label)
        return false;

    int title_size = 0;
    if (ReadInt(p_project_file, title_size) == false)
        return false;

    ChAutoPtr<_TCHAR> title_buffer(title_size);
    if (ReadData<_TCHAR>(p_project_file, title_buffer, title_size) == false)
        return false;

    field.title = title_buffer;

    int type_value = 0;
    if (ReadInt(p_project_file, type_value) == false)
        return false;

    field.type = (FieldType)type_value;

    if (ReadInt(p_project_file, field.data_size) == false)
        return false;

    field.data_size = ch_Max(field.data_size, 0);
    if (field.data_size == 0)
    {
        _DELETE_PTRS(field.p_data);
    }
    else
    {
        _NEW_PTRS(field.p_data, BYTE, field.data_size);
        if (ReadData<BYTE>(p_project_file, field.p_data, field.data_size) == false)
            return false;
    }

    return true;
}

bool AnsiProjectManager::LoadProject(LPCTSTR project_path, AnsiProjectData &project_data, LPCTSTR temp_folder)
{
    // Store all information of the project in project_data.
    // For binary files, they are generated into temp_folder.

    m_error_message.clear();

    if (project_path == NULL)
    {
        m_error_message = _T("無法讀取專案檔案");
        return false;
    }

    FILE *p_project_file = _tfopen(project_path, _T("rb"));
    if (p_project_file == NULL)
    {
        m_error_message = _T("無法讀取專案檔案");
        return false;
    }

    std::vector<DataField> all_fields;
    bool is_success = ReadAllDataFields(p_project_file, all_fields);

    fclose(p_project_file);

    if (is_success == false)
    {
        m_error_message = _T("檔案格式錯誤");
        return false;
    }

    is_success = ProcessDataFields(all_fields, project_data, temp_folder);

    return is_success;
}

bool AnsiProjectManager::ReadAllDataFields(FILE *p_project_file, std::vector<DataField> &all_fields)
{
    all_fields.clear();

    bool is_success = true;

    while (true)
    {
        DataField new_field;
        bool is_eof = false;
        is_success = ReadDataField(p_project_file, new_field, is_eof);
        if (is_success == false)
            break;
        if (is_eof)
            break;

        all_fields.push_back(DataField());
        new_field.TransitTo(all_fields.back());
    }

    return is_success;
}

bool AnsiProjectManager::ProcessDataFields(const std::vector<DataField> &all_fields,
                                           AnsiProjectData &project_data, LPCTSTR temp_folder)
{
    BinaryInfo image_info;
    BinaryInfo ansi_info;

    for (int i = 0; i < (int)all_fields.size(); i++)
    {
        const DataField &field = all_fields[i];

        if (field.IsMatchTitle(AP_IMAGE_FILE_DATA))
        {
            if (field.data_size > 0)
            {
                image_info.SetData(field.p_data, field.data_size);
            }
        }
        else if (field.IsMatchTitle(AP_IMAGE_EXTENSION))
        {
            image_info.path = temp_folder;
            image_info.path += g_project_temp_image_name;
            image_info.path += field.GetString();
        }
        else if (field.IsMatchTitle(AP_ANSI_FILE_DATA)) 
        {
            if (field.data_size > 0)
            {
                ansi_info.SetData(field.p_data, field.data_size);
                ansi_info.path = temp_folder;
                ansi_info.path += g_project_temp_ansi_name;
            }
        }
        else if (field.IsMatchTitle(AP_BLOCK_SIZE))
        {
            project_data.block_size = field.GetInteger();
        }
        else if (field.IsMatchTitle(AP_SCALED_IMAGE_WIDTH))
        {
            project_data.scaled_image_size.width = field.GetInteger();
        }
        else if (field.IsMatchTitle(AP_SCALED_IMAGE_HEIGHT))
        {
            project_data.scaled_image_size.height = field.GetInteger();
        }
        else if (field.IsMatchTitle(AP_IMAGE_ROTATE_DEGREE))
        {
            project_data.image_rotate_degree = field.GetFloat();
        }
        else if (field.IsMatchTitle(AP_IMAGE_SCALE_X))
        {
            project_data.image_scale_x = field.GetFloat();
        }
        else if (field.IsMatchTitle(AP_IMAGE_SCALE_Y))
        {
            project_data.image_scale_y = field.GetFloat();
        }
        else if (field.IsMatchTitle(AP_IMAGE_ROI_OFFSET_X))
        {
            project_data.image_roi_offset.x = field.GetFloat();
        }
        else if (field.IsMatchTitle(AP_IMAGE_ROI_OFFSET_Y))
        {
            project_data.image_roi_offset.y = field.GetFloat();
        }
        else if (field.IsMatchTitle(AP_IMAGE_ALPHA_RATIO))
        {
            project_data.image_alpha_ratio = field.GetInteger();
        }
    }

    if (image_info.IsValid())
    {
        image_info.GenerateFile();
        project_data.image_path = image_info.path;
    }
    if (ansi_info.IsValid())
    {
        ansi_info.GenerateFile();
        project_data.ansi_path = ansi_info.path;
    }

    return true;
}
