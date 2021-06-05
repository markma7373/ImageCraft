
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "ImageCraft.h"
#include "AdvancedSettingDialog.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEFAULT_APP_WIDTH   1304
#define DEFAULT_APP_HEIGHT  640
#define MIN_APP_WIDTH       640
#define MIN_APP_HEIGHT      480

#define EMPTY_PATH  _T("")


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
    ON_WM_SIZE()
    ON_WM_CLOSE()
    ON_COMMAND(ID_FILE_NEW_PROJECT, &CMainFrame::OnNewProject)
    ON_COMMAND(ID_FILE_SAVE_PROJECT, &CMainFrame::OnSaveProject)
    ON_COMMAND(ID_FILE_SAVE_PROJECT_AS, &CMainFrame::OnSaveProjectAs)
    ON_COMMAND(ID_FILE_LOAD_PROJECT, &CMainFrame::OnLoadProject)
    ON_COMMAND(ID_FILE_LOAD_IMAGE, &CMainFrame::OnLoadImage)
    ON_COMMAND(ID_FILE_LOAD_ANSI, &CMainFrame::OnLoadAnsi)
    ON_COMMAND(ID_FILE_SAVE_ANSI, &CMainFrame::OnSaveAnsi)
    ON_COMMAND(ID_FILE_EXPORT_ANSI, &CMainFrame::OnExportAnsi)
    ON_COMMAND(ID_SETTING_EXPORT_FILE, &CMainFrame::OnSettingExportFile)
    ON_COMMAND(ID_HELP_USAGE, &CMainFrame::OnHelpUsage)
    ON_COMMAND(ID_EDIT_UNDO, &CMainFrame::OnEditUndo)
    ON_COMMAND(ID_EDIT_REDO, &CMainFrame::OnEditRedo)
    ON_COMMAND(ID_EDIT_CUT, &CMainFrame::OnEditCut)
    ON_COMMAND(ID_EDIT_COPY, &CMainFrame::OnEditCopy)
    ON_COMMAND(ID_EDIT_PASTE, &CMainFrame::OnEditPaste)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
    ID_INDICATOR_RATIO, 
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

static UINT guide_buttons[] =
{
    ID_FILE_LOAD_PROJECT,
    ID_FILE_LOAD_IMAGE,
    ID_FILE_LOAD_ANSI,
};

enum SpliterWindowColumnIndex
{
    SPLITER_WINDOW_COL_IMAGE_CRAFT_VIEW = 0,
    SPLITER_WINDOW_COL_IMAGE_CRAFT_DIALOG,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
    m_is_create_view = false;
    mp_usage_dialog = NULL;
}

CMainFrame::~CMainFrame()
{
    _DELETE_PTR(mp_usage_dialog);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

    m_base_title = m_strTitle;

    if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

    if (!m_wndToolBar.Create(this) || !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE0("Failed to create toolbar\n");
        return -1;      // fail to create
    }
    m_wndToolBar.SetBarStyle( (m_wndToolBar.GetBarStyle() & ~CBRS_ALIGN_TOP ) 
                              | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_ALIGN_RIGHT);

    HICON icon_handle = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
    SetIcon(icon_handle, TRUE);

    SetInitialWindowRect();
    GetMainDialog()->ScrollToPosition(CPoint(0, theApp.m_control_dialog_y_position));

    GetMainView()->SetBlockSize(theApp.m_block_size, false);

    bool is_auto_save = LoadAnsiProject(GetAutoSaveProjectPath().c_str());
    if (is_auto_save)
        ChangeTitle(true);

    return 0;
}

