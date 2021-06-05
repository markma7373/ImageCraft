#pragma once


// AdvancedSettingDialog dialog

class AdvancedSettingDialog : public CDialog
{
	DECLARE_DYNAMIC(AdvancedSettingDialog)

public:
	AdvancedSettingDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~AdvancedSettingDialog();

// Dialog Data
	enum { IDD = IDD_ADVANCED_SETTING_DIALOG };

    virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    virtual void OnOK();
    virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

private:
    void SetValueText(int item_id, LPCTSTR format, ...);
    void SetIntegerText(int item_id, int value);
    void GetIntegerValue(int item_id, int &value);

public:
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
    afx_msg void OnBnClickedButtonExportWidthDescription();
    afx_msg void OnBnClickedButtonBlockSizeDescription();
};
