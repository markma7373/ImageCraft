#pragma once

#include "use_hylib.h"
#include "AnsiShare.h"
#include "AnsiTemplate.h"

using namespace AnsiShare;

enum AC_EditMode
{
    AC_EDIT_VIEW = 0,
    AC_EDIT_DRAW_SPACE,
    AC_EDIT_DRAW_BLOCK,
    AC_EDIT_CHANGE_COLOR,
    AC_EDIT_REFINE_BOUNDARY,
    AC_EDIT_REFINE_BOUNDARY_HALF,
    AC_EDIT_REFINE_BOUNDARY_TRIANGLE,
    AC_EDIT_REFINE_BOUNDARY_REGULAR_TRIANGLE,
    AC_EDIT_DRAW_SMALL_SQUARE,
    AC_EDIT_MERGE_BLOCK,
    AC_EDIT_INSERT_DELETE_SPACE,
    AC_EDIT_DRAW_LARGE_BRUSH,
    AC_EDIT_ADD_TEXT,
    AC_EDIT_EXTEND_LINE,
    AC_EDIT_CLEAR_CANVAS,
};

enum AC_MergeBlockType
{
    CANNOT_MERGE = 0,
    SAFE_MERGE,
    FORCE_MERGE_LEFT_INVALID,
    FORCE_MERGE_RIGHT_INVALID,
    FORCE_MERGE_BOTH_INVALID,
};

struct AC_EditOption
{
    bool is_alternated_edit;
    int brush_size;
    int brush_shape;
    bool is_change_color_area_mode;

    AC_EditOption()
    {
        is_alternated_edit = false;
        brush_size = MIN_LARGE_BRUSH_SIZE;
        brush_shape = ANSI_BRUSH_SQUARE;
        is_change_color_area_mode = false;
    }
};

struct AC_AdditionalMergeData
{
    HyRect merge_rect;
    AnsiCell cell;
    AnsiColor left_color;
    AnsiColor right_color;
};

struct AC_EditInfo
{
    // shared
    bool is_valid;
    AC_EditMode mode;
    HyRect target_rect;

    // for AC_EDIT_VIEW / AC_EDIT_MERGE_BLOCK
    AnsiCell cell;
    AnsiColor color;
    AnsiColor color2;

    // for AC_EDIT_MERGE_BLOCK
    std::vector<AC_AdditionalMergeData> additional_merge_data;

    // for AC_EDIT_DRAW_SPACE
    std::vector<HyRect> covered_rects;

    // for AC_EDIT_REFINE_BOUNDARY / AC_EDIT_REFINE_BOUNDARY_HALF
    HyPoint line_start; 
    HyPoint line_end;

    // for AC_EDIT_REFINE_BOUNDARY_HALF / AC_EDIT_REFINE_BOUNDARY_TRIANGLE / AC_EDIT_REFINE_BOUNDARY_REGULAR_TRIANGLE
    bool is_have_second_line;
    HyPoint second_line_start;
    HyPoint second_line_end;

    // for AC_EDIT_REFINE_BOUNDARY_HALF
    bool is_left_half;

    // for AC_EDIT_CHANGE_COLOR / AC_EDIT_LARGE_BRUSH
    std::vector<HyPoint> target_region;

    // for AC_EDIT_CHANGE_COLOR (area mode)
    HyRect change_mask_roi;
    std::vector<BYTE> change_mask_buffer;

    AC_EditInfo()
    {
        is_valid = false;
    }
};

typedef std::vector<AnsiCell> AnsiRow;

class FullAnsiCanvas;
class AnsiCanvas
{
public:
    AnsiCanvas();
    ~AnsiCanvas();

    static const int m_min_block_size = 8;
    static const int m_max_block_size = 32;

    static const int UNLIMITED_SIZE = -1;

    void ClearCanvasAction();

    void SetBlockSize(int size);
    void MakeImage(HyImage *p_image);
    void UpdateImage(HyImage *p_image, const HyRect &roi);
    void SaveAnsi(LPCTSTR path, int max_width = UNLIMITED_SIZE);
    bool LoadAnsi(LPCTSTR path);

    HySize GetDisplayUnitSize();
    void GetDisplayOffset(int &x_offset, int &y_offset);

