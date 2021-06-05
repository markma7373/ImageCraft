#pragma once

#include <vector>
#include "AnsiCanvas.h"
#include "use_gdiplus.h"
#include "ImageCraftShare.h"
#include "MyEdit.h"

using namespace Gdiplus;

class ImageCraftView : public CWnd
{
    DECLARE_DYNCREATE(ImageCraftView)

public:
    ImageCraftView();
    virtual ~ImageCraftView();

    afx_msg void OnTimer(UINT_PTR event_id);
    afx_msg void OnDestroy();
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnKillFocus(CWnd* pWnd);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnMouseLeave();
    afx_msg BOOL OnMouseWheel(UINT flags, short delta, CPoint pt);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

    void InitPainter(LPCTSTR image_path, LPCTSTR ansi_path,
                     const OldFormatData &old_format_data = OldFormatData());

    bool IsImageLoaded();
    void LoadImageFile(LPCTSTR image_path, bool is_use_init_roi = false,
                       const OldFormatData &old_format_data = OldFormatData());
    void ChangeResizeRatio(float ratio);
    void ChangeRotateDegree(float degree);

    void SetBlockSize(int block_size, bool is_invalidate = true);
    void SaveAnsi(LPCTSTR path);
    void LoadAnsi(LPCTSTR path);
    void ExportAnsi(LPCTSTR path, int max_width);

    void ClearCanvas();
    void SetTestContents();
    void ChangeDrawMode(int mode);

    void UpdateViewImage();

    void MoveDisplayOffset(int x_shift, int y_shift);
    void MoveDisplayPageUp();
    void MoveDisplayPageDown();

    void SetForegroundColor(int color)
    {
        m_current_color.fg_color = color;
    }
    void SetBackgroundColor(int color)
    {
        m_current_color.bg_color = color;
    }

    void ChangeBrushSize(int amount);
    void SetBrushSize(int new_size);
    void ChangeBrushShape(int amount);
    void ChangeSelectBlockTable(int amount);
    void SetBrushShape(int new_shape);
    void SetSelectBlockMode(int new_mode);
    void SetChangeColorAreaMode(bool is_enable);
    void SetRefineTriangleMode(bool is_enable);
    void SetActionIndex(int new_index);
    void UndoByHotKey();
    void RedoByHotKey();
    void SetPressKeyForCombo(bool is_press_ctrl, bool is_press_shift);

    void DebugDump();

protected:
    DECLARE_MESSAGE_MAP()

    // Overrides
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    afx_msg void OnPaint();

private:
    HyImage *mp_full_image;     // source size
    HyImage *mp_scaled_image;   // scaled, not rotated
    HyImage *mp_image;          // scaled and rotated for background
    HyPoint2D32f m_image_offset;  // for the first row/column
    HyRect m_image_roi;     // for current display
    ImageScale m_scale_to_full;
    ImageScale m_scale_to_resized;
    HyPoint2D32f m_rotated_image_shift;

    HyImage *mp_view_image;
    HyImage *mp_ansi_image;

    AnsiCanvas m_ansi_canvas;
    HySize m_canvas_unit_size;

    AnsiTemplate m_ansi_template;
    HyImage *mp_info_image;
    AnsiCell m_info_cell;
    AnsiColor m_info_color;
    HyImage *mp_prompt_string_image;

    bool m_is_left_button_down;
    bool m_is_right_button_down;
    bool m_is_press_control;
    bool m_is_press_shift;
    bool m_is_press_control_for_combo;
    bool m_is_press_shift_for_combo;

    bool m_is_alternated_edit;
    CPoint m_current_mouse_point;

    CPoint m_start_drag_pt;
    CPoint m_current_drag_pt;
    int m_prev_drag_time;
    bool m_is_start_drag;
    bool m_is_pressed_shift_when_start_drag;

    // Handle the mouse move out of the view while dragging.
    bool m_is_need_reset_leave_event;

    AnsiColor m_current_color;

    IC_DrawMode m_draw_mode;

    AC_EditInfo m_edit_info;
    bool m_is_during_refine_boundary;
    bool m_is_during_add_text;

    bool m_is_enable_draw_blink_region;
    int m_draw_blink_region_progress;
    bool m_is_draw_blink_region_progress_increase;

    __forceinline bool IsMoveImage();
    __forceinline bool IsViewAnsi();
    __forceinline bool IsDrawSpaces();
    __forceinline bool IsChangeColor();
    __forceinline bool IsRefineBoundary();
    __forceinline bool IsDrawBlock();
    __forceinline bool IsDrawSmallSquare();
    __forceinline bool IsMergeBlock();
    __forceinline bool IsInsertDeleteSpace();
    __forceinline bool IsDrawLargeBrush();
    __forceinline bool IsAddText();
    __forceinline bool IsMouseButtonDown();
    __forceinline float ScaleOffset(float value, float scale);
    __forceinline void GetRotateFactors(float degree, float &cos_theta, float &sin_theta);
    __forceinline float GetResizeRatio();
    __forceinline AC_EditMode GetRefineBoundaryMode();

    __forceinline bool IsChar(UINT input_char, UINT test_char);
    __forceinline CPoint cPoint(const HyPoint &hy_point);
    __forceinline HyPoint2D32f VP2IP(const HyPoint2D32f &point);
    __forceinline HyPoint2D32f VP2IP(const CPoint &point);
    __forceinline HyPoint2D32f FIP2IP(const HyPoint2D32f &point);
    __forceinline HyPoint2D32f IP2FIP(const HyPoint2D32f &point);

    __forceinline void GetTwoColorLinePoints(const HyPoint &p1, const HyPoint &p2,
                                             HyPoint &outer_p1, HyPoint &outer_p2,
                                             HyPoint &inner_p1, HyPoint &inner_p2);

