#include "stdafx.h"
#include "ImageCraft.h"
#include "MainFrm.h"
#include "ImageCraftView.h"
#include "TextInputDialog.h"

#define _DRAG_MOVE_PROCESS_INTERVAL   20 //ms

const UINT_PTR TIMER_DRAW_BLINK_REGION = 100;
const int DRAW_BLINK_REGION_DURATION_MS = 100;
const int MAX_DRAW_BLINK_REGION_PROGRESS = 10;

IMPLEMENT_DYNCREATE(ImageCraftView, CWnd)

ImageCraftView::ImageCraftView()
{
    mp_full_image = NULL;
    mp_scaled_image = NULL;
    mp_image = NULL;

    mp_view_image = hyCreateImage(hySize(1, 1), HY_DEPTH_8U, 3);
    mp_ansi_image = hyCreateImage(hySize(1, 1), HY_DEPTH_8U, 3);
    mp_info_image = NULL;
    mp_prompt_string_image = NULL;

    m_is_left_button_down = false;
    m_is_right_button_down = false;
    m_is_press_control = false;
    m_is_press_shift = false;
    m_is_press_control_for_combo = false;
    m_is_press_shift_for_combo = false;

    m_is_alternated_edit = false;

    m_is_start_drag = false;
    m_is_pressed_shift_when_start_drag = false;
    m_prev_drag_time = -1;
    m_is_need_reset_leave_event = true;

    m_draw_mode = IC_VIEW_ANSI;

    m_current_color.fg_color = ANSI_DEFAULT_FOREGROUND;
    m_current_color.bg_color = ANSI_DEFAULT_BACKGROUND;

    m_is_during_refine_boundary = false;
    m_is_during_add_text = false;

    m_is_enable_draw_blink_region = false;
    m_draw_blink_region_progress = 0;
    m_is_draw_blink_region_progress_increase = false;
}

ImageCraftView::~ImageCraftView()
{
    hyReleaseImage(&mp_full_image);
    hyReleaseImage(&mp_scaled_image);
    hyReleaseImage(&mp_image);
    hyReleaseImage(&mp_view_image);
    hyReleaseImage(&mp_ansi_image);
    hyReleaseImage(&mp_info_image);
    hyReleaseImage(&mp_prompt_string_image);
}

__forceinline bool ImageCraftView::IsMoveImage()
{
    return theApp.m_is_move_image;
}

__forceinline bool ImageCraftView::IsViewAnsi()
{
    return (IsMoveImage() == false && m_draw_mode == IC_VIEW_ANSI);
}

__forceinline bool ImageCraftView::IsDrawSpaces()
{
    return (IsMoveImage() == false && m_draw_mode == IC_DRAW_SPACES);
}

__forceinline bool ImageCraftView::IsChangeColor()
{
    return (IsMoveImage() == false && m_draw_mode == IC_CHANGE_COLOR);
}

__forceinline bool ImageCraftView::IsRefineBoundary()
{
    return (IsMoveImage() == false && m_draw_mode == IC_REFINE_BOUNDARY);
}

__forceinline bool ImageCraftView::IsDrawBlock()
{
    return (IsMoveImage() == false && m_draw_mode == IC_DRAW_BLOCK);
}

__forceinline bool ImageCraftView::IsDrawSmallSquare()
{
    return (IsMoveImage() == false && m_draw_mode == IC_DRAW_SMALL_SQUARE);
}

__forceinline bool ImageCraftView::IsMergeBlock()
{
    return (IsMoveImage() == false && m_draw_mode == IC_MERGE_BLOCK);
}

__forceinline bool ImageCraftView::IsInsertDeleteSpace()
{
    return (IsMoveImage() == false && m_draw_mode == IC_INSERT_DELETE_SPACE);
}

__forceinline bool ImageCraftView::IsDrawLargeBrush()
{
    return (IsMoveImage() == false && m_draw_mode == IC_DRAW_LARGE_BRUSH);
}

__forceinline bool ImageCraftView::IsAddText()
{
    return (IsMoveImage() == false && m_draw_mode == IC_ADD_TEXT);
}

__forceinline bool ImageCraftView::IsMouseButtonDown()
{
    return m_is_left_button_down || m_is_right_button_down;
}

__forceinline float ImageCraftView::ScaleOffset(float value, float scale)
{
    return (value + 0.5f) * scale - 0.5f;
}

__forceinline void ImageCraftView::GetRotateFactors(float degree, float &cos_theta, float &sin_theta)
{
    double radian = TO_RADIAN(degree);
    cos_theta = (float)cos(radian);
    sin_theta = (float)sin(radian);
}

__forceinline float ImageCraftView::GetResizeRatio()
{
    // Only consider x direction. Better for scale bounding.
    return m_scale_to_resized.x;
}

__forceinline AC_EditMode ImageCraftView::GetRefineBoundaryMode()
{
    AC_EditMode mode = AC_EDIT_REFINE_BOUNDARY;

    if (theApp.m_is_refine_triangle_mode)
    {
        if (m_is_alternated_edit)
            mode = AC_EDIT_REFINE_BOUNDARY_REGULAR_TRIANGLE;
        else
            mode = AC_EDIT_REFINE_BOUNDARY_TRIANGLE;
    }
    else
    {
        if (m_is_alternated_edit)
            mode = AC_EDIT_REFINE_BOUNDARY_HALF;
        else
            mode = AC_EDIT_REFINE_BOUNDARY;
    }

    return mode;
}

__forceinline bool ImageCraftView::IsChar(UINT input_char, UINT test_char)
{
    if (input_char == test_char)
        return true;

    if (test_char >= 'A' && test_char <= 'Z')
        if (input_char == test_char + 32)
            return true;

    if (test_char >= 'a' && test_char <= 'z')
        if (input_char == test_char - 32)
            return true;

    return false;
}

__forceinline CPoint ImageCraftView::cPoint(const HyPoint &hy_point)
{
    CPoint point(hy_point.x, hy_point.y);
    return point;
}

__forceinline HyPoint2D32f ImageCraftView::VP2IP(const HyPoint2D32f &point)
{
    float dst_x = point.x + (float)m_image_roi.x;
    float dst_y = point.y + (float)m_image_roi.y;

    return hyPoint2D32f(dst_x, dst_y);
}

__forceinline HyPoint2D32f ImageCraftView::VP2IP(const CPoint &point)
{
    return VP2IP(hyPoint2D32f((float)point.x, (float)point.y));
}

__forceinline HyPoint2D32f ImageCraftView::FIP2IP(const HyPoint2D32f &point)
{
    // Transform a point on the source image to the scaled and rotated image:
    // 1. Scale the point by the source-to-scale factors.
    // 2. Rotate the point with (0, 0) as center.
    // 3. Shift the point by the shift offset of rotated image.

    float scaled_x = ScaleOffset(point.x, m_scale_to_resized.x);
    float scaled_y = ScaleOffset(point.y, m_scale_to_resized.y);

    float cos_theta = 1.0f;
    float sin_theta = 0.0f;
    GetRotateFactors(theApp.m_image_rotate_degree, cos_theta, sin_theta);

    float rotated_x = scaled_x * cos_theta - scaled_y * sin_theta;
    float rotated_y = scaled_x * sin_theta + scaled_y * cos_theta;

    float dst_x = rotated_x + m_rotated_image_shift.x;
    float dst_y = rotated_y + m_rotated_image_shift.y;

    return hyPoint2D32f(dst_x, dst_y);
}

__forceinline HyPoint2D32f ImageCraftView::IP2FIP(const HyPoint2D32f &point)
{
    // Transform a point on the rotated image back to source image:
    // 1. Shift the point back by the shift offset of rotated image.
    // 2. Rotate the point back with (0, 0) as center.
    // 3. Scale the point by the scale-to-source factors.

    float rotated_x = point.x - m_rotated_image_shift.x;
    float rotated_y = point.y - m_rotated_image_shift.y;

    float cos_theta = 1.0f;
    float sin_theta = 0.0f;
    GetRotateFactors(theApp.m_image_rotate_degree, cos_theta, sin_theta);

    float scaled_x =  rotated_x * cos_theta + rotated_y * sin_theta;
    float scaled_y = -rotated_x * sin_theta + rotated_y * cos_theta;

    float dst_x = ScaleOffset(scaled_x, m_scale_to_full.x);
    float dst_y = ScaleOffset(scaled_y, m_scale_to_full.y);

    return hyPoint2D32f(dst_x, dst_y);
}

__forceinline void ImageCraftView::GetTwoColorLinePoints(const HyPoint &p1, const HyPoint &p2,
                                                         HyPoint &outer_p1, HyPoint &outer_p2,
                                                         HyPoint &inner_p1, HyPoint &inner_p2)
{
    outer_p1 = p1;
    outer_p2 = p2;
    inner_p1 = p1;
    inner_p2 = p2;

    if (p1.x < p2.x)
        inner_p2.x++;
    else if (p1.x > p2.x)
        inner_p2.x--;

    if (p1.y < p2.y)
        inner_p2.y++;
    else if (p1.y > p2.y)
        inner_p2.y--;
}

CRect ImageCraftView::GetClientRectInScreen()
{
    CRect client_rect;
    GetClientRect(client_rect);

    CPoint left_top(client_rect.left, client_rect.top);
    CPoint right_bottom(client_rect.right, client_rect.bottom);
    ClientToScreen(&left_top);
    ClientToScreen(&right_bottom);

    return CRect(left_top.x, left_top.y, right_bottom.x, right_bottom.y);
}

BEGIN_MESSAGE_MAP(ImageCraftView, CWnd)
    ON_WM_TIMER()
    ON_WM_DESTROY()
    ON_WM_SETFOCUS()
    ON_WM_KILLFOCUS()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSELEAVE()
    ON_WM_MOUSEWHEEL()
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
    ON_WM_PAINT()
END_MESSAGE_MAP()


BOOL ImageCraftView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void ImageCraftView::OnTimer(UINT_PTR event_id)
{
    if (event_id == TIMER_DRAW_BLINK_REGION)
    {
        if (m_is_draw_blink_region_progress_increase)
        {
            m_draw_blink_region_progress++;
            if (m_draw_blink_region_progress == MAX_DRAW_BLINK_REGION_PROGRESS)
                m_is_draw_blink_region_progress_increase = false;
        }
        else
        {
            m_draw_blink_region_progress--;
            if (m_draw_blink_region_progress == 0)
                m_is_draw_blink_region_progress_increase = true;
        }

        Invalidate(FALSE);
    }

    CWnd::OnTimer(event_id);
}

bool ImageCraftView::CheckToKillTimer(UINT_PTR evend_id, bool &is_enable)
{
    if (is_enable == false)
        return false;

    KillTimer(evend_id);
    is_enable = false;

    return true;
}

void ImageCraftView::OnDestroy() 
{
    CheckToKillTimer(TIMER_DRAW_BLINK_REGION, m_is_enable_draw_blink_region);

    std_tstring project_path = GetAutoSaveProjectPath();

    theApp.GetMainFrame()->SaveAnsiProject(project_path, theApp.m_image_path);

	CWnd::OnDestroy();
}

void ImageCraftView::OnSetFocus(CWnd* pOldWnd)
{
    CWnd::OnSetFocus(pOldWnd);
    
    
}

void ImageCraftView::OnKillFocus(CWnd* pWnd)
{
    CWnd::OnKillFocus(pWnd);

    CheckToKillTimer(TIMER_DRAW_BLINK_REGION, m_is_enable_draw_blink_region);

    m_is_left_button_down = false;
    m_is_right_button_down = false;
    m_is_press_control = false;
    m_is_press_shift = false;

    EndDrag();
    if (GetCapture() == this)
        ReleaseCapture();

    if (IsRefineBoundary())
    {
        if (m_is_during_refine_boundary)
        {
            m_ansi_canvas.EndRefineBoundary();
            m_is_during_refine_boundary = false;
        }
    }
}

BOOL ImageCraftView::OnEraseBkgnd(CDC* pDC)
{
	//	The DIB Surface handles erasing the background.
	return TRUE;
}

void ImageCraftView::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    if (cx <= 0 || cy <= 0)
        return;

    CRect client_rect;
    GetClientRect(&client_rect);
    int width = client_rect.Width();
    int height = client_rect.Height();

    hyReleaseImage(&mp_view_image);
    hyReleaseImage(&mp_ansi_image);

    mp_view_image = hyCreateImage(hySize(width, height), HY_DEPTH_8U, 3);
    mp_ansi_image = hyCreateImage(hySize(width, height), HY_DEPTH_8U, 3);

    m_image_roi.width = width;
    m_image_roi.height = height;
    UpdateImageROIInfo();

    MakeAnsiImage();
}