    bool MoveDisplayOffset(int x_shift, int y_shift, bool &is_canvas_changed);
    void ResetDisplayOffset();

    void DrawSpaces(const HyPoint &start_pt, const HyPoint &end_pt,
                    int color, bool is_continuous, HyRect &change_rect, bool &is_action_list_changed);
    void DrawLargeBrush(const HyPoint &start_pt, const HyPoint &end_pt,
                        int brush_level, int brush_shape,
                        int color, bool is_continuous, HyRect &change_rect, bool &is_action_list_changed);
    void InsertSpace(const HyPoint &point, int color, HyRect &change_rect);
    void DeleteSpace(const HyPoint &point, HyRect &change_rect);
    void DrawBlock(const HyPoint &point, const AnsiCell &cell, const AnsiColor &color, HyRect &change_rect);
    bool DrawSmallSquare(const HyPoint &start_point, const HyPoint &end_point,
                         int color, bool is_continuous, HyRect &change_rect, bool &is_action_list_changed);
    bool ChangeColor(const HyPoint &start_pt, const HyPoint &end_pt, 
                     const AnsiColor &color, bool is_continuous, HyRect &change_rect, bool &is_action_list_changed);
    bool ChangeColorArea(const HyPoint &seed, const AnsiColor &color, HyRect &change_rect);
    bool MergeBlockAction(const HyPoint &point, HyRect &change_rect, bool is_force_merge);
    bool AddText(const HyPoint &point, LPCTSTR text_string, int text_color, HyRect &change_rect);

    void GetEditInfo(int x, int y, AC_EditMode mode, AC_EditInfo &edit_info,
                     const AC_EditOption &option = AC_EditOption());
    bool GetColorAtLocation(int x, int y, AnsiColor &color);
    bool GetChangeableColorAtLocation(int x, int y, AnsiColor &color);

    bool StartRefineBoundary(int x, int y, AC_EditMode mode, AC_EditInfo &edit_info, HyRect &change_rect);
    void ContinueRefineBoundary(int x, int y, AC_EditInfo &edit_info, HyRect &change_rect, bool &is_action_list_changed);
    void EndRefineBoundary();

    int GetTotalActionCount();
    int GetCurrentActionIndex();
    std::vector<std_tstring> GetActionNameList();
    bool SetActionIndex(int new_index, HyRect &change_rect, bool &is_display_offset_moved);

    // debug only        
    void SetTestContents();

private:
    enum AnsiCharLocation
    {
        SINGLE_CHAR = 0,
        DOUBLE_CHAR_LEFT,
        DOUBLE_CHAR_RIGHT,
    };

    struct AnsiWriteInfo
    {
        int y_offset;
        int color_offset;
        int cell_offset;
        AnsiCharLocation char_location;
    };

    enum AC_RefineBoundaryType
    {
        REFINE_HORIZONTAL_BOUNDARY = 0,
        REFINE_VERTICAL_BOUNDARY,
        REFINE_HORIZONTAL_HALF_BOUNDARY,
        REFINE_TRIANGLE_BOUNDARY,
        REFINE_REGULAR_TRIANGLE_BOUNDARY,
    };

    enum AC_RefineBoundaryOrientation
    {
        REFINE_FROM_LEFT = 0,
        REFINE_FROM_TOP,
        REFINE_FROM_RIGHT,
        REFINE_FROM_BOTTOM,
        REFINE_FROM_UNDEFINED, // used at error state
    };

    enum RefineTriangleType
    {
        REFINE_TRIANGLE_FULL_BLOCK = 0,
        REFINE_TRIANGLE_LEFT_HALF,
        REFINE_TRIANGLE_RIGHT_HALF,
    };

    struct RefineBoundaryInfo
    {
        bool is_valid;
        int x_offset;
        int y_offset;
        AC_RefineBoundaryType type;
        int level;
        AnsiColor left_color;
        AnsiColor right_color;
        AnsiColor initial_left_color;
        AnsiColor initial_right_color;
        int edit_distance;
        RefineTriangleType refine_triangle_type;

        // only for REFINE_HORIZONTAL_HALF_BOUNDARY
        int right_level; 
        bool is_left_half;

        // only for REFINE_REGULAR_TRIANGLE_BOUNDARY
        int shape;

