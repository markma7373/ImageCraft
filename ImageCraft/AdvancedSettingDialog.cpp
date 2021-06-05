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
    message_string += _T("���qBBS�������A�i���`����C�⪺�̤j�e�׬�79�ӥb�Φr���C�Y�W�L���e�סA�b�W�U���ʵe���ɷ|��ܿ��~���C��C\n");
    message_string += _T("��ĳ�ϥΪ���X�e�׬�78�ӥb�Φr���A�b�x�sANSI�ɮ׮ɡA���|�x�s�ܲ�79�檺�d��A�ñN�̫�@�Ӧr���@���w�ġC\n");
    message_string += _T("�Y���@�ӥ��Φr������78-79��A��N�u�|��ܥ��b�䪺���e�P�C��A�k�b��h�Q�]�����¦�C\n");

    MessageBox(message_string.c_str(), _T("��X���e�e��"), MB_OK | MB_ICONINFORMATION);
}

void AdvancedSettingDialog::OnBnClickedButtonBlockSizeDescription()
{
    std_tstring message_string;
    message_string += _T("�]�w�C��ANSI���Τ��b�ù��W�e�ڪ��ؤo�A�H���������C\n");
    message_string += _T("���F�A����ANSI�r�����e�A���ƭȭ��w��16-32���������ơC\n");
    message_string += _T("��䬰8�����ƮɡA�|���̦n����ܫ~��C4�����ƫh�����C\n");
    message_string += _T("�Y��J�ƭȶW�X�d��ά��_�ơA�h�|�۰��ഫ���̱��񪺦��ļƭȡC\n");
    message_string += _T("���ܦ��ƭȮɡA�I���Ϥ����|���ʡA��ANSI���e�|���sø�s�C��ĳ�bø�ϫe�M�w�A���ƭȡA���n�bø�ϹL�{����ʡC\n");

    MessageBox(message_string.c_str(), _T("���Τ��ؤo"), MB_OK | MB_ICONINFORMATION);

}