void ImageCraftView::InitPainter(LPCTSTR image_path, LPCTSTR ansi_path, const OldFormatData &old_format_data)
{
    SetFocus();

    LoadImageFile(image_path, true, old_format_data);
    LoadAnsi(ansi_path);
}

void ImageCraftView::StartDrag(const CPoint &point)
{
    m_start_drag_pt = point;
    m_prev_drag_time = -1;
    m_is_alternated_edit = m_is_press_shift;

    m_is_start_drag = true;
    m_is_pressed_shift_when_start_drag = m_is_press_shift;
}

void ImageCraftView::EndDrag()
{
    m_start_drag_pt = CPoint(0, 0);
    m_prev_drag_time = -1;
    m_is_alternated_edit = m_is_press_shift;

    m_is_start_drag = false;
    m_is_pressed_shift_when_start_drag = false;
}

bool ImageCraftView::IsDragging() const
{
    return m_is_start_drag;
}

bool ImageCraftView::IsProcessDrag(int curr_time) const
{
    return (m_prev_drag_time == -1) || ((curr_time - m_prev_drag_time) > _DRAG_MOVE_PROCESS_INTERVAL);
}

void ImageCraftView::UpdateDragInfo(const CPoint &curr_point, const int curr_time)
{
    m_start_drag_pt = curr_point;
    m_prev_drag_time = curr_time;
}

void ImageCraftView::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetCapture();

    m_is_left_button_down = true;
    if (m_is_right_button_down)
    {
        EndDrag();
        CWnd::OnLButtonDown(nFlags, point);
        return;
    }
    
    StartDrag(point);

    bool is_update_ansi = false;
    bool is_invalidate = false;
    HyRect redraw_rect(0, 0, 0, 0);

    HyPoint hy_point = hyPoint(point.x, point.y);
    int block_size = (int)theApp.m_block_size;
    if (IsDrawSpaces())
    {
        DrawSpaceAction(hy_point, hy_point, m_current_color.fg_color, false, redraw_rect);

        is_update_ansi = true;
        is_invalidate = true;
    }
    else if (IsDrawLargeBrush())
    {
        DrawLargeBrushAction(hy_point, hy_point,
                             theApp.m_large_brush_size, theApp.m_large_brush_shape,
                             m_current_color.fg_color, false, redraw_rect);

        is_update_ansi = true;
        is_invalidate = true;
    }
    else if (IsInsertDeleteSpace())
    {
        InsertDeleteSpaceAction(hy_point, m_current_color.fg_color, redraw_rect);

        is_update_ansi = true;
        is_invalidate = true;
    }
    else if (IsDrawBlock())
    {
        AnsiCell block_cell = theApp.GetMainFrame()->GetMainDialog()->GetSelectedBlock();
        DrawBlockAction(hy_point, block_cell, m_current_color, redraw_rect);

        is_update_ansi = true;
        is_invalidate = true;
    }
    else if (IsDrawSmallSquare())
    {
        DrawSmallSquareAction(hy_point, hy_point, m_current_color.fg_color, false, redraw_rect);
        
        is_update_ansi = true;
        is_invalidate = true;
    }
    else if (IsChangeColor())
    {
        bool is_success = false;
        if (theApp.m_is_change_color_area_mode)
            is_success = ChangeColorAreaAction(hy_point, m_current_color, redraw_rect);
        else
            is_success = ChangeColorAction(hy_point, hy_point, m_current_color, false, redraw_rect);

        if (is_success)
        {
            is_update_ansi = true;
            is_invalidate = true;
        }
    }
    else if (IsMergeBlock())
    {
        bool is_success = MergeBlockAction(hy_point, redraw_rect, m_is_alternated_edit);
        if (is_success)
        {
            is_update_ansi = true;
            is_invalidate = true;
        }
    }
    else if (IsRefineBoundary())
    {
        m_is_during_refine_boundary = StartRefineBoundaryAction(hy_point, GetRefineBoundaryMode(), m_edit_info, redraw_rect);
        if (m_is_during_refine_boundary)
        {
            is_update_ansi = true;
            is_invalidate = true;
        }
    }

    if (is_update_ansi)
        UpdateAnsiImage(redraw_rect);
    if (is_invalidate)
        Invalidate(FALSE);

    CWnd::OnLButtonDown(nFlags, point);
}

void ImageCraftView::OnLButtonUp(UINT nFlags, CPoint point)
{
    CPoint start_pt = m_start_drag_pt;
    bool is_valid_drag = m_is_start_drag && (start_pt != point);

    m_is_left_button_down = false;

    EndDrag();
    if (GetCapture() == this)
        ReleaseCapture();

    bool is_update_ansi = false;
    bool is_invalidate = false;
    HyRect redraw_rect(0, 0, 0, 0);

    if (IsRefineBoundary())
    {
        if (m_is_during_refine_boundary)
        {
            m_ansi_canvas.EndRefineBoundary();
            m_is_during_refine_boundary = false;
        }

        UpdateRefineBoundaryInfo(point.x, point.y);
        is_invalidate = true;
    }
    else if (IsAddText())
    {
        if (m_edit_info.is_valid)
        {
            CRect screen_rect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
            HMONITOR monitor_handle = MonitorFromRect(screen_rect, MONITOR_DEFAULTTONEAREST);
            if (monitor_handle != NULL)
            {
                MONITORINFOEX monitor_info;
                monitor_info.cbSize = sizeof(monitor_info);
                if (GetMonitorInfo(monitor_handle, &monitor_info))
                    screen_rect = monitor_info.rcWork;
            }

            CRect view_rect;
            GetClientRect(view_rect);
            ClientToScreen(&view_rect);

            HyPoint input_text_offset = hyPoint(m_edit_info.target_rect.x, m_edit_info.target_rect.y);

            TextInputDialog dialog;
            dialog.SetLayoutInfo(screen_rect, view_rect, input_text_offset, theApp.m_block_size);

            if (dialog.DoModal() == IDOK)
            {
                std_tstring input_string = dialog.GetInputText();

                std::vector<UnicodeTextInfo> string_info = AnalyzeUnicodeString(input_string);

                std::vector<std::wstring> invalid_text_list;
                std::wstring valid_string = GetValidStringForANSIFile(string_info, invalid_text_list);

                const int invalid_text_count = (int)invalid_text_list.size();
                if (invalid_text_count)
                {
                    const int max_list_count = 8;
                    const int list_count = ch_Min(invalid_text_count, max_list_count);

                    std::wstring invalid_text_message;
                    for (int i = 0; i < list_count; i++)
                    {
                        if (i > 0)
                            invalid_text_message += _T("、");

                        invalid_text_message += invalid_text_list[i];
                    }

                    if (list_count < invalid_text_count)
                        invalid_text_message += _T("……");

                    std_tstring message_string;
                    message_string += _T("下列字元無法在BBS中顯示：\n\n");
                    message_string += invalid_text_message;
                    message_string += _T("\n\n");
                    message_string += _T("這些字元將各自替換為兩個連續的半形空白。");

                    MessageBox(message_string.c_str(), _T("部分字元無法顯示"), MB_OK | MB_ICONWARNING);
                }

                bool is_success = AddTextAction(input_text_offset, valid_string.c_str(), m_current_color.fg_color, redraw_rect);
                if (is_success)
                    is_update_ansi = true;
            }
        }

        UpdateAddTextInfo(point.x, point.y);
        is_invalidate = true;
    }

    if (is_update_ansi)
        UpdateAnsiImage(redraw_rect);
    if (is_invalidate)
        Invalidate(FALSE);

    CWnd::OnLButtonUp(nFlags, point);
}

void ImageCraftView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    bool is_update_ansi = false;
    bool is_invalidate = false;
    HyRect redraw_rect(0, 0, 0, 0);

    HyPoint hy_point = hyPoint(point.x, point.y);

    // Only process actions that may need to do at the same location (and give different results).
    if (IsInsertDeleteSpace())
    {
        InsertDeleteSpaceAction(hy_point, m_current_color.fg_color, redraw_rect);

        is_update_ansi = true;
        is_invalidate = true;
    }

    if (is_update_ansi)
        UpdateAnsiImage(redraw_rect);
    if (is_invalidate)
        Invalidate(FALSE);

    CWnd::OnLButtonDblClk(nFlags, point);
}

void ImageCraftView::OnRButtonDown(UINT nFlags, CPoint point)
{
    SetCapture();

    m_is_right_button_down = true;
    if (m_is_left_button_down)
    {
        EndDrag();
        CWnd::OnRButtonDown(nFlags, point);
        return;
    }
    
    StartDrag(point);

    bool is_invalidate = false;
    if (IsDrawSpaces() || IsDrawSmallSquare() ||
        IsChangeColor() || IsInsertDeleteSpace() || IsDrawLargeBrush())
    {
        AnsiColor color;
        bool is_success = m_ansi_canvas.GetChangeableColorAtLocation(point.x, point.y, color);
        if (is_success)
            theApp.GetMainFrame()->GetMainDialog()->SetColor(color);
    }
    else if (IsDrawBlock())
    {
        AnsiColor color;
        bool is_success = m_ansi_canvas.GetColorAtLocation(point.x, point.y, color);
        if (is_success)
            theApp.GetMainFrame()->GetMainDialog()->SetColor(color);
    }

    if (is_invalidate)
        Invalidate(FALSE);

    CWnd::OnRButtonDown(nFlags, point);
}

void ImageCraftView::OnRButtonUp(UINT nFlags, CPoint point)
{
    CPoint start_pt = m_start_drag_pt;
    bool is_valid_drag = m_is_start_drag && (start_pt != point);

    m_is_right_button_down = false;

    EndDrag();
    if (GetCapture() == this)
        ReleaseCapture();

    CWnd::OnRButtonUp(nFlags, point);
}

