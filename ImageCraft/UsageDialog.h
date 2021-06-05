#pragma once

#include "UsageData.h"

// UsageDialog dialog

class UsageDialog : public CDialog
{
	DECLARE_DYNAMIC(UsageDialog)

public:
	UsageDialog(CWnd* pParent = NULL);
	virtual ~UsageDialog();

// Dialog Data
	enum { IDD = IDD_USAGE_DIALOG };

    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG *p_message);
    virtual void OnClose();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

    virtual void OnOK();
    virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

private:
    CWnd *mp_parent;
    CToolTipCtrl *mp_tooltip_control;
    CEdit m_edit_usage_text; // Read-only

    int m_usage_menu;
    int m_usage_page;

    CDrawStatic m_usage_image;

    void SetValueText(int item_id, LPCTSTR format, ...);
    void SetIntegerText(int item_id, int value);
    void GetIntegerValue(int item_id, int &value);
    void AddToolTip(int item_id, LPCTSTR tip);

    void SetUsageMenuAndPage(int usage_menu, int usage_page);
    void ShowUsagePage();
    void SetUsageImage(int resource_id);

public:
    afx_msg void OnBnClickedRadioUsageMenu();
    afx_msg void OnBnClickedButtonUsagePrevPage();
    afx_msg void OnBnClickedButtonUsageNextPage();
};
