#include "stdafx.h"
#include "ImageCraft.h"
#include "TextInputDialog.h"

static const int DIALOG_ITEM_MARGIN = 8;

IMPLEMENT_DYNAMIC(TextInputDialog, CDialog)

TextInputDialog::TextInputDialog(CWnd* pParent /*=NULL*/)
: CDialog(TextInputDialog::IDD, pParent)
, mp_edit_font(NULL)
, mp_button_font(NULL)
{
    m_edit_text_size = 16;
    m_button_text_size = 16;

    m_is_layout_valid = false;
}

TextInputDialog::~TextInputDialog()
{
    _DELETE_PTR(mp_edit_font);
    _DELETE_PTR(mp_button_font);
}

void TextInputDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_EDIT_TEXT_INPUT, m_edit_text_input);
    DDX_Control(pDX, IDOK, m_button_ok);
    DDX_Control(pDX, IDCANCEL, m_button_cancel);
}

BEGIN_MESSAGE_MAP(TextInputDialog, CDialog)
    ON_EN_CHANGE(IDC_EDIT_TEXT_INPUT, &TextInputDialog::OnEnChangeEditTextInput)
END_MESSAGE_MAP()

void TextInputDialog::SetLayoutInfo(const CRect &screen_rect, const CRect &view_rect, 
                                    const HyPoint &text_offset, int text_size)
{
    m_text_offset.x = view_rect.left + text_offset.x;
    m_text_offset.y = view_rect.top + text_offset.y;
    m_edit_text_size = text_size;

    m_button_text_size = 16;
    if (m_edit_text_size > 16)
        m_button_text_size = 16 + (m_edit_text_size - 16) / 2;

    m_screen_rect = hyRect(screen_rect.left, screen_rect.top, screen_rect.Width(), screen_rect.Height());

    m_is_layout_valid = true;
}

BOOL TextInputDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect text_window_rect, text_client_rect;
    m_edit_text_input.GetWindowRect(text_window_rect);
    m_edit_text_input.GetClientRect(text_client_rect);
    int left_right_border = text_window_rect.Width() - text_client_rect.Width();
    int top_bottom_border = text_window_rect.Height() - text_client_rect.Height();
    m_text_left_shift = left_right_border / 2 + 1;
    m_text_right_extend = left_right_border + m_edit_text_size / 8;
    m_text_bottom_extend = top_bottom_border;

    m_button_ok_size = DetermineButtonSize(m_button_ok);
    m_button_cancel_size = DetermineButtonSize(m_button_cancel);

    m_nontext_total_size.width = m_button_ok_size.cx + DIALOG_ITEM_MARGIN + m_button_cancel_size.cx;
    m_nontext_total_size.height = ch_Max(m_button_ok_size.cy, m_button_cancel_size.cy);

    m_min_text_amount = 8;
    m_max_text_amount = m_min_text_amount;
    if (m_is_layout_valid)
    {
        int max_text_region_width = m_screen_rect.Right() - (m_text_offset.x - m_text_left_shift);
        max_text_region_width -= m_text_right_extend;
        m_max_text_amount = ch_Max(max_text_region_width / (m_edit_text_size / 2), m_min_text_amount);
    }

    m_text_amount = m_min_text_amount;

    mp_edit_font = new CFont();
    mp_edit_font->CreateFont(m_edit_text_size, 0, 0, 0,
                             FW_NORMAL, false, false, 0,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                             FIXED_PITCH|FF_MODERN, _T("細明體"));
    m_edit_text_input.SetFont(mp_edit_font);

    mp_button_font = new CFont();
    mp_button_font->CreateFont(m_button_text_size, 0, 0, 0,
                               FW_NORMAL, false, false, 0,
                               ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                               FIXED_PITCH|FF_MODERN, _T("細明體"));
    m_button_ok.SetFont(mp_button_font);
    m_button_cancel.SetFont(mp_button_font);

    UpdateLayout();

    GetDlgItem(IDC_EDIT_TEXT_INPUT)->PostMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
    GetDlgItem(IDC_EDIT_TEXT_INPUT)->PostMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));

    return TRUE;
}