void ImageCraftView::OnMouseMove(UINT nFlags, CPoint point)
{
    m_current_mouse_point = point;

    HyRect view_rect = hyRect(0, 0, mp_view_image->width, mp_view_image->height);
    if (view_rect.IsPtInRect(hyPoint(point.x, point.y)))
        SetFocus();

    m_current_drag_pt = point;

    if (m_is_need_reset_leave_event)
    {
        TRACKMOUSEEVENT leave_event;
        leave_event.cbSize = sizeof(TRACKMOUSEEVENT);
        leave_event.dwFlags = TME_LEAVE;
        leave_event.hwndTrack = this->m_hWnd;
        TrackMouseEvent(&leave_event);

        m_is_need_reset_leave_event = false;
    }

    bool is_update_ansi = false;
    bool is_update_view = false;
    bool is_invalidate = false;
    HyRect redraw_rect(0, 0, 0, 0);
    HyPoint hy_point(point.x, point.y);

    if (IsDragging())
    {
        int curr_time = clock();
        if (IsProcessDrag(curr_time))
        {
            if (IsMoveImage())
            {
                if (IsImageLoaded())
                {
                    if (m_is_pressed_shift_when_start_drag)
                    {
                        // Change the rotate degree according to the current point and start drag point.
                        // The rotate center is the center of current view image.
                        float center_x = (float)mp_view_image->width * 0.5f;
                        float center_y = (float)mp_view_image->height * 0.5f;
                        
                        float start_degree = GetSafeDegree(m_start_drag_pt.x - center_x, m_start_drag_pt.y - center_y);
                        float current_degree = GetSafeDegree(point.x - center_x, point.y - center_y);
                        float degree_difference = ValidDegree(current_degree - start_degree);

                        float new_degree = ValidDegree(theApp.m_image_rotate_degree + degree_difference);
                        ChangeRotateDegree(new_degree);
                    }
                    else
                    {
                        m_image_offset.x -= (float)(point.x - m_start_drag_pt.x);
                        m_image_offset.y -= (float)(point.y - m_start_drag_pt.y);
                        UpdateImageROIInfo();
                    }
                
                    is_update_view = true;
                }
            }
            else
            {
                if (IsViewAnsi())
                {
                    UpdateViewAnsiInfo(point.x, point.y);
                }
                else if (IsDrawSpaces())
                {
                    UpdateDrawSpaceInfo(point.x, point.y);

                    if (m_is_left_button_down)
                    {
                        DrawSpaceAction(hyPoint(m_start_drag_pt.x, m_start_drag_pt.y),
                                        hy_point, m_current_color.fg_color, true, redraw_rect);

                        is_update_ansi = true;
                    }
                }
                else if (IsDrawLargeBrush())
                {
                    UpdateDrawLargeBrushInfo(point.x, point.y);

                    if (m_is_left_button_down)
                    {
                        DrawLargeBrushAction(hyPoint(m_start_drag_pt.x, m_start_drag_pt.y), hy_point,
                                             theApp.m_large_brush_size, theApp.m_large_brush_shape,
                                             m_current_color.fg_color, true, redraw_rect);

                        is_update_ansi = true;
                    }
                }
                else if (IsChangeColor())
                {
                    UpdateChangeColorInfo(point.x, point.y);

                    if (m_is_left_button_down)
                    {
                        // Don't apply "area mode" change color during dragging.
                        bool is_success = false;
                        if (theApp.m_is_change_color_area_mode == false)
                        {
                            is_success = ChangeColorAction(hyPoint(m_start_drag_pt.x, m_start_drag_pt.y), 
                                                           hy_point, m_current_color, true, redraw_rect);
                        }

                        if (is_success)
                            is_update_ansi = true;
                    }
                }
                else if (IsRefineBoundary())
                {
                    if (m_is_left_button_down)
                    {
                        ContinueRefineBoundaryAction(hy_point, m_edit_info, redraw_rect);
                        is_update_ansi = true;
                    }
                }
                else if (IsDrawBlock())
                {
                    UpdateDrawBlockInfo(point.x, point.y);
                }
                else if (IsDrawSmallSquare())
                {
                    UpdateDrawSmallSquareInfo(point.x, point.y);

                    if (m_is_left_button_down)
                    {
                        DrawSmallSquareAction(hyPoint(m_start_drag_pt.x, m_start_drag_pt.y), hy_point, 
                                              m_current_color.fg_color, true, redraw_rect);
                        is_update_ansi = true;
                    }
                }
                else if (IsMergeBlock())
                {
                    UpdateMergeBlockInfo(point.x, point.y);
                }
                else if (IsInsertDeleteSpace())
                {
                    UpdateInsertDeleteSpaceInfo(point.x, point.y);
                }
            }

            UpdateDragInfo(point, curr_time);
            is_invalidate = true;
        }
    }
    else
    {
        if (IsViewAnsi())
        {
            UpdateViewAnsiInfo(point.x, point.y);
            is_invalidate = true;
        }
        else if (IsDrawSpaces())
        {
            UpdateDrawSpaceInfo(point.x, point.y);
            is_invalidate = true;
        }
        else if (IsChangeColor())
        {
            UpdateChangeColorInfo(point.x, point.y);
            is_invalidate = true;
        }
        else if (IsRefineBoundary())
        {
            UpdateRefineBoundaryInfo(point.x, point.y);
            is_invalidate = true;
        }
        else if (IsDrawBlock())
        {
            UpdateDrawBlockInfo(point.x, point.y);
            is_invalidate = true;
        }
        else if (IsDrawSmallSquare())
        {
            UpdateDrawSmallSquareInfo(point.x, point.y);
            is_invalidate = true;
        }
        else if (IsMergeBlock())
        {
            UpdateMergeBlockInfo(point.x, point.y);
            is_invalidate = true;
        }
        else if (IsInsertDeleteSpace())
        {
            UpdateInsertDeleteSpaceInfo(point.x, point.y);
            is_invalidate = true;
        }
        else if (IsDrawLargeBrush())
        {
            UpdateDrawLargeBrushInfo(point.x, point.y);
            is_invalidate = true;
        }
        else if (IsAddText())
        {
            UpdateAddTextInfo(point.x, point.y);
            is_invalidate = true;
        }
    }

    if (is_update_ansi)
        UpdateAnsiImage(redraw_rect);
    else if (is_update_view)
        UpdateViewImage();

    if (is_invalidate)
        Invalidate(FALSE);

    CWnd::OnMouseMove(nFlags, point);
}

void ImageCraftView::OnMouseLeave()
{
    m_is_need_reset_leave_event = true;

    CWnd::OnMouseLeave();
}

BOOL ImageCraftView::OnMouseWheel(UINT flags, short delta, CPoint pt)
{
    if (delta != 0)
    {
        if (IsMoveImage())
        {
            if (mp_full_image != NULL)
                ProcessMouseWheelForMoveImage(delta, pt);
        }
        else
        {
            if (IsDrawLargeBrush() && IsDragging() == false)
                ProcessMouseWheelForDrawLargeBrush(delta, pt);
        }
    }

    return CWnd::OnMouseWheel(flags, delta, pt);
}

void ImageCraftView::ProcessMouseWheelForMoveImage(short delta, const CPoint &pt)
{
    float ratio = GetResizeRatio();

    const float ratio_step = 0.02f;
    float new_ratio = ratio;
    if (delta > 0)
        new_ratio += ratio_step;
    else
        new_ratio -= ratio_step;

    new_ratio = FitInRange(new_ratio, MIN_IMAGE_SCALE_RATIO, MAX_IMAGE_SCALE_RATIO);

    CRect client_rect = GetClientRectInScreen();
    CPoint view_pt(pt.x - client_rect.left, pt.y - client_rect.top);

    AdjustImageROI(new_ratio, new_ratio, theApp.m_image_rotate_degree, view_pt.x + 0.5f, view_pt.y + 0.5f);

    MakeViewImage();
    Invalidate(FALSE);
}

void ImageCraftView::ProcessMouseWheelForDrawLargeBrush(short delta, const CPoint &pt)
{
    if (delta > 0)
        ChangeBrushSize(1);
    else if (delta < 0)
        ChangeBrushSize(-1);
}

void ImageCraftView::ChangeBrushSize(int amount)
{
    int new_size = theApp.m_large_brush_size + amount;
    new_size = FitInRange(new_size, MIN_LARGE_BRUSH_SIZE, MAX_LARGE_BRUSH_SIZE);

    SetBrushSize(new_size);
}

void ImageCraftView::SetBrushSize(int new_size)
{
    if (new_size < MIN_LARGE_BRUSH_SIZE || new_size > MAX_LARGE_BRUSH_SIZE)
        return;
    if (theApp.m_large_brush_size == new_size)
        return;

    theApp.m_large_brush_size = new_size;

    ImageCraftDialog *p_dialog = theApp.GetMainFrame()->GetMainDialog();
    if (p_dialog)
        p_dialog->UpdateBrushSizeUI();

    if (IsDrawLargeBrush())
    {
        UpdateDrawLargeBrushInfo(m_current_mouse_point.x, m_current_mouse_point.y);
        Invalidate(FALSE);
    }
}

void ImageCraftView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    ImageCraftDialog *p_dialog = theApp.GetMainFrame()->GetMainDialog();
    bool is_old_alternated_edit = m_is_alternated_edit;

    if (IsChar(nChar, 'V'))
    {
        if (p_dialog)
            p_dialog->SetDrawMode(IC_VIEW_ANSI);
    }
    else if (IsChar(nChar, 'Q'))
    {
        if (p_dialog)
            p_dialog->SetDrawMode(IC_DRAW_SPACES);
    }
    else if (IsChar(nChar, 'W'))
    {
        if (p_dialog)
            p_dialog->SetDrawMode(IC_DRAW_SMALL_SQUARE);
    }
    else if (IsChar(nChar, 'E'))
    {
        if (p_dialog)
            p_dialog->SetDrawMode(IC_CHANGE_COLOR);
    }
    else if (IsChar(nChar, 'R'))
    {
        if (p_dialog)
            p_dialog->SetDrawMode(IC_REFINE_BOUNDARY);
    }
    else if (IsChar(nChar, 'B'))
    {
        if (p_dialog)
            p_dialog->SetDrawMode(IC_DRAW_BLOCK);
    }
    else if (IsChar(nChar, 'D'))
    {
        if (p_dialog)
            p_dialog->SetDrawMode(IC_MERGE_BLOCK);
    }
    else if (IsChar(nChar, 'F'))
    {
        if (p_dialog)
            p_dialog->SetDrawMode(IC_INSERT_DELETE_SPACE);
    }
    else if (IsChar(nChar, 'A'))
    {
        if (p_dialog)
            p_dialog->SetDrawMode(IC_DRAW_LARGE_BRUSH);
    }
    else if (IsChar(nChar, 'S'))
    {
        if (p_dialog)
            p_dialog->SetDrawMode(IC_ADD_TEXT);
    }
    else if (IsChar(nChar, 'T'))
    {
        if (p_dialog)
        {
            int old_fg = m_current_color.fg_color;
            int old_bg = m_current_color.bg_color;

            int new_fg = old_bg;
            int new_bg = ToNormalColor(old_fg);
            if (IsBrightColor(old_fg))
                new_fg = ToBrightColor(new_fg);

            p_dialog->SetColor(AnsiColor(new_fg, new_bg));
            p_dialog->SetLastColorDrawMode();
        }
    }
    else if (IsChar(nChar, 'C'))
    {
        if (p_dialog)
        {
            p_dialog->SetHideImage(!theApp.m_is_hide_image);
        }
    }
    else if (nChar == VK_UP)
    {
        MoveDisplayOffset(0, -1);
    }
    else if (nChar == VK_DOWN)
    {
        MoveDisplayOffset(0, 1);
    }
    else if (nChar == VK_PRIOR)
    {
        MoveDisplayPageUp();
    }
    else if (nChar == VK_NEXT)
    {
        MoveDisplayPageDown();
    }
    else if (nChar == VK_CONTROL)
    {
        if (m_is_press_control == false)
        {
            m_is_press_control = true;
        }
    }
    else if (nChar == VK_SHIFT)
    {
        if (IsMouseButtonDown() == false)
            m_is_alternated_edit = true;

        if (m_is_press_shift == false)
        {
            m_is_press_shift = true;

            if (IsChangeColor() && theApp.m_is_change_color_area_mode)
            {
                CheckToKillTimer(TIMER_DRAW_BLINK_REGION, m_is_enable_draw_blink_region);

                SetTimer(TIMER_DRAW_BLINK_REGION, DRAW_BLINK_REGION_DURATION_MS, NULL);
                m_is_enable_draw_blink_region = true;
                m_draw_blink_region_progress = 0;
                m_is_draw_blink_region_progress_increase = true;
            }
        }
    }

    bool is_invalidate = false;
    if (m_is_alternated_edit != is_old_alternated_edit)
    {
        if (IsRefineBoundary())
        {
            UpdateRefineBoundaryInfo(m_current_mouse_point.x, m_current_mouse_point.y);
            is_invalidate = true;
        }
        else if (IsMergeBlock())
        {
            UpdateMergeBlockInfo(m_current_mouse_point.x, m_current_mouse_point.y);
            is_invalidate = true;
        }
    }

    if (is_invalidate)
        Invalidate(FALSE);

    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void ImageCraftView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    bool is_old_alternated_edit = m_is_alternated_edit;

    bool is_invalidate = false;

    if (nChar == VK_CONTROL)
    {
        if (m_is_press_control)
        {
            m_is_press_control = false;

            if (m_is_press_control_for_combo)
            {
                m_is_press_control_for_combo = false;
            }
            else
            {
                if (IsDrawLargeBrush())
                    ChangeBrushShape(1);
                else if (IsDrawBlock())
                    ChangeSelectBlockTable(1);
                else if (IsChangeColor())
                    SetChangeColorAreaMode(!theApp.m_is_change_color_area_mode);
                else if (IsRefineBoundary())
                    SetRefineTriangleMode(!theApp.m_is_refine_triangle_mode);
            }
        }
    }
    else if (nChar == VK_SHIFT)
    {
        if (IsMouseButtonDown() == false)
            m_is_alternated_edit = false;

        if (m_is_press_shift)
        {
            m_is_press_shift = false;

            if (CheckToKillTimer(TIMER_DRAW_BLINK_REGION, m_is_enable_draw_blink_region))
                is_invalidate = true;

            if (m_is_press_shift_for_combo)
            {
                m_is_press_shift_for_combo = false;
            }
            else
            {
                if (IsDrawLargeBrush())
                    ChangeBrushShape(-1);
                else if (IsDrawBlock())
                    ChangeSelectBlockTable(-1);
            }
        }
    }

    if (m_is_alternated_edit != is_old_alternated_edit)
    {
        if (IsRefineBoundary())
        {
            UpdateRefineBoundaryInfo(m_current_mouse_point.x, m_current_mouse_point.y);
            is_invalidate = true;
        }
        else if (IsMergeBlock())
        {
            UpdateMergeBlockInfo(m_current_mouse_point.x, m_current_mouse_point.y);
            is_invalidate = true;
        }
    }

    if (is_invalidate)
        Invalidate(FALSE);

    CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void ImageCraftView::AdjustImageROI(float scale_x, float scale_y,
                                    float rotate_degree, float center_x, float center_y)
{
    if (IsImageLoaded() == false)
        return;

    HyPoint2D32f old_image_point = VP2IP(hyPoint2D32f(center_x, center_y));
    HyPoint2D32f full_image_point = IP2FIP(old_image_point);

    MakeTransformedImage(scale_x, scale_y, rotate_degree, m_rotated_image_shift);

    // Solve ROI offset (x, y) so that the view point will match the full image point.
    HyPoint2D32f new_image_point = FIP2IP(full_image_point);

    m_image_offset.x = new_image_point.x - center_x;
    m_image_offset.y = new_image_point.y - center_y;

    UpdateImageROIInfo();
}

