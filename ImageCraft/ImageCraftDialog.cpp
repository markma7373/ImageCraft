#include "stdafx.h"
#include "ImageCraft.h"
#include "ImageCraftDialog.h"
#include "ImageCraftShare.h"
#include <afxtoolbarimages.h>

using namespace AnsiShare;

#define HIDE_DEBUG_ONLY_UI_ITEMS    1


// ImageCraftDialog

IMPLEMENT_DYNCREATE(ImageCraftDialog, CFormView)

AnsiBlockTable ImageCraftDialog::m_select_block_tables[IC_SELECT_BLOCK_MODE_AMOUNT];
int ImageCraftDialog::m_init_dummy = ImageCraftDialog::InitGlobalData();

int ImageCraftDialog::InitGlobalData()
{
    // geometric shapes
    AnsiBlockTable &geometric_table = m_select_block_tables[IC_SELECT_BLOCK_GEOMETRIC];
    geometric_table.resize(4);

    // Row 0: horizontal blocks
    for (int i = 1; i <= 8; i++)
        geometric_table[0].push_back(AnsiCell(HORI_BLOCK, i));

    // Row 1: vertical blocks
    for (int i = 1; i < 8; i++)
        geometric_table[1].push_back(AnsiCell(VERT_BLOCK, i));

    // Row 2: triangles
    for (int i = 0; i < 4; i++)
        geometric_table[2].push_back(AnsiCell(TRIANGLE, i));
    for (int i = 0; i < 2; i++)
        geometric_table[2].push_back(AnsiCell(REGULAR_TRIANGLE, i));

    // Row 3: special shapes;
    const wchar_t special_shape_list[] = {L'◆', L'●', L'■', L'▓', L'★'};
    const int special_shape_list_size = sizeof(special_shape_list) / sizeof(special_shape_list[0]);
    for (int i = 0; i < special_shape_list_size; i++)
        geometric_table[3].push_back(CharToAnsiCell(special_shape_list[i]));

    // signs and marks
    AnsiBlockTable &sign_mark_table = m_select_block_tables[IC_SELECT_BLOCK_SIGN_MARK];
    sign_mark_table.resize(4);

    // Row 1/2: signs
    const wchar_t sign_list1[] = {L'○', L'⊙', L'◎', L'●', L'☆', L'★', L'□', L'■'};
    const int sign_list1_size = sizeof(sign_list1) / sizeof(sign_list1[0]);
    for (int i = 0; i < sign_list1_size; i++)
        sign_mark_table[0].push_back(CharToAnsiCell(sign_list1[i]));

    const wchar_t sign_list2[] = {L'▼', L'▲', L'▽', L'△', L'◇', L'◆', L'♀', L'♂'};
    const int sign_list2_size = sizeof(sign_list2) / sizeof(sign_list2[0]);
    for (int i = 0; i < sign_list2_size; i++)
        sign_mark_table[1].push_back(CharToAnsiCell(sign_list2[i]));

    // Row 3/4: marks
    const wchar_t mark_list1[] = {L'﹌', L'﹏', L'︴', L'¯', L'＿', L'—', L'∥', L'∣'};
    const int mark_list1_size = sizeof(mark_list1) / sizeof(mark_list1[0]);
    for (int i = 0; i < mark_list1_size; i++)
        sign_mark_table[2].push_back(CharToAnsiCell(mark_list1[i]));

    const wchar_t mark_list2[] = {L'▕', L'／', L'＼', L'╳', L'╱', L'╲', L'∕', L'﹨'};
    const int mark_list2_size = sizeof(mark_list2) / sizeof(mark_list2[0]);
    for (int i = 0; i < mark_list2_size; i++)
        sign_mark_table[3].push_back(CharToAnsiCell(mark_list2[i]));

    // math symbols
    AnsiBlockTable &math_table = m_select_block_tables[IC_SELECT_BLOCK_MATH];
    math_table.resize(4);
    
    const wchar_t math_list1[] = {L'＋', L'－', L'×', L'÷', L'√', L'±', L'＝', L'≡'};
    const int math_list1_size = sizeof(math_list1) / sizeof(math_list1[0]);
    for (int i = 0; i < math_list1_size; i++)
        math_table[0].push_back(CharToAnsiCell(math_list1[i]));

    const wchar_t math_list2[] = {L'≠', L'≒', L'≦', L'≧', L'＜', L'＞', L'∵', L'∴'};
    const int math_list2_size = sizeof(math_list2) / sizeof(math_list2[0]);
    for (int i = 0; i < math_list2_size; i++)
        math_table[1].push_back(CharToAnsiCell(math_list2[i]));

    const wchar_t math_list3[] = {L'∞', L'～', L'∩', L'∪', L'∫', L'∮', L'＆', L'⊥'};
    const int math_list3_size = sizeof(math_list3) / sizeof(math_list3[0]);
    for (int i = 0; i < math_list3_size; i++)
        math_table[2].push_back(CharToAnsiCell(math_list3[i]));

    const wchar_t math_list4[] = {L'∠', L'∟', L'⊿', L'﹢', L'﹣', L'﹤', L'﹥', L'﹦'};
    const int math_list4_size = sizeof(math_list4) / sizeof(math_list4[0]);
    for (int i = 0; i < math_list4_size; i++)
        math_table[3].push_back(CharToAnsiCell(math_list4[i]));

    // bracelets
    AnsiBlockTable &bracelet_table = m_select_block_tables[IC_SELECT_BLOCK_BRACELET];
    bracelet_table.resize(4);

    const wchar_t bracelet_list1[] = {L'【', L'】', L'「', L'」', L'『', L'』', L'〈', L'〉'};
    const int bracelet_list1_size = sizeof(bracelet_list1) / sizeof(bracelet_list1[0]);
    for (int i = 0; i < bracelet_list1_size; i++)
        bracelet_table[0].push_back(CharToAnsiCell(bracelet_list1[i]));

    const wchar_t bracelet_list2[] = {L'《', L'》', L'〔', L'〕', L'｛', L'｝', L'（', L'）'};
    const int bracelet_list2_size = sizeof(bracelet_list2) / sizeof(bracelet_list2[0]);
    for (int i = 0; i < bracelet_list2_size; i++)
        bracelet_table[1].push_back(CharToAnsiCell(bracelet_list2[i]));

    const wchar_t bracelet_list3[] = {L'︹', L'︺', L'︷', L'︸', L'︻', L'︼', L'︿', L'﹀'};
    const int bracelet_list3_size = sizeof(bracelet_list3) / sizeof(bracelet_list3[0]);
    for (int i = 0; i < bracelet_list3_size; i++)
        bracelet_table[2].push_back(CharToAnsiCell(bracelet_list3[i]));

    const wchar_t bracelet_list4[] = {L'︽', L'︾', L'﹁', L'﹂', L'﹃', L'﹄', L'︵', L'︶'};
    const int bracelet_list4_size = sizeof(bracelet_list4) / sizeof(bracelet_list4[0]);
    for (int i = 0; i < bracelet_list4_size; i++)
        bracelet_table[3].push_back(CharToAnsiCell(bracelet_list4[i]));

    // arrow, punctuation, and some other marks
    AnsiBlockTable &arrow_punctuation_others_table = m_select_block_tables[IC_SELECT_BLOCK_ARROW_PUNCTUATION_OTHERS];
    arrow_punctuation_others_table.resize(4);

    // Row 1: arrows
    const wchar_t arrow_list[] = {L'↑', L'↓', L'←', L'→', L'↖', L'↗', L'↙', L'↘'};
    const int arrow_list_size = sizeof(arrow_list) / sizeof(arrow_list[0]);
    for (int i = 0; i < arrow_list_size; i++)
        arrow_punctuation_others_table[0].push_back(CharToAnsiCell(arrow_list[i]));  

    // Row 2/3: punctuations
    const wchar_t punctuation_list1[] = {L'，', L'；', L'：', L'、', L'。', L'？', L'！', L'‧'};
    const int punctuation_list1_size = sizeof(punctuation_list1) / sizeof(punctuation_list1[0]);
    for (int i = 0; i < punctuation_list1_size; i++)
        arrow_punctuation_others_table[1].push_back(CharToAnsiCell(punctuation_list1[i]));  

    const wchar_t punctuation_list2[] = {L'〝', L'〞', L'‵', L'′', L'…', L'‥', L'﹑', L'※'};
    const int punctuation_list2_size = sizeof(punctuation_list2) / sizeof(punctuation_list2[0]);
    for (int i = 0; i < punctuation_list2_size; i++)
        arrow_punctuation_others_table[2].push_back(CharToAnsiCell(punctuation_list2[i]));  

    // Row 4: others
    const wchar_t others_list[] = {L'﹡', L'＊', L'§', L'＠', L'⊕', L'㊣', L'﹉', L'﹍'};
    const int others_list_size = sizeof(others_list) / sizeof(others_list[0]);
    for (int i = 0; i < others_list_size; i++)
        arrow_punctuation_others_table[3].push_back(CharToAnsiCell(others_list[i]));

    // table line (1)
    AnsiBlockTable &table_line1_table = m_select_block_tables[IC_SELECT_BLOCK_TABLE_LINE1];
    table_line1_table.resize(4);

    const wchar_t table_line1_list1[] = {L'┌', L'┬', L'┐', L'╔', L'╦', L'╗', L'╭', L'╮'};
    const int table_line1_list1_size = sizeof(table_line1_list1) / sizeof(table_line1_list1[0]);
    for (int i = 0; i < table_line1_list1_size; i++)
        table_line1_table[0].push_back(CharToAnsiCell(table_line1_list1[i]));

    const wchar_t table_line1_list2[] = {L'├', L'┼', L'┤', L'╠', L'╬', L'╣', L'╰', L'╯'};
    const int table_line1_list2_size = sizeof(table_line1_list2) / sizeof(table_line1_list2[0]);
    for (int i = 0; i < table_line1_list2_size; i++)
        table_line1_table[1].push_back(CharToAnsiCell(table_line1_list2[i]));

    const wchar_t table_line1_list3[] = {L'└', L'┴', L'┘', L'╚', L'╩', L'╝'};
    const int table_line1_list3_size = sizeof(table_line1_list3) / sizeof(table_line1_list3[0]);
    for (int i = 0; i < table_line1_list3_size; i++)
        table_line1_table[2].push_back(CharToAnsiCell(table_line1_list3[i]));

    const wchar_t table_line1_list4[] = {L'─', L'│', L'═', L'║'};
    const int table_line1_list4_size = sizeof(table_line1_list4) / sizeof(table_line1_list4[0]);
    for (int i = 0; i < table_line1_list4_size; i++)
        table_line1_table[3].push_back(CharToAnsiCell(table_line1_list4[i]));
    
    // table line (2)
    AnsiBlockTable &table_line2_table = m_select_block_tables[IC_SELECT_BLOCK_TABLE_LINE2];
    table_line2_table.resize(3);

    const wchar_t table_line2_list1[] = {L'╓', L'╥', L'╖', L'╒', L'╤', L'╕'};
    const int table_line2_list1_size = sizeof(table_line2_list1) / sizeof(table_line2_list1[0]);
    for (int i = 0; i < table_line2_list1_size; i++)
        table_line2_table[0].push_back(CharToAnsiCell(table_line2_list1[i]));

    const wchar_t table_line2_list2[] = {L'╟', L'╫', L'╢', L'╞', L'╪', L'╡'};
    const int table_line2_list2_size = sizeof(table_line2_list2) / sizeof(table_line2_list2[0]);
    for (int i = 0; i < table_line2_list2_size; i++)
        table_line2_table[1].push_back(CharToAnsiCell(table_line2_list2[i]));

    const wchar_t table_line2_list3[] = {L'╙', L'╨', L'╜', L'╘', L'╧', L'╛'};
    const int table_line2_list3_size = sizeof(table_line2_list3) / sizeof(table_line2_list3[0]);
    for (int i = 0; i < table_line2_list3_size; i++)
        table_line2_table[2].push_back(CharToAnsiCell(table_line2_list3[i]));

    // greek uppercase alphabets
    AnsiBlockTable &greek_upper_table = m_select_block_tables[IC_SELECT_BLOCK_GREEK_UPPER];
    greek_upper_table.resize(3);

    const wchar_t greek_upper_list1[] = {L'Α', L'Β', L'Γ', L'Δ', L'Ε', L'Ζ', L'Η', L'Θ'};
    const int greek_upper_list1_size = sizeof(greek_upper_list1) / sizeof(greek_upper_list1[0]);
    for (int i = 0; i < greek_upper_list1_size; i++)
        greek_upper_table[0].push_back(CharToAnsiCell(greek_upper_list1[i]));

    const wchar_t greek_upper_list2[] = {L'Ι', L'Κ', L'Λ', L'Μ', L'Ν', L'Ξ', L'Ο', L'Π'};
    const int greek_upper_list2_size = sizeof(greek_upper_list2) / sizeof(greek_upper_list2[0]);
    for (int i = 0; i < greek_upper_list2_size; i++)
        greek_upper_table[1].push_back(CharToAnsiCell(greek_upper_list2[i]));

    const wchar_t greek_upper_list3[] = {L'Ρ', L'Σ', L'Τ', L'Υ', L'Φ', L'Χ', L'Ψ', L'Ω'};
    const int greek_upper_list3_size = sizeof(greek_upper_list3) / sizeof(greek_upper_list3[0]);
    for (int i = 0; i < greek_upper_list3_size; i++)
        greek_upper_table[2].push_back(CharToAnsiCell(greek_upper_list3[i]));

    // greek lowercase alphabets
    AnsiBlockTable &greek_lower_table = m_select_block_tables[IC_SELECT_BLOCK_GREEK_LOWER];
    greek_lower_table.resize(3);

    const wchar_t greek_lower_list1[] = {L'α', L'β', L'γ', L'δ', L'ε', L'ζ', L'η', L'θ'};
    const int greek_lower_list1_size = sizeof(greek_lower_list1) / sizeof(greek_lower_list1[0]);
    for (int i = 0; i < greek_lower_list1_size; i++)
        greek_lower_table[0].push_back(CharToAnsiCell(greek_lower_list1[i]));

    const wchar_t greek_lower_list2[] = {L'ι', L'κ', L'λ', L'μ', L'ν', L'ξ', L'ο', L'π'};
    const int greek_lower_list2_size = sizeof(greek_lower_list2) / sizeof(greek_lower_list2[0]);
    for (int i = 0; i < greek_lower_list2_size; i++)
        greek_lower_table[1].push_back(CharToAnsiCell(greek_lower_list2[i]));

    const wchar_t greek_lower_list3[] = {L'ρ', L'σ', L'τ', L'υ', L'φ', L'χ', L'ψ', L'ω'};
    const int greek_lower_list3_size = sizeof(greek_lower_list3) / sizeof(greek_lower_list3[0]);
    for (int i = 0; i < greek_lower_list3_size; i++)
        greek_lower_table[2].push_back(CharToAnsiCell(greek_lower_list3[i]));

    // japanese hiragana (1)
    AnsiBlockTable &japanese_hiragana1_table = m_select_block_tables[IC_SELECT_BLOCK_JAPANESE_HIRAGANA1];
    japanese_hiragana1_table.resize(4);

    const wchar_t japanese_hiragana1_list1[] = {0x3041, 0x3042, 0x3043, 0x3044, 0x3045, 0x3046, 0x3047, 0x3048};
    const int japanese_hiragana1_list1_size = sizeof(japanese_hiragana1_list1) / sizeof(japanese_hiragana1_list1[0]);
    for (int i = 0; i < japanese_hiragana1_list1_size; i++)
        japanese_hiragana1_table[0].push_back(CharToAnsiCell(japanese_hiragana1_list1[i]));

    const wchar_t japanese_hiragana1_list2[] = {0x3049, 0x304a, 0x304b, 0x304c, 0x304d, 0x304e, 0x304f, 0x3050};
    const int japanese_hiragana1_list2_size = sizeof(japanese_hiragana1_list2) / sizeof(japanese_hiragana1_list2[0]);
    for (int i = 0; i < japanese_hiragana1_list2_size; i++)
        japanese_hiragana1_table[1].push_back(CharToAnsiCell(japanese_hiragana1_list2[i]));

    const wchar_t japanese_hiragana1_list3[] = {0x3051, 0x3052, 0x3053, 0x3054, 0x3055, 0x3056, 0x3057, 0x3058};
    const int japanese_hiragana1_list3_size = sizeof(japanese_hiragana1_list3) / sizeof(japanese_hiragana1_list3[0]);
    for (int i = 0; i < japanese_hiragana1_list3_size; i++)
        japanese_hiragana1_table[2].push_back(CharToAnsiCell(japanese_hiragana1_list3[i]));

    const wchar_t japanese_hiragana1_list4[] = {0x3059, 0x305a, 0x305b, 0x305c, 0x305d, 0x305e, 0x305f, 0x3060};
    const int japanese_hiragana1_list4_size = sizeof(japanese_hiragana1_list4) / sizeof(japanese_hiragana1_list4[0]);
    for (int i = 0; i < japanese_hiragana1_list4_size; i++)
        japanese_hiragana1_table[3].push_back(CharToAnsiCell(japanese_hiragana1_list4[i]));

    // japanese hiragana (2)
    AnsiBlockTable &japanese_hiragana2_table = m_select_block_tables[IC_SELECT_BLOCK_JAPANESE_HIRAGANA2];
    japanese_hiragana2_table.resize(4);

    const wchar_t japanese_hiragana2_list1[] = {0x3061, 0x3062, 0x3063, 0x3064, 0x3065, 0x3066, 0x3067, 0x3068};
    const int japanese_hiragana2_list1_size = sizeof(japanese_hiragana2_list1) / sizeof(japanese_hiragana2_list1[0]);
    for (int i = 0; i < japanese_hiragana2_list1_size; i++)
        japanese_hiragana2_table[0].push_back(CharToAnsiCell(japanese_hiragana2_list1[i]));

    const wchar_t japanese_hiragana2_list2[] = {0x3069, 0x306a, 0x306b, 0x306c, 0x306d, 0x306e, 0x306f, 0x3070};
    const int japanese_hiragana2_list2_size = sizeof(japanese_hiragana2_list2) / sizeof(japanese_hiragana2_list2[0]);
    for (int i = 0; i < japanese_hiragana2_list2_size; i++)
        japanese_hiragana2_table[1].push_back(CharToAnsiCell(japanese_hiragana2_list2[i]));

    const wchar_t japanese_hiragana2_list3[] = {0x3071, 0x3072, 0x3073, 0x3074, 0x3075, 0x3076, 0x3077, 0x3078};
    const int japanese_hiragana2_list3_size = sizeof(japanese_hiragana2_list3) / sizeof(japanese_hiragana2_list3[0]);
    for (int i = 0; i < japanese_hiragana2_list3_size; i++)
        japanese_hiragana2_table[2].push_back(CharToAnsiCell(japanese_hiragana2_list3[i]));

    const wchar_t japanese_hiragana2_list4[] = {0x3079, 0x307a, 0x307b, 0x307c, 0x307d, 0x307e, 0x307f, 0x3080};
    const int japanese_hiragana2_list4_size = sizeof(japanese_hiragana2_list4) / sizeof(japanese_hiragana2_list4[0]);
    for (int i = 0; i < japanese_hiragana2_list4_size; i++)
        japanese_hiragana2_table[3].push_back(CharToAnsiCell(japanese_hiragana2_list4[i]));

    // japanese hiragana (3)
    AnsiBlockTable &japanese_hiragana3_table = m_select_block_tables[IC_SELECT_BLOCK_JAPANESE_HIRAGANA3];
    japanese_hiragana3_table.resize(4);

    const wchar_t japanese_hiragana3_list1[] = {0x3081, 0x3082, 0x3083, 0x3084, 0x3085, 0x3086, 0x3087, 0x3088};
    const int japanese_hiragana3_list1_size = sizeof(japanese_hiragana3_list1) / sizeof(japanese_hiragana3_list1[0]);
    for (int i = 0; i < japanese_hiragana3_list1_size; i++)
        japanese_hiragana3_table[0].push_back(CharToAnsiCell(japanese_hiragana3_list1[i]));

    const wchar_t japanese_hiragana3_list2[] = {0x3089, 0x308a, 0x308b, 0x308c, 0x308d, 0x308e, 0x308f, 0x3090};
    const int japanese_hiragana3_list2_size = sizeof(japanese_hiragana3_list2) / sizeof(japanese_hiragana3_list2[0]);
    for (int i = 0; i < japanese_hiragana3_list2_size; i++)
        japanese_hiragana3_table[1].push_back(CharToAnsiCell(japanese_hiragana3_list2[i]));

    const wchar_t japanese_hiragana3_list3[] = {0x3091, 0x3092, 0x3093, 0x3094, 0x30fc};
    const int japanese_hiragana3_list3_size = sizeof(japanese_hiragana3_list3) / sizeof(japanese_hiragana3_list3[0]);
    for (int i = 0; i < japanese_hiragana3_list3_size; i++)
        japanese_hiragana3_table[2].push_back(CharToAnsiCell(japanese_hiragana3_list3[i]));

    const wchar_t japanese_hiragana3_list4[] = {0x309d, 0x309e, 0x309b, 0x309c};
    const int japanese_hiragana3_list4_size = sizeof(japanese_hiragana3_list4) / sizeof(japanese_hiragana3_list4[0]);
    for (int i = 0; i < japanese_hiragana3_list4_size; i++)
        japanese_hiragana3_table[3].push_back(CharToAnsiCell(japanese_hiragana3_list4[i]));

    // japanese katakana (1)
    AnsiBlockTable &japanese_katakana1_table = m_select_block_tables[IC_SELECT_BLOCK_JAPANESE_KATAKANA1];
    japanese_katakana1_table.resize(4);

    const wchar_t japanese_katakana1_list1[] = {0x30a1, 0x30a2, 0x30a3, 0x30a4, 0x30a5, 0x30a6, 0x30a7, 0x30a8};
    const int japanese_katakana1_list1_size = sizeof(japanese_katakana1_list1) / sizeof(japanese_katakana1_list1[0]);
    for (int i = 0; i < japanese_katakana1_list1_size; i++)
        japanese_katakana1_table[0].push_back(CharToAnsiCell(japanese_katakana1_list1[i]));

    const wchar_t japanese_katakana1_list2[] = {0x30a9, 0x30aa, 0x30ab, 0x30ac, 0x30ad, 0x30ae, 0x30af, 0x30b0};
    const int japanese_katakana1_list2_size = sizeof(japanese_katakana1_list2) / sizeof(japanese_katakana1_list2[0]);
    for (int i = 0; i < japanese_katakana1_list2_size; i++)
        japanese_katakana1_table[1].push_back(CharToAnsiCell(japanese_katakana1_list2[i]));

    const wchar_t japanese_katakana1_list3[] = {0x30b1, 0x30b2, 0x30b3, 0x30b4, 0x30b5, 0x30b6, 0x30b7, 0x30b8};
    const int japanese_katakana1_list3_size = sizeof(japanese_katakana1_list3) / sizeof(japanese_katakana1_list3[0]);
    for (int i = 0; i < japanese_katakana1_list3_size; i++)
        japanese_katakana1_table[2].push_back(CharToAnsiCell(japanese_katakana1_list3[i]));

    const wchar_t japanese_katakana1_list4[] = {0x30b9, 0x30ba, 0x30bb, 0x30bc, 0x30bd, 0x30be, 0x30bf, 0x30c0};
    const int japanese_katakana1_list4_size = sizeof(japanese_katakana1_list4) / sizeof(japanese_katakana1_list4[0]);
    for (int i = 0; i < japanese_katakana1_list4_size; i++)
        japanese_katakana1_table[3].push_back(CharToAnsiCell(japanese_katakana1_list4[i]));

    // japanese katakana (2)
    AnsiBlockTable &japanese_katakana2_table = m_select_block_tables[IC_SELECT_BLOCK_JAPANESE_KATAKANA2];
    japanese_katakana2_table.resize(4);

    const wchar_t japanese_katakana2_list1[] = {0x30c1, 0x30c2, 0x30c3, 0x30c4, 0x30c5, 0x30c6, 0x30c7, 0x30c8};
    const int japanese_katakana2_list1_size = sizeof(japanese_katakana2_list1) / sizeof(japanese_katakana2_list1[0]);
    for (int i = 0; i < japanese_katakana2_list1_size; i++)
        japanese_katakana2_table[0].push_back(CharToAnsiCell(japanese_katakana2_list1[i]));

    const wchar_t japanese_katakana2_list2[] = {0x30c9, 0x30ca, 0x30cb, 0x30cc, 0x30cd, 0x30ce, 0x30cf, 0x30d0};
    const int japanese_katakana2_list2_size = sizeof(japanese_katakana2_list2) / sizeof(japanese_katakana2_list2[0]);
    for (int i = 0; i < japanese_katakana2_list2_size; i++)
        japanese_katakana2_table[1].push_back(CharToAnsiCell(japanese_katakana2_list2[i]));

    const wchar_t japanese_katakana2_list3[] = {0x30d1, 0x30d2, 0x30d3, 0x30d4, 0x30d5, 0x30d6, 0x30d7, 0x30d8};
    const int japanese_katakana2_list3_size = sizeof(japanese_katakana2_list3) / sizeof(japanese_katakana2_list3[0]);
    for (int i = 0; i < japanese_katakana2_list3_size; i++)
        japanese_katakana2_table[2].push_back(CharToAnsiCell(japanese_katakana2_list3[i]));

    const wchar_t japanese_katakana2_list4[] = {0x30d9, 0x30da, 0x30db, 0x30dc, 0x30dd, 0x30de, 0x30df, 0x30e0};
    const int japanese_katakana2_list4_size = sizeof(japanese_katakana2_list4) / sizeof(japanese_katakana2_list4[0]);
    for (int i = 0; i < japanese_katakana2_list4_size; i++)
        japanese_katakana2_table[3].push_back(CharToAnsiCell(japanese_katakana2_list4[i]));

    // japanese katakana (3)
    AnsiBlockTable &japanese_katakana3_table = m_select_block_tables[IC_SELECT_BLOCK_JAPANESE_KATAKANA3];
    japanese_katakana3_table.resize(4);

    const wchar_t japanese_katakana3_list1[] = {0x30e1, 0x30e2, 0x30e3, 0x30e4, 0x30e5, 0x30e6, 0x30e7, 0x30e8};
    const int japanese_katakana3_list1_size = sizeof(japanese_katakana3_list1) / sizeof(japanese_katakana3_list1[0]);
    for (int i = 0; i < japanese_katakana3_list1_size; i++)
        japanese_katakana3_table[0].push_back(CharToAnsiCell(japanese_katakana3_list1[i]));

    const wchar_t japanese_katakana3_list2[] = {0x30e9, 0x30ea, 0x30eb, 0x30ec, 0x30ed, 0x30ee, 0x30ef, 0x30f0};
    const int japanese_katakana3_list2_size = sizeof(japanese_katakana3_list2) / sizeof(japanese_katakana3_list2[0]);
    for (int i = 0; i < japanese_katakana3_list2_size; i++)
        japanese_katakana3_table[1].push_back(CharToAnsiCell(japanese_katakana3_list2[i]));

    const wchar_t japanese_katakana3_list3[] = {0x30f1, 0x30f2, 0x30f3, 0x30f4, 0x30f5, 0x30f6, 0x30f7, 0x30f8};
    const int japanese_katakana3_list3_size = sizeof(japanese_katakana3_list3) / sizeof(japanese_katakana3_list3[0]);
    for (int i = 0; i < japanese_katakana3_list3_size; i++)
        japanese_katakana3_table[2].push_back(CharToAnsiCell(japanese_katakana3_list3[i]));

    const wchar_t japanese_katakana3_list4[] = {0x30f9, 0x30fa, 0x30fc, 0x30fb, 0x30fd, 0x30fe, 0x309b, 0x309c};
    const int japanese_katakana3_list4_size = sizeof(japanese_katakana3_list4) / sizeof(japanese_katakana3_list4[0]);
    for (int i = 0; i < japanese_katakana3_list4_size; i++)
        japanese_katakana3_table[3].push_back(CharToAnsiCell(japanese_katakana3_list4[i]));

    // simple texts for ANSI drawing
    AnsiBlockTable &drawing_text_table = m_select_block_tables[IC_SELECT_BLOCK_DRAWING_TEXT];
    drawing_text_table.resize(4);

    const wchar_t drawing_text_list1[] = {L'一', L'ㄧ', 0x4e5b, 0x4ea0, 0x5196, 0x4e28, 0x4e85, 0x4e3f};
    const int drawing_text_list1_size = sizeof(drawing_text_list1) / sizeof(drawing_text_list1[0]);
    for (int i = 0; i < drawing_text_list1_size; i++)
        drawing_text_table[0].push_back(CharToAnsiCell(drawing_text_list1[i]));

    const wchar_t drawing_text_list2[] = {0x31cf, 0x4e5a, 0x4e36, L'二', L'丁', 0x4e06, 0x4e04, L'乂'};
    const int drawing_text_list2_size = sizeof(drawing_text_list2) / sizeof(drawing_text_list2[0]);
    for (int i = 0; i < drawing_text_list2_size; i++)
        drawing_text_table[1].push_back(CharToAnsiCell(drawing_text_list2[i]));

    const wchar_t drawing_text_list3[] = {0x4e44, 0x4ebb, 0x5182, L'几', L'凵', L'刁', L'刀', 0x5202};
    const int drawing_text_list3_size = sizeof(drawing_text_list3) / sizeof(drawing_text_list3[0]);
    for (int i = 0; i < drawing_text_list3_size; i++)
        drawing_text_table[2].push_back(CharToAnsiCell(drawing_text_list3[i]));

    const wchar_t drawing_text_list4[] = {0x52f9, L'匚', 0x5338, 0x5369, L'厂', 0x53b6, 0x5ef4, 0x4e37};
    const int drawing_text_list4_size = sizeof(drawing_text_list4) / sizeof(drawing_text_list4[0]);
    for (int i = 0; i < drawing_text_list4_size; i++)
        drawing_text_table[3].push_back(CharToAnsiCell(drawing_text_list4[i]));

    return 1;
}