CSize TextInputDialog::DetermineButtonSize(CButton &button)
{
    // Use the text size and content to determine the button size for layout.
    // Assume we use the font with uniform half/full width size.
    // (Otherwise we need to use Gdiplus::Graphics::MeasureCharacterRanges().)

    CString button_text;
    button.GetWindowText(button_text);

    int text_count = GetStringHalfWidthCount((LPCWSTR)button_text);
    int text_region_width = text_count * (m_button_text_size / 2);
    int text_region_height = m_button_text_size;

    // Add spaces for the button's border.
    text_region_width += 8;
    text_region_height += 8;

    return CSize(text_region_width, text_region_height);
}

void TextInputDialog::UpdateLayout()
{
    int text_input_width = m_text_amount * (m_edit_text_size / 2) + m_text_right_extend;
    int text_input_height = m_edit_text_size + m_text_bottom_extend;
    int total_width = ch_Max(text_input_width, m_nontext_total_size.width);
    int total_height = text_input_height + m_nontext_total_size.height;

    int dialog_offset_x = m_text_offset.x;
    int dialog_offset_y = m_text_offset.y + m_edit_text_size;
    if (m_is_layout_valid)
    {
        dialog_offset_x = ch_Min(dialog_offset_x, m_screen_rect.Right() - total_width);
        dialog_offset_x = ch_Max(dialog_offset_x, 0);

        int top_space = m_text_offset.y;
        int bottom_space = m_screen_rect.Bottom() - dialog_offset_y;
        if (bottom_space < ch_Min(total_height, top_space))
            dialog_offset_y = m_text_offset.y - total_height;
    }

    dialog_offset_x -= m_text_left_shift;

    CRect dialog_rect(CPoint(dialog_offset_x, dialog_offset_y), CSize(total_width, total_height));
    MoveWindow(dialog_rect);

    CRect text_input_rect(CPoint(0, 0), CSize(text_input_width, text_input_height));
    m_edit_text_input.MoveWindow(text_input_rect);
    
    CRect button_ok_rect(CPoint(0, text_input_height), m_button_ok_size);
    m_button_ok.MoveWindow(button_ok_rect);

    CRect button_cancel_rect(CPoint(button_ok_rect.Width() + DIALOG_ITEM_MARGIN, button_ok_rect.top), m_button_cancel_size);
    m_button_cancel.MoveWindow(button_cancel_rect);
}

BOOL TextInputDialog::PreTranslateMessage(MSG *p_message) 
{
    return CDialog::PreTranslateMessage(p_message);
}

void TextInputDialog::OnOK()
{
    CString input_text;
    m_edit_text_input.GetWindowText(input_text);

    m_input_text = input_text;

    CDialog::OnOK();
}

void TextInputDialog::OnCancel()
{
    CDialog::OnCancel();
}

void TextInputDialog::OnEnChangeEditTextInput()
{
    CString current_text;
    m_edit_text_input.GetWindowText(current_text);

    int text_count = GetStringHalfWidthCount((LPCWSTR)current_text);
    int new_text_amount = FitInRange(text_count + 2, m_min_text_amount, m_max_text_amount);
    if (new_text_amount != m_text_amount)
    {
        int old_select_start = 0;
        int old_select_end = 0;
        m_edit_text_input.GetSel(old_select_start, old_select_end);

        m_text_amount = new_text_amount;
        UpdateLayout();

        // Move the caret to start to show all texts, then set it back to old position.
        m_edit_text_input.SetSel(0, 0);
        m_edit_text_input.SetSel(old_select_start, old_select_end);
    }
}

std_tstring TextInputDialog::GetInputText()
{
    return m_input_text;
}