void ImageCraftView::ChangeResizeRatio(float ratio)
{
    float center_x = (float)mp_view_image->width * 0.5f;
    float center_y = (float)mp_view_image->height * 0.5f;

    AdjustImageROI(ratio, ratio, theApp.m_image_rotate_degree, center_x, center_y);

    MakeViewImage();
    Invalidate(FALSE);
}

void ImageCraftView::ChangeRotateDegree(float degree)
{
    float center_x = (float)mp_view_image->width * 0.5f;
    float center_y = (float)mp_view_image->height * 0.5f;

    AdjustImageROI(m_scale_to_resized.x, m_scale_to_resized.y, degree, center_x, center_y);

    MakeViewImage();
    Invalidate(FALSE);
}

bool ImageCraftView::IsImageLoaded()
{
    return (mp_full_image != NULL && mp_image != NULL);
};

void ImageCraftView::LoadImageFile(LPCTSTR image_path, bool is_use_init_roi,
                                   const OldFormatData &old_format_data)
{
    m_ansi_canvas.ResetDisplayOffset();
    m_ansi_canvas.GetDisplayOffset(theApp.m_display_x_offset, theApp.m_display_y_offset);

    HyImage *p_image = hyLoadImageByGdiplus(image_path);
    if (p_image)
    {
        theApp.m_image_path = image_path;

        hyReleaseImage(&mp_full_image);
        mp_full_image = p_image;

        int view_width = mp_view_image->width;
        int view_height = mp_view_image->height;
        int src_width = mp_full_image->width;
        int src_height = mp_full_image->height;

        bool is_have_init_roi = false;
        if (is_use_init_roi)
        {
            if (old_format_data.scaled_image_width > 0 && old_format_data.scaled_image_height > 0)
            {
                // Old data format. Convert them to new image transform format.
                theApp.m_image_scale_x = (float)(old_format_data.scaled_image_width) / src_width;
                theApp.m_image_scale_y = (float)(old_format_data.scaled_image_height) / src_height;

                // Rotation = 0. The ROI offset is the same as the one in old data.
                theApp.m_image_rotate_degree = 0.0f;
            }

            is_have_init_roi = (theApp.m_image_scale_x > 0.0f && theApp.m_image_scale_y > 0.0f);
        }

        m_image_roi.width = view_width;
        m_image_roi.height = view_height;

        float rotate_degree = 0.0f;
        float scale_x = 1.0f;
        float scale_y = 1.0f;

        if (is_have_init_roi)
        {
            rotate_degree = theApp.m_image_rotate_degree;
            scale_x = theApp.m_image_scale_x;
            scale_y = theApp.m_image_scale_y;
            m_image_offset = theApp.m_image_roi_offset;
        }
        else
        {
            float resize_ratio = 1.0f;
            if (src_width * view_height >= src_height * view_width)
                resize_ratio = (float)view_height / src_height;
            else
                resize_ratio = (float)view_width / src_width;

            resize_ratio = FitInRange(resize_ratio, MIN_IMAGE_SCALE_RATIO, MAX_IMAGE_SCALE_RATIO);

            scale_x = resize_ratio;
            scale_y = resize_ratio;
            rotate_degree = 0.0f;

            m_image_offset.x = (scale_x * src_width - view_width) * 0.5f;
            m_image_offset.y = (scale_y * src_height - view_height) * 0.5f;
        }

        // Release the scaled image, so MakeTransformedImage() will make it by new image.
        hyReleaseImage(&mp_scaled_image);

        MakeTransformedImage(scale_x, scale_y, rotate_degree, m_rotated_image_shift);
    }
    else
    {
        hyReleaseImage(&mp_full_image);
        hyReleaseImage(&mp_scaled_image);
        hyReleaseImage(&mp_image);

        m_image_roi = hyRect(0, 0, 0, 0);
        m_image_offset.x = 0.0f;
        m_image_offset.y = 0.0f;

        HY_ZEROIMAGE(mp_view_image);
        HY_ZEROIMAGE(mp_ansi_image);

        m_scale_to_full.Reset();
        m_scale_to_resized.Reset();
        theApp.m_image_rotate_degree = 0.0f;
        theApp.m_image_scale_x = 0.0f;
        theApp.m_image_scale_y = 0.0f;
        theApp.m_image_roi_offset.x = 0.0f;
        theApp.m_image_roi_offset.y = 0.0f;
    }

    UpdateImageSizeInfo();
    UpdateImageROIInfo();
    SetEnableImageUI();

    UpdateViewImage();
}

void ImageCraftView::MakeTransformedImage(float scale_x, float scale_y,
                                          float rotate_degree, HyPoint2D32f &rotate_shift)
{
    if (mp_full_image == NULL)
        return;

    const int src_width = mp_full_image->width;
    const int src_height = mp_full_image->height;
    const int scaled_width = ch_Max(ch_Round(src_width * scale_x), 1);
    const int scaled_height = ch_Max(ch_Round(src_height * scale_y), 1);

    bool is_need_resize = true;
    if (mp_scaled_image)
    {
        if (hyGetSize(mp_scaled_image) == hySize(scaled_width, scaled_height))
        {
            is_need_resize = false;
        }
    }

    if (is_need_resize)
    {
        hyReleaseImage(&mp_scaled_image);
        mp_scaled_image = hyCreateImage(hySize(scaled_width, scaled_height), HY_DEPTH_8U, mp_full_image->nChannels);

        ImageResize(mp_full_image, mp_scaled_image);

        m_scale_to_full.x = (float)src_width / scaled_width;
        m_scale_to_full.y = (float)src_height / scaled_height;
        m_scale_to_resized.x = (float)scaled_width / src_width;
        m_scale_to_resized.y = (float)scaled_height / src_height;
    }

    theApp.m_image_rotate_degree = rotate_degree;
    theApp.m_image_scale_x = scale_x;
    theApp.m_image_scale_y = scale_y;

    hyReleaseImage(&mp_image);
    mp_image = CreateRotatedImage(mp_scaled_image, rotate_degree, rotate_shift);
}

void ImageCraftView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
    CDC dc_mem;
    dc_mem.CreateCompatibleDC(&dc);
    CBitmap display_bmp;
    CRect display_rect;
    GetClientRect(&display_rect);
    display_bmp.CreateCompatibleBitmap(&dc, display_rect.Width(), display_rect.Height());
    CBitmap *p_old_bitmap = dc_mem.SelectObject(&display_bmp);

    CDC *p_dc = &dc_mem;

    /////////////////////////////////////////////////////////////////////
    //  NOTICE!!  You should use p_dc to paint

    Graphics graphics(p_dc->GetSafeHdc());
    Bitmap draw_image(mp_view_image->width, mp_view_image->height, 
                      mp_view_image->widthStep, PixelFormat24bppRGB, mp_view_image->imageData);

    graphics.DrawImage(&draw_image, Rect(0, 0, mp_view_image->width, mp_view_image->height),
                       0, 0, mp_view_image->width, mp_view_image->height, UnitPixel);

    DrawBlinkRegion(p_dc, graphics, mp_view_image);

    DrawEditingPattern(p_dc, graphics);

    DrawCanvasFrame(p_dc, graphics);

    if (IsViewAnsi() && m_edit_info.is_valid)
    {
        int center_x = m_edit_info.target_rect.x + m_edit_info.target_rect.width / 2;
        int center_y = m_edit_info.target_rect.y + m_edit_info.target_rect.height / 2;
        DrawInfoImage(graphics, center_x, center_y);
    }

    dc.BitBlt(0, 0, display_rect.Width(), display_rect.Height(), &dc_mem, 0, 0, SRCCOPY);
    dc_mem.SelectObject(p_old_bitmap);
}

void ImageCraftView::DrawInfoImage(Graphics &graphics, int center_x, int center_y)
{
    if (mp_info_image == NULL)
        return;

    int image_width = mp_info_image->width;
    int image_height = mp_info_image->height;
    int view_width = mp_view_image->width;
    int view_height = mp_view_image->height;
    int block_size = (int)theApp.m_block_size;

    HyRect draw_roi;
    draw_roi.width = image_width;
    draw_roi.height = image_height;

    draw_roi.x = FitInRange(center_x - image_width / 2, 0, view_width - image_width);
    draw_roi.y = center_y - block_size - image_height;
    if (draw_roi.y < 0)
        draw_roi.y = center_y + block_size;

    DrawImageToView(graphics, mp_info_image, hyPoint(draw_roi.x, draw_roi.y));
}

void ImageCraftView::DrawCanvasFrame(CDC *p_dc, Graphics &graphics)
{
    int block_size = (int)theApp.m_block_size;
    int canvas_width = m_canvas_unit_size.width * (block_size / 2);
    int canvas_height = m_canvas_unit_size.height * block_size;
    int view_width = mp_view_image->width;
    int view_height = mp_view_image->height;

    DrawCanvasRowInfos(p_dc, graphics);

    int thickness = 3;

    HyPoint line_start, line_end;

    line_start.x = canvas_width + (thickness / 2);
    line_start.y = 0;
    line_end.x = line_start.x;
    line_end.y = view_height - 1;
    DrawSolidLine(p_dc, line_start, line_end, RGB(255, 64, 64), thickness);

    line_start.x = 0;
    line_start.y = canvas_height + (thickness / 2);
    line_end.x = view_width - 1;
    line_end.y = line_start.y;
    DrawSolidLine(p_dc, line_start, line_end, RGB(255, 64, 64), thickness);
}

void ImageCraftView::DrawCanvasRowInfos(CDC *p_dc, Graphics &graphics)
{
    int block_size = (int)theApp.m_block_size;
    int canvas_width = m_canvas_unit_size.width * (block_size / 2);
    int canvas_height = m_canvas_unit_size.height * block_size;
    int view_width = mp_view_image->width;
    int view_height = mp_view_image->height;

    if (view_width <= canvas_width)
        return;

    int row_info_color = HY_COLOR_PINK;

    HyImage *p_row_info_image = hyCreateImage(hySize(view_width - canvas_width, view_height), HY_DEPTH_8U, 3);
    HY_ZEROIMAGE(p_row_info_image);

    int row_count = m_canvas_unit_size.height;
    int first_row_label = theApp.m_display_y_offset + 1;

    int text_size = block_size - 1;

    HyPoint line_start, line_end;
    for (int i = 0; i < row_count; i++)
    {
        hyPutText(p_row_info_image, hyPoint(0, i * block_size), HY_COLOR_PINK, text_size, _T("%3d"), first_row_label + i);

        if (i > 0)
        {
            line_start.x = 0;
            line_start.y = i * block_size - 1;
            line_end.x = view_width - 1;
            line_end.y = line_start.y;
            hyLine(p_row_info_image, line_start, line_end, row_info_color);

            line_start.y++;
            line_end.y++;
            hyLine(p_row_info_image, line_start, line_end, row_info_color);
        }
    }

    DrawImageToView(graphics, p_row_info_image, hyPoint(canvas_width, 0));

    hyReleaseImage(&p_row_info_image);
}