ImageCraftDialog::ImageCraftDialog()
: CFormView(ImageCraftDialog::IDD)
, m_initialized(false)
, mp_main_view(NULL)
, mp_tooltip_control(NULL)
, m_draw_mode(IC_VIEW_ANSI)
, m_last_color_draw_mode(IC_DRAW_SPACES)
, m_current_action_index(0)
{

}

ImageCraftDialog::~ImageCraftDialog()
{
    _DELETE_PTR(mp_tooltip_control);
}

void ImageCraftDialog::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_SLIDER_IMAGE_ALPHA_RATIO, m_image_alpha_ratio_slider);
    DDX_Control(pDX, IDC_SLIDER_IMAGE_RESIZE_RATIO, m_image_resize_ratio_slider);
    DDX_Control(pDX, IDC_SLIDER_IMAGE_ROTATE_DEGREE, m_image_rotate_degree_slider);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_BLACK, m_color_selectors[0]);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_RED, m_color_selectors[1]);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_GREEN, m_color_selectors[2]);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_YELLOW, m_color_selectors[3]);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_BLUE, m_color_selectors[4]);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_PURPLE, m_color_selectors[5]);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_CYAN, m_color_selectors[6]);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_WHITE, m_color_selectors[7]);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_BLACK_BRIGHT, m_color_selectors[8]);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_RED_BRIGHT, m_color_selectors[9]);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_GREEN_BRIGHT, m_color_selectors[10]);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_YELLOW_BRIGHT, m_color_selectors[11]);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_BLUE_BRIGHT, m_color_selectors[12]);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_PURPLE_BRIGHT, m_color_selectors[13]);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_CYAN_BRIGHT, m_color_selectors[14]);
    DDX_Control(pDX, IDC_STATIC_COLOR_SELECTOR_WHITE_BRIGHT, m_color_selectors[15]);
    DDX_Control(pDX, IDC_STATIC_COLOR_CURRENT_FOREGROUND, m_foreground_color);
    DDX_Control(pDX, IDC_STATIC_COLOR_CURRENT_BACKGROUND, m_background_color);
    DDX_Radio(pDX, IDC_RADIO_VIEW_ANSI, m_draw_mode);
    DDX_Control(pDX, IDC_STATIC_BLOCK_SELECTOR, m_static_block_selector);
    DDX_Control(pDX, IDC_COMBO_SELECT_BLOCK_MODE, m_combo_select_block_mode);
    DDX_Control(pDX, IDC_COMBO_LARGE_BRUSH_SHAPE, m_combo_large_brush_shape);
    DDX_Control(pDX, IDC_COMBO_ACTION_LIST, m_combo_action_list);
}