        RefineBoundaryInfo()
        {
            is_valid = false;
        }
    };

    enum ColorAreaBoundarySide
    {
        LEFT_SIDE = 0,
        TOP_SIDE,
        RIGHT_SIDE,
        BOTTOM_SIDE,
    };

    struct ColorAreaBoundarySegment
    {
        int start;
        int end;

        ColorAreaBoundarySegment()
        {
            SetInvalid();
        }

        void SetInvalid()
        {
            start = 0;
            end = 0;
        }

        bool IsValid() const
        {
            return (end > start);
        }

        void SetRange(int new_start, int new_end)
        {
            start = new_start;
            end = new_end;
        }

        bool IsOverlap(const ColorAreaBoundarySegment &other) const
        {
            return IsValid() && other.IsValid() &&
                   (end > other.start) && (other.end > start);
        }
    };

    struct ColorAreaBoundary
    {
        ColorAreaBoundarySegment left;
        ColorAreaBoundarySegment top;
        ColorAreaBoundarySegment right;
        ColorAreaBoundarySegment bottom;

        ColorAreaBoundary()
        {
            SetInvalid();
        }

        void SetInvalid()
        {
            left.SetInvalid();
            top.SetInvalid();
            right.SetInvalid();
            bottom.SetInvalid();
        }

        void SetAllValid(int width, int height)
        {
            left.SetRange(0, height);
            top.SetRange(0, width);
            right.SetRange(0, height);
            bottom.SetRange(0, width);
        }

        bool IsValid()
        {
            return (left.IsValid() || top.IsValid() || right.IsValid() || bottom.IsValid());
        }
    };

    struct ColorAreaUnit
    {
        int x_offset;
        int y_offset;
        AnsiCell cell;
        AnsiCharLocation char_location;

        bool is_foreground_valid;
        bool is_background_valid;

        ColorAreaBoundary boundary;

        ColorAreaUnit()
        {
            x_offset = 0;
            y_offset = 0;

            SetInvalid();
        }
        
        void SetInvalid()
        {
            is_foreground_valid = false;
            is_background_valid = false;
            boundary.SetInvalid();
        }

        bool IsValid() const
        {
            return is_foreground_valid || is_background_valid;
        }

        void PrintData()
        {
            _TCHAR buffer[64];
            _stprintf(buffer, _T("Offset (%d, %d), "), x_offset, y_offset);
            std_tstring dump_string = buffer;
            
            if (is_foreground_valid && is_background_valid)
                dump_string += _T("Valid: FG & BG");
            else if (is_foreground_valid)
                dump_string += _T("Valid: FG");
            else if (is_background_valid)
                dump_string += _T("Valid: BG");
            else
                dump_string += _T("Valid: None");

            dump_string += _T(", Boundary: ");

            if (boundary.left.IsValid())
            {
                _stprintf(buffer, _T("L %d-%d, "), boundary.left.start, boundary.left.end);
                dump_string += buffer;
            }
            if (boundary.top.IsValid())
            {
                _stprintf(buffer, _T("T %d-%d, "), boundary.top.start, boundary.top.end);
                dump_string += buffer;
            }
            if (boundary.right.IsValid())
            {
                _stprintf(buffer, _T("R %d-%d, "), boundary.right.start, boundary.right.end);
                dump_string += buffer;
            }
            if (boundary.bottom.IsValid())
            {
                _stprintf(buffer, _T("B %d-%d, "), boundary.bottom.start, boundary.bottom.end);
                dump_string += buffer;
            }

            ch_dprintf(_T("%s"), dump_string.c_str());
        }
    };

    struct ActionUnit
    {
        AC_EditMode type;
        bool is_alternated_edit;
        HyRect roi;
        std::vector<AnsiColor> old_colors;
        std::vector<AnsiRow> old_cells;
        std::vector<AnsiColor> new_colors;
        std::vector<AnsiRow> new_cells;

        ActionUnit()
        {
            type = AC_EDIT_VIEW;
            is_alternated_edit = false;
        }
    };

    struct ActionInfo
    {
        AC_EditMode type;
        HyRect change_rect;
        bool is_alternated_edit;
        bool is_continuous;
        int old_canvas_height;