void ImageCraftView::DrawBlinkRegion(CDC *p_dc, Graphics &graphics, const HyImage *p_original_image)
{
    if (m_is_enable_draw_blink_region == false)
        return;

    if (p_original_image == NULL)
        return;

    const int image_width = p_original_image->width;
    const int image_height = p_original_image->height;

    const float blink_alpha = (float)m_draw_blink_region_progress / MAX_DRAW_BLINK_REGION_PROGRESS;

    HyImage *p_draw_image = NULL;
    HyPoint draw_offset;

    if (m_edit_info.mode == AC_EDIT_CHANGE_COLOR)
    {
        if (theApp.m_is_change_color_area_mode)
        {
            const HyRect &mask_rect = m_edit_info.change_mask_roi;
            const int mask_width = mask_rect.width;
            const int mask_height = mask_rect.height;

            HyRect valid_mask_rect = hyIntersectRect(mask_rect, hyRect(0, 0, image_width, image_height));
            const int valid_width = valid_mask_rect.width;
            const int valid_height = valid_mask_rect.height;

            if (valid_width > 0 && valid_height > 0 &&
                (int)m_edit_info.change_mask_buffer.size() >= (mask_width * mask_height))
            {
                const int draw_color = ToNormalColor(m_current_color.fg_color);
                const int draw_bgr[3] =
                {
                    HY_GetBValue(draw_color), HY_GetGValue(draw_color), HY_GetRValue(draw_color),
                };
                
                p_draw_image = hyCreateImage(hySize(valid_width, valid_height), HY_DEPTH_8U, 3);

                const BYTE *p_src_valid_start = p_original_image->get_pixels<BYTE>(valid_mask_rect);
                const BYTE *p_mask_valid_start = (const BYTE *)(&m_edit_info.change_mask_buffer[0])
                                               + (valid_mask_rect.y - mask_rect.y) * mask_width + (valid_mask_rect.x - mask_rect.x);

                for (int y = 0; y < valid_height; y++)
                {
                    const BYTE *p_src_scan = p_src_valid_start + y * p_original_image->widthStep;
                    const BYTE *p_mask_scan = p_mask_valid_start + y * mask_width;
                    BYTE *p_dst_scan = p_draw_image->imageData + y * p_draw_image->widthStep;

                    for (int x = 0; x < valid_width; x++)
                    {
                        int alpha = (int)p_mask_scan[x] * m_draw_blink_region_progress / MAX_DRAW_BLINK_REGION_PROGRESS;
                        int src_alpha = 255 - alpha;

                        const BYTE *p_src_pixel = p_src_scan + x * 3;
                        BYTE *p_dst_pixel = p_dst_scan + x * 3;
                        for (int c = 0; c < 3; c++)
                            p_dst_pixel[c] = (BYTE)((alpha * draw_bgr[c] + src_alpha * p_src_pixel[c] + 255) >> 8);
                    }
                }

                draw_offset.x = valid_mask_rect.x;
                draw_offset.y = valid_mask_rect.y;
            }
        }
    }

    if (p_draw_image)
    {
        DrawImageToView(graphics, p_draw_image, draw_offset);
        hyReleaseImage(&p_draw_image);
    }
}

void ImageCraftView::DrawEditingPattern(CDC *p_dc, Graphics &graphics)
{
    if (IsMoveImage())
        return;
    if (m_edit_info.is_valid == false)
        return;

    if (m_edit_info.mode == AC_EDIT_ADD_TEXT)
        DrawPromptString(m_edit_info.target_rect, graphics);

    int base_color1 = RGB(255, 255, 0);
    int base_color2 = RGB(0, 255, 255);
    int covered_color1 = RGB(255, 128, 0);
    int covered_color2 = RGB(0, 128, 255);

    int edit_color1 = RGB(255, 0, 0);
    int edit_color2 = RGB(255, 255, 255);
    int edit_subcolor1 = RGB(192, 64, 64);
    int edit_subcolor2 = RGB(192, 192, 192);

    int alert_color1 = RGB(255, 0, 0);
    int alert_color2 = RGB(255, 255, 255);

    if (m_edit_info.mode == AC_EDIT_VIEW ||
        m_edit_info.mode == AC_EDIT_DRAW_SPACE || 
        m_edit_info.mode == AC_EDIT_DRAW_BLOCK ||
        m_edit_info.mode == AC_EDIT_DRAW_SMALL_SQUARE ||
        m_edit_info.mode == AC_EDIT_MERGE_BLOCK ||
        m_edit_info.mode == AC_EDIT_INSERT_DELETE_SPACE ||
        m_edit_info.mode == AC_EDIT_ADD_TEXT)
    {
        int covered_color1 = RGB(255, 128, 0);
        int covered_color2 = RGB(0, 128, 255);

        for (int i = 0; i < (int)m_edit_info.covered_rects.size(); i++)
            DrawTwoColorRect(p_dc, m_edit_info.covered_rects[i], covered_color1, covered_color2);

        DrawTwoColorRect(p_dc, m_edit_info.target_rect, base_color1, base_color2);

        if (m_edit_info.mode == AC_EDIT_MERGE_BLOCK)
        {
            for (int i = 0; i < (int)m_edit_info.additional_merge_data.size(); i++)
                DrawTwoColorRect(p_dc, m_edit_info.additional_merge_data[i].merge_rect, base_color1, base_color2);
        }
    }
    else if (m_edit_info.mode == AC_EDIT_DRAW_LARGE_BRUSH)
    {
        DrawRegionContour(p_dc, m_edit_info.target_region, base_color1);
    }
    else if (m_edit_info.mode == AC_EDIT_CHANGE_COLOR)
    {
        int covered_color1 = RGB(255, 128, 0);
        int covered_color2 = RGB(0, 128, 255);

        if (theApp.m_is_change_color_area_mode == false)
        {
            for (int i = 0; i < (int)m_edit_info.covered_rects.size(); i++)
                DrawTwoColorRect(p_dc, m_edit_info.covered_rects[i], covered_color1, covered_color2);
        }
        else
        {
            const int region_point_count = (int)m_edit_info.target_region.size();
            if (region_point_count > 0)
            {
                int min_x = m_edit_info.target_region[0].x;
                int min_y = m_edit_info.target_region[0].y;
                int max_x = min_x;
                int max_y = min_y;
                for (int i = 1; i < region_point_count; i++)
                {
                    int px = m_edit_info.target_region[i].x;
                    int py = m_edit_info.target_region[i].y;
                    min_x = ch_Min(min_x, px);
                    min_y = ch_Min(min_y, py);
                    max_x = ch_Max(max_x, px);
                    max_y = ch_Max(max_y, py);
                }

                int space_size = ch_Max((int)theApp.m_block_size, 16);

                HyPoint center((min_x + max_x) / 2, (min_y + max_y) / 2);
                HyPoint left_down_vector(-space_size, space_size);
                HyPoint right_down_vector(space_size, space_size);
                HyPoint hori_vector(space_size / 2, 0);
                HyPoint vert_vector(0, space_size / 2);

                HyPoint arrow_head = center - right_down_vector * 2;
                DrawTwoColorDiagonalArrow(p_dc, arrow_head, right_down_vector,
                                          hori_vector, vert_vector, alert_color1, alert_color2);
                arrow_head = center - left_down_vector * 2;
                DrawTwoColorDiagonalArrow(p_dc, arrow_head, left_down_vector,
                                          -hori_vector, vert_vector, alert_color1, alert_color2);
                arrow_head = center + left_down_vector * 2;
                DrawTwoColorDiagonalArrow(p_dc, arrow_head, -left_down_vector,
                                          hori_vector, -vert_vector, alert_color1, alert_color2);
                arrow_head = center + right_down_vector * 2;
                DrawTwoColorDiagonalArrow(p_dc, arrow_head, -right_down_vector,
                                          -hori_vector, -vert_vector, alert_color1, alert_color2);
            }
        }

        DrawRegionContour(p_dc, m_edit_info.target_region, base_color1);
    }
    else if (m_edit_info.mode == AC_EDIT_REFINE_BOUNDARY)
    {
        DrawTwoColorRect(p_dc, m_edit_info.target_rect, base_color1, base_color2);
        DrawTwoColorLine(p_dc, m_edit_info.line_start, m_edit_info.line_end, edit_color1, edit_color2);
    }
    else if (m_edit_info.mode == AC_EDIT_REFINE_BOUNDARY_HALF)
    {
        DrawTwoColorRect(p_dc, m_edit_info.target_rect, base_color1, base_color2);

        if (m_edit_info.is_left_half)
        {
            DrawTwoColorLine(p_dc, m_edit_info.second_line_start, m_edit_info.second_line_end, edit_subcolor1, edit_subcolor2);
            DrawTwoColorLine(p_dc, m_edit_info.line_start, m_edit_info.line_end, edit_color1, edit_color2);
        }
        else
        {
            DrawTwoColorLine(p_dc, m_edit_info.line_start, m_edit_info.line_end, edit_subcolor1, edit_subcolor2);
            DrawTwoColorLine(p_dc, m_edit_info.second_line_start, m_edit_info.second_line_end, edit_color1, edit_color2);
        }
    }
    else if (m_edit_info.mode == AC_EDIT_REFINE_BOUNDARY_TRIANGLE || AC_EDIT_REFINE_BOUNDARY_REGULAR_TRIANGLE)
    {
        DrawTwoColorRect(p_dc, m_edit_info.target_rect, base_color1, base_color2);

        DrawTriangleEditBoundary(p_dc, m_edit_info.line_start, m_edit_info.line_end, m_edit_info.is_have_second_line,
                                 m_edit_info.second_line_start, m_edit_info.second_line_end, edit_color1, edit_color2);
    }
}

void ImageCraftView::DrawSolidLine(CDC *p_dc, const HyPoint &p1, const HyPoint &p2, COLORREF color, int thickness)
{
    CPen pen(PS_SOLID, thickness, color);
    CPen *p_old_pen = p_dc->SelectObject(&pen);

    p_dc->MoveTo(cPoint(p1));
    p_dc->LineTo(cPoint(p2));

    p_dc->SelectObject(p_old_pen);
}

void ImageCraftView::DrawTwoColorLine(CDC *p_dc, const HyPoint &p1, const HyPoint &p2, COLORREF inner_color, COLORREF outer_color)
{
    HyPoint outer_p1 = p1;
    HyPoint outer_p2 = p2;
    if (p1.x == p2.x)
        outer_p2.y--;
    else
        outer_p2.x--;

    DrawSolidLine(p_dc, outer_p1, outer_p2, outer_color, 3);
    DrawSolidLine(p_dc, p1, p2, inner_color, 1);
}

void ImageCraftView::DrawTriangleEditBoundary(CDC *p_dc, const HyPoint &start1, const HyPoint &end1, bool is_have_line2,
                                              const HyPoint &start2, const HyPoint &end2, COLORREF inner_color, COLORREF outer_color)
{
    HyPoint outer_start1, outer_end1, inner_start1, inner_end1;
    GetTwoColorLinePoints(start1, end1, outer_start1, outer_end1, inner_start1, inner_end1);

    HyPoint outer_start2, outer_end2, inner_start2, inner_end2;
    if (is_have_line2)
        GetTwoColorLinePoints(start2, end2, outer_start2, outer_end2, inner_start2, inner_end2);

    DrawSolidLine(p_dc, outer_start1, outer_end1, outer_color, 3);
    if (is_have_line2)
        DrawSolidLine(p_dc, outer_start2, outer_end2, outer_color, 3);

    DrawSolidLine(p_dc, inner_start1, inner_end1, inner_color, 1);
    if (is_have_line2)
        DrawSolidLine(p_dc, inner_start2, inner_end2, inner_color, 1);
}

void ImageCraftView::DrawRect(CDC *p_dc, const HyRect &rect, COLORREF color, int thickness)
{
    CPen pen(PS_SOLID, thickness, color);
    CPen *p_old_pen = p_dc->SelectObject(&pen);

    p_dc->MoveTo(CPoint(rect.x, rect.y));
    p_dc->LineTo(CPoint(rect.x + rect.width, rect.y));
    p_dc->LineTo(CPoint(rect.x + rect.width, rect.y + rect.height));
    p_dc->LineTo(CPoint(rect.x, rect.y + rect.height));
    p_dc->LineTo(CPoint(rect.x, rect.y));

    p_dc->SelectObject(p_old_pen);
}

void ImageCraftView::DrawTwoColorRect(CDC *p_dc, const HyRect &rect, COLORREF inner_color, COLORREF outer_color)
{
    HyRect inner_rect = rect;
    inner_rect.width--;
    inner_rect.height--;
    HyRect outer_rect(inner_rect.x - 1, inner_rect.y - 1, inner_rect.width + 2, inner_rect.height + 2); 

    DrawRect(p_dc, inner_rect, inner_color, 1);
    DrawRect(p_dc, outer_rect, outer_color, 1);
}

void ImageCraftView::DrawArrow(CDC *p_dc, const HyPoint &tail, const HyPoint &head,
                               COLORREF color, int thickness, float head_length_ratio/* = 0.20f*/, float head_angle_degree/* = 30.0f*/)
{
    // Draw arbitrary arrow, but may have some aliasing.

    CPen pen(PS_SOLID, thickness, color);
    CPen *p_old_pen = p_dc->SelectObject(&pen);

    float base_dx = (tail.x - head.x) * head_length_ratio;
    float base_dy = (tail.y - head.y) * head_length_ratio;

    float radian = (head_angle_degree * HY_F_PI) / 180.0f;
    float cos_theta = cosf(radian);
    float sin_theta = sinf(radian);

    float branch1_dx = base_dx * cos_theta - base_dy * sin_theta;
    float branch1_dy = base_dx * sin_theta + base_dy * cos_theta;
    CPoint branch1(head.x + ch_Round(branch1_dx), head.y + ch_Round(branch1_dy));

    float branch2_dx =  base_dx * cos_theta + base_dy * sin_theta;
    float branch2_dy = -base_dx * sin_theta + base_dy * cos_theta;
    CPoint branch2(head.x + ch_Round(branch2_dx), head.y + ch_Round(branch2_dy));

    p_dc->MoveTo(cPoint(tail));
    p_dc->LineTo(cPoint(head));
    p_dc->LineTo(branch1);
    p_dc->MoveTo(cPoint(head));
    p_dc->LineTo(branch2);

    p_dc->SelectObject(p_old_pen);
}

