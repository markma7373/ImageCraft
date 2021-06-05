
// ImageCraft.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "ImageCraft.h"
#include "ImageCraftShare.h"
#include "MainFrm.h"


#define KEY_APNAME      L"SOFTWARE\\MarkMa\\ImageCraft"

// CImageCraftApp

BEGIN_MESSAGE_MAP(CImageCraftApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CImageCraftApp::OnAppAbout)
END_MESSAGE_MAP()


// CImageCraftApp construction

CImageCraftApp::CImageCraftApp()
{
    _TCHAR module_full_path[MAX_PATH];
    GetModuleFileName(NULL, module_full_path, MAX_PATH);
    m_module_path = ch_GetFilePath(module_full_path);
    _TCHAR module_base_name[MAX_PATH];
    _tcscpy(module_base_name, ch_GetFileBaseName(module_full_path).c_str());

    m_module_name = module_base_name;

    m_document_folder = ch_GetSpecialFolder(CSIDL_PERSONAL, module_base_name);

    m_debug_folder = m_document_folder + _T("/Debug");
    if (::PathFolderExists(m_debug_folder.c_str()) == false)
        _tmkdir(m_debug_folder.c_str());

    m_window_left = m_window_top = m_window_right = m_window_bottom = 0;

    m_block_size = GetDefaultBlockSize();
    m_image_alpha_ratio = 35;
    m_image_resize_ratio = 100;
    m_is_hide_image = false;
    m_is_move_image = false;
    m_export_width = 78;

    m_usage_menu = IC_USAGE_BASIC;
    m_usage_page = 0;

    m_select_block_mode = IC_SELECT_BLOCK_GEOMETRIC;

    m_control_dialog_y_position = 0;

    m_large_brush_size = 6;
    m_large_brush_shape = ANSI_BRUSH_CIRCLE;

    m_image_rotate_degree = 0.0f;
    m_image_scale_x = 0.0f;
    m_image_scale_y = 0.0f;
    m_image_roi_offset = hyPoint2D32f(0.0f, 0.0f);

    m_display_x_offset = 0;
    m_display_y_offset = 0;

    m_is_change_color_area_mode = false;
    m_is_refine_triangle_mode = false;
}

CImageCraftApp::~CImageCraftApp()
{
    Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

// The one and only CImageCraftApp object

CImageCraftApp theApp;


// CImageCraftApp initialization

BOOL CImageCraftApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    CRegKey regKey;
    if(regKey.Open(HKEY_CURRENT_USER, KEY_APNAME) != ERROR_SUCCESS)
        regKey.Create(HKEY_CURRENT_USER, KEY_APNAME);
    else
        LoadRegistryKey();

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

    CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);

	// The one and only window has been initialized, so show and update it
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}

int CImageCraftApp::ExitInstance()
{
    SaveRegistryKey();

    return CWinApp::ExitInstance();
}

void CImageCraftApp::LoadRegistryKey()
{
    CRegKey regKey;
    regKey.Open(HKEY_CURRENT_USER, KEY_APNAME);

    regKey.QueryDWORDValue(_T("ExportWidth"), m_export_width);
    regKey.QueryDWORDValue(_T("UsageMenu"), m_usage_menu);
    regKey.QueryDWORDValue(_T("UsagePage"), m_usage_page);

    QueryBoolValue(regKey, _T("ChangeColorAreaMode"), m_is_change_color_area_mode);
    QueryBoolValue(regKey, _T("RefineTriangleMode"), m_is_refine_triangle_mode);

    QueryIntValue(regKey, _T("ControlDialogYPosition"), m_control_dialog_y_position);
    QueryIntValue(regKey, _T("WindowLeftVer25"), m_window_left);
    QueryIntValue(regKey, _T("WindowTopVer25"), m_window_top);
    QueryIntValue(regKey, _T("WindowRightVer25"), m_window_right);
    QueryIntValue(regKey, _T("WindowBottomVer25"), m_window_bottom);
    QueryIntValue(regKey, _T("LargeBrushSize"), m_large_brush_size);
    QueryIntValue(regKey, _T("LargeBrushShape"), m_large_brush_shape);
    QueryIntValue(regKey, _T("SelectBlockMode"), m_select_block_mode);

    QueryStringValue(regKey, _T("LoadImageFolder"), m_load_image_folder);
    QueryStringValue(regKey, _T("AnsiFolder"), m_ansi_folder);
    QueryStringValue(regKey, _T("SaveProjectPath"), m_save_project_path);

    CheckSafeRegistryValue();
}