BEGIN_MESSAGE_MAP(ImageCraftDialog, CFormView)
    ON_WM_DESTROY()
    ON_WM_HSCROLL()
    ON_WM_VSCROLL()
    ON_WM_LBUTTONDOWN()
    ON_NOTIFY_RANGE(NM_CLICK, IDC_STATIC_COLOR_SELECTOR_BLACK, IDC_STATIC_COLOR_SELECTOR_WHITE_BRIGHT, OnClickedSelectColor)
    ON_NOTIFY_RANGE(NM_RCLICK, IDC_STATIC_COLOR_SELECTOR_BLACK, IDC_STATIC_COLOR_SELECTOR_WHITE_BRIGHT, OnClickedSelectColor)
    ON_BN_CLICKED(IDC_BUTTON_TEST_CONTENTS, &ImageCraftDialog::OnBnClickedButtonTestContents)
    ON_BN_CLICKED(IDC_RADIO_VIEW_ANSI, OnBnClickedRadioDrawMode)
    ON_BN_CLICKED(IDC_RADIO_DRAW_SPACES, OnBnClickedRadioDrawMode)
    ON_BN_CLICKED(IDC_RADIO_CHANGE_COLOR, OnBnClickedRadioDrawMode)
    ON_BN_CLICKED(IDC_RADIO_REFINE_BOUNDARY, OnBnClickedRadioDrawMode)
    ON_BN_CLICKED(IDC_RADIO_DRAW_BLOCK, OnBnClickedRadioDrawMode)
    ON_BN_CLICKED(IDC_RADIO_DRAW_SMALL_SQUARE, OnBnClickedRadioDrawMode)
    ON_BN_CLICKED(IDC_RADIO_MERGE_BLOCK, OnBnClickedRadioDrawMode)
    ON_BN_CLICKED(IDC_RADIO_INSERT_DELETE_SPACE, OnBnClickedRadioDrawMode)
    ON_BN_CLICKED(IDC_RADIO_DRAW_LARGE_BRUSH, OnBnClickedRadioDrawMode)
    ON_BN_CLICKED(IDC_RADIO_ADD_TEXT, OnBnClickedRadioDrawMode)
    ON_BN_CLICKED(IDC_CHECK_MOVE_IMAGE, &ImageCraftDialog::OnBnClickedCheckMoveImage)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_CANVAS, &ImageCraftDialog::OnBnClickedButtonClearCanvas)
    ON_BN_CLICKED(IDC_CHECK_HIDE_IMAGE, &ImageCraftDialog::OnBnClickedCheckHideImage)
    ON_STN_CLICKED(IDC_STATIC_BLOCK_SELECTOR, &ImageCraftDialog::OnStnClickedStaticBlockSelector)
    ON_BN_CLICKED(IDC_BUTTON_DISPLAY_ROW_UP, &ImageCraftDialog::OnBnClickedButtonDisplayRowUp)
    ON_BN_CLICKED(IDC_BUTTON_DISPLAY_ROW_DOWN, &ImageCraftDialog::OnBnClickedButtonDisplayRowDown)
    ON_BN_CLICKED(IDC_BUTTON_DISPLAY_PAGE_UP, &ImageCraftDialog::OnBnClickedButtonDisplayPageUp)
    ON_BN_CLICKED(IDC_BUTTON_DISPLAY_PAGE_DOWN, &ImageCraftDialog::OnBnClickedButtonDisplayPageDown)
    ON_BN_CLICKED(IDC_BUTTON_TEST_DEBUG_DUMP, &ImageCraftDialog::OnBnClickedButtonTestDebugDump)
    ON_CBN_SELCHANGE(IDC_COMBO_SELECT_BLOCK_MODE, &ImageCraftDialog::OnCbnSelchangeComboSelectBlockMode)
    ON_CBN_SELCHANGE(IDC_COMBO_LARGE_BRUSH_SHAPE, &ImageCraftDialog::OnCbnSelchangeComboLargeBrushShape)
    ON_BN_CLICKED(IDC_BUTTON_BRUSH_INCREASE_SIZE, &ImageCraftDialog::OnBnClickedButtonBrushIncreaseSize)
    ON_BN_CLICKED(IDC_BUTTON_BRUSH_DECREASE_SIZE, &ImageCraftDialog::OnBnClickedButtonBrushDecreaseSize)
    ON_BN_CLICKED(IDC_CHECK_CHANGE_COLOR_AREA_MODE, &ImageCraftDialog::OnBnClickedCheckChangeColorAreaMode)
    ON_BN_CLICKED(IDC_CHECK_REFINE_TRIANGLE_MODE, &ImageCraftDialog::OnBnClickedCheckRefineTriangleMode)
    ON_BN_CLICKED(IDC_BUTTON_SELECT_BLOCK_PREV_TABLE, &ImageCraftDialog::OnBnClickedButtonSelectBlockPrevTable)
    ON_BN_CLICKED(IDC_BUTTON_SELECT_BLOCK_NEXT_TABLE, &ImageCraftDialog::OnBnClickedButtonSelectBlockNextTable)
    ON_CBN_SELCHANGE(IDC_COMBO_ACTION_LIST, &ImageCraftDialog::OnCbnSelchangeComboActionList)
    ON_BN_CLICKED(IDC_BUTTON_UNDO_ONE_ACTION, &ImageCraftDialog::OnBnClickedButtonUndoOneAction)
    ON_BN_CLICKED(IDC_BUTTON_REDO_ONE_ACTION, &ImageCraftDialog::OnBnClickedButtonRedoOneAction)
    ON_BN_CLICKED(IDC_BUTTON_CANCEL_ROTATE, &ImageCraftDialog::OnBnClickedButtonCancelRotate)