void ImageCraftView::DrawTwoColorDiagonalArrow(CDC *p_dc, const HyPoint &head, const HyPoint &diagonal_vector,
                                               const HyPoint &hori_vector, const HyPoint &vert_vector,
                                               COLORREF inner_color, COLORREF outer_color)
{
    HyPoint diagonal_start = head;
    HyPoint diagonal_end = head + diagonal_vector;
    HyPoint hori_start = head;
    HyPoint hori_end = head + hori_vector;
    HyPoint vert_start = head;
    HyPoint vert_end = head + vert_vector;

    HyPoint diagonal_shift1 = (diagonal_vector.x > 0) ? hyPoint(1, 0) : hyPoint(-1, 0);
    HyPoint diagonal_shift2 = (diagonal_vector.y > 0) ? hyPoint(0, 1) : hyPoint(0, -1);

    DrawSolidLine(p_dc, hori_start, hori_end, outer_color, 3);
    DrawSolidLine(p_dc, vert_start, vert_end, outer_color, 3);

    DrawSolidLine(p_dc, diagonal_start + diagonal_shift1, diagonal_end + diagonal_shift1, outer_color, 1);
    DrawSolidLine(p_dc, diagonal_start + diagonal_shift2, diagonal_end + diagonal_shift2, outer_color, 1);

    DrawSolidLine(p_dc, diagonal_start, diagonal_end, inner_color, 1);
    DrawSolidLine(p_dc, hori_start, hori_end, inner_color, 1);
    DrawSolidLine(p_dc, vert_start, vert_end, inner_color, 1);
}

void ImageCraftView::DrawImageToView(Graphics &graphics, HyImage *p_image, const HyPoint &offset)
{
    if (p_image == NULL)
        return;

    HyRect draw_roi(offset.x, offset.y, p_image->width, p_image->height);
    HyRect valid_draw_roi = hyIntersectRect(draw_roi, hyRect(0, 0, mp_view_image->width, mp_view_image->height));
    HyRect valid_image_roi(valid_draw_roi.x - draw_roi.x,
                           valid_draw_roi.y - draw_roi.y,
                           valid_draw_roi.width, valid_draw_roi.height);

    Bitmap draw_image(valid_image_roi.width, valid_image_roi.height,
                      p_image->widthStep, PixelFormat24bppRGB,
                      p_image->get_pixels<BYTE>(valid_image_roi));

    Rect dst_rect(valid_draw_roi.x, valid_draw_roi.y, valid_draw_roi.width, valid_draw_roi.height);
    graphics.DrawImage(&draw_image, dst_rect, 0, 0, valid_image_roi.width, valid_image_roi.height, UnitPixel);
}

void ImageCraftView::DrawRegionContour(CDC *p_dc, const std::vector<HyPoint> &region, COLORREF color)
{
    int line_count = (int)region.size();

    std::vector<HyPoint> contour;
    for (int i = 0; i < line_count; i++)
    {
        const HyPoint &p1 = region[i];
        const HyPoint &p2 = region[(i + 1) % line_count];
        if (p1 == p2)
            continue;

        contour.push_back(p1);
    }

    line_count = (int)contour.size();
    if (line_count < 2)
        return;

    ShrinkContourForDrawing(contour);

    contour.push_back(contour[0]);

    CPen pen(PS_SOLID, 1, color);

    CPen *p_old_pen = p_dc->SelectObject(&pen);

    p_dc->MoveTo(CPoint(contour[0].x, contour[0].y));
    for (int i = 1; i <= line_count; i++)
        p_dc->LineTo(CPoint(contour[i].x, contour[i].y));

    p_dc->SelectObject(p_old_pen);
}

void ImageCraftView::ShrinkContourForDrawing(std::vector<HyPoint> &contour)
{
    const int line_count = (int)contour.size();
    if (line_count < 3)
        return;

    for (int i = 0; i < line_count; i++)
    {
        HyPoint &p1 = contour[i];
        HyPoint &p2 = contour[(i + 1) % line_count];
        const HyPoint &p0 = contour[(i + line_count - 1) % line_count];
        const HyPoint &p3 = contour[(i + 2) % line_count];

        bool is_horizontal = false;
        bool is_vertical = false;
        bool is_diagonal = false;
        if (p1.x == p2.x && p1.y < p2.y)
        {
            is_horizontal = true;
        }
        else if (p1.x > p2.x && p1.y == p2.y)
        {
            is_vertical = true;
        }
        else if (p1.x > p2.x && p1.y < p2.y)
        {
            if (p0.y == p1.y && p2.y == p3.y)
                is_horizontal = true;
            else if (p0.x == p1.x && p2.x == p3.x)
                is_vertical = true;
            else
                is_diagonal = true;
        }

        if (is_horizontal)
        {
            p1.x--;
            if (p0.y < p1.y)
                p1.y--;
            else if (p0.y > p1.y)
                p1.y++;

            p2.x--;
            if (p3.y < p2.y)
                p2.y--;
            else if (p3.y > p2.y)
                p2.y++;
        }
        else if (is_vertical)
        {
            p1.y--;
            if (p0.x < p1.x)
                p1.x--;
            else if (p0.x > p1.x)
                p1.x++;

            p2.y--;
            if (p3.x < p2.x)
                p2.x--;
            else if (p3.x > p2.x)
                p2.x++;
        }
        else if (is_diagonal)
        {
            p1.x--;
            if (p0.y < p1.y)
                p1.y--;
            else if (p0.y > p1.y)
                p1.y++;

            p2.y--;
            if (p3.x < p2.x)
                p2.x--;
            else if (p3.x > p2.x)
                p2.x++;
        }
    }
}

void ImageCraftView::DrawPromptString(const HyRect &text_roi, Graphics &graphics)
{
    if (mp_prompt_string_image == NULL)
        return;

    int block_size = (int)theApp.m_block_size;
    int canvas_width = m_canvas_unit_size.width * (block_size / 2);
    int canvas_height = m_canvas_unit_size.height * block_size;
    int view_width = mp_view_image->width;
    int view_height = mp_view_image->height;

    HyRect display_rect(0, 0, ch_Min(view_width, canvas_width), ch_Min(view_height, canvas_height));
    HyRect valid_roi = hyIntersectRect(text_roi, display_rect);

    const int valid_width = valid_roi.width;
    const int valid_height = valid_roi.height;
    if (valid_width <= 0 || valid_height <= 0)
        return;

    HyImage *p_roi_image = hyCreateImage(hySize(valid_width, valid_height), HY_DEPTH_8U, 3);
    ippiCopy_8u_C3R(mp_view_image->get_pixels<BYTE>(valid_roi.x, valid_roi.y), mp_view_image->widthStep,
                    p_roi_image->imageData, p_roi_image->widthStep, ippiSize(p_roi_image));

    const int blend_width = ch_Min(valid_width, mp_prompt_string_image->width);
    const int blend_height = ch_Min(valid_width, mp_prompt_string_image->height);
    for (int y = 0; y < blend_height; y++)
    {
        const BYTE *p_src_scan = mp_prompt_string_image->imageData + y * mp_prompt_string_image->widthStep;
        BYTE *p_dst_scan = p_roi_image->imageData + y * p_roi_image->widthStep;

        for (int x = 0; x < blend_width; x++)
        {
            const BYTE *p_src_pixel = p_src_scan + x * 3;
            BYTE *p_dst_pixel = p_dst_scan + x * 3;

            for (int c = 0; c < 3; c++)
            {
                int base_value = p_dst_pixel[c] / 2;
                int alpha = p_src_pixel[c];

                p_dst_pixel[c] = (BYTE)((255 * alpha + base_value * (255 - alpha) + 255) >> 8);
            }
        }
    }

    Bitmap draw_image(p_roi_image->width, p_roi_image->height, 
                      p_roi_image->widthStep, PixelFormat24bppRGB, p_roi_image->imageData);

    graphics.DrawImage(&draw_image, Rect(valid_roi.x, valid_roi.y, p_roi_image->width, p_roi_image->height),
                       0, 0, p_roi_image->width, p_roi_image->height, UnitPixel);

    hyReleaseImage(&p_roi_image);
}

void ImageCraftView::SetBlockSize(int block_size, bool is_invalidate/* = true*/)
{
    int valid_block_size = GetValidBlockSize(block_size);

    theApp.m_block_size = valid_block_size;
    m_ansi_canvas.SetBlockSize(valid_block_size);
    m_ansi_template.SetSize(ch_Max(valid_block_size, 16));

    hyReleaseImage(&mp_prompt_string_image);
    mp_prompt_string_image = hyCreateImage(hySize(valid_block_size * 2, valid_block_size), HY_DEPTH_8U, 3);
    m_ansi_template.GetStringImage(mp_prompt_string_image, _T("文字"));

    m_canvas_unit_size = m_ansi_canvas.GetDisplayUnitSize();

    MakeAnsiImage();

    if (is_invalidate)
        Invalidate(FALSE);
}

void ImageCraftView::ImageResize(const HyImage *p_src_image, HyImage *p_dst_image)
{
    if (p_src_image == NULL || p_dst_image == NULL)
        return;
    if (p_src_image->nChannels != p_dst_image->nChannels)
        return;

    if (hyGetSize(p_src_image) == hyGetSize(p_dst_image))
    {
        ippiCopy(p_src_image, p_dst_image);
        return;
    }

    int method = IPPI_INTER_LINEAR;
    if (p_src_image->width > p_dst_image->width && p_src_image->height > p_dst_image->height)
        method = IPPI_INTER_SUPER;

    ippiResize(p_src_image, p_dst_image, method);
}

void ImageCraftView::UpdateImageSizeInfo()
{
    ImageCraftDialog *p_dialog = theApp.GetMainFrame()->GetMainDialog();
    if (p_dialog == NULL)
        return;

    int image_width = 0;
    int image_height = 0;
    if (mp_full_image)
    {
        image_width = mp_full_image->width;
        image_height = mp_full_image->height;
    }

    CString info_string;
    info_string.Format(_T("%d x %d"), image_width, image_height);
    (p_dialog->GetDlgItem(IDC_TEXT_IMAGE_FULL_SIZE))->SetWindowText(info_string);
}

void ImageCraftView::UpdateImageROIInfo()
{
    ImageCraftDialog *p_dialog = theApp.GetMainFrame()->GetMainDialog();
    if (p_dialog == NULL)
        return;

    int src_x_min = 0;
    int src_x_max = 0;
    int src_y_min = 0;
    int src_y_max = 0;

    if (mp_full_image && mp_image)
    {
        theApp.m_image_roi_offset = m_image_offset;

        int x_unit = (int)theApp.m_block_size / 2;
        int y_unit = (int)theApp.m_block_size;
        float scale_x = m_scale_to_full.x;
        float scale_y = m_scale_to_full.y;

        m_image_roi.x = ch_Round(m_image_offset.x) + x_unit * theApp.m_display_x_offset;
        m_image_roi.y = ch_Round(m_image_offset.y) + y_unit * theApp.m_display_y_offset;

        float x_min = (float)m_image_roi.x;
        float y_min = (float)m_image_roi.y;
        float x_max = x_min + (float)m_image_roi.width;
        float y_max = y_min + (float)m_image_roi.height;

        src_x_min = ch_Round(scale_x * x_min);
        src_x_max = ch_Round(scale_x * x_max);
        src_y_min = ch_Round(scale_y * y_min);
        src_y_max = ch_Round(scale_y * y_max);
    }

    CString rotate_string;
    int rounded_degree_10x = ch_Round(theApp.m_image_rotate_degree * 10.0f);
    if (rounded_degree_10x > 0)
        rotate_string.Format(_T("順時針旋轉%.1f°"), rounded_degree_10x * 0.1f);
    else if (rounded_degree_10x < 0)
        rotate_string.Format(_T("逆時針旋轉%.1f°"), -rounded_degree_10x * 0.1f);
    else
        rotate_string.Format(_T("無旋轉"));

    CString info_string;
    info_string.Format(_T("x: %d ~ %d\ny: %d ~ %d\n%s"), src_x_min, src_x_max, src_y_min, src_y_max, rotate_string);
    (p_dialog->GetDlgItem(IDC_TEXT_IMAGE_ROI_RANGE))->SetWindowText(info_string);

    theApp.m_image_resize_ratio = ch_Round(GetResizeRatio() * 100.0f);
    info_string.Format(_T("%d%%"), theApp.m_image_resize_ratio);
    (p_dialog->GetDlgItem(IDC_TEXT_IMAGE_RESIZE_RATIO))->SetWindowText(info_string);
    p_dialog->m_image_resize_ratio_slider.SetPos(theApp.m_image_resize_ratio);

    p_dialog->UpdateRotateDegreeUI();
}

