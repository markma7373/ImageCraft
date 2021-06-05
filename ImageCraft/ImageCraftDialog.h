#pragma once

#include "afxwin.h"
#include "afxcmn.h"
#include "DrawStatic.h"
#include "ColorSelector.h"
#include "AnsiShare.h"
#include "SelectorPanel.h"
#include "MyComboBox.h"

typedef std::vector<std::vector<AnsiCell> > AnsiBlockTable;

class ImageCraftDialog : public CFormView
{
    DECLARE_DYNCREATE(ImageCraftDialog)

protected:
    ImageCraftDialog();           // protected constructor used by dynamic creation
    virtual ~ImageCraftDialog();

public:
    enum { IDD = IDD_IMAGE_CRAFT_DIALOG };

    bool m_initialized;

    CSliderCtrl m_image_resize_ratio_slider;

    virtual void OnInitialUpdate();
    virtual BOOL PreTranslateMessage(MSG *p_message);

    void SetForegroundColor(int color);
    void SetBackgroundColor(int color);
    void SetColor(AnsiColor color);
    AnsiCell GetSelectedBlock();
    void SetDrawMode(int new_mode);
    void SetLastColorDrawMode();
    void SetHideImage(bool is_hide_image);
    void UpdateRotateDegreeUI();
    void UpdateBrushSizeUI();
    void UpdateBrushShapeUI();
    void UpdateSelectBlockModeUI();
    void UpdateChangeColorUI();
    void UpdateRefineBoundaryUI();
    void UpdateActionHistoryUI(int action_index, const std::vector<std_tstring> &action_list);
    void UpdateActionHistoryUI(int action_index); // Update index only when the list is not changed.

    static int m_init_dummy;
    static int InitGlobalData();

    void SetEnableImageUI();
    void UpdateUIByProjectSettings();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

private:
    ImageCraftView *mp_main_view;
    CToolTipCtrl *mp_tooltip_control;

    CSliderCtrl m_image_alpha_ratio_slider;
    CSliderCtrl m_image_rotate_degree_slider;

    ColorSelector m_color_selectors[ANSI_ALL_COLOR_COUNT];

    ColorSelector m_foreground_color;
    ColorSelector m_background_color;

    CDrawStatic m_static_block_selector;
    SelectorPanel m_block_selector;
    HySize m_block_selector_size;
    AnsiCell m_selected_block;

    CComboBox m_combo_select_block_mode;
    CComboBox m_combo_large_brush_shape;
    MyComboBox m_combo_action_list;

    int m_draw_mode;
    int m_last_color_draw_mode;
    int m_current_action_index;

    static AnsiBlockTable m_select_block_tables[IC_SELECT_BLOCK_MODE_AMOUNT];
    static int m_block_table_row_max_row_count;

    __forceinline bool IsScrollEnd(UINT sb_code);
    __forceinline void SetWindowTextToValue(int nID, int value);
    __forceinline void SetWindowTextToFormat(int nID, LPCTSTR format, ...);

    void AddToolTip(int item_id, LPCTSTR tip);
    void AddToolTip(const std::vector<int> &item_ids, LPCTSTR tip);
    bool IsUserConfirm(LPCTSTR title, LPCTSTR message);

    const AnsiBlockTable &GetBlockTable();
    void InitBlockSelector();
    void SelectDrawingBlock(int row_index, int col_index);
    void EnableMoveImageItems(bool is_enable);

public:
    afx_msg void OnDestroy();
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

    afx_msg void OnClickedSelectColor(UINT id, NMHDR *pNotifyStruct, LRESULT *result);
    afx_msg void OnBnClickedButtonTestContents();
    afx_msg void OnBnClickedRadioDrawMode();
    afx_msg void OnBnClickedCheckMoveImage();
    afx_msg void OnBnClickedButtonClearCanvas();
    afx_msg void OnBnClickedCheckHideImage();
    afx_msg void OnStnClickedStaticBlockSelector();
    afx_msg void OnBnClickedButtonDisplayRowUp();
    afx_msg void OnBnClickedButtonDisplayRowDown();
    afx_msg void OnBnClickedButtonDisplayPageUp();
    afx_msg void OnBnClickedButtonDisplayPageDown();
    afx_msg void OnBnClickedButtonTestDebugDump();
    afx_msg void OnCbnSelchangeComboSelectBlockMode();
    afx_msg void OnCbnSelchangeComboLargeBrushShape();
    afx_msg void OnBnClickedButtonBrushIncreaseSize();
    afx_msg void OnBnClickedButtonBrushDecreaseSize();
    afx_msg void OnBnClickedCheckChangeColorAreaMode();
    afx_msg void OnBnClickedCheckRefineTriangleMode();
    afx_msg void OnBnClickedButtonSelectBlockPrevTable();
    afx_msg void OnBnClickedButtonSelectBlockNextTable();
    afx_msg void OnCbnSelchangeComboActionList();
    afx_msg void OnBnClickedButtonUndoOneAction();
    afx_msg void OnBnClickedButtonRedoOneAction();
    afx_msg void OnBnClickedButtonCancelRotate();
};
