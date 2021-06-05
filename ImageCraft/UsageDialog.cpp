// ObjProcessDialog.cpp : implementation file
//

#include "stdafx.h"
#include <afxtoolbarimages.h>
#include "ImageCraft.h"
#include "UsageDialog.h"

IMPLEMENT_DYNAMIC(UsageDialog, CDialog)

UsageDialog::UsageDialog(CWnd* pParent /*=NULL*/)
: CDialog(UsageDialog::IDD, pParent)
, mp_tooltip_control(NULL)
{
    mp_parent = pParent;

    m_usage_menu = IC_USAGE_BASIC;
    m_usage_page = 0;
}

UsageDialog::~UsageDialog()
{
    _DELETE_PTR(mp_tooltip_control);
}

void UsageDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

    DDX_Radio(pDX, IDC_RADIO_USAGE_BASIC, m_usage_menu);
    DDX_Control(pDX, IDC_USAGE_IMAGE, m_usage_image);
    DDX_Control(pDX, IDC_TEXT_USAGE_DESCRIPTION, m_edit_usage_text);
}

BEGIN_MESSAGE_MAP(UsageDialog, CDialog)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_RADIO_USAGE_BASIC, OnBnClickedRadioUsageMenu)
    ON_BN_CLICKED(IDC_RADIO_USAGE_DRAW, OnBnClickedRadioUsageMenu)
    ON_BN_CLICKED(IDC_RADIO_USAGE_TEXT, OnBnClickedRadioUsageMenu)
    ON_BN_CLICKED(IDC_RADIO_USAGE_BRUSH, OnBnClickedRadioUsageMenu)
    ON_BN_CLICKED(IDC_RADIO_USAGE_COLOR, OnBnClickedRadioUsageMenu)
    ON_BN_CLICKED(IDC_RADIO_USAGE_BOUNDARY, OnBnClickedRadioUsageMenu)
    ON_BN_CLICKED(IDC_RADIO_USAGE_BOUNDARY_TRIANGLE, OnBnClickedRadioUsageMenu)
    ON_BN_CLICKED(IDC_RADIO_USAGE_MERGE, OnBnClickedRadioUsageMenu)
    ON_BN_CLICKED(IDC_RADIO_USAGE_ACTION, OnBnClickedRadioUsageMenu)
    ON_BN_CLICKED(IDC_BUTTON_USAGE_PREV_PAGE, &UsageDialog::OnBnClickedButtonUsagePrevPage)
    ON_BN_CLICKED(IDC_BUTTON_USAGE_NEXT_PAGE, &UsageDialog::OnBnClickedButtonUsageNextPage)
END_MESSAGE_MAP()

void UsageDialog::SetValueText(int item_id, LPCTSTR format, ...)
{
    _TCHAR buffer[256];
    va_list marker;
    va_start(marker, format);
    _vstprintf(buffer, format, marker);
    va_end(marker);

    GetDlgItem(item_id)->SetWindowText(buffer);
}

void UsageDialog::SetIntegerText(int item_id, int value)
{
    SetValueText(item_id, _T("%d"), value);
}

void UsageDialog::GetIntegerValue(int item_id, int &value)
{
    CString value_str;
    GetDlgItem(item_id)->GetWindowText(value_str);

    value = _ttoi(value_str);
}


BOOL UsageDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect view_rect;
    theApp.GetMainFrame()->GetMainView()->GetWindowRect(view_rect);
    CRect dialog_rect;
    GetWindowRect(dialog_rect);

    int new_left = (view_rect.left + view_rect.right - dialog_rect.Width()) / 2;
    int new_top = (view_rect.top + view_rect.bottom - dialog_rect.Height()) / 2;
    dialog_rect.MoveToXY(new_left, new_top);
    MoveWindow(dialog_rect);

    SetUsageMenuAndPage(theApp.m_usage_menu, theApp.m_usage_page);

    mp_tooltip_control = new CToolTipCtrl();
    mp_tooltip_control->Create(this);

    AddToolTip(IDC_RADIO_USAGE_BASIC, _T("基本使用與儲存／讀取功能"));
    AddToolTip(IDC_RADIO_USAGE_DRAW, _T("各種填入顏色的繪圖功能"));
    AddToolTip(IDC_RADIO_USAGE_TEXT, _T("在ANSI內容中輸入任意文字"));
    AddToolTip(IDC_RADIO_USAGE_BRUSH, _T("使用特殊形狀的筆刷填入顏色"));
    AddToolTip(IDC_RADIO_USAGE_COLOR, _T("挑選顏色與改變內容顏色的方式"));
    AddToolTip(IDC_RADIO_USAGE_BOUNDARY, _T("「邊緣調整」中一般模式的使用方法"));
    AddToolTip(IDC_RADIO_USAGE_BOUNDARY_TRIANGLE, _T("「邊緣調整」中三角形模式的使用方法"));
    AddToolTip(IDC_RADIO_USAGE_MERGE, _T("「合併全形方格」的使用方式與和其他功能的互動"));
    AddToolTip(IDC_RADIO_USAGE_ACTION, _T("繪圖動作的記錄方式與「復原／重做」功能"));

    mp_tooltip_control->Activate(TRUE);

    return TRUE;
}