        ActionInfo(AC_EditMode init_type, const HyRect &init_rect = hyRect(0, 0, 0, 0))
        {
            type = init_type;
            change_rect = init_rect;
            is_alternated_edit = false;
            is_continuous = false;
            old_canvas_height = 0;
        }
    };

    int m_width;
    int m_height;
    int m_block_size;
    int m_half_block_size;

    AnsiTemplate m_ansi_template;

    std::vector<AnsiRow> m_cells;
    AnsiColor *mp_colors;
    BYTE *mp_small_square_mask;

    RefineBoundaryInfo m_refine_boundary_info;

    int m_display_x_offset, m_display_y_offset;

    std::vector<AnsiRow> m_old_cells;
    AnsiColor *mp_old_colors;

    std::vector<ActionUnit> m_action_history;
    int m_current_action_index;

    ChCritSec m_data_lock;

    __forceinline int FloorDivision(int a, int b);
    __forceinline int ColorValue(int label, bool is_bright);
    __forceinline int ColorLabel(char color_char);
    __forceinline void StartAnsiCommand(FILE *p_file);
    __forceinline void EndAnsiCommand(FILE *p_file);
    __forceinline void EndLine(FILE *p_file);
    __forceinline bool IsLineEnd(const char *p_line);
    __forceinline bool IsAnsiCommand(const char *p_line);

    __forceinline const AnsiColor &CanvasColorAt(int x, int y) const
    {
        return mp_colors[y * m_width + x];
    }
    __forceinline AnsiColor &CanvasColorAt(int x, int y)
    {
        return mp_colors[y * m_width + x];
    }

    __forceinline const AnsiCell &CanvasCellAt(int x, int y) const
    {
        return m_cells[y][x];
    }
    __forceinline AnsiCell &CanvasCellAt(int x, int y)
    {
        return m_cells[y][x];
    }

    __forceinline bool IsEmptyBlock(const AnsiCell &cell)
    {
        return (cell.type == HORI_BLOCK || cell.type == VERT_BLOCK) && (cell.label == 0);
    }
    __forceinline bool IsFullBlock(const AnsiCell &cell)
    {
        return (cell.type == HORI_BLOCK || cell.type == VERT_BLOCK) && (cell.label == 8);
    }

    __forceinline int ToCoordX(int x_offset)
    {
        return (x_offset - m_display_x_offset) * m_half_block_size;
    }
    __forceinline int ToCoordY(int y_offset)
    {
        return (y_offset - m_display_y_offset) * m_block_size;
    }

    __forceinline int ToCoordY_2x(int y_offset_2x)
    {
        // y_offset_2x has 2x resolution than regular "line" offset.
        // This is used for drawing multiple small squares.

        return (y_offset_2x - m_display_y_offset * 2) * m_half_block_size;
    }

    __forceinline int GetHoriBorder(int label)
    {
        return m_block_size - ch_Round((float)m_block_size * label / 8);
    }

    __forceinline int GetVertBorder(int label)
    {
        return ch_Round((float)m_block_size * label / 8);
    }

    __forceinline bool IsForegroundTriangle(int x_shift, int y_shift, AnsiCharLocation char_location, int triangle_label);
    __forceinline bool IsForegroundRegularTriangle(int x_shift, int y_shift, AnsiCharLocation char_location, int triangle_label);

    __forceinline HyRect UnionChangeRect(const HyRect &rect1, const HyRect &rect2)
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

    bool IsValidRow(const AnsiRow &cell_row, const std::vector<AnsiColor> &color_row);
    bool IsValidRow(const AnsiRow &cell_row, const AnsiColor *p_color_row, int color_row_size);

    AnsiColor RandomColor(bool is_enable_bright = true);
    bool GetSingleBlockColor(const AnsiCell &cell, const AnsiColor &left_color, const AnsiColor &right_color, int &block_color);
    bool GetSplitBlockColor(const AnsiCell &cell, const AnsiColor &left_color, const AnsiColor &right_color, int &split_color1, int &split_color2);

    bool GetLeftBoundaryColor(int x, int y, int &boundary_color);
    bool GetRightBoundaryColor(int x, int y, int &boundary_color);
    bool GetTopBoundaryColor(int x, int y, int &boundary_color);
    bool GetTopBoundaryColor(int x1, int x2, int y, int &boundary_color);
    bool GetBottomBoundaryColor(int x, int y, int &boundary_color);
    bool GetBottomBoundaryColor(int x1, int x2, int y, int &boundary_color);