void ImageCraftView::SetEnableImageUI()
{
    ImageCraftDialog *p_dialog = theApp.GetMainFrame()->GetMainDialog();
    if (p_dialog == NULL)
        return;

    p_dialog->SetEnableImageUI();
}

void ImageCraftView::MakeAnsiImage()
{
    m_ansi_canvas.MakeImage(mp_ansi_image);
    MakeViewImage();
}

void ImageCraftView::UpdateAnsiImage(const HyRect &redraw_rect)
{
    HyRect roi_rect = hyIntersectRect(redraw_rect, hyRect(0, 0, mp_ansi_image->width, mp_ansi_image->height));

    m_ansi_canvas.UpdateImage(mp_ansi_image, roi_rect);
    MakeViewImage(roi_rect);
}

void ImageCraftView::MakeViewImage()
{
    MakeViewImage(hyRect(0, 0, mp_view_image->width, mp_view_image->height));
}

void ImageCraftView::MakeViewImage(const HyRect &redraw_rect)
{
    if (mp_ansi_image == NULL)
        return;
    if (IsValidRoi(redraw_rect, mp_view_image) == false)
        return;

    if (IsImageLoaded() == false)
    {
        ippiROICopy(mp_ansi_image, mp_view_image, redraw_rect);
        return;
    }

    HyRect image_rect;
    image_rect.x = m_image_roi.x + redraw_rect.x;
    image_rect.y = m_image_roi.y + redraw_rect.y;
    image_rect.width = redraw_rect.width;
    image_rect.height = redraw_rect.height;

    HyRect valid_image_rect = hyIntersectRect(image_rect, hyRect(0, 0, mp_image->width, mp_image->height));
    HyRect valid_redraw_rect = valid_image_rect;
    valid_redraw_rect.x -= m_image_roi.x;
    valid_redraw_rect.y -= m_image_roi.y;

    ippiROICopy(mp_ansi_image, mp_view_image, redraw_rect);

    float ratio = 0.01f * theApp.m_image_alpha_ratio;
    ratio = FitInRange(ratio, 0.0f, 1.0f);
    int alpha = ch_Round(255 * ratio);
    if (theApp.m_is_hide_image)
        alpha = 0;

    int alpha2 = 255 - alpha;

    const BYTE *p_src1_start = mp_image->get_pixels<BYTE>(valid_image_rect.x, valid_image_rect.y);
    const BYTE *p_src2_start = mp_ansi_image->get_pixels<BYTE>(valid_redraw_rect.x, valid_redraw_rect.y);
    BYTE *p_dst_start = mp_view_image->get_pixels<BYTE>(valid_redraw_rect.x, valid_redraw_rect.y);

    for (int y = 0; y < valid_redraw_rect.height; y++)
    {
        const BYTE *p_src1_scan = p_src1_start + y * mp_image->widthStep;
        const BYTE *p_src2_scan = p_src2_start + y * mp_ansi_image->widthStep;
        BYTE *p_dst_scan = p_dst_start + y * mp_view_image->widthStep;

        for (int x = 0; x < valid_redraw_rect.width; x++)
        {
            const BYTE *p_src1_pixel = p_src1_scan + x * 3;
            const BYTE *p_src2_pixel = p_src2_scan + x * 3;
            BYTE *p_dst_pixel = p_dst_scan + x * 3;

            for (int c = 0; c < 3; c++)
                p_dst_pixel[c] = (BYTE)((alpha * p_src1_pixel[c] + alpha2 * p_src2_pixel[c] + 255) >> 8);
        }
    }
}

void ImageCraftView::UpdateViewImage()
{
    MakeViewImage();

    Invalidate(FALSE);
}

void ImageCraftView::SaveAnsi(LPCTSTR path)
{
    m_ansi_canvas.SaveAnsi(path);
}

void ImageCraftView::LoadAnsi(LPCTSTR path)
{
    m_ansi_canvas.LoadAnsi(path);

    UpdateDisplayOffset();

    MakeAnsiImage();
    Invalidate(FALSE);

    UpdateActionHistory();
}

void ImageCraftView::ExportAnsi(LPCTSTR path, int max_width)
{
    m_ansi_canvas.SaveAnsi(path, max_width);
}

void ImageCraftView::ClearCanvas()
{
    m_ansi_canvas.ClearCanvasAction();

    UpdateActionHistory();

    MakeAnsiImage();

    Invalidate(FALSE);
}

void ImageCraftView::SetTestContents()
{
    m_ansi_canvas.SetTestContents();

    MakeAnsiImage();

    Invalidate(FALSE);
}

void ImageCraftView::ChangeDrawMode(int mode)
{
    if (mode == IC_VIEW_ANSI)
    {
        m_draw_mode = IC_VIEW_ANSI;
        UpdateViewAnsiInfo(m_current_mouse_point.x, m_current_mouse_point.y);
    }
    else if (mode == IC_DRAW_SPACES)
    {
        m_draw_mode = IC_DRAW_SPACES;
        UpdateDrawSpaceInfo(m_current_mouse_point.x, m_current_mouse_point.y);
    }
    else if (mode == IC_CHANGE_COLOR)
    {
        m_draw_mode = IC_CHANGE_COLOR;
        UpdateChangeColorInfo(m_current_mouse_point.x, m_current_mouse_point.y);
    }
    else if (mode == IC_REFINE_BOUNDARY)
    {
        m_draw_mode = IC_REFINE_BOUNDARY;
        UpdateRefineBoundaryInfo(m_current_mouse_point.x, m_current_mouse_point.y);
    }
    else if (mode == IC_DRAW_BLOCK)
    {
        m_draw_mode = IC_DRAW_BLOCK;
        UpdateDrawBlockInfo(m_current_mouse_point.x, m_current_mouse_point.y);
    }
    else if (mode == IC_DRAW_SMALL_SQUARE)
    {
        m_draw_mode = IC_DRAW_SMALL_SQUARE;
        UpdateDrawSmallSquareInfo(m_current_mouse_point.x, m_current_mouse_point.y);
    }
    else if (mode == IC_MERGE_BLOCK)
    {
        m_draw_mode = IC_MERGE_BLOCK;
        UpdateMergeBlockInfo(m_current_mouse_point.x, m_current_mouse_point.y);
    }
    else if (mode == IC_INSERT_DELETE_SPACE)
    {
        m_draw_mode = IC_INSERT_DELETE_SPACE;
        UpdateInsertDeleteSpaceInfo(m_current_mouse_point.x, m_current_mouse_point.y);
    }
    else if (mode == IC_DRAW_LARGE_BRUSH)
    {
        m_draw_mode = IC_DRAW_LARGE_BRUSH;
        UpdateDrawLargeBrushInfo(m_current_mouse_point.x, m_current_mouse_point.y);
    }
    else if (mode == IC_ADD_TEXT)
    {
        m_draw_mode = IC_ADD_TEXT;
        UpdateAddTextInfo(m_current_mouse_point.x, m_current_mouse_point.y);
    }

    Invalidate(FALSE);
}

void ImageCraftView::UpdateViewAnsiInfo(int x, int y)
{
    m_ansi_canvas.GetEditInfo(x, y, AC_EDIT_VIEW, m_edit_info);
    UpdateInfoImage(m_edit_info.cell, m_edit_info.color);
}

void ImageCraftView::UpdateDrawSpaceInfo(int x, int y)
{
    m_ansi_canvas.GetEditInfo(x, y, AC_EDIT_DRAW_SPACE, m_edit_info);
}

void ImageCraftView::UpdateChangeColorInfo(int x, int y)
{
    AC_EditOption option;
    option.is_change_color_area_mode = theApp.m_is_change_color_area_mode;
    m_ansi_canvas.GetEditInfo(x, y, AC_EDIT_CHANGE_COLOR, m_edit_info, option);
}

void ImageCraftView::UpdateRefineBoundaryInfo(int x, int y)
{
    m_ansi_canvas.GetEditInfo(x, y, GetRefineBoundaryMode(), m_edit_info);
}

void ImageCraftView::UpdateDrawBlockInfo(int x, int y)
{
    m_ansi_canvas.GetEditInfo(x, y, AC_EDIT_DRAW_BLOCK, m_edit_info);
}

void ImageCraftView::UpdateDrawSmallSquareInfo(int x, int y)
{
    m_ansi_canvas.GetEditInfo(x, y, AC_EDIT_DRAW_SMALL_SQUARE, m_edit_info);
}

void ImageCraftView::UpdateMergeBlockInfo(int x, int y)
{
    AC_EditOption option;
    option.is_alternated_edit = m_is_alternated_edit;
    m_ansi_canvas.GetEditInfo(x, y, AC_EDIT_MERGE_BLOCK, m_edit_info, option);
}

void ImageCraftView::UpdateInsertDeleteSpaceInfo(int x, int y)
{
    m_ansi_canvas.GetEditInfo(x, y, AC_EDIT_INSERT_DELETE_SPACE, m_edit_info);
}

void ImageCraftView::UpdateDrawLargeBrushInfo(int x, int y)
{
    AC_EditOption option;
    option.brush_size = theApp.m_large_brush_size;
    option.brush_shape = theApp.m_large_brush_shape;
    m_ansi_canvas.GetEditInfo(x, y, AC_EDIT_DRAW_LARGE_BRUSH, m_edit_info, option);
}

void ImageCraftView::UpdateAddTextInfo(int x, int y)
{
    m_ansi_canvas.GetEditInfo(x, y, AC_EDIT_ADD_TEXT, m_edit_info);
}

void ImageCraftView::UpdateInfoImage(const AnsiCell &cell, const AnsiColor &color)
{
    int block_size = m_ansi_template.GetSize();
    int half_block_size = block_size / 2;

    const int outer_space = 4;
    const int inner_space = half_block_size;

    int cell_width = (cell.IsDoubleChar() ? block_size : half_block_size);
    int cell_height = block_size;

    int image_width = cell_width + block_size * 2 + inner_space * 6 + outer_space * 2;
    int image_height = cell_height + inner_space * 2 + outer_space * 2;

    bool is_allocate_image = (mp_info_image == NULL);
    if (is_allocate_image == false)
        is_allocate_image = (mp_info_image->width != image_width || mp_info_image->height != image_height);

    if (is_allocate_image)
    {
        hyReleaseImage(&mp_info_image);
        mp_info_image = hyCreateImage(hySize(image_width, image_height), HY_DEPTH_8U, 3);
    }
    else
    {
        if (m_info_cell == cell && m_info_color == color)
            return;
    }

    m_info_cell = cell;
    m_info_color = color;

    const int border_color = HY_RGB(0, 0, 170);
    const int background_color = HY_RGB(128, 255, 255);
    const int boundary_color = HY_RGB(255, 128, 128);

    hyRectangle(mp_info_image, hyRect(0, 0, image_width, image_height), border_color, outer_space);
    hyFillRectangle(mp_info_image, hyRect(outer_space, outer_space, image_width - outer_space * 2, image_height - outer_space * 2), background_color);

    int rect_x = outer_space + inner_space;
    int rect_y = outer_space + inner_space;

    hyRectangle(mp_info_image, hyRect(rect_x - 2, rect_y - 2, cell_width + 4, cell_height + 4), boundary_color, 2);
    m_ansi_template.DrawBlock(mp_info_image, rect_x, rect_y, AnsiColor(ANSI_WHITE_BRIGHT, ANSI_BLACK), cell);

    int text_size = block_size - 1;
    int text_shift = ch_Round(block_size * 0.20f);

    rect_x += (cell_width + inner_space);
    int text_color = HY_COLOR_BLACK;
    HyPoint text_offset = hyPoint(rect_x - text_shift, rect_y);
    hyPutText(mp_info_image, text_offset, text_color, text_size, (IsBrightColor(color.fg_color) ? _T("1") : _T("0")));

    rect_x += (inner_space * 2);
    hyRectangle(mp_info_image, hyRect(rect_x - 2, rect_y - 2, block_size + 4, block_size + 4), boundary_color, 2);
    hyFillRectangle(mp_info_image, hyRect(rect_x, rect_y, block_size, block_size), color.fg_color);
    text_offset = hyPoint(rect_x - text_shift, rect_y);
    text_color = (IsBrightColor(color.fg_color) ? HY_COLOR_BLACK : HY_COLOR_WHITE);    
    hyPutText(mp_info_image, text_offset, text_color, text_size, _T("3"));
    hyPutText(mp_info_image, text_offset + HyPoint(half_block_size, 0), text_color, text_size, _T("%c"), ColorChar(color.fg_color));

    rect_x += (block_size + inner_space);
    hyRectangle(mp_info_image, hyRect(rect_x - 2, rect_y - 2, block_size + 4, block_size + 4), boundary_color, 2);
    hyFillRectangle(mp_info_image, hyRect(rect_x, rect_y, block_size, block_size), color.bg_color);
    text_offset = hyPoint(rect_x - text_shift, rect_y);
    text_color = HY_COLOR_WHITE;
    hyPutText(mp_info_image, text_offset, text_color, text_size, _T("4"));
    hyPutText(mp_info_image, text_offset + HyPoint(half_block_size, 0), text_color, text_size, _T("%c"), ColorChar(color.bg_color));
}