void CImageCraftApp::SaveRegistryKey()
{
    CRegKey regKey;
    regKey.Open(HKEY_CURRENT_USER, KEY_APNAME);

    regKey.SetDWORDValue(_T("ExportWidth"), m_export_width);
    regKey.SetDWORDValue(_T("UsageMenu"), m_usage_menu);
    regKey.SetDWORDValue(_T("UsagePage"), m_usage_page);
    
    SetBoolValue(regKey, _T("ChangeColorAreaMode"), m_is_change_color_area_mode);
    SetBoolValue(regKey, _T("RefineTriangleMode"), m_is_refine_triangle_mode);

    SetIntValue(regKey, _T("ControlDialogYPosition"), m_control_dialog_y_position);
    SetIntValue(regKey, _T("WindowLeftVer25"), m_window_left);
    SetIntValue(regKey, _T("WindowTopVer25"), m_window_top);
    SetIntValue(regKey, _T("WindowRightVer25"), m_window_right);
    SetIntValue(regKey, _T("WindowBottomVer25"), m_window_bottom);
    SetIntValue(regKey, _T("LargeBrushSize"), m_large_brush_size);
    SetIntValue(regKey, _T("LargeBrushShape"), m_large_brush_shape);
    SetIntValue(regKey, _T("SelectBlockMode"), m_select_block_mode);

    regKey.SetStringValue(_T("LoadImageFolder"), m_load_image_folder.c_str());
    regKey.SetStringValue(_T("AnsiFolder"), m_ansi_folder.c_str());
    regKey.SetStringValue(_T("SaveProjectPath"), m_save_project_path.c_str());
}

void CImageCraftApp::CheckSafeRegistryValue()
{
    m_large_brush_size = FitInRange(m_large_brush_size, MIN_LARGE_BRUSH_SIZE, MAX_LARGE_BRUSH_SIZE);
    m_large_brush_shape = FitInRange(m_large_brush_shape, 0, ANSI_BRUSH_SHAPE_AMOUNT - 1);
    m_select_block_mode = FitInRange(m_select_block_mode, 0, IC_SELECT_BLOCK_MODE_AMOUNT - 1);
}

CMainFrame *CImageCraftApp::GetMainFrame()
{
    return (CMainFrame *)AfxGetMainWnd();
}

void CImageCraftApp::QueryBoolValue(CRegKey &regkey, LPCTSTR name, bool &value)
{
    BYTE queried_value = 0;
    ULONG size_of_byte = sizeof(BYTE);
    regkey.QueryBinaryValue(name, &queried_value, &size_of_byte);

    value = (queried_value != 0);
}

void CImageCraftApp::QueryFloatValue(CRegKey &regkey, LPCTSTR name, float &value)
{
    ULONG size_of_float = sizeof(float);
    regkey.QueryBinaryValue(name, &value, &size_of_float);
}

void CImageCraftApp::QueryIntValue(CRegKey &regkey, LPCTSTR name, int &value)
{
    ULONG size_of_int = sizeof(int);
    regkey.QueryBinaryValue(name, &value, &size_of_int);
}

void CImageCraftApp::QueryStringValue(CRegKey &regkey, LPCTSTR name,
                                      std_tstring &dst_string, bool is_check_exist/* = true*/)
{
    _TCHAR temp_string[512]; // MAX_PATH may be not enough
    temp_string[0] = _T('\0');
    ULONG length = sizeof(temp_string) / sizeof(_TCHAR);
    regkey.QueryStringValue(name, temp_string, &length);

    bool is_valid = true;
    if (is_check_exist)
        is_valid = (::PathFileExists(temp_string) == TRUE);

    if (is_valid)
        dst_string = temp_string;
}

void CImageCraftApp::SetBoolValue(CRegKey &regkey, LPCTSTR name, bool value)
{
    BYTE byte_value = (value ? 1 : 0);

    regkey.SetBinaryValue(name, &byte_value, sizeof(BYTE));
}

void CImageCraftApp::SetFloatValue(CRegKey &regkey, LPCTSTR name, float value)
{
    ULONG size_of_float = sizeof(float);
    regkey.SetBinaryValue(name, &value, size_of_float);
}

void CImageCraftApp::SetIntValue(CRegKey &regkey, LPCTSTR name, int value)
{
    ULONG size_of_int = sizeof(int);
    regkey.SetBinaryValue(name, &value, size_of_int);
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CImageCraftApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();

    CMainFrame *p_mainframe = GetMainFrame();
    if (p_mainframe)
    {
        ImageCraftView *p_view = p_mainframe->GetMainView();
        if (p_view)
            p_view->SetFocus();
    }
}