    void GetOutsideBoundaryColors(int x_offset, int y_offset,
                                  int boundary_colors[4], bool is_boundary_valid_array[4]);
    void GetOutsideBlockBoundaryColors(int x_offset, int y_offset,
                                       int boundary_colors[6], bool is_boundary_valid_array[6]);

    void UpdateBlockSize(int block_size);

    void MakeWriteRowAndColors(const AnsiRow &src_row, const AnsiColor *p_src_row_colors,
                               AnsiRow &dst_row, AnsiColor *p_dst_row_colors, int max_width);

    void AddColorCommand(FILE *p_file, const AnsiColor &curr_color, const AnsiColor &old_color);
    void ReadAnsiCommand(const char *&p_line, AnsiColor &curr_color);
    void LineDataToAnsiRow(const char *p_ansi_string, AnsiRow &row);

    void ClearCanvas();
    void TruncateInvalidRows(std::vector<AnsiRow> &cell_rows, std::vector<std::vector<AnsiColor> > &color_rows);
    void InitAnsiCanvas(const std::vector<AnsiRow> &cell_rows, const std::vector<std::vector<AnsiColor> > &color_rows);

    bool GetXYOffset(int x, int y, int &x_offset, int &y_offset);
    bool GetBlockXYOffset(int x, int y, int &x_offset, int &y_offset);
    void GetLineStepParameters(const HyPoint &start_pt, const HyPoint &end_pt,
                               float &step_x, float &step_y, int &step_count);
    bool SetCanvasColor(int x, int y, const AnsiColor &color, HyRect &change_rect);
    bool SearchCellInfo(const AnsiRow &row, int x, int &cell_offset, AnsiCharLocation &char_location);
    bool GetCharLocation(const AnsiRow &row, int x, AnsiCharLocation &char_location);
    int GetSingleColorInCell(const AnsiColor &color, const AnsiCell &cell, 
                             int x_shift, int y_shift, AnsiCharLocation location);
    void SetSingleColorInCell(int x_offset, int y_offset,
                              AnsiCell &cell, int new_color,
                              int x_shift, int y_shift, AnsiCharLocation location);

    std::vector<AnsiColor> InvalidColor();

    void WriteBlock(const HyPoint &point, const AnsiCell &cell,
                    const AnsiColor &left_color, const AnsiColor &right_color,
                    HyRect &redraw_rect, bool is_convert_to_spaces = true);
    void WriteBlock(int x_offset, int y_offset, const AnsiCell &cell,
                    const AnsiColor &left_color, const AnsiColor &right_color,
                    HyRect &redraw_rect, bool is_convert_to_spaces = true);
    
    bool CheckIfWriteSpaces(const std::vector<AnsiColor> &color,
                            const AnsiCell &cell, int space_colors[2]);

    void WriteAnsiData(int x_offset, int y_offset,
                       const AnsiColor &color, const AnsiCell &cell, HyRect &change_rect);
    void WriteAnsiData(int x_offset, int y_offset,
                       const std::vector<AnsiColor> &color, const AnsiCell &cell, HyRect &change_rect);
    bool FindWriteInfo(int x_offset, int y_offset, bool is_double_char, AnsiWriteInfo &info);
    void WriteDoubleCharData(const std::vector<AnsiColor> &color, const AnsiCell &cell, const AnsiWriteInfo &info, HyRect &change_rect);
    void WriteSingleCharData(const std::vector<AnsiColor> &color, const AnsiCell &cell, const AnsiWriteInfo &info, HyRect &change_rect);

    bool DrawSingleSmallSquare(const HyPoint &point, int color, HyRect &change_rect);
    bool MergeBlock(const HyPoint &point, HyRect &redraw_rect, bool is_force_merge);

    void GetSpaceColor(const AnsiColor &color, const AnsiCell &cell,
                       bool is_right_half, int &space_color, bool &is_pure_color);
    bool GetSpaceColor(int x_offset, int y_offset, int &space_color, bool &is_pure_color);
    bool GetSpaceColor(int x_offset, int y_offset, int &space_color);
    bool GetPureSpaceColor(const AnsiColor &color, const AnsiCell &cell, bool is_right_half, int &space_color);
    bool GetPureSpaceColor(int x_offset, int y_offset, const AnsiCell &cell, bool is_right_half, int &space_color);
    bool GetPureSpaceColor(int x_offset, int y_offset, int &space_color);
    void SetSpaceColor(int x_offset, int y_offset, const AnsiCell &cell, bool is_right_half);

