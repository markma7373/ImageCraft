
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "ImageCraftView.h"
#include "ImageCraftDialog.h"
#include "AnsiProjectManager.h"
#include "UsageDialog.h"

class CustomCSplitterWnd : public CSplitterWnd
{
public:

    void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect)
    {
        if (nType == splitBorder)
        { 
            m_cxSplitterGap = 1;
            m_cySplitterGap = 1;
        }

        CSplitterWnd::OnDrawSplitter(pDC, nType, rect);
    }
};

class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* p_context);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CToolBar          m_wndToolBar;
	CStatusBar        m_wndStatusBar;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
    afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()

    bool m_is_create_view;
    CustomCSplitterWnd m_wnd_view_splitter; 

    void ReLayout();

public:
    UsageDialog * mp_usage_dialog;

    ImageCraftView *GetMainView();
    ImageCraftDialog *GetMainDialog();

    bool SaveAnsiProject(const std_tstring &project_path, const std_tstring &image_path);
    bool LoadAnsiProject(LPCTSTR project_path);

    void ChangeTitle(bool is_auto_save);

    afx_msg void OnSize(UINT type, int cx, int cy);
    afx_msg void OnLoadImage();
    afx_msg void OnLoadAnsi();
    afx_msg void OnSaveAnsi();
    afx_msg void OnExportAnsi();
    afx_msg void OnNewProject();
    afx_msg void OnSaveProject();
    afx_msg void OnSaveProjectAs();
    afx_msg void OnLoadProject();
    afx_msg void OnSettingExportFile();
    afx_msg void OnHelpUsage();
    afx_msg void OnEditUndo();
    afx_msg void OnEditRedo();
    afx_msg void OnEditCut();
    afx_msg void OnEditCopy();
    afx_msg void OnEditPaste();

private:
    std_tstring m_base_title;
    AnsiProjectManager m_project_manager;

    void SetInitialWindowRect();

    void SaveProjectAs();
    void SaveAnsiProjectToPath(LPCTSTR project_path);
    void ApplyProjectSettings(const AnsiProjectData &project_data);
    void CheckVirtualKeyForCombo();
};


