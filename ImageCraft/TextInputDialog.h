#pragma once

#include "afxwin.h"
#include "use_hylib.h"
#include "MyEdit.h"

class TextInputDialog : public CDialog
{
	DECLARE_DYNAMIC(TextInputDialog)

public:
	TextInputDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~TextInputDialog();

// Dialog Data
	enum { IDD = IDD_TEXT_INPUT_DIALOG };

    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG *p_message);

    void SetLayoutInfo(const CRect &screen_rect, const CRect &view_rect, 
                       const HyPoint &text_offset, int text_size);
    std_tstring GetInputText();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    virtual void OnOK();
    virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

private:
    bool m_is_layout_valid;
    HyRect m_screen_rect;
    HyPoint m_text_offset;
    int m_edit_text_size;
    int m_button_text_size;

    CFont *mp_edit_font;
    CFont *mp_button_font;

    MyEdit m_edit_text_input;
    CButton m_button_ok;
    CButton m_button_cancel;

    CSize m_button_ok_size;
    CSize m_button_cancel_size;
    HySize m_nontext_total_size;
    int m_text_amount; // in half-width characters
    int m_min_text_amount;
    int m_max_text_amount;

    int m_text_left_shift;      // shift to align text with ANSI contents
    int m_text_right_extend;    // enlarge the text input region to show characters appropriately
    int m_text_bottom_extend;

    std_tstring m_input_text;

    CSize DetermineButtonSize(CButton &button);
    void UpdateLayout();

public:
    afx_msg void OnEnChangeEditTextInput();
};