    bool GetSmallSquareColor(int x_offset, int y_offset_2x, int &square_color, bool &is_pure_color);
    bool GetSmallSquareColor(int x_offset, int y_offset_2x, int &square_color);

    void SearchViewInfo(int x, int y, AC_EditInfo &info);
    void SearchDrawSpaceInfo(int x, int y, AC_EditInfo &info);
    void SearchDrawBlockInfo(int x, int y, AC_EditInfo &info);
    void SearchDrawSmallSquareInfo(int x, int y, AC_EditInfo &info);
    bool IsCanDrawSmallSquare(int x_offset, int y_offset);
    void SearchChangeColorInfo(int x, int y, AC_EditInfo &info, bool is_area_mode);
    void SearchMergeBlockInfo(int x, int y, AC_EditInfo &info, bool is_force_merge);
    void SearchMergeBlockInfoAtOffset(int x_offset, int y_offset, AC_EditInfo &info, bool is_force_merge);
    AC_MergeBlockType SearchMergeOneBlockInfoAtOffset(int x_offset, int y_offset, AC_EditInfo &info);

    void SearchInsertDeleteSpaceInfo(int x, int y, AC_EditInfo &info);
    void SearchDrawLargeBrushInfo(int x, int y, int brush_level, int brush_shape, AC_EditInfo &info);
    void GetLargeBrushOffsetRange(int x, int y, int brush_level, int brush_shape,
                                  HyRect &offset_range, HyRect &unbounded_offset_range);
    bool GetBrushMask(const HyRect &offset_range,
                      const HyRect &unbounded_offset_range,
                      int brush_shape, HyImage *p_brush_mask);
    bool MakeBrushTargetRegion(const HyRect &offset_range,
                               HyImage *p_brush_mask, std::vector<HyPoint> &target_region);

    void SearchAddTextInfo(int x, int y, AC_EditInfo &info);

    void FindTargetRegion(const HyRect &space_rect, std::vector<HyPoint> &target_region,
                          const AnsiCell &cell, int x_shift, int y_shift, AnsiCharLocation location);
    void SetTargetRegionToRect(std::vector<HyPoint> &target_region, const HyRect &rect);

    bool IsCanMergeBlock(int x_offset, int y_offset, 
                         const AnsiCell &left_cell, AnsiCharLocation left_location, 
                         const AnsiCell &right_cell, AnsiCharLocation right_location,
                         AnsiCell &merged_cell, AnsiColor merged_colors[2]);

    void SearchRefineBoundaryInfo(int x, int y, AC_EditMode mode, RefineBoundaryInfo &info);

    void SearchHorizontalRefineBoundaryInfo(int x, int y, RefineBoundaryInfo &info);
    int GetHorizontalEditDistance(int coordinate, int x, int y, int &level, 
                                  AnsiColor &left_color, AnsiColor &right_color);
    bool IsColorBlockHorizontalEditable(int coordinate, int x, int y, int block_color,
                                        int &level, AnsiColor &left_color, AnsiColor &right_color);
    bool IsColorSpacesHorizontalEditable(int coordinate, int x, int y, int left_color, int right_color,
                                         int &level, AnsiColor &left_edit_color, AnsiColor &right_edit_color);
    void SetHorizontalRefineBoundaryInfo(int x_offset, int y_offset, int level, 
                                         const AnsiColor &left_color, const AnsiColor &right_color,
                                         int edit_distance, RefineBoundaryInfo &info);

    void SearchVerticalRefineBoundaryInfo(int x, int y, RefineBoundaryInfo &info);
    int GetVerticalEditDistance(int coordinate, int x, int y, int &level, 
                                AnsiColor &left_color, AnsiColor &right_color);
    bool IsColorBlockVerticalEditable(int coordinate, int x, int y, int block_color,
                                      int &level, AnsiColor &left_color, AnsiColor &right_color);
    void SetVerticalRefineBoundaryInfo(int x_offset, int y_offset, int level, 
                                       const AnsiColor &left_color, const AnsiColor &right_color, 
                                       int edit_distance, RefineBoundaryInfo &info);

