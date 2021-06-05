#pragma once

#include "AnsiShare.h"
#include "ImageCraftShare.h"

enum AnsiProjectFieldLabel
{
    AP_IMAGE_FILE_DATA = 0,
    AP_IMAGE_EXTENSION,
    AP_ANSI_FILE_DATA,
    AP_BLOCK_SIZE,
    AP_SCALED_IMAGE_WIDTH,
    AP_SCALED_IMAGE_HEIGHT,
    AP_IMAGE_ROTATE_DEGREE,
    AP_IMAGE_SCALE_X,
    AP_IMAGE_SCALE_Y,
    AP_IMAGE_ROI_OFFSET_X,
    AP_IMAGE_ROI_OFFSET_Y,
    AP_IMAGE_ALPHA_RATIO,
    AP_FIELD_LABEL_AMOUNT,
};

struct AnsiProjectData
{
    std_tstring image_path;
    std_tstring ansi_path;
    int block_size;
    HySize scaled_image_size;
    float image_rotate_degree;
    float image_scale_x;
    float image_scale_y;
    HyPoint2D32f image_roi_offset;
    int image_alpha_ratio;

    AnsiProjectData()
    {
        Reset();
    }

    void Reset()
    {
        image_path.clear();
        ansi_path.clear();
        block_size = GetDefaultBlockSize();
        scaled_image_size.width = 0;
        scaled_image_size.height = 0;
        image_rotate_degree = 0.0f;
        image_scale_x = 0.0f;
        image_scale_y = 0.0f;
        image_roi_offset.x = 0.0f;
        image_roi_offset.y = 0.0f;
        image_alpha_ratio = 50;
    }
};

class AnsiProjectManager
{
public:
    AnsiProjectManager();
    ~AnsiProjectManager();

    bool SaveProject(const std_tstring &project_path,
                     const std_tstring &image_path, const std_tstring &ansi_path);
    bool LoadProject(LPCTSTR project_path, AnsiProjectData &project_data, LPCTSTR temp_folder);

    LPCTSTR GetErrorMessage()
    {
        return m_error_message.c_str();
    }

private:
    enum FieldType
    {
        UNKNOWN = 0,
        INTEGER,
        FLOAT,
        STRING,
        BINARY,
    };

    struct DataField
    {
        std_tstring title;
        FieldType type;
        int data_size;
        BYTE *p_data;

        DataField()
        {
            title = _T("");
            type = UNKNOWN;
            data_size = 0;
            p_data = NULL;
        }

        DataField(LPCTSTR init_title, FieldType init_type)
        {
            title = init_title;
            type = init_type;
            data_size = 0;
            p_data = NULL;
        }

        DataField(const DataField &src_field)
        {
            p_data = NULL;
            CopyFrom(src_field);
        }

        ~DataField()
        {
            _DELETE_PTRS(p_data);
        }

        DataField& operator=(const DataField &src_field)
        {
            CopyFrom(src_field);
            return *this;
        }

        void CopyFrom(const DataField &src_field)
        {
            title = src_field.title;
            type = src_field.type;
            data_size = ch_Max(src_field.data_size, 0);

            if (data_size == 0)
            {
                _DELETE_PTRS(p_data);
            }
            else
            {
                _NEW_PTRS(p_data, BYTE, data_size);
                memcpy(p_data, src_field.p_data, data_size);
            }
        }

        void SetData(const void *p_new_data, int new_size)
        {
            _DELETE_PTRS(p_data);
            data_size = 0;

            if (p_new_data != NULL && new_size > 0)
            {
                data_size = new_size;
                _NEW_PTRS(p_data, BYTE, data_size);
                memcpy(p_data, p_new_data, data_size);
            }
        }

        void TransitTo(DataField &dst_field)
        {
            // Move the data and pointer to dst_field without memory copy.
            dst_field.title = title;
            dst_field.type = type;
            dst_field.data_size = data_size;
            dst_field.p_data = p_data;

            title = _T("");
            type = UNKNOWN;
            data_size = 0;
            p_data = NULL;
        }

        __forceinline bool IsMatchTitle(int title_index) const;

        void SetInteger(int value);
        int GetInteger() const;

        void SetFloat(float value);
        float GetFloat() const;

        void SetString(const std_tstring &string);
        std_tstring GetString() const;

        void SetBinaryFile(FILE *p_file);
    };

    struct BinaryInfo
    {
        ChAutoPtr<BYTE> data;
        int size;
        std_tstring path;

        BinaryInfo()
        {
            size = 0;
        }

        void SetData(const BYTE *p_data, int data_size)
        {
            if (data_size > 0)
            {
                size = data_size;
                data.Alloc(size);
                memcpy(data, p_data, size);
            }
            else
            {
                size = 0;
                data.Free();
            }
        }

        bool IsValid()
        {
            return (size > 0 && path.length() > 0);
        }

        bool GenerateFile();
    };

    std_tstring m_error_message;

    void SaveImageToProject(FILE *p_project_file, const std_tstring &image_path);
    void SaveAnsiToProject(FILE *p_project_file, const std_tstring &ansi_path);

    template<typename T> bool ReadData(FILE *p_file, T *p_data, int count);

    void WriteInt(FILE *p_file, int value);
    bool ReadInt(FILE *p_file, int &value);

    void WriteIntegerField(FILE *p_file, int value, int label_index);

    void WriteFloatField(FILE *p_file, float value, int label_index);

    void WriteDataField(FILE *p_project_file, const DataField &field);
    bool ReadDataField(FILE *p_project_file, DataField &field, bool &is_eof);

    bool ReadAllDataFields(FILE *p_project_file, std::vector<DataField> &all_fields);
    bool ProcessDataFields(const std::vector<DataField> &all_fields,
                           AnsiProjectData &project_data, LPCTSTR temp_folder);

    static const BYTE datafield_label = 0xdf;
};