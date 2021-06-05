// ObjProcessDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ImageCraft.h"
#include "AdvancedSettingDialog.h"

// AdvancedSettingDialog dialog

IMPLEMENT_DYNAMIC(AdvancedSettingDialog, CDialog)

AdvancedSettingDialog::AdvancedSettingDialog(CWnd* pParent /*=NULL*/)
: CDialog(AdvancedSettingDialog::IDD, pParent)
{

}

AdvancedSettingDialog::~AdvancedSettingDialog()
{

}

void AdvancedSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(AdvancedSettingDialog, CDialog)
    ON_WM_HSCROLL()
    ON_BN_CLICKED(IDC_BUTTON_EXPORT_WIDTH_DESCRIPTION, &AdvancedSettingDialog::OnBnClickedButtonExportWidthDescription)
    ON_BN_CLICKED(IDC_BUTTON_BLOCK_SIZE_DESCRIPTION, &AdvancedSettingDialog::OnBnClickedButtonBlockSizeDescription)
END_MESSAGE_MAP()

void AdvancedSettingDialog::SetValueText(int item_id, LPCTSTR format, ...)
{
    _TCHAR buffer[256];
    va_list marker;
    va_start(marker, format);
    _vstprintf(buffer, format, marker);
    va_end(marker);

    GetDlgItem(item_id)->SetWindowText(buffer);
}

void AdvancedSettingDialog::SetIntegerText(int item_id, int value)
{
    SetValueText(item_id, _T("%d"), value);
}

void AdvancedSettingDialog::GetIntegerValue(int item_id, int &value)
{
    CString value_str;
    GetDlgItem(item_id)->GetWindowText(value_str);

    value = _ttoi(value_str);
}

// AdvancedSettingDialog message handlers

BOOL AdvancedSettingDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    SetIntegerText(IDC_EDIT_BLOCK_SIZE, theApp.m_block_size);
    SetIntegerText(IDC_EDIT_EXPORT_WIDTH, theApp.m_export_width);

    return TRUE;
}

void AdvancedSettingDialog::OnOK()
{
    int new_value = 0;

    GetIntegerValue(IDC_EDIT_BLOCK_SIZE, new_value);
    new_value = FitInRange(new_value, GetMinBlockSize(), GetMaxBlockSize());
    if (new_value % 2 == 1) new_value++;

    theApp.GetMainFrame()->GetMainView()->SetBlockSize(new_value);

    GetIntegerValue(IDC_EDIT_EXPORT_WIDTH, new_value);
    if (new_value > 0)
        theApp.m_export_width = (DWORD)new_value;

    CDialog::OnOK();
}

void AdvancedSettingDialog::OnCancel()
{
    CDialog::OnCancel();
}

void AdvancedSettingDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{

}

void AdvancedSettingDialog::OnBnClickedButtonExportWidthDescription()
{
    std_tstring message_string;
    message_string += _T("普通BBS頁面中，可正常顯示顏色的最大寬度為79個半形字元。若超過此寬度，在上下捲動畫面時會顯示錯誤的顏色。\n");
    message_string += _T("建議使用的輸出寬度為78個半形字元，在儲存ANSI檔案時，仍會儲存至第79行的範圍，並將最後一個字元作為緩衝。\n");
    message_string += _T("若有一個全形字元橫跨第78-79行，其將只會顯示左半邊的內容與顏色，右半邊則被設為全黑色。\n");

    MessageBox(message_string.c_str(), _T("輸出內容寬度"), MB_OK | MB_ICONINFORMATION);
}

void AdvancedSettingDialog::OnBnClickedButtonBlockSizeDescription()
{
    std_tstring message_string;
    message_string += _T("設定每個ANSI全形方格在螢幕上占據的尺寸，以像素為單位。\n");
    message_string += _T("為了適當表示ANSI字元內容，此數值限定為16-32之間的偶數。\n");
    message_string += _T("當其為8的倍數時，會有最好的顯示品質。4的倍數則次之。\n");
    message_string += _T("若輸入數值超出範圍或為奇數，則會自動轉換為最接近的有效數值。\n");
    message_string += _T("改變此數值時，背景圖片不會移動，但ANSI內容會重新繪製。建議在繪圖前決定適當的數值，不要在繪圖過程中改動。\n");

    MessageBox(message_string.c_str(), _T("全形方格尺寸"), MB_OK | MB_ICONINFORMATION);

}