void ImageCraftView::MoveDisplayOffset(int x_shift, int y_shift)
{
    // This action may change the canvas.
    // Only need to handle action when the canvas is changed.

    bool is_canvas_changed = false;
    bool is_valid = m_ansi_canvas.MoveDisplayOffset(x_shift, y_shift, is_canvas_changed);
    if (is_valid == false)
        return;

    if (is_canvas_changed)
        UpdateActionHistory();

    UpdateDisplayOffset();

    MakeAnsiImage();
    Invalidate(FALSE);
}

void ImageCraftView::UpdateDisplayOffset()
{
    m_ansi_canvas.GetDisplayOffset(theApp.m_display_x_offset, theApp.m_display_y_offset);

    ImageCraftDialog *p_dialog = theApp.GetMainFrame()->GetMainDialog();
    CString range_text;
    range_text.Format(_T("直行 %d - %d (半形)\n橫列 %d - %d"),
                      theApp.m_display_x_offset + 1, theApp.m_display_x_offset + m_canvas_unit_size.width,
                      theApp.m_display_y_offset + 1, theApp.m_display_y_offset + m_canvas_unit_size.height);
    p_dialog->GetDlgItem(IDC_TEXT_DISPLAY_RANGE)->SetWindowTextW(range_text);

    UpdateImageROIInfo();
}

void ImageCraftView::MoveDisplayPageUp()
{
    MoveDisplayOffset(0, -m_canvas_unit_size.height);
}

void ImageCraftView::MoveDisplayPageDown()
{
    MoveDisplayOffset(0, m_canvas_unit_size.height);
}

void ImageCraftView::DebugDump()
{
    LPCTSTR debug_folder = theApp.m_debug_folder.c_str();

    SaveDebugImage(mp_full_image, _T("%s/mp_full_image.png"), debug_folder);
    SaveDebugImage(mp_image, _T("%s/mp_image.png"), debug_folder);
    SaveDebugImage(mp_view_image, _T("%s/mp_view_image.png"), debug_folder);
    SaveDebugImage(mp_ansi_image, _T("%s/mp_ansi_image.png"), debug_folder);
}

void ImageCraftView::SaveDebugImage(HyImage *p_image, LPCTSTR format, ...)
{
    _TCHAR save_path[512];
    va_list marker;
    va_start(marker, format);
    _vstprintf(save_path, format, marker);
    va_end(marker);

    if (p_image == NULL)
    {
        HyImage *p_empty_image = hyCreateImage(hySize(320, 120), HY_DEPTH_8U, 3);
        HY_ZEROIMAGE(p_empty_image);
        hyPutText(p_empty_image, hyPoint(20, 15), HY_COLOR_RED, 80, _T("EMPTY"));
        hySaveImage(p_empty_image, save_path);
        hyReleaseImage(&p_empty_image);
    }
    else
    {
        hySaveImage(p_image, save_path);
    }
}

void ImageCraftView::ChangeBrushShape(int amount)
{
    int shift = 0;
    if (amount > 0)
        shift = amount % ANSI_BRUSH_SHAPE_AMOUNT;
    else if (amount < 0)
        shift = ANSI_BRUSH_SHAPE_AMOUNT - (-amount % ANSI_BRUSH_SHAPE_AMOUNT);

    int new_shape = (theApp.m_large_brush_shape + shift) % ANSI_BRUSH_SHAPE_AMOUNT;
    SetBrushShape(new_shape);
}

void ImageCraftView::ChangeSelectBlockTable(int amount)
{
    int shift = 0;
    if (amount > 0)
        shift = amount % IC_SELECT_BLOCK_MODE_AMOUNT;
    else if (amount < 0)
        shift = IC_SELECT_BLOCK_MODE_AMOUNT - (-amount % IC_SELECT_BLOCK_MODE_AMOUNT);

    int new_mode = (theApp.m_select_block_mode + shift) % IC_SELECT_BLOCK_MODE_AMOUNT;
    SetSelectBlockMode(new_mode);
}

void ImageCraftView::SetBrushShape(int new_shape)
{
    if (new_shape < 0 || new_shape >= ANSI_BRUSH_SHAPE_AMOUNT)
        return;
    if (theApp.m_large_brush_shape == new_shape)
        return;

    theApp.m_large_brush_shape = new_shape;

    ImageCraftDialog *p_dialog = theApp.GetMainFrame()->GetMainDialog();
    if (p_dialog)
        p_dialog->UpdateBrushShapeUI();

    UpdateDrawLargeBrushInfo(m_current_mouse_point.x, m_current_mouse_point.y);
    Invalidate(FALSE);
}

void ImageCraftView::SetSelectBlockMode(int new_mode)
{
    if (new_mode < 0 || new_mode >= IC_SELECT_BLOCK_MODE_AMOUNT)
        return;
    if (theApp.m_select_block_mode == new_mode)
        return;

    theApp.m_select_block_mode = new_mode;

    ImageCraftDialog *p_dialog = theApp.GetMainFrame()->GetMainDialog();
    if (p_dialog)
        p_dialog->UpdateSelectBlockModeUI();
}

void ImageCraftView::SetChangeColorAreaMode(bool is_enable)
{
    if (theApp.m_is_change_color_area_mode == is_enable)
        return;

    theApp.m_is_change_color_area_mode = is_enable;

    ImageCraftDialog *p_dialog = theApp.GetMainFrame()->GetMainDialog();
    if (p_dialog)
        p_dialog->UpdateChangeColorUI();

    UpdateChangeColorInfo(m_current_mouse_point.x, m_current_mouse_point.y);
    Invalidate(FALSE);
}

void ImageCraftView::SetRefineTriangleMode(bool is_enable)
{
    if (theApp.m_is_refine_triangle_mode == is_enable)
        return;

    theApp.m_is_refine_triangle_mode  = is_enable;

    ImageCraftDialog *p_dialog = theApp.GetMainFrame()->GetMainDialog();
    if (p_dialog)
        p_dialog->UpdateRefineBoundaryUI();

    UpdateRefineBoundaryInfo(m_current_mouse_point.x, m_current_mouse_point.y);
    Invalidate(FALSE);
}

void ImageCraftView::UpdateActionHistory(bool is_change_action_list/* = true*/)
{
    ImageCraftDialog *p_dialog = theApp.GetMainFrame()->GetMainDialog();
    if (p_dialog == NULL)
        return;

    int action_index = m_ansi_canvas.GetCurrentActionIndex();

    if (is_change_action_list)
    {
        std::vector<std_tstring> action_list = m_ansi_canvas.GetActionNameList();
        p_dialog->UpdateActionHistoryUI(action_index, action_list);
    }
    else 
    {
        p_dialog->UpdateActionHistoryUI(action_index);
    }
}

void ImageCraftView::DrawBlockAction(const HyPoint &point, const AnsiCell &cell,
                                     const AnsiColor &color, HyRect &change_rect)
{
    m_ansi_canvas.DrawBlock(point, cell, color, change_rect);

    UpdateActionHistory();
}

void ImageCraftView::InsertDeleteSpaceAction(const HyPoint &point, int space_color, HyRect &change_rect)
{
    if (m_is_alternated_edit)
        m_ansi_canvas.DeleteSpace(point, change_rect);
    else
        m_ansi_canvas.InsertSpace(point, space_color, change_rect);

    UpdateActionHistory();
}

void ImageCraftView::DrawSpaceAction(const HyPoint &start_point, const HyPoint &end_point,
                                     int space_color, bool is_continuous, HyRect &change_rect)
{
    bool is_action_list_changed = false;
    m_ansi_canvas.DrawSpaces(start_point, end_point,
                             space_color, is_continuous,
                             change_rect, is_action_list_changed);

    if (is_action_list_changed)
        UpdateActionHistory();
}

bool ImageCraftView::DrawSmallSquareAction(const HyPoint &start_point, const HyPoint &end_point,
                                           int color, bool is_continuous, HyRect &change_rect)
{
    bool is_action_list_changed = false;
    bool is_success = m_ansi_canvas.DrawSmallSquare(start_point, end_point,
                                                    color, is_continuous,
                                                    change_rect, is_action_list_changed);
    if (is_success == false)
        return false;

    if (is_action_list_changed)
        UpdateActionHistory();

    return true;
}

void ImageCraftView::DrawLargeBrushAction(const HyPoint &start_point, const HyPoint &end_point,
                                          int brush_level, int brush_shape,
                                          int color, bool is_continuous, HyRect &change_rect)
{
    bool is_action_list_changed = false;
    m_ansi_canvas.DrawLargeBrush(start_point, end_point,
                                 brush_level, brush_shape,
                                 color, is_continuous,
                                 change_rect, is_action_list_changed);

    if (is_action_list_changed)
        UpdateActionHistory();
}

bool ImageCraftView::StartRefineBoundaryAction(const HyPoint &point, AC_EditMode mode,
                                               AC_EditInfo &edit_info, HyRect &change_rect)
{
    bool is_success = m_ansi_canvas.StartRefineBoundary(point.x, point.y, mode, edit_info, change_rect);
    if (is_success == false)
        return false;

    UpdateActionHistory();

    return true;
}

void ImageCraftView::ContinueRefineBoundaryAction(const HyPoint &point, AC_EditInfo &edit_info, HyRect &change_rect)
{
    bool is_action_list_changed = false;
    m_ansi_canvas.ContinueRefineBoundary(point.x, point.y, edit_info, change_rect, is_action_list_changed);

    if (is_action_list_changed)
        UpdateActionHistory();
}

bool ImageCraftView::MergeBlockAction(const HyPoint &point, HyRect &change_rect, bool is_force_merge)
{
    bool is_success = m_ansi_canvas.MergeBlockAction(point, change_rect, is_force_merge);
    if (is_success == false)
        return false;

    UpdateActionHistory();

    return true;
}

bool ImageCraftView::AddTextAction(const HyPoint &point, LPCTSTR text_string, int text_color, HyRect &change_rect)
{
    bool is_success = m_ansi_canvas.AddText(point, text_string, text_color,change_rect);
    if (is_success == false)
        return false;

    UpdateActionHistory();

    return true;
}

bool ImageCraftView::ChangeColorAction(const HyPoint &start_point, const HyPoint &end_point, 
                                       const AnsiColor &color, bool is_continuous, HyRect &change_rect)
{
    bool is_action_list_changed = false;
    bool is_success = m_ansi_canvas.ChangeColor(start_point, end_point,
                                                color, is_continuous,
                                                change_rect, is_action_list_changed);
    if (is_success == false)
        return false;

    if (is_action_list_changed)
        UpdateActionHistory();

    return true;
}

bool ImageCraftView::ChangeColorAreaAction(const HyPoint &seed, const AnsiColor &color, HyRect &change_rect)
{
    bool is_success = m_ansi_canvas.ChangeColorArea(seed, color, change_rect);
    if (is_success == false)
        return false;

    UpdateActionHistory();

    return true;
}

void ImageCraftView::SetActionIndex(int new_index)
{
    HyRect change_rect;
    bool is_display_offset_moved = false;

    bool is_changed = m_ansi_canvas.SetActionIndex(new_index, change_rect, is_display_offset_moved);
    if (is_changed == false)
        return;

    UpdateActionHistory(false);

    if (is_display_offset_moved)
    {
        UpdateDisplayOffset();
        MakeAnsiImage();
        Invalidate(FALSE);
    }
    else if (change_rect.width > 0 && change_rect.height > 0)
    {
        UpdateAnsiImage(change_rect);
        Invalidate(FALSE);
    }
}

void ImageCraftView::UndoByHotKey()
{
    if (m_is_start_drag)
        return;

    SetActionIndex(m_ansi_canvas.GetCurrentActionIndex() - 1);
}

void ImageCraftView::RedoByHotKey()
{
    if (m_is_start_drag)
        return;

    SetActionIndex(m_ansi_canvas.GetCurrentActionIndex() + 1);
}

void ImageCraftView::SetPressKeyForCombo(bool is_press_control, bool is_press_shift)
{
    m_is_press_control_for_combo = is_press_control;
    m_is_press_shift_for_combo = is_press_shift;
}