
// ImageCraft.h : main header file for the ImageCraft application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "use_gdiplus.h"

#include "MainFrm.h"

#define _IS_CHECKED(nID)            (((CButton *)GetDlgItem(nID))->GetCheck() == BST_CHECKED)
#define _SET_CHECK(nID, nCheck)     (nCheck)? ((CButton *)GetDlgItem(nID))->SetCheck(BST_CHECKED): ((CButton *)GetDlgItem(nID))->SetCheck(BST_UNCHECKED)


// CImageCraftApp:
// See ImageCraft.cpp for the implementation of this class
//

class CImageCraftApp : public CWinApp
{
public:
	CImageCraftApp();
    ~CImageCraftApp();

	virtual BOOL InitInstance();
    virtual int ExitInstance();

	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	afx_msg void OnAppAbout();

    std_tstring m_module_path;
    std_tstring m_module_name;
    std_tstring m_document_folder;
    std_tstring m_debug_folder;
    std_tstring m_load_image_folder;
    std_tstring m_ansi_folder;

    std_tstring m_image_path;
    std_tstring m_save_project_path;

    int m_window_left, m_window_top, m_window_right, m_window_bottom;

    DWORD m_block_size;
    DWORD m_image_alpha_ratio;
    DWORD m_image_resize_ratio;
    DWORD m_export_width;
    DWORD m_usage_menu;
    DWORD m_usage_page;

    bool m_is_hide_image;
    bool m_is_move_image;
    bool m_is_change_color_area_mode;
    bool m_is_refine_triangle_mode;
    
    int m_control_dialog_y_position;
    int m_large_brush_size;
    int m_large_brush_shape;
    int m_select_block_mode;

    float m_image_rotate_degree;
    float m_image_scale_x;
    float m_image_scale_y;
    HyPoint2D32f m_image_roi_offset;

    int m_display_x_offset, m_display_y_offset;

    CMainFrame *GetMainFrame();

protected:
	DECLARE_MESSAGE_MAP()

    ULONG_PTR m_gdiplusToken;

    void LoadRegistryKey();
    void SaveRegistryKey();

private:
    void QueryBoolValue(CRegKey &regkey, LPCTSTR name, bool &value);
    void QueryFloatValue(CRegKey &regkey, LPCTSTR name, float &value);
    void QueryIntValue(CRegKey &regkey, LPCTSTR name, int &value);
    void QueryStringValue(CRegKey &regkey, LPCTSTR name,
                          std_tstring &dst_string, bool is_check_exist = true);

    void SetBoolValue(CRegKey &regkey, LPCTSTR name, bool value);
    void SetFloatValue(CRegKey &regkey, LPCTSTR name, float value);
    void SetIntValue(CRegKey &regkey, LPCTSTR name, int value);

    void CheckSafeRegistryValue();
};

extern CImageCraftApp theApp;