END_MESSAGE_MAP()

void ImageCraftDialog::OnInitialUpdate()
{
    if (m_initialized)
        return;

    CFormView::OnInitialUpdate();

    mp_main_view = theApp.GetMainFrame()->GetMainView();

    m_image_alpha_ratio_slider.SetRange(0, 100);
    m_image_alpha_ratio_slider.SetPos(theApp.m_image_alpha_ratio);
    SetWindowTextToValue(IDC_TEXT_IMAGE_ALPHA_RATIO, theApp.m_image_alpha_ratio);

    m_image_resize_ratio_slider.SetRange(ch_Round(MIN_IMAGE_SCALE_RATIO * 100.0f),
                                         ch_Round(MAX_IMAGE_SCALE_RATIO * 100.0f));  
    m_image_resize_ratio_slider.SetPos(theApp.m_image_resize_ratio);
    SetWindowTextToFormat(IDC_TEXT_IMAGE_RESIZE_RATIO, _T("%d%%"), theApp.m_image_resize_ratio);

    m_image_rotate_degree_slider.SetRange(0, 360);

    _SET_CHECK(IDC_CHECK_MOVE_IMAGE, theApp.m_is_move_image);
    SetEnableImageUI();

    for (int i = 0; i < ANSI_ALL_COLOR_COUNT; i++)
        m_color_selectors[i].SetColor(g_all_color_list[i]);

    m_foreground_color.SetColor(ANSI_DEFAULT_FOREGROUND);
    m_background_color.SetColor(ANSI_DEFAULT_BACKGROUND);

    CRect block_selector_rect;
    GetDlgItem(IDC_STATIC_BLOCK_SELECTOR)->GetWindowRect(&block_selector_rect);
    m_block_selector_size.width = block_selector_rect.Width();
    m_block_selector_size.height = block_selector_rect.Height();

    m_combo_select_block_mode.ResetContent();
    m_combo_select_block_mode.AddString(_T("幾何形狀"));
    m_combo_select_block_mode.AddString(_T("標記／標線"));
    m_combo_select_block_mode.AddString(_T("數學符號"));
    m_combo_select_block_mode.AddString(_T("括弧號"));
    m_combo_select_block_mode.AddString(_T("箭頭／標點／其他"));
    m_combo_select_block_mode.AddString(_T("表格線條(一)"));
    m_combo_select_block_mode.AddString(_T("表格線條(二)"));
    m_combo_select_block_mode.AddString(_T("希臘文 - 大寫"));
    m_combo_select_block_mode.AddString(_T("希臘文 - 小寫"));
    m_combo_select_block_mode.AddString(_T("日文 - 平假名(一)"));
    m_combo_select_block_mode.AddString(_T("日文 - 平假名(二)"));
    m_combo_select_block_mode.AddString(_T("日文 - 平假名(三)"));
    m_combo_select_block_mode.AddString(_T("日文 - 片假名(一)"));
    m_combo_select_block_mode.AddString(_T("日文 - 片假名(二)"));
    m_combo_select_block_mode.AddString(_T("日文 - 片假名(三)"));
    m_combo_select_block_mode.AddString(_T("繪圖用簡單文字"));

    m_combo_large_brush_shape.ResetContent();
    m_combo_large_brush_shape.AddString(_T("正方形"));
    m_combo_large_brush_shape.AddString(_T("圓形"));
    m_combo_large_brush_shape.AddString(_T("菱形"));
    m_combo_large_brush_shape.AddString(_T("水平線"));
    m_combo_large_brush_shape.AddString(_T("垂直線"));

    UpdateRotateDegreeUI();
    UpdateBrushSizeUI();
    UpdateBrushShapeUI();
    UpdateSelectBlockModeUI();
    UpdateChangeColorUI();
    UpdateRefineBoundaryUI();
    UpdateActionHistoryUI(0, std::vector<std_tstring>());

#if HIDE_DEBUG_ONLY_UI_ITEMS
    GetDlgItem(IDC_GROUP_DEBUG_ONLY_ITEMS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_BUTTON_TEST_CONTENTS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_BUTTON_TEST_DEBUG_DUMP)->ShowWindow(SW_HIDE);
#endif

    mp_tooltip_control = new CToolTipCtrl();
    mp_tooltip_control->Create(this);

    mp_tooltip_control->SetMaxTipWidth(400);

    std::vector<int> image_alpha_item_ids
    {
        IDC_STATIC_IMAGE_ALPHA_RATIO,
        IDC_TEXT_IMAGE_ALPHA_RATIO,
        IDC_SLIDER_IMAGE_ALPHA_RATIO,
    };
    AddToolTip(image_alpha_item_ids, _T("參考圖片在繪圖區域中的顯示濃度"));

    std::vector<int> image_resize_item_ids
    {
        IDC_STATIC_IMAGE_RESIZE_RATIO,
        IDC_TEXT_IMAGE_RESIZE_RATIO,
        IDC_SLIDER_IMAGE_RESIZE_RATIO,
    };
    AddToolTip(image_resize_item_ids, _T("參考圖片與其原始大小的比例"));

    std::vector<int> image_rotate_item_ids
    {
        IDC_STATIC_IMAGE_ROTATE_DEGREE,
        IDC_TEXT_IMAGE_ROTATE_DEGREE,
        IDC_SLIDER_IMAGE_ROTATE_DEGREE,
    };
    AddToolTip(image_rotate_item_ids, _T("參考圖片的旋轉角度，正值為順時針方向"));

    AddToolTip(IDC_BUTTON_CANCEL_ROTATE, _T("將旋轉角度歸零，回復圖片原本的方向"));

    AddToolTip(IDC_CHECK_HIDE_IMAGE, _T("暫時隱藏圖片，不改變濃度設定"));

    std::vector<int> image_size_item_ids
    {
        IDC_STATIC_IMAGE_FULL_SIZE,
        IDC_TEXT_IMAGE_FULL_SIZE,
    };
    AddToolTip(image_size_item_ids, _T("圖片的實際尺寸"));

    std::vector<int> image_roi_item_ids
    {
        IDC_STATIC_IMAGE_ROI_RANGE,
        IDC_TEXT_IMAGE_ROI_RANGE,
    };
    AddToolTip(image_roi_item_ids, _T("圖片在繪圖區域內的像素範圍"));
    AddToolTip(IDC_CHECK_MOVE_IMAGE, _T("勾選後才可移動圖片，避免在繪圖中誤觸"));

    AddToolTip(IDC_BUTTON_DISPLAY_ROW_UP, _T("往上捲動一列"));
    AddToolTip(IDC_BUTTON_DISPLAY_ROW_DOWN, _T("往下捲動一列"));
    AddToolTip(IDC_BUTTON_DISPLAY_PAGE_UP, _T("往上捲動一整頁(23列)"));
    AddToolTip(IDC_BUTTON_DISPLAY_PAGE_DOWN, _T("往下捲動一整頁(23列)"));
    AddToolTip(IDC_TEXT_DISPLAY_RANGE, _T("繪圖區域中ANSI字元的範圍"));

    std::vector<int> select_color_ids
    {
        IDC_STATIC_COLOR_SELECTOR_BLACK,
        IDC_STATIC_COLOR_SELECTOR_RED,
        IDC_STATIC_COLOR_SELECTOR_GREEN,
        IDC_STATIC_COLOR_SELECTOR_YELLOW,
        IDC_STATIC_COLOR_SELECTOR_BLUE,
        IDC_STATIC_COLOR_SELECTOR_PURPLE,
        IDC_STATIC_COLOR_SELECTOR_CYAN,
        IDC_STATIC_COLOR_SELECTOR_WHITE,
        IDC_STATIC_COLOR_SELECTOR_BLACK_BRIGHT,
        IDC_STATIC_COLOR_SELECTOR_RED_BRIGHT,
        IDC_STATIC_COLOR_SELECTOR_GREEN_BRIGHT,
        IDC_STATIC_COLOR_SELECTOR_YELLOW_BRIGHT,
        IDC_STATIC_COLOR_SELECTOR_BLUE_BRIGHT,
        IDC_STATIC_COLOR_SELECTOR_PURPLE_BRIGHT,
        IDC_STATIC_COLOR_SELECTOR_CYAN_BRIGHT,
        IDC_STATIC_COLOR_SELECTOR_WHITE_BRIGHT,
    };
    AddToolTip(select_color_ids, _T("左鍵選擇為前景色，右鍵選擇為背景色。\n在僅使用單色的模式中會同時設定兩者。"));

    AddToolTip(IDC_COMBO_SELECT_BLOCK_MODE, _T("選擇特定類別的字元列表"));
    AddToolTip(IDC_BUTTON_SELECT_BLOCK_PREV_TABLE, _T("切換至前一個字元列表"));
    AddToolTip(IDC_BUTTON_SELECT_BLOCK_NEXT_TABLE, _T("切換至後一個字元列表"));

    AddToolTip(IDC_RADIO_VIEW_ANSI, _T("檢視ANSI字元與顏色設定"));
    AddToolTip(IDC_RADIO_DRAW_SPACES, _T("以純色半形空白為單位著色"));
    AddToolTip(IDC_RADIO_DRAW_SMALL_SQUARE, _T("以長寬皆為半形的小正方格著色"));
    AddToolTip(IDC_RADIO_CHANGE_COLOR, _T("改變ANSI字元中部分區塊的顏色"));
    AddToolTip(IDC_RADIO_REFINE_BOUNDARY, _T("調整不同顏色間的邊緣"));
    AddToolTip(IDC_RADIO_DRAW_BLOCK, _T("填入指定類型與顏色的全形字元"));
    AddToolTip(IDC_RADIO_MERGE_BLOCK, _T("將特定位置的內容合併為單一全形方格"));
    AddToolTip(IDC_RADIO_INSERT_DELETE_SPACE, _T("在特定位置插入或刪除空白，並平移右方內容"));
    AddToolTip(IDC_RADIO_DRAW_LARGE_BRUSH, _T("用較大的筆刷進行大範圍著色"));

    AddToolTip(IDC_BUTTON_CLEAR_CANVAS, _T("清除所有ANSI內容，不影響圖片設定"));

    mp_tooltip_control->Activate(TRUE);

    UpdateData(FALSE);

    m_initialized = true;
}