void CMainFrame::SetInitialWindowRect()
{
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);

    int old_window_left = theApp.m_window_left;
    int old_window_top = theApp.m_window_top;
    int old_window_width = theApp.m_window_right - old_window_left;
    int old_window_height = theApp.m_window_bottom - old_window_top;

    bool is_valid_window = (old_window_width > 0 && old_window_height > 0) &&
                           (theApp.m_window_right > 0 && theApp.m_window_bottom > 0 &&
                            theApp.m_window_left < screen_width && theApp.m_window_top < screen_height);

    CRect window_rect;
    if (is_valid_window)
    {
        int window_width = ch_Max(old_window_width, MIN_APP_WIDTH);
        int window_height = ch_Max(old_window_height, MIN_APP_HEIGHT);

        window_rect.left = old_window_left;
        window_rect.top = old_window_top;
        window_rect.right = window_rect.left + window_width;
        window_rect.bottom = window_rect.top + window_height;
    }
    else
    {
        int window_width = ch_Min(DEFAULT_APP_WIDTH, screen_width);
        int window_height = ch_Min(DEFAULT_APP_HEIGHT, screen_height);

        window_rect.left = (screen_width - window_width) / 2;
        window_rect.top = (screen_height - window_height) / 2;
        window_rect.right = window_rect.left + window_width;
        window_rect.bottom = window_rect.top + window_height;
    }

    MoveWindow(window_rect);
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* p_context)
{
    UNREFERENCED_PARAMETER(lpcs);

    if (m_wnd_view_splitter.CreateStatic(this, 1, 2) == FALSE)
        return FALSE;

    if (!m_wnd_view_splitter.CreateView(0, SPLITER_WINDOW_COL_IMAGE_CRAFT_VIEW, RUNTIME_CLASS(ImageCraftView), CSize(0, 0), p_context) || 
        !m_wnd_view_splitter.CreateView(0, SPLITER_WINDOW_COL_IMAGE_CRAFT_DIALOG, RUNTIME_CLASS(ImageCraftDialog), CSize(0, 0), p_context))
    {
        m_wnd_view_splitter.DestroyWindow();
        return FALSE;
    }

    ReLayout();

    m_is_create_view = true;

    return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
    CFrameWnd::OnSize(nType, cx, cy);

    if (m_is_create_view)
    {
        ReLayout(); 
        GetMainView()->SetFocus();
    }
}

void CMainFrame::OnClose()
{
    CRect window_rect;
    GetWindowRect(&window_rect);
    theApp.m_window_left = window_rect.left;
    theApp.m_window_top = window_rect.top;
    theApp.m_window_right = window_rect.right;
    theApp.m_window_bottom = window_rect.bottom;

    CFrameWnd::OnClose();
}

void CMainFrame::ReLayout()
{
    CRect whole_view_rect;
    m_wnd_view_splitter.GetClientRect(&whole_view_rect);

    int dialog_panel_width = 250;

    m_wnd_view_splitter.SetColumnInfo(SPLITER_WINDOW_COL_IMAGE_CRAFT_VIEW, ch_Max(whole_view_rect.Width() - dialog_panel_width, 0), 0);
    m_wnd_view_splitter.SetColumnInfo(SPLITER_WINDOW_COL_IMAGE_CRAFT_DIALOG, dialog_panel_width, 0);
    
    m_wnd_view_splitter.RecalcLayout();
}

// CMainFrame diagnostics

// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	GetMainView()->SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (GetMainView()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// base class does the real work

	if (!CFrameWnd::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	// enable customization button for all user toolbars
	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	return TRUE;
}

ImageCraftView* CMainFrame::GetMainView()
{
    CWnd *p_wnd = m_wnd_view_splitter.GetPane(0, SPLITER_WINDOW_COL_IMAGE_CRAFT_VIEW);
    ImageCraftView *p_view = DYNAMIC_DOWNCAST(ImageCraftView, p_wnd);

    return p_view;
}

ImageCraftDialog* CMainFrame::GetMainDialog()
{
    CWnd *p_wnd = m_wnd_view_splitter.GetPane(0, SPLITER_WINDOW_COL_IMAGE_CRAFT_DIALOG);
    ImageCraftDialog *p_dialog = DYNAMIC_DOWNCAST(ImageCraftDialog, p_wnd);

    return p_dialog;
}

void CMainFrame::OnLoadImage()
{
    CheckVirtualKeyForCombo();

    CString file_filter = _T("圖片檔案 (*.jpg;*.jpeg;*.JPG;*.JPEG;*.png)|*.jpg;*.jpeg;*.JPG;*.JPEG;*.png|所有檔案 (*.*)|*.*||");
    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, file_filter);

    if (::PathFileExists(theApp.m_load_image_folder.c_str()))
        dlg.m_ofn.lpstrInitialDir = theApp.m_load_image_folder.c_str();

    if (dlg.DoModal() == IDOK)
    {
        CString file_path = dlg.GetPathName();

        // Record the folder of the loaded image.
        std_tstring new_folder = ch_GetFilePath(file_path);
        theApp.m_load_image_folder = new_folder.c_str();

        GetMainView()->LoadImageFile(file_path);
    }
}