    CRect GetClientRectInScreen();

    void ImageResize(const HyImage *p_src_image, HyImage *p_dst_image);
    void UpdateImageSizeInfo();
    void UpdateImageROIInfo();
    void SetEnableImageUI();

    void MakeTransformedImage(float scale_x, float scale_y,
                              float rotate_degree, HyPoint2D32f &rotate_shift);
    void AdjustImageROI(float scale_x, float scale_y,
                        float rotate_degree, float center_x, float center_y);

    void MakeAnsiImage();
    void UpdateAnsiImage(const HyRect &redraw_rect);

    void MakeViewImage();
    void MakeViewImage(const HyRect &redraw_rect);

    void UpdateDisplayOffset();

    void SaveDebugImage(HyImage *p_image, LPCTSTR format, ...);

    void StartDrag(const CPoint &point);
    void EndDrag();
    bool IsDragging() const;
    bool IsProcessDrag(int curr_time) const;
    void UpdateDragInfo(const CPoint &curr_point, const int curr_time);

    bool CheckToKillTimer(UINT_PTR evend_id, bool &is_enable);

    void DrawInfoImage(Graphics &graphics, int center_x, int center_y);
    void DrawBlinkRegion(CDC *p_dc, Graphics &graphics, const HyImage *p_original_image);
    void DrawEditingPattern(CDC *p_dc, Graphics &graphics);
    void DrawCanvasFrame(CDC *p_dc, Graphics &graphics);
    void DrawCanvasRowInfos(CDC *p_dc, Graphics &graphics);
    
    void DrawSolidLine(CDC *p_dc, const HyPoint &p1, const HyPoint &p2, COLORREF color, int thickness);
    void DrawTwoColorLine(CDC *p_dc, const HyPoint &p1, const HyPoint &p2, COLORREF inner_color, COLORREF outer_color);
    void DrawTriangleEditBoundary(CDC *p_dc, const HyPoint &start1, const HyPoint &end1, bool is_have_line2,
                                  const HyPoint &start2, const HyPoint &end2, COLORREF inner_color, COLORREF outer_color);
    void DrawRect(CDC *p_dc, const HyRect &rect, COLORREF color, int thickness);
    void DrawTwoColorRect(CDC *p_dc, const HyRect &rect, COLORREF inner_color, COLORREF outer_color);
    void DrawArrow(CDC *p_dc, const HyPoint &tail, const HyPoint &head,
                   COLORREF color, int thickness, float head_length_ratio = 0.40f, float head_angle_degree = 30.0f);
    void DrawTwoColorDiagonalArrow(CDC *p_dc, const HyPoint &head, const HyPoint &diagonal_vector,
                                   const HyPoint &hori_vector, const HyPoint &vert_vector,
                                   COLORREF inner_color, COLORREF outer_color);
    void DrawImageToView(Graphics &graphics, HyImage *p_image, const HyPoint &offset);
    void DrawRegionContour(CDC *p_dc, const std::vector<HyPoint> &region, COLORREF color);
    void ShrinkContourForDrawing(std::vector<HyPoint> &contour);
    void DrawPromptString(const HyRect &text_roi, Graphics &graphics);

    void UpdateViewAnsiInfo(int x, int y);
    void UpdateDrawSpaceInfo(int x, int y);
    void UpdateChangeColorInfo(int x, int y);
    void UpdateRefineBoundaryInfo(int x, int y);
    void UpdateDrawBlockInfo(int x, int y);
    void UpdateDrawSmallSquareInfo(int x, int y);
    void UpdateMergeBlockInfo(int x, int y);
    void UpdateInsertDeleteSpaceInfo(int x, int y);
    void UpdateDrawLargeBrushInfo(int x, int y);
    void UpdateAddTextInfo(int x, int y);
    void UpdateInfoImage(const AnsiCell &cell, const AnsiColor &color);

    void ProcessMouseWheelForMoveImage(short delta, const CPoint &pt);
    void ProcessMouseWheelForDrawLargeBrush(short delta, const CPoint &pt);

    // Drawing actions on canvas with Undo/Redo management.
    void UpdateActionHistory(bool is_change_action_list = true);
    void DrawBlockAction(const HyPoint &point, const AnsiCell &cell,
                         const AnsiColor &color, HyRect &change_rect);
    void DrawSpaceAction(const HyPoint &start_point, const HyPoint &end_point,
                         int space_color, bool is_continuous, HyRect &change_rect);
    bool DrawSmallSquareAction(const HyPoint &start_point, const HyPoint &end_point,
                               int color, bool is_continuous, HyRect &change_rect);
    void DrawLargeBrushAction(const HyPoint &start_point, const HyPoint &end_point,
                              int brush_level, int brush_shape,
                              int color, bool is_continuous, HyRect &change_rect);
    bool StartRefineBoundaryAction(const HyPoint &point, AC_EditMode mode,
                                   AC_EditInfo &edit_info, HyRect &change_rect);
    void ContinueRefineBoundaryAction(const HyPoint &point, AC_EditInfo &edit_info, HyRect &change_rect);
    void InsertDeleteSpaceAction(const HyPoint &point, int space_color, HyRect &change_rect);
    bool MergeBlockAction(const HyPoint &point, HyRect &redraw_rect, bool is_force_merge);
    bool AddTextAction(const HyPoint &point, LPCTSTR text_string, int text_color, HyRect &change_rect);
    bool ChangeColorAction(const HyPoint &start_point, const HyPoint &end_point, 
                           const AnsiColor &color, bool is_continuous, HyRect &change_rect);
    bool ChangeColorAreaAction(const HyPoint &seed, const AnsiColor &color, HyRect &change_rect);
};