void ImageCraftDialog::AddToolTip(int item_id, LPCTSTR tip)
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

void ImageCraftDialog::AddToolTip(const std::vector<int> &item_ids, LPCTSTR tip)
{
    for (int i = 0; i < (int)item_ids.size(); i++)
        AddToolTip(item_ids[i], tip);
}

BOOL ImageCraftDialog::PreTranslateMessage(MSG *p_message) 
{
    if(mp_tooltip_control != NULL)
       mp_tooltip_control->RelayEvent(p_message);

    return CFormView::PreTranslateMessage(p_message);
}

void ImageCraftDialog::SetEnableImageUI()
{
    if (mp_main_view->IsImageLoaded())
    {
        GetDlgItem(IDC_CHECK_MOVE_IMAGE)->EnableWindow(TRUE);
        GetDlgItem(IDC_CHECK_HIDE_IMAGE)->EnableWindow(TRUE);
        GetDlgItem(IDC_SLIDER_IMAGE_ALPHA_RATIO)->EnableWindow(TRUE);
        EnableMoveImageItems(theApp.m_is_move_image);
    }
    else
    {
        GetDlgItem(IDC_CHECK_MOVE_IMAGE)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK_HIDE_IMAGE)->EnableWindow(FALSE);
        GetDlgItem(IDC_SLIDER_IMAGE_ALPHA_RATIO)->EnableWindow(FALSE);
        EnableMoveImageItems(false);
    }
}