void CMainFrame::OnLoadAnsi()
{
    CheckVirtualKeyForCombo();

    CString file_filter = _T("ANSI檔案 (*.ans)|*.ans||");
    CFileDialog dialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, file_filter);

    if (::PathFileExists(theApp.m_ansi_folder.c_str()))
        dialog.m_ofn.lpstrInitialDir = theApp.m_ansi_folder.c_str();

    if (dialog.DoModal() == IDOK)
    {
        CString file_path = dialog.GetPathName();
        theApp.m_ansi_folder = ch_GetFilePath(file_path);

        GetMainView()->LoadAnsi(file_path);
        GetMainView()->Invalidate(FALSE);
    }
}

void CMainFrame::OnSaveAnsi()
{
    CheckVirtualKeyForCombo();

    CString file_filter = _T("ANSI檔案 (*.ans)|*.ans||");
    CFileDialog dialog(FALSE, _T("ans"), _T("mypaint.ans"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, file_filter);
    dialog.m_ofn.lpstrDefExt = _T("ans");

    if (::PathFileExists(theApp.m_ansi_folder.c_str()))
        dialog.m_ofn.lpstrInitialDir = theApp.m_ansi_folder.c_str();

    if (dialog.DoModal() == IDOK)
    {
        CString file_path = dialog.GetPathName();
        theApp.m_ansi_folder = ch_GetFilePath(file_path);

        GetMainView()->SaveAnsi(file_path);
    }
}

void CMainFrame::OnExportAnsi()
{
    CheckVirtualKeyForCombo();

    CString file_filter = _T("ANSI檔案 (*.ans)|*.ans||");
    CFileDialog dialog(FALSE, _T("ans"), _T("mypaint_export.ans"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, file_filter);
    dialog.m_ofn.lpstrDefExt = _T("ans");

    if (::PathFileExists(theApp.m_ansi_folder.c_str()))
        dialog.m_ofn.lpstrInitialDir = theApp.m_ansi_folder.c_str();

    if (dialog.DoModal() == IDOK)
    {
        CString file_path = dialog.GetPathName();
        theApp.m_ansi_folder = ch_GetFilePath(file_path);

        GetMainView()->ExportAnsi(file_path, theApp.m_export_width);
    }
}

void CMainFrame::OnNewProject()
{
    CheckVirtualKeyForCombo();

    theApp.m_save_project_path.clear();

    // Change the setting to the initial state of AnsiProjectData.
    AnsiProjectData initial_data;
    ApplyProjectSettings(initial_data);

    GetMainView()->InitPainter(EMPTY_PATH, EMPTY_PATH);

    ChangeTitle(false);
}

void CMainFrame::OnSaveProject()
{
    CheckVirtualKeyForCombo();

    if (::PathFileExists(theApp.m_save_project_path.c_str()) == false)
        return SaveProjectAs();

    SaveAnsiProjectToPath(theApp.m_save_project_path.c_str());
}

void CMainFrame::OnSaveProjectAs()
{
    CheckVirtualKeyForCombo();

    SaveProjectAs();
}

void CMainFrame::SaveProjectAs()
{
    CString file_filter = _T("ANSI專案檔 (*.aproj)|*.aproj||");
    CFileDialog dialog(FALSE, _T("aproj"), _T("myproject.aproj"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, file_filter);
    dialog.m_ofn.lpstrDefExt = _T("aproj");

    std_tstring save_folder = ch_GetFilePath(theApp.m_save_project_path.c_str());
    if (::PathFileExists(save_folder.c_str()))
        dialog.m_ofn.lpstrInitialDir = save_folder.c_str();

    if (dialog.DoModal() == IDOK)
    {
        SaveAnsiProjectToPath(dialog.GetPathName());
    }
}

void CMainFrame::SaveAnsiProjectToPath(LPCTSTR project_path)
{
    bool is_success = SaveAnsiProject(project_path, theApp.m_image_path);

    if (is_success)
    {
        theApp.m_save_project_path = project_path;
        ChangeTitle(false);
    }
    else
    {
        CString error_string;
        error_string.Format(_T("儲存專案錯誤：\n%s"), m_project_manager.GetErrorMessage());
        MessageBox(error_string, _T("錯誤"), MB_OK | MB_ICONSTOP);
    }
}

bool CMainFrame::SaveAnsiProject(const std_tstring &project_path, const std_tstring &image_path)
{
    std_tstring ansi_path = GetTempSaveAnsiPath();
    GetMainView()->SaveAnsi(ansi_path.c_str());

    bool is_success = m_project_manager.SaveProject(project_path, image_path, ansi_path);
    if (is_success == false)
    {
        CString error_string;
        error_string.Format(_T("儲存專案錯誤：\n%s"), m_project_manager.GetErrorMessage());
        MessageBox(error_string, _T("錯誤"), MB_OK | MB_ICONSTOP);
    }

    return is_success;
}

void CMainFrame::OnLoadProject()
{
    CheckVirtualKeyForCombo();

    CString file_filter = _T("ANSI專案檔 (*.aproj)|*.aproj||");
    CFileDialog dialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, file_filter);

    std_tstring load_folder = ch_GetFilePath(theApp.m_save_project_path.c_str());
    if (::PathFileExists(load_folder.c_str()))
        dialog.m_ofn.lpstrInitialDir = load_folder.c_str();

    if (dialog.DoModal() == IDOK)
    {
        CString file_path = dialog.GetPathName();

        bool is_success = LoadAnsiProject(file_path);
        if (is_success)
        {
            theApp.m_save_project_path = file_path;
            ChangeTitle(false);
        }
        else
        {
            CString error_string;
            error_string.Format(_T("讀取專案錯誤：\n%s"), m_project_manager.GetErrorMessage());
            MessageBox(error_string, _T("錯誤"), MB_OK | MB_ICONSTOP);
        }
    }
}

bool CMainFrame::LoadAnsiProject(LPCTSTR project_path)
{
    AnsiProjectData project_data;

    bool is_success = m_project_manager.LoadProject(project_path, project_data,
                                                    theApp.m_document_folder.c_str());
    
    if (is_success)
        ApplyProjectSettings(project_data);

    return is_success;
}

void CMainFrame::ApplyProjectSettings(const AnsiProjectData &project_data)
{
    theApp.m_image_path = project_data.image_path;
    theApp.m_image_rotate_degree = project_data.image_rotate_degree;
    theApp.m_image_scale_x = project_data.image_scale_x;
    theApp.m_image_scale_y = project_data.image_scale_y;
    theApp.m_image_roi_offset.x = project_data.image_roi_offset.x;
    theApp.m_image_roi_offset.y = project_data.image_roi_offset.y;
    theApp.m_image_alpha_ratio = project_data.image_alpha_ratio;

    OldFormatData old_format_data;
    old_format_data.scaled_image_width = project_data.scaled_image_size.width;
    old_format_data.scaled_image_height = project_data.scaled_image_size.height;

    GetMainView()->SetBlockSize(project_data.block_size, false);
    GetMainView()->InitPainter(theApp.m_image_path.c_str(), project_data.ansi_path.c_str(), old_format_data);

    GetMainDialog()->UpdateUIByProjectSettings();
}

void CMainFrame::OnSettingExportFile()
{
    CheckVirtualKeyForCombo();

    AdvancedSettingDialog dialog;
    dialog.DoModal();
}

void CMainFrame::OnHelpUsage()
{
    CheckVirtualKeyForCombo();

    _DELETE_PTR(mp_usage_dialog);
    
    mp_usage_dialog = new UsageDialog(this);
    mp_usage_dialog->Create(UsageDialog::IDD);
    mp_usage_dialog->SetForegroundWindow();
    mp_usage_dialog->ShowWindow(SW_SHOW);
}

void CMainFrame::OnEditUndo()
{
    CheckVirtualKeyForCombo();

    GetMainView()->UndoByHotKey();
}

void CMainFrame::OnEditRedo()
{
    CheckVirtualKeyForCombo();

    GetMainView()->RedoByHotKey();
}

void CMainFrame::OnEditCut()
{
    CheckVirtualKeyForCombo();
}

void CMainFrame::OnEditCopy()
{
    CheckVirtualKeyForCombo();
}

void CMainFrame::OnEditPaste()
{
    CheckVirtualKeyForCombo();
}

void CMainFrame::CheckVirtualKeyForCombo()
{
    bool is_press_control = (GetKeyState(VK_CONTROL) < 0);
    bool is_press_shift = (GetKeyState(VK_SHIFT) < 0);

    GetMainView()->SetPressKeyForCombo(is_press_control, is_press_shift);
}

void CMainFrame::ChangeTitle(bool is_auto_save)
{
    std_tstring project_name = _T("未命名");

    LPCTSTR current_project_path = theApp.m_save_project_path.c_str();
    if (::PathFileExists(current_project_path))
        project_name = ch_GetFileBaseName(current_project_path);

    std_tstring new_title = project_name;;
    if (is_auto_save)
        new_title += _T(" (自動儲存)");

    new_title += _T(" - ");
    new_title += m_base_title;

    m_strTitle = new_title.c_str();
    OnUpdateFrameTitle(TRUE);
}