    void SearchHorizontalHalfRefineBoundaryInfo(int x, int y, RefineBoundaryInfo &info);
    int GetHorizontalHalfEditDistance(int coordinate, int x, int y,
                                      bool is_left_half, int level[2], AnsiColor color[2]);
    bool IsHalfEditValid(int x, int y, int y_coordinate, bool is_left_half,
                         int left_space_color, int right_space_color,
                         AnsiColor &left_color, AnsiColor &right_color, int &base_level);
    bool GetHalfEditLevels(int x, int y, int y_coordinate, 
                           AnsiColor &left_color, AnsiColor &right_color,
                           bool is_left_half, int base_level, int level[2]);
    void SetHorizontalHalfRefineBoundaryInfo(int x_offset, int y_offset, bool is_left_half,
                                             const int level[2], const AnsiColor color[2],
                                             int edit_distance, RefineBoundaryInfo &info);

    void SearchTriangleRefineBoundaryInfo(int x, int y, RefineBoundaryInfo &info);
    bool GetTriangleRefineStatus(int x_coord, int y_coord, int x_offset, int y_offset,
                                 int &shape, int &level, RefineTriangleType &type,
                                 AnsiColor &left_color, AnsiColor &right_color);
    int GetShape0TriangleRefineLevel(int block_dx, int block_dy, const AnsiCell &src_cell,
                                     const AnsiColor &src_left_color, const AnsiColor &src_right_color, 
                                     bool is_left_pure_color, int left_space_color,
                                     bool is_right_pure_color, int right_space_color,
                                     const bool is_boundary_valid_array[6], const int boundary_colors[6],
                                     AnsiColor &left_color, AnsiColor &right_color, RefineTriangleType &type);
    int GetShape1TriangleRefineLevel(int block_dx, int block_dy, const AnsiCell &src_cell,
                                     const AnsiColor &src_left_color, const AnsiColor &src_right_color, 
                                     bool is_left_pure_color, int left_space_color,
                                     bool is_right_pure_color, int right_space_color,
                                     const bool is_boundary_valid_array[6], const int boundary_colors[6],
                                     AnsiColor &left_color, AnsiColor &right_color, RefineTriangleType &type);
    void SetTriangleRefineBoundaryInfo(int x_offset, int y_offset,
                                       int shape, int level, RefineTriangleType type,
                                       const AnsiColor &left_color, const AnsiColor &right_color,
                                       RefineBoundaryInfo &info);

    void SearchRegularTriangleRefineBoundaryInfo(int x, int y, RefineBoundaryInfo &info);
    bool GetRegularTriangleRefineStatus(int x_coord, int y_coord, int x_offset, int y_offset,
                                        int &shape, int &level, AnsiColor &left_color, AnsiColor &right_color);
    int GetShape0RegularTriangleRefineLevel(int block_dx, int block_dy, const AnsiCell &src_cell,
                                            const AnsiColor &src_left_color, const AnsiColor &src_right_color, 
                                            bool is_left_pure_color, int left_space_color,
                                            bool is_right_pure_color, int right_space_color,
                                            const bool is_left_boundary_valid_array[4], const int left_boundary_colors[4],
                                            const bool is_right_boundary_valid_array[4], const int right_boundary_colors[4],
                                            AnsiColor &left_color, AnsiColor &right_color);
    int GetShape1RegularTriangleRefineLevel(int block_dx, int block_dy, const AnsiCell &src_cell,
                                            const AnsiColor &src_left_color, const AnsiColor &src_right_color, 
                                            bool is_left_pure_color, int left_space_color,
                                            bool is_right_pure_color, int right_space_color,
                                            const bool is_left_boundary_valid_array[4], const int left_boundary_colors[4],
                                            const bool is_right_boundary_valid_array[4], const int right_boundary_colors[4],
                                            AnsiColor &left_color, AnsiColor &right_color);
    void SetRegularTriangleRefineBoundaryInfo(int x_offset, int y_offset, int shape, int level, 
                                              const AnsiColor &left_color, const AnsiColor &right_color,
                                              RefineBoundaryInfo &info);