__forceinline bool ImageCraftDialog::IsScrollEnd(UINT sb_code)
{
    // Return true when we want to update result by the scroll bar.
    // (for the actions that are too slow for real-time update)
    return (sb_code == SB_THUMBPOSITION || sb_code == SB_ENDSCROLL);
}

__forceinline void ImageCraftDialog::SetWindowTextToValue(int nID, int value)
{
    CString str;
    str.Format(_T("%d"), value);
    GetDlgItem(nID)->SetWindowText(str);
}

__forceinline void ImageCraftDialog::SetWindowTextToFormat(int nID, LPCTSTR format, ...)
{
    _TCHAR buffer[256];
    va_list marker;
    va_start(marker, format);
    _vstprintf(buffer, format, marker);
    va_end(marker);

    GetDlgItem(nID)->SetWindowText(buffer);
}

bool ImageCraftDialog::IsUserConfirm(LPCTSTR title, LPCTSTR message)
{
    bool is_confirm = (MessageBox(message, title, MB_YESNO) == IDYES);

    return is_confirm;
}

void ImageCraftDialog::OnDestroy()
{
    CPoint scroll_position = GetScrollPosition();
    theApp.m_control_dialog_y_position = scroll_position.y;

    CFormView::OnDestroy();
}

void ImageCraftDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    if (pScrollBar == GetDlgItem(IDC_SLIDER_IMAGE_ALPHA_RATIO))
    {
        int old_ratio = (int)theApp.m_image_alpha_ratio;
        int new_ratio = m_image_alpha_ratio_slider.GetPos();

        if (new_ratio != old_ratio)
        {
            SetWindowTextToValue(IDC_TEXT_IMAGE_ALPHA_RATIO, new_ratio);

            theApp.m_image_alpha_ratio = new_ratio;
            mp_main_view->UpdateViewImage();
        }
    }
    else if (pScrollBar == GetDlgItem(IDC_SLIDER_IMAGE_RESIZE_RATIO))
    {
        int old_ratio = (int)theApp.m_image_resize_ratio;
        int new_ratio = m_image_resize_ratio_slider.GetPos();

        if (new_ratio != old_ratio)
        {
            float resize_ratio = 0.01f * new_ratio;
            mp_main_view->ChangeResizeRatio(resize_ratio);
        }
    }
    else if (pScrollBar == GetDlgItem(IDC_SLIDER_IMAGE_ROTATE_DEGREE))
    {
        int old_value = ch_Round(theApp.m_image_rotate_degree);
        int new_value = m_image_rotate_degree_slider.GetPos() - 180;

        if (new_value != old_value)
        {
            float rotate_degree = (float)new_value;
            mp_main_view->ChangeRotateDegree(rotate_degree);
        }
    }

    CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void ImageCraftDialog::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    if (pScrollBar == NULL)
    {
        // The view's scroll bar is scrolled.

        if (IsScrollEnd(nSBCode))
        {
            if (mp_main_view)
                mp_main_view->SetFocus();
        }
    }
   

    CFormView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void ImageCraftDialog::OnClickedSelectColor(UINT id, NMHDR *pNotifyStruct, LRESULT *result)
{
    SetLastColorDrawMode();

    bool is_single_color_mode = (m_draw_mode == IC_DRAW_SPACES || m_draw_mode == IC_DRAW_SMALL_SQUARE ||
                                 m_draw_mode == IC_INSERT_DELETE_SPACE || m_draw_mode == IC_DRAW_LARGE_BRUSH ||
                                 m_draw_mode == IC_ADD_TEXT);

    int color = ((ColorSelector *)GetDlgItem(id))->GetColor();

    if (is_single_color_mode)
    {
        SetForegroundColor(color);
        SetBackgroundColor(ToNormalColor(color));
    }
    else
    {
        bool is_set_foreground = (pNotifyStruct->code == NM_CLICK);
        if (is_set_foreground)
            SetForegroundColor(color);
        else
            SetBackgroundColor(ToNormalColor(color));
    }
}