void UsageDialog::AddToolTip(int item_id, LPCTSTR tip)
{
    if (mp_tooltip_control == NULL)
        return;

    CWnd *p_item = GetDlgItem(item_id);
    if (p_item == NULL)
        return;

    if (tip == NULL)
        mp_tooltip_control->DelTool(p_item);
    else
        mp_tooltip_control->AddTool(p_item, tip);
}

BOOL UsageDialog::PreTranslateMessage(MSG *p_message) 
{
    if(mp_tooltip_control != NULL)
       mp_tooltip_control->RelayEvent(p_message);

    return CDialog::PreTranslateMessage(p_message);
}

void UsageDialog::OnOK()
{
    // Prevent closing dialog from "OK" operation, like pressing Enter.
}

void UsageDialog::OnCancel()
{
    CDialog::OnCancel();
}

void UsageDialog::OnClose()
{
    CDialog::OnClose();
}

void UsageDialog::SetUsageMenuAndPage(int usage_menu, int usage_page)
{
    m_usage_menu = FitInRange(usage_menu, 0, (int)g_all_usage_data.size() - 1);
    m_usage_page = FitInRange(usage_page, 0, (int)g_all_usage_data[m_usage_menu].size() - 1);

    theApp.m_usage_menu = m_usage_menu;
    theApp.m_usage_page = m_usage_page;

    ShowUsagePage();

    UpdateData(FALSE);
}

void UsageDialog::ShowUsagePage()
{
    const std::vector<UsagePageData> &menu_data = g_all_usage_data[m_usage_menu];
    const UsagePageData &page_data = menu_data[m_usage_page];
    GetDlgItem(IDC_TEXT_USAGE_DESCRIPTION)->SetWindowText(page_data.description.c_str());

    SetValueText(IDC_TEXT_USAGE_PAGE_INDEX, _T("%d / %d"), m_usage_page + 1, menu_data.size());
    
    SetUsageImage(page_data.image_resource_id);
}

void UsageDialog::OnBnClickedRadioUsageMenu()
{
    UpdateData(TRUE);

    SetUsageMenuAndPage(m_usage_menu, 0);
}

void UsageDialog::OnBnClickedButtonUsagePrevPage()
{
    if (m_usage_page <= 0)
        return;

    m_usage_page--;
    theApp.m_usage_page = m_usage_page;

    ShowUsagePage();
}

void UsageDialog::OnBnClickedButtonUsageNextPage()
{
    if (m_usage_page >= (int)g_all_usage_data[m_usage_menu].size() - 1)
        return;

    m_usage_page++;
    theApp.m_usage_page = m_usage_page;

    ShowUsagePage();
}

void UsageDialog::SetUsageImage(int resource_id)
{
    CPngImage png_image;
    BOOL success = png_image.Load(resource_id, AfxGetInstanceHandle());

    BITMAP png_bitmap;
    png_image.GetBitmap(&png_bitmap);

    int width = png_bitmap.bmWidth;
    int height = png_bitmap.bmHeight;
    int channels = png_bitmap.bmBitsPixel / 8;
    BYTE *p_scan_start = (BYTE *)png_bitmap.bmBits + (height - 1) * png_bitmap.bmWidthBytes;
    int stride = -png_bitmap.bmWidthBytes; // BITMAP is upside-down

    m_usage_image.SetColorImage(p_scan_start, width, height, stride, channels);
    m_usage_image.Invalidate(FALSE);
}