    void ConvertToEditInfo(const RefineBoundaryInfo &info, AC_EditInfo &edit_info);
    void SetRefineTriangleEditLine(int base_x, int base_y,
                                   int shape, int level, RefineTriangleType type,
                                   HyPoint &line_start, HyPoint &line_end,
                                   HyPoint &line_start2, HyPoint &line_end2,
                                   bool &is_have_second_line);
    void SetRefineRegularTriangleEditLine(int base_x, int base_y,
                                          int shape, int level,
                                          HyPoint &line_start, HyPoint &line_end,
                                          HyPoint &line_start2, HyPoint &line_end2,
                                          bool &is_have_second_line);

    void ChangeNeighborBlockIfNeeded(const RefineBoundaryInfo &info, AC_RefineBoundaryOrientation orientation);
    void UpdateDataForRefineBoundary(int x, int y, HyRect &change_rect, bool is_start_refine);
    bool IsValidToMoveForRefineBoundary(int x_offset, int y_offset, int &neighbor_color, bool is_accept_bright_color);

    void UpdateRefineHalfBoundaryInfo(int x, int y, RefineBoundaryInfo &info, int &level);
    int GetRefineHorizontalHalfLevel(int y_coord, int y_offset, const AnsiColor &edit_color, int fix_level);

    int GetTriangleLocation(int dx, int dy, int width, int height, bool is_backslash);
    void UpdateRefineTriangleInfo(int x, int y, RefineBoundaryInfo &info, 
                                  AnsiCell &write_cell, AnsiColor &left_write_color, AnsiColor &right_write_color);
    void UpdateRefineRegularTriangleInfo(int x, int y, RefineBoundaryInfo &info, 
                                         AnsiCell &write_cell, AnsiColor &left_write_color, AnsiColor &right_write_color);

    void MergeBlockBeforeEditing(const HyRect &block_rect, HyRect &change_rect);

    void WriteSmallSquaresAtLine(const BYTE *p_mask, int y_offset,
                                 int x_offset_min, int x_offset_max,
                                 int color, HyRect &change_rect);

    void WriteSpace(int x_offset, int y_offset, int space_color, HyRect &redraw_rect);
    void WriteSpaces(int x_offset_min, int x_offset_max, int y_offset, int space_color, HyRect &redraw_rect);
    void WriteRegion(int x_offset, int y_offset, const AnsiCell &cell,
                     const AnsiColor &left_color, const AnsiColor &right_color,
                     HyRect &redraw_rect, bool is_convert_to_spaces = true);
    void WriteRegion(int x_offset, int y_offset,
                     const std::vector<AnsiCell> &cells,
                     const std::vector<AnsiColor> &colors,
                     HyRect &redraw_rect, bool is_handle_small_square,
                     bool is_convert_to_spaces = true);

    bool GetAllChangedColorAreaUnits(int seed_x, int seed_y, std::vector<ColorAreaUnit> &all_changed_units);
    bool GetColorAreaSeedUnit(int seed_x, int seed_y, ColorAreaUnit &seed_unit);
    bool GetColorAreaSearchUnit(int x_offset, int y_offset,
                                const BYTE *p_visit_mask, int target_color,
                                ColorAreaBoundarySide boundary_side,
                                const ColorAreaBoundarySegment &boundary_segment, ColorAreaUnit &unit);
    ColorAreaBoundary GetColorAreaBoundary(const AnsiCell &cell, AnsiCharLocation char_location,
                                           bool is_foreground_valid, bool is_background_valid);
    void FindChangeMask(const HyPoint &seed, HyRect &change_roi, std::vector<BYTE> &change_mask);

    std_tstring GetActionName(const ActionUnit &unit);

    void ClearActionHistory();
    void DumpHistory();
    bool RecordAction(const ActionInfo &action_info);
    void PrepareOldCanvasDataForContinuousAction(const ActionUnit &old_action,
                                                 AnsiColor *p_old_colors, 
                                                 std::vector<AnsiRow> &old_cells);
    void UpdateOldCanvasData(const HyRect &offset_roi);
    bool UndoOneAction(HyRect &offset_roi);
    bool RedoOneAction(HyRect &offset_roi);
    bool IsContinuousAction(const ActionInfo &action_info);

};