void ImageCraftDialog::OnBnClickedButtonTestContents()
{
    mp_main_view->SetTestContents();
}

void ImageCraftDialog::OnBnClickedRadioDrawMode()
{
    UpdateData(TRUE);

    // Call SetDrawMode() to arrange other UI as well.
    SetDrawMode(m_draw_mode);
}

void ImageCraftDialog::OnBnClickedCheckMoveImage()
{
    theApp.m_is_move_image = _IS_CHECKED(IDC_CHECK_MOVE_IMAGE);
    EnableMoveImageItems(theApp.m_is_move_image);
}

void ImageCraftDialog::EnableMoveImageItems(bool is_enable)
{
    BOOL enable = (BOOL)is_enable;
    GetDlgItem(IDC_SLIDER_IMAGE_RESIZE_RATIO)->EnableWindow(enable);
    GetDlgItem(IDC_SLIDER_IMAGE_ROTATE_DEGREE)->EnableWindow(enable);
    GetDlgItem(IDC_BUTTON_CANCEL_ROTATE)->EnableWindow(enable);
}

void ImageCraftDialog::OnBnClickedButtonClearCanvas()
{
    if (mp_main_view == NULL)
        return;

    if (IsUserConfirm(_T("清除畫板"), _T("確定清除？")))
        mp_main_view->ClearCanvas();

    mp_main_view->SetFocus();
}

void ImageCraftDialog::OnBnClickedCheckHideImage()
{
    bool is_hide_image = _IS_CHECKED(IDC_CHECK_HIDE_IMAGE);
    SetHideImage(is_hide_image);

    mp_main_view->SetFocus();
}

void ImageCraftDialog::SetHideImage(bool is_hide_image)
{
    theApp.m_is_hide_image = is_hide_image;
    _SET_CHECK(IDC_CHECK_HIDE_IMAGE, is_hide_image);
    mp_main_view->UpdateViewImage();
}

void ImageCraftDialog::UpdateRotateDegreeUI()
{
    int degree = FitInRange(ch_Round(theApp.m_image_rotate_degree), -180, 180);
    m_image_rotate_degree_slider.SetPos(degree + 180);

    CString size_text = _T("0°");
    if (degree != 0)
        size_text.Format(_T("%+d°"), degree);

    SetWindowTextToFormat(IDC_TEXT_IMAGE_ROTATE_DEGREE, (LPCTSTR)size_text);
}

void ImageCraftDialog::UpdateBrushSizeUI()
{
    CString size_text;
    size_text.Format(_T("%d"), theApp.m_large_brush_size);
    SetWindowTextToFormat(IDC_TEXT_LARGE_BRUSH_SIZE, size_text);
}

void ImageCraftDialog::UpdateBrushShapeUI()
{
    m_combo_large_brush_shape.SetCurSel(theApp.m_large_brush_shape);
}

void ImageCraftDialog::UpdateSelectBlockModeUI()
{
    m_combo_select_block_mode.SetCurSel(theApp.m_select_block_mode);

    InitBlockSelector();
}

void ImageCraftDialog::UpdateChangeColorUI()
{
    _SET_CHECK(IDC_CHECK_CHANGE_COLOR_AREA_MODE, theApp.m_is_change_color_area_mode);
}

void ImageCraftDialog::UpdateRefineBoundaryUI()
{
    _SET_CHECK(IDC_CHECK_REFINE_TRIANGLE_MODE, theApp.m_is_refine_triangle_mode);
}

void ImageCraftDialog::UpdateActionHistoryUI(int action_index, const std::vector<std_tstring> &action_list)
{
    const int action_count = (int)action_list.size();

    m_combo_action_list.ResetContent();
    m_combo_action_list.ClearItemMap();

    if (action_count == 0)
    {
        m_combo_action_list.AddString(_T("目前無操作步驟"));
    }
    else
    {
        // Insert actions in reverse order, putting the newest action at the top.
        // Then add an "Undo all" item at the bottom of the list.

        for (int i = 0; i < action_count; i++)
            m_combo_action_list.AddString(action_list[action_count - 1 - i].c_str());

        m_combo_action_list.AddString(_T("復原所有步驟"));
    }

    UpdateActionHistoryUI(action_index);
}

void ImageCraftDialog::UpdateActionHistoryUI(int action_index)
{
    const int action_count = m_combo_action_list.GetCount() - 1;
    int item_index = action_count - action_index;

    if (action_count > 0)
    {
        m_combo_action_list.SetAllItemsUnmarked();
        m_combo_action_list.SetItemMarked(item_index, true);
    }

    if (m_combo_action_list.GetDroppedState() != 0)
    {
        // Workaround to "Redraw" the old marked item to unmarked.
        int old_item_index = action_count - m_current_action_index;
        if (old_item_index >= 0 && old_item_index < m_combo_action_list.GetCount())
            m_combo_action_list.SetCurSel(old_item_index);
    }

    m_combo_action_list.SetCurSel(item_index);

    m_combo_action_list.EnableWindow((BOOL)(action_count > 0));
    GetDlgItem(IDC_BUTTON_UNDO_ONE_ACTION)->EnableWindow((BOOL)(action_index > 0));
    GetDlgItem(IDC_BUTTON_REDO_ONE_ACTION)->EnableWindow((BOOL)(action_index < action_count));

    CString action_status;
    action_status.Format(_T("目前步驟：%s\n步驟總數：%s"),
                         FullSizeValue(action_index).c_str(), FullSizeValue(action_count).c_str());
    GetDlgItem(IDC_TEXT_ACTION_STATUS)->SetWindowText(action_status);

    m_current_action_index = action_index;
}

void ImageCraftDialog::SetForegroundColor(int color)
{
    m_foreground_color.SetColor(color);
    mp_main_view->SetForegroundColor(color);
}

void ImageCraftDialog::SetBackgroundColor(int color)
{
    m_background_color.SetColor(color);
    mp_main_view->SetBackgroundColor(color);
}

void ImageCraftDialog::SetColor(AnsiColor color)
{
    SetForegroundColor(color.fg_color);
    SetBackgroundColor(color.bg_color);
}

AnsiCell ImageCraftDialog::GetSelectedBlock()
{
    return m_selected_block;
}

const AnsiBlockTable &ImageCraftDialog::GetBlockTable()
{
    int mode = FitInRange(theApp.m_select_block_mode, 0, IC_SELECT_BLOCK_MODE_AMOUNT - 1);
    return m_select_block_tables[mode];
}

void ImageCraftDialog::InitBlockSelector()
{
    const AnsiBlockTable &block_table = GetBlockTable();

    int row_count = block_table.size();
    std::vector<int> row_item_counts(row_count);
    for (int i = 0; i < row_count; i++)
        row_item_counts[i] = block_table[i].size();

    m_block_selector.SetStructure(m_block_selector_size.width,
                                  m_block_selector_size.height,
                                  row_count, &row_item_counts[0], 2);

    HySize icon_size = m_block_selector.GetIconSize();
    
    // Required for out ANSI block template.
    _MYASSERT(icon_size.width % 2 == 0);
    _MYASSERT(icon_size.width == icon_size.height);
    
    const AnsiColor icon_color(ANSI_WHITE_BRIGHT, ANSI_BLACK);
    AnsiTemplate block_template;
    block_template.SetSize(icon_size.width);
    HyImage *p_icon_image = hyCreateImage(icon_size, HY_DEPTH_8U, 3);

    for (int i = 0; i < row_count; i++)
    {
        for (int j = 0; j < row_item_counts[i]; j++)
        {
            block_template.DrawBlock(p_icon_image, 0, 0, icon_color, block_table[i][j]);
            m_block_selector.SetIconImage(p_icon_image, i, j);
        }
    }

    hyReleaseImage(&p_icon_image);

    SelectDrawingBlock(0, 0);
}

void ImageCraftDialog::OnStnClickedStaticBlockSelector()
{
    POINT point;
    GetCursorPos(&point);
    GetDlgItem(IDC_STATIC_BLOCK_SELECTOR)->ScreenToClient(&point);

    int row_index = -1, col_index = -1;
    bool is_valid = m_block_selector.GetItemIndex(point.x, point.y, row_index, col_index);
    if (is_valid == false)
        return;

    SelectDrawingBlock(row_index, col_index);

    SetDrawMode(IC_DRAW_BLOCK);
}

void ImageCraftDialog::SelectDrawingBlock(int row_index, int col_index)
{
    bool is_success = m_block_selector.Select(row_index, col_index);
    if (is_success == false)
        return;

    const AnsiBlockTable &block_table = GetBlockTable();
    m_selected_block = block_table[row_index][col_index];
    
    HyImage *p_display_image = m_block_selector.GetDisplayImage();
    m_static_block_selector.SetColorImage(p_display_image->imageData, p_display_image->width, 
                                          p_display_image->height, p_display_image->widthStep, 3);
    m_static_block_selector.Invalidate(FALSE);
}

void ImageCraftDialog::OnBnClickedButtonDisplayRowUp()
{
    mp_main_view->MoveDisplayOffset(0, -1);
    mp_main_view->SetFocus();
}

void ImageCraftDialog::OnBnClickedButtonDisplayRowDown()
{
    mp_main_view->MoveDisplayOffset(0, 1);
    mp_main_view->SetFocus();
}

void ImageCraftDialog::OnBnClickedButtonDisplayPageUp()
{
    mp_main_view->MoveDisplayPageUp();
    mp_main_view->SetFocus();
}

void ImageCraftDialog::OnBnClickedButtonDisplayPageDown()
{
    mp_main_view->MoveDisplayPageDown();
    mp_main_view->SetFocus();
}

void ImageCraftDialog::SetDrawMode(int new_mode)
{
    _SET_CHECK(IDC_CHECK_MOVE_IMAGE, false);
    theApp.m_is_move_image = false;
    SetEnableImageUI();

    m_draw_mode = new_mode;
    if (IsColorDrawMode(m_draw_mode))
        m_last_color_draw_mode = m_draw_mode;

    UpdateData(FALSE);

    mp_main_view->ChangeDrawMode(m_draw_mode);
    mp_main_view->SetFocus();
}

void ImageCraftDialog::SetLastColorDrawMode()
{
    SetDrawMode(m_last_color_draw_mode);
}

void ImageCraftDialog::OnBnClickedButtonTestDebugDump()
{
    mp_main_view->DebugDump();
}

void ImageCraftDialog::UpdateUIByProjectSettings()
{
    // Update only the UIs that may be changed by the settings of ANSI project.

    m_image_alpha_ratio_slider.SetPos(theApp.m_image_alpha_ratio);
    SetWindowTextToValue(IDC_TEXT_IMAGE_ALPHA_RATIO, theApp.m_image_alpha_ratio);
}

void ImageCraftDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (mp_main_view)
        mp_main_view->SetFocus();

    CFormView::OnLButtonDown(nFlags, point);
}

void ImageCraftDialog::OnCbnSelchangeComboSelectBlockMode()
{
    if (mp_main_view == NULL)
        return;

    int new_mode = m_combo_select_block_mode.GetCurSel();
    new_mode = FitInRange(new_mode, 0, IC_SELECT_BLOCK_MODE_AMOUNT - 1);

    mp_main_view->SetSelectBlockMode(new_mode);
    mp_main_view->SetFocus();
}

void ImageCraftDialog::OnCbnSelchangeComboLargeBrushShape()
{
    if (mp_main_view == NULL)
        return;

    int select = m_combo_large_brush_shape.GetCurSel();
    mp_main_view->SetBrushShape(select);
    mp_main_view->SetFocus();
}

void ImageCraftDialog::OnBnClickedButtonBrushIncreaseSize()
{
    if (mp_main_view == NULL)
        return;

    mp_main_view->ChangeBrushSize(1);
    mp_main_view->SetFocus();
}
    
void ImageCraftDialog::OnBnClickedButtonBrushDecreaseSize()
{
    if (mp_main_view == NULL)
        return;

    mp_main_view->ChangeBrushSize(-1);
    mp_main_view->SetFocus();
}

void ImageCraftDialog::OnBnClickedCheckChangeColorAreaMode()
{
    if (mp_main_view == NULL)
        return;

    mp_main_view->SetChangeColorAreaMode(_IS_CHECKED(IDC_CHECK_CHANGE_COLOR_AREA_MODE));
    mp_main_view->SetFocus();
}

void ImageCraftDialog::OnBnClickedCheckRefineTriangleMode()
{
    if (mp_main_view == NULL)
        return;

    mp_main_view->SetRefineTriangleMode(_IS_CHECKED(IDC_CHECK_REFINE_TRIANGLE_MODE));
    mp_main_view->SetFocus();
}

void ImageCraftDialog::OnBnClickedButtonSelectBlockPrevTable()
{
    if (mp_main_view == NULL)
        return;

    mp_main_view->ChangeSelectBlockTable(-1);
    mp_main_view->SetFocus();
}

void ImageCraftDialog::OnBnClickedButtonSelectBlockNextTable()
{
    if (mp_main_view == NULL)
        return;

    mp_main_view->ChangeSelectBlockTable(1);
    mp_main_view->SetFocus();
}

void ImageCraftDialog::OnCbnSelchangeComboActionList()
{
    const int action_count = m_combo_action_list.GetCount() - 1;
    int item_index = m_combo_action_list.GetCurSel();
    if (item_index < 0 || item_index > action_count)
        return;

    int new_action_index = action_count - item_index;
    mp_main_view->SetActionIndex(new_action_index);
    mp_main_view->SetFocus();
}

void ImageCraftDialog::OnBnClickedButtonUndoOneAction()
{
    if (mp_main_view == NULL)
        return;

    mp_main_view->SetActionIndex(m_current_action_index - 1);
    mp_main_view->SetFocus();
}

void ImageCraftDialog::OnBnClickedButtonRedoOneAction()
{
    if (mp_main_view == NULL)
        return;

    mp_main_view->SetActionIndex(m_current_action_index + 1);
    mp_main_view->SetFocus();
}

void ImageCraftDialog::OnBnClickedButtonCancelRotate()
{
    if (IsUserConfirm(_T("取消圖片旋轉"), _T("確定要將圖片轉回原本的方向嗎？\n（將以繪圖區域中心做為旋轉中心）")))
        mp_main_view->ChangeRotateDegree(0.0f);

    mp_main_view->SetFocus();
}
