#include "stdafx.h"
#include "UsageData.h"
#include "resource.h"

std::vector<std::vector<UsagePageData> > g_all_usage_data;

bool InitAllUsageData()
{
    g_all_usage_data.resize(IC_USAGE_MENU_AMOUNT);

    g_all_usage_data[IC_USAGE_BASIC].resize(11);

    g_all_usage_data[IC_USAGE_BASIC][0].description =
        std_tstring(_T("本程式是一個以繪圖功能為主的ANSI編輯軟體，設計靈感來自於小畫家。\r\n")) +
        _T("視窗中左側較大的部分為「繪圖區域」，用於顯示與繪製ANSI內容。\r\n") +
        _T("右側的「功能選單」則放置繪圖過程中會使用到的各種功能。\r\n") +
        _T("在開始使用前，請確認繪圖區域的大小足夠顯示其中橫豎兩條紅線，\r\n") +
        _T("以及紅線右側中從「１」到「２３」的粉紅色數字。\r\n") +
        _T("如果這些內容被遮擋，請調整視窗大小，以放大繪圖區域。\r\n") +
        _T("若您為第一次開啟本程式，則視窗應會具有適當的預設尺寸。");
    g_all_usage_data[IC_USAGE_BASIC][0].image_resource_id = IDB_USAGE_BASIC_INTRO;

    g_all_usage_data[IC_USAGE_BASIC][1].description =
        std_tstring(_T("每個ANSI作品的製作過程，通常會伴隨著一張圖片作為參考。\r\n")) +
        _T("在本程式中，ANSI內容與參考圖片的組合被稱為一「專案」。\r\n") +
        _T("此章節中將介紹專案的基本架構，以及存檔／讀檔等常用功能。\r\n") +
        _T("首先，請從上方選單的「檔案」→「開啟圖片」中，讀入一張圖片。\r\n") +
        _T("您可以直接使用所附帶的xpbliss.jpg，或是選擇任何喜歡的圖片。\r\n") +
        _T("在讀入後，圖片會以半透明狀態出現在繪圖區域中，用於在繪製ANSI內容時作即時比對，此即為俗稱的「墊圖」功能。");
    g_all_usage_data[IC_USAGE_BASIC][1].image_resource_id = IDB_USAGE_BASIC_IMAGE;

    g_all_usage_data[IC_USAGE_BASIC][2].description =
        std_tstring(_T("與圖片相關的選項位於功能選單的最上端，其中的「允許移動圖片」為一安全鎖，只有在勾選時才能改變圖片的位置、尺寸與角度。\r\n")) +
        _T("在開始作畫前，可以先將想要繪製的內容移動到繪圖區域中以橫豎直線圍成的區域，其相當於橫向80半形、縱向23行的ANSI內容。\r\n") +
        _T("作畫中則建議將「允許移動圖片」取消，以避免移動到調整好的圖片。\r\n") +
        _T("「濃度」滑桿可以調整圖片的不透明度，以方便與ANSI內容進行比對。\r\n") +
        _T("也可用「隱藏圖片」（熱鍵Ｃ）做快速切換，而不改變濃度的設定值。");
    g_all_usage_data[IC_USAGE_BASIC][2].image_resource_id = IDB_USAGE_BASIC_IMAGE_CONTROL;

    g_all_usage_data[IC_USAGE_BASIC][3].description =
        std_tstring(_T("圖片的平移、縮放與旋轉可以藉由滑鼠與滾輪操作，或選單中的「縮放」與「旋轉」滑桿來調整。在繪圖區域中拖曳游標，圖片即會隨之平移。\r\n")) +
        _T("使用滑鼠滾輪時，圖片將以游標位置為固定點進行縮放。若調整「縮放」滑桿，縮放的固定點則會改為繪圖區域的中心。\r\n") +
        _T("旋轉圖片時，可以直接調整「旋轉」滑桿來改變角度，也可按住Shift鍵後再拖曳游標，此時將根據繪圖區域中心往游標位置的方向來旋轉圖片。\r\n") +
        _T("「取消旋轉」按鈕則可將旋轉角度歸零，讓圖片回到原本的方向。");
    g_all_usage_data[IC_USAGE_BASIC][3].image_resource_id = IDB_USAGE_BASIC_IMAGE_MOVE;

    g_all_usage_data[IC_USAGE_BASIC][4].description =
        std_tstring(_T("在繪圖區域中，ANSI字元的行高與全形字元的寬度預設為24像素。\r\n")) +
        _T("上方選單的「設定」→「進階設定」中可以更改此數值。理想數值為8或4的倍數，") +
        _T("可減少階梯類型方格顯示時的誤差。但建議先維持基本設定。\r\n") +
        _T("24像素能夠提供最好的顯示品質，也較接近常見的BBS視窗顯示尺寸。\r\n") +
        _T("若您的螢幕解析度較小，則可嘗試20或16像素的設定值。\r\n") +
        _T("請注意，當改變全形方格的尺寸時，參考圖片不會隨之縮放。\r\n") +
        _T("最好於繪圖前決定適合的尺寸，盡量不要於繪圖過程中更改設定。");
    g_all_usage_data[IC_USAGE_BASIC][4].image_resource_id = IDB_USAGE_BASIC_BLOCK_SIZE;

    g_all_usage_data[IC_USAGE_BASIC][5].description =
        std_tstring(_T("最基本的繪圖功能為「填入半形空白」，請從右側「繪圖功能」中選擇。\r\n")) +
        _T("此功能以及部分的其他繪圖功能，具有類似的上色方法：\r\n") +
        _T("以滑鼠左鍵填入目前選擇的顏色，滑鼠右鍵則可從游標位置挑選顏色。\r\n") +
        _T("您也可用滑鼠左鍵從右側的調色盤中挑選顏色，但在此功能下，由於半形空白必須以背景色表現，繪圖時使用的顏色將自動轉換成暗色。\r\n") +
        _T("若需要填入大片區域的空白，可用點擊左鍵並拖曳的方式進行。\r\n") +
        _T("在這個簡單的範例中，僅以藍色和綠色的半形空格填滿畫面。");
    g_all_usage_data[IC_USAGE_BASIC][5].image_resource_id = IDB_USAGE_BASIC_DRAW_SPACES;

    g_all_usage_data[IC_USAGE_BASIC][6].description =
        std_tstring(_T("若想要繪製的目標縱向尺寸較大，可以擴大繪圖區域以涵蓋多頁。\r\n")) +
        _T("右側的三角箭頭與「上頁」「下頁」可以改變繪圖區域所對應的橫列。\r\n") +
        _T("此時參考圖片的內容會自動平移至對應的區域，並維持原本的縮放比例。\r\n") +
        _T("鍵盤中的上下方向鍵、PgDn及PgUp也可用於進行同樣的操作。\r\n") +
        _T("本範例中暫時不需要作此操作，但其可適用於較大型的ANSI作品。\r\n") +
        _T("建議在一開始讀入圖片時，將想要繪製的區域之最上端對應至1-23列，\r\n") +
        _T("在調整好適合的位置和尺寸後，再將行數增加至足夠的數量。");
    g_all_usage_data[IC_USAGE_BASIC][6].image_resource_id = IDB_USAGE_BASIC_SCROLL;

    g_all_usage_data[IC_USAGE_BASIC][7].description =
        std_tstring(_T("ANSI作品常常需要多次修改，儲存／讀取工作狀態為一常用的重要功能。\r\n")) +
        _T("上方選單的「檔案」→「儲存專案」可以將目前狀態儲存為單一檔案：\r\n") +
        _T("包含ANSI內容、參考圖片內容、以及圖片在畫面中的位置與縮放比例。\r\n") +
        _T("ANSI內容最下端的空白（可能因為調整行數時而產生）會被自動省略。\r\n") +
        _T("「檔案」→「讀取專案」則可從所儲存的專案檔將此狀態完整還原。\r\n") +
        _T("意即，參考圖片的內容會被自動讀入，並放置到已調整好的理想位置。\r\n") +
        _T("只要保存專案檔，即使沒有原本的圖片也可以在同樣的配置下進行繪圖。\r\n");
    g_all_usage_data[IC_USAGE_BASIC][7].image_resource_id = IDB_USAGE_BASIC_PROJECT;

    g_all_usage_data[IC_USAGE_BASIC][8].description =
        std_tstring(_T("除了主動儲存／讀取專案外，本程式也有自動儲存的功能。\r\n")) +
        _T("每次開啟本程式時，會自動讀入上一次關閉前的使用狀態。\r\n") +
        _T("此時視窗標題欄會出現「(自動儲存)」，以表示目前狀態可能包含一些未主動儲存的改動，並非與之前儲存的專案檔內容相同。\r\n") +
        _T("若在此狀態下進行「儲存專案」，則內容會被更新至該專案檔內。\r\n") +
        _T("您可以從現存狀態繼續編輯，或讀入之前儲存的專案檔以放棄最新改動。");
    g_all_usage_data[IC_USAGE_BASIC][8].image_resource_id = IDB_USAGE_BASIC_AUTO_SAVE;

    g_all_usage_data[IC_USAGE_BASIC][9].description =
        std_tstring(_T("在「檔案」選單中，也可單獨處理ANSI內容，而不影響圖片設置。\r\n")) +
        _T("本程式繪製的圖形存為ANSI檔(.ans)後，可在其他BBS程式中開啟。\r\n") +
        _T("若將其他來源的ANSI檔讀入本程式後，則其橫向寬度會受到限制。\r\n") +
        _T("每一橫列中，本程式最多只能處理80個半形字元，超過的部分將被忽略。\r\n") +
        _T("但縱向高度則沒有限制，可用前述的上下翻頁功能來移動顯示內容。\r\n") +
        _T("當準備要把繪製的作品發布為文章時，由於BBS顯示上的限制，\r\n") +
        _T("建議使用「輸出ANSI檔」的選項，其原因將於下一頁說明。");
    g_all_usage_data[IC_USAGE_BASIC][9].image_resource_id = IDB_USAGE_BASIC_ANSI;

    g_all_usage_data[IC_USAGE_BASIC][10].description =
        std_tstring(_T("在「設定」→「進階設定」中可以調整輸出檔案的尺寸設置。\r\n")) +
        _T("在一般PTT程式中，美工圖案的橫向尺寸若超過79個半形字元，") +
        _T("則在使用上下按鍵瀏覽較大的美工圖案時，會出現錯誤的著色顯示。\r\n") +
        _T("「輸出ANSI檔」可以將美工圖案裁剪為較小的尺寸以避免此狀況。\r\n") +
        _T("裁剪後的內容橫向尺寸將限制為78個半形字元，縱向尺寸不影響。");
    g_all_usage_data[IC_USAGE_BASIC][10].image_resource_id = IDB_USAGE_BASIC_EXPORT;

    g_all_usage_data[IC_USAGE_DRAW].resize(7);

    g_all_usage_data[IC_USAGE_DRAW][0].description =
        std_tstring(_T("本章節將說明數種基本的輸入功能，其皆位於右側的「繪圖功能」選單。\r\n")) +
        _T("「填入小正方格」與前一章中介紹的「填入半形空白」具有類似的操作方式，將選定的顏色以特定尺寸填入繪圖區域。\r\n") +
        _T("「填入全形字元」可用於放置各種ANSI美工作品中常用的字元。\r\n") +
        _T("「插入／刪除空白」以類似插入／刪除文字的方式改變一橫列的內容。\r\n") +
        _T("「檢視內容」則用於在繪圖過程中確認各位置的字元與顏色配置。\r\n") +
        _T("在操作上，可以使用所標示的字母做為熱鍵來快速切換繪圖功能。");
    g_all_usage_data[IC_USAGE_DRAW][0].image_resource_id = IDB_USAGE_DRAW_INTRO;

    g_all_usage_data[IC_USAGE_DRAW][1].description =
        std_tstring(_T("「填入小正方格」能夠以全形方格的一半大小繪製內容。\r\n")) +
        _T("其利用ANSI橫向階梯方格中高度半行的「▄」來實現各種顏色組合。\r\n") +
        _T("暗色可以出現在任意位置，但亮色只能出現在每一行的下半部分。\r\n") +
        _T("若嘗試將亮色繪製到一行的上半部，其將會自動轉變為暗色。\r\n") +
        _T("您可以從右側的調色盤選擇顏色，或是用滑鼠右鍵從游標位置挑選。\r\n") +
        _T("在此功能下，當游標位於繪圖區域中時，會在其位置顯示粉紅色方框。\r\n") +
        _T("其用於表示實際ANSI內容中，「▄」方格被放入的位置。");
    g_all_usage_data[IC_USAGE_DRAW][1].image_resource_id = IDB_USAGE_DRAW_SQUARE;

    g_all_usage_data[IC_USAGE_DRAW][2].description =
        std_tstring(_T("「填入全形字元」為一較複雜的功能，可在游標位置處放入全形字元。\r\n")) +
        _T("在此功能下選擇顏色的方式略有不同，會同時使用到前景與背景色。\r\n") +
        _T("在調色盤中，以滑鼠左鍵選擇前景色，滑鼠右鍵選擇背景色。\r\n") +
        _T("在繪圖區域內，同樣可以用滑鼠右鍵從游標位置挑選顏色。\r\n") +
        _T("此時，位於該位置的字元所屬的前景色與背景色會同時被作為選擇。\r\n") +
        _T("調色盤下方的列表可選擇欲輸入的字元，被選擇者將以紅色方框標示。\r\n") +
        _T("另有一選單可切換字元列表的類型，此部分將於下一頁說明。");
    g_all_usage_data[IC_USAGE_DRAW][2].image_resource_id = IDB_USAGE_DRAW_BLOCK;

    g_all_usage_data[IC_USAGE_DRAW][3].description =
        std_tstring(_T("初次使用本程式時，字元列表中包含了繪圖常用的幾何形狀。\r\n")) +
        _T("使用者也可從列表上方的選單來選擇列表種類，以輸入其他類型的字元。\r\n") +
        _T("這些列表中的每個字元均可使用「填入文字」功能來輸入，\r\n") +
        _T("然而因其較為常用，故特別分類列出，方便使用者快速挑選。\r\n") +
        _T("右側的「前頁」「後頁」按鈕則可跳至選單中的前一個／後一個列表。\r\n") 
        _T("輸入類別相近但分至不同類表的字元（如日文、表格）時較為方便。\r\n") +
        _T("在「填入全形字元」功能下，也可用熱鍵Shift／Ctrl切換前頁／後頁。");
    g_all_usage_data[IC_USAGE_DRAW][3].image_resource_id = IDB_USAGE_DRAW_BLOCK_SELECT;

    g_all_usage_data[IC_USAGE_DRAW][4].description =
        std_tstring(_T("全形方格容易破壞原本的ANSI內容，在填入時須特別注意。\r\n")) +
        _T("游標位置處顯示的粉紅色方框，用於提示會被改動到的字元。\r\n") +
        _T("此範例中，所填入的方格，會同時改動到左右兩個全形字元。\r\n") +
        _T("按下滑鼠左鍵後，新的全形方格會被強制填入至游標位置。\r\n") +
        _T("舊的三角形字元將消失，左右側的空間則被半形空格所取代。\r\n") +
        _T("半型空格的顏色會自動選擇最接近者，但仍會使原本內容產生改變。\r\n") +
        _T("在其他可能破壞內容的繪圖功能中，也具有同樣的提示與改動機制。");
    g_all_usage_data[IC_USAGE_DRAW][4].image_resource_id = IDB_USAGE_DRAW_BLOCK_OVERLAP;

    g_all_usage_data[IC_USAGE_DRAW][5].description =
        std_tstring(_T("在一般的文字輸入介面中，可以使用空白鍵與Del鍵來水平移動內容。\r\n")) +
        _T("本程式以繪圖板的模式設計，因此以「插入／刪除空白」進行這些動作。\r\n") +
        _T("在游標位置處，以滑鼠左鍵插入空白，Shift+左鍵刪除空白。\r\n") +
        _T("空白的顏色選擇方式與「填入半形空白」相同，也可用滑鼠右鍵選擇。\r\n") +
        _T("游標左側的內容不會變化，右側的內容則會往右或往左滑動。\r\n") +
        _T("若游標位於全形字元的左側，此動作將破壞該全形字元。\r\n") +
        _T("另外，插入空白時，往右側滑動超出邊緣的內容將會消失。");
    g_all_usage_data[IC_USAGE_DRAW][5].image_resource_id = IDB_USAGE_DRAW_INSERT_DELETE;

    g_all_usage_data[IC_USAGE_DRAW][6].description =
        std_tstring(_T("有時候，您會需要觀察特定位置的字元結構，以思考如何產生美工效果。\r\n")) +
        _T("「檢視內容」可以顯示游標位置的實際ANSI字元與顏色設定。\r\n") +
        _T("在此功能下，為了防止意外，點擊滑鼠不會改變任何內容與顏色。\r\n") +
        _T("游標位置的粉紅色方框同樣用來表示字元的範圍，黃色方框則表示檢視的半形區域。（若該字元為半形，粉紅色方框將與黃色方框相同。）\r\n") +
        _T("同時，游標附近會出現一長方形區域，顯示字元內容與前景／背景色。\r\n") +
        _T("區域內的數字代表BBS顏色碼，如此範例中該顏色相當於「*[0;37;46m」。");
    g_all_usage_data[IC_USAGE_DRAW][6].image_resource_id = IDB_USAGE_DRAW_VIEW;

    g_all_usage_data[IC_USAGE_TEXT].resize(4);

    g_all_usage_data[IC_USAGE_TEXT][0].description =
        std_tstring(_T("「填入文字」（熱鍵Ｓ）可用來在指定位置輸入任意文字。\r\n")) +
        _T("本程式以類似小畫家的方式放入文字，而非BBS中的文字編輯介面。\r\n") +
        _T("文字會取代所在位置的原本字元，文字右側的內容則不會移動。\r\n") +
        _T("使用此功能時，必須先從調色盤中選擇想要使用的文字顏色。\r\n") +
        _T("當游標移到繪圖區域中時，會出現提示訊息，表示輸入的起始字元位置。\r\n") +
        _T("此時一部份的區域將會變暗以凸顯提示，但實際內容並不會產生改變。\r\n") +
        _T("當確定位置後，點下滑鼠左鍵將產生一個可輸入文字的小型介面。");
    g_all_usage_data[IC_USAGE_TEXT][0].image_resource_id = IDB_USAGE_TEXT_INTRO;

    g_all_usage_data[IC_USAGE_TEXT][1].description =
        std_tstring(_T("在白色的輸入區域中，您可以切換輸入法來輸入文字，或從他處剪下／複製任意字元並貼上至輸入區域，但內容限定為一橫列。\r\n")) +
        _T("當文字進入輸入區域後，視窗會向右延展，其字型將與現在使用的全形方格大小相符，可依此觀察文字在原本內容中將會覆蓋的區域。\r\n") +
        _T("（請注意，輸入內容將會被放置於「文字」所在的橫列。）\r\n") +
        _T("以滑鼠點擊「確定」按鈕或按下Enter鍵時，將會實際執行輸入動作。\r\n") +
        _T("若欲放棄輸入，請點擊「取消」按鈕或按下Esc鍵。");
    g_all_usage_data[IC_USAGE_TEXT][1].image_resource_id = IDB_USAGE_TEXT_INPUT;

    g_all_usage_data[IC_USAGE_TEXT][2].description =
        std_tstring(_T("輸入的文字將採用目前選擇的顏色，其位置的背景色則與原本內容相符。\r\n")) +
         _T("然而，若所選擇的顏色與原本的背景色完全相同，文字顏色將自動轉為黑色或白色（依背景色決定較適合者），以讓其變得可見。\r\n") +
         _T("此設計用於避免因選錯顏色，而難以在內容中找到文字位置的狀況。\r\n") +
         _T("在輸入文字後，仍可用「改變顏色」功能調整文字區域的前景與背景色。\r\n") +
         _T("請參考左側「顏色設定」目錄中的內容，以得知使用「改變顏色」功能調整文字顏色的方法，以及應注意的事項。");
    g_all_usage_data[IC_USAGE_TEXT][2].image_resource_id = IDB_USAGE_TEXT_COLOR;

    g_all_usage_data[IC_USAGE_TEXT][3].description =
        std_tstring(_T("本功能可以用來輸入常見BBS介面（如PCMan）可顯示的文字與符號。\r\n")) +
         _T("然而，仍有許多字元無法在BBS中顯示，其多半為較少見的中文字。\r\n") +
         _T("雖然這些字元大部分仍能在輸入文字的介面中顯示，但在儲存.ans檔時將無法轉換為合乎格式的內容。在BBS介面中也無法將其貼上至文章中。\r\n") +
         _T("在確定進行填入文字的動作時，如果輸入區域中包含了此類字元，則會出現一警告視窗，並將這些字元列出。") +
         _T("欲輸入的文字中，有效字元的部分仍然會被正常填入，但無效字元的位置將各自被兩個半形空白所替換。");
    g_all_usage_data[IC_USAGE_TEXT][3].image_resource_id = IDB_USAGE_TEXT_INVALID_CHAR;

    g_all_usage_data[IC_USAGE_BRUSH].resize(4);

    g_all_usage_data[IC_USAGE_BRUSH][0].description =
        std_tstring(_T("使用半形空白填入大範圍區域時，雖然可以拖曳滑鼠連續填入多個位置，但對於面積較大的區域，在操作上仍然相當不便。\r\n")) +
        _T("「大型筆刷」（熱鍵Ａ）為一延伸的著色功能，提供數種特定形狀、可調整尺寸的筆刷，以快速處理大範圍區域，或產生特殊形狀的圖樣。\r\n") +
        _T("由於ANSI字元內容與顏色的天生限制，大型筆刷以半形寬度的小正方格為作畫上的最小單位，並且只能使用非亮色的顏色。\r\n") +
        _T("本圖中對每種筆刷形狀各舉一例，並使用不同的顏色與尺寸。");
    g_all_usage_data[IC_USAGE_BRUSH][0].image_resource_id = IDB_USAGE_BRUSH_INTRO;

    g_all_usage_data[IC_USAGE_BRUSH][1].description =
        std_tstring(_T("在右方的功能選單中，大型筆刷位於較下端的部分。\r\n")) +
        _T("其選項下端的元件顯示目前的筆刷形狀與尺寸，並可供使用者調整。\r\n") +
        _T("當游標移至繪圖區域中時，提示訊息將顯示筆刷的形狀，與其對應至ANSI字元的實際位置。這兩者的精細程度皆受到小正方格結構的限制。\r\n") +
        _T("圓形與菱形的筆刷由小正方格組成最接近的形狀，橫線與直線筆刷則為長度為指定尺寸，粗度等同半形空格寬的細長條形。\r\n") +
        _T("操作上可用Ctrl與Shift鍵切換前／後一種筆刷，或用滑鼠滾輪改變尺寸。");
    g_all_usage_data[IC_USAGE_BRUSH][1].image_resource_id = IDB_USAGE_BRUSH_SHAPE;

    g_all_usage_data[IC_USAGE_BRUSH][2].description =
        std_tstring(_T("在填入內容的操作上，大型筆刷與「填入半形空白」完全一致。\r\n")) +
        _T("差別僅在大型筆刷具有較大的尺寸與特定的形狀，「填入半形空白」則使用單一半形空白的範圍做為其「筆刷」。\r\n") +
        _T("單擊滑鼠左鍵時，將依所選顏色與筆刷形狀改變所標示的範圍。\r\n") +
        _T("若按住滑鼠左鍵並拖曳，則會沿著軌跡改變筆刷形狀所通過的區域。\r\n") +
        _T("欲更改選擇的顏色時，除了於調色盤處操作，也可在現有內容上點擊滑鼠右鍵以選擇該處的顏色，但選擇亮色時將會轉變為暗色。");
    g_all_usage_data[IC_USAGE_BRUSH][2].image_resource_id = IDB_USAGE_BRUSH_OPERATION;

    g_all_usage_data[IC_USAGE_BRUSH][3].description =
        std_tstring(_T("大型筆刷的邊緣以半形空白或小正方格組合而成，容易與原有的全形字元衝突。填入內容時，本程式會自動調整字元，以產生最小程度的破壞。\r\n")) +
        _T("如果原本內容也是半形空白與小正方格的組合（通常也由筆刷所產生），便能完整保存筆刷外的內容。其他全形字元則較容易受到影響。\r\n") +
        _T("本範例中，新填入的圓形接觸了原本的菱形與三角形斜邊。") +
        _T("菱形上的一個小正方格因進入圓形筆刷範圍而被改為紅色，但其他內容完全沒有變化。") +
        _T("斜邊部分則為了配合圓形邊緣的結構，而有兩個三角形字元遭到破壞。");
    g_all_usage_data[IC_USAGE_BRUSH][3].image_resource_id = IDB_USAGE_BRUSH_CONFLICT;

    g_all_usage_data[IC_USAGE_COLOR].resize(9);

    g_all_usage_data[IC_USAGE_COLOR][0].description =
        std_tstring(_T("本程式中，有兩種主要方式可以改變目前使用的顏色：\r\n")) +
        _T("(1)從右側的調色盤中，以滑鼠左鍵選擇前景色，右鍵選擇背景色。\r\n") +
        _T("(2)在部分繪圖功能下，以滑鼠右鍵從繪圖區域中的游標位置挑選顏色。\r\n") +
        _T("在一般BBS介面中，可以針對每個字元分別調整其前景／背景色。\r\n") +
        _T("然而，於本程式中，許多的繪圖功能在操作時只需要選擇單一顏色。根據實際使用的ANSI字元種類，該顏色可能會以前景／背景色的方式呈現。\r\n") +
        _T("為了提升操作效率，在不同的繪圖功能下挑選顏色時，反應將略有不同。");
    g_all_usage_data[IC_USAGE_COLOR][0].image_resource_id = IDB_USAGE_COLOR_INTRO;

    g_all_usage_data[IC_USAGE_COLOR][1].description =
        std_tstring(_T("在「填入半形空白」、「填入小正方格」、「插入／刪除空白」、\r\n")) +
        _T("「大型筆刷」與「填入文字」功能下，無論如何選擇顏色，皆會同時改變前景與背景色。") +
        _T("選擇的顏色為亮色時，背景色仍會轉換為對應的暗色。\r\n") +
        _T("「填入小正方格」與「填入文字」可以使用亮色，其他一律使用暗色。\r\n") +
        _T("在「填入全形字元」功能下，可從調色盤中以滑鼠左鍵／右鍵分別選擇前景／背景色，") +
        _T("或以滑鼠右鍵從繪圖區域中一次挑選該處的前景／背景色。\r\n") +
        _T("在稍後介紹的「改變顏色」功能中，此反應將根據字元內容而產生變化。");
    g_all_usage_data[IC_USAGE_COLOR][1].image_resource_id = IDB_USAGE_COLOR_PICK;

    g_all_usage_data[IC_USAGE_COLOR][2].description =
        std_tstring(_T("「改變顏色」用於變更ANSI字元中部分區塊的顏色，而不改變字元內容。\r\n")) +
        _T("在此功能下，將游標移至任一字元上時，會以粉紅色方框顯示字元的完整範圍，並以黃色形狀顯示會產生改變的目標區塊。\r\n") +
        _T("若該字元為半形空白，或是數種特定的幾何形狀全形字元之一時，\r\n") +
        _T("則黃色形狀為在半形字元寬度內，與游標所指處顏色相同的區域之邊界。\r\n") +
        _T("點下滑鼠左鍵後，黃色形狀內的區域將會被改為目前所選擇的顏色。\r\n") +
        _T("若以滑鼠右鍵從這些字元選擇顏色，反應將與前述的「單色選擇」相同。");
    g_all_usage_data[IC_USAGE_COLOR][2].image_resource_id = IDB_USAGE_COLOR_CHANGE;

    g_all_usage_data[IC_USAGE_COLOR][3].description =
        std_tstring(_T("「改變顏色」也可以用來改變文字部分內容的顏色。\r\n")) +
        _T("此時其運作方式與在BBS中調整色碼相同，必須指定前景／背景色。\r\n") +
        _T("黃色形狀將占滿半形字元寬度，代表整個範圍的顏色都會改變。\r\n") +
        _T("在「改變顏色」功能下，挑選顏色的方式將有較複雜的變化：\r\n") +
        _T("(1)在右側調色盤中，可用滑鼠左鍵／右鍵選擇不同的前景／背景色。\r\n")
        _T("(2)在繪圖區域內以滑鼠右鍵挑選顏色時，視該處字元種類而定。若為一般文字，則一次挑選其前景／背景色，反之則根據游標位置挑選單色。");
    g_all_usage_data[IC_USAGE_COLOR][3].image_resource_id = IDB_USAGE_COLOR_TEXT;

    g_all_usage_data[IC_USAGE_COLOR][4].description =
        std_tstring(_T("在常用的幾何形狀全形字元中，正三角形字元具有特別的外觀與特性。\r\n")) +
        _T("其可用來表現斜向通過半形區域的對角線，以形成特定的傾斜邊緣。\r\n") +
        _T("這與字元的實際內容並不完全相符，因此會產生誤差與細長狀的空隙。\r\n") +
        _T("繪圖區域內會顯示正三角形字元的實際外形，供繪圖者判斷這些誤差。\r\n") +
        _T("然而，繪圖功能中的提示訊息則會將字元的前景／背景區域均表示為分割半形區域的直角三角形。在改變顏色時，可以僅改變點選的部分。\r\n") +
        _T("圖中為數種應用方式，下端部分顯示了實際內容與提示區域的差異。");
    g_all_usage_data[IC_USAGE_COLOR][4].image_resource_id = IDB_USAGE_COLOR_REGULAR_TRIANGLE;

    g_all_usage_data[IC_USAGE_COLOR][5].description =
        std_tstring(_T("「改變顏色」功能中另有一「範圍模式」選項，其運作方式與小畫家中的「範圍填色」工具相似，可以從指定位置一次將相鄰的同色區域上色。\r\n")) + 
        _T("半形空白、水平／垂直階梯狀方格、與直角三角形字元，在區域間相臨與否的判斷上，與繪圖區域顯示的外觀完全相符。") + 
        _T("正三角形字元的水平邊緣則視為與相臨區域接觸，即使實際上中間具有背景顏色的部分內容。\r\n") + 
        _T("其餘字元（主要是一般文字）則視為被背景色填滿整個區域。\r\n") + 
        _T("相臨的區域必須在水平或垂直方向上連接，角落的碰撞並不視為相臨。"); 
    g_all_usage_data[IC_USAGE_COLOR][5].image_resource_id = IDB_USAGE_COLOR_AREA_CONNECT;

    g_all_usage_data[IC_USAGE_COLOR][6].description =
        std_tstring(_T("「範圍模式」可從右側選單中勾選，在「改變顏色」功能下也能以鍵盤的Ctrl鍵切換一般模式與範圍模式。")) +
        _T("在此模式下，提示訊息中會出現朝向外側的箭頭，提醒使用者此操作可能會改變大範圍區域的顏色。\r\n") + 
        _T("按下滑鼠後，所有相鄰的同色區域皆會被改變成目前選擇的前景色。\r\n") + 
        _T("由於亮色在ANSI中的限制，此功能採用的顏色會轉變為一般顏色。\r\n") + 
        _T("對於文字等非空格或幾何形狀方塊的內容，其前景色會保持不變，僅背景色會被改變。若顏色相符，相鄰區域可以通過文字部分連接到另一端。");
    g_all_usage_data[IC_USAGE_COLOR][6].image_resource_id = IDB_USAGE_COLOR_AREA_CHANGE;

    g_all_usage_data[IC_USAGE_COLOR][7].description =
        std_tstring(_T("當使用者以範圍模式改變顏色時，如果對於相鄰區域範圍的判斷有錯誤，便可能在操作後改動到預期以外的部分。\r\n")) +
        _T("為了避免這種狀況，此模式具有預覽功能。將游標放在想要改動的位置後按住Shift鍵不放，便可在繪圖區域中看見動態預覽結果。") +
        _T("與游標位置相鄰的同色區域，其顏色會在原本顏色與目前選擇的顏色之間來回變化。\r\n") + 
        _T("預覽時顯示的變化並不會改變任何ANSI內容，僅是為了讓使用者確認相鄰區域的正確範圍，以決定是否要實際進行改動。");
    g_all_usage_data[IC_USAGE_COLOR][7].image_resource_id = IDB_USAGE_COLOR_AREA_HINT;

    g_all_usage_data[IC_USAGE_COLOR][8].description =
        std_tstring(_T("「檢視內容」、「邊緣調整」與「合併全形方格」這些功能的效果與目前選擇的顏色無關，也並非用來在繪圖區域上著色。\r\n")) +
        _T("當在這些功能下從調色盤選擇顏色時，本程式將會把繪圖功能切換至其他「需要選擇顏色」的功能中，最後使用過的一種。\r\n") + 
        _T("如圖所示，在「邊緣調整」功能下選擇綠色時，繪圖功能也會自動改變。\r\n") + 
        _T("此設計可讓使用者在進行一些調整後，在選擇顏色的同時快速切換至常用的著色功能來繼續繪圖，而不需要手動改變繪圖功能。");
    g_all_usage_data[IC_USAGE_COLOR][8].image_resource_id = IDB_USAGE_COLOR_MODE_SWITCH;

    g_all_usage_data[IC_USAGE_BOUNDARY].resize(12);

    g_all_usage_data[IC_USAGE_BOUNDARY][0].description =
        std_tstring(_T("「邊緣調整」是本程式最重要的功能，用來調整ANSI內容中的顏色邊界。\r\n")) +
        _T("搭配繪圖區域中的參考圖片，可以快速產生相似的美工圖案。\r\n") +
        _T("此功能具有兩種不同的模式，「一般模式」用於調整橫向和縱向的邊緣，「三角形模式」則專注於三角形字元可產生的變化。\r\n") +
        _T("勾選或取消右方選單中的「三角形模式」選項，可選擇欲使用的模式。\r\n") +
        _T("在「邊緣調整」功能下，也可使用Ctrl鍵於兩種模式之間切換。\r\n") +
        _T("本章節將介紹一般模式的部分，三角形模式則於下一章節介紹。");
    g_all_usage_data[IC_USAGE_BOUNDARY][0].image_resource_id = IDB_USAGE_BOUNDARY_INTRO;

    g_all_usage_data[IC_USAGE_BOUNDARY][1].description =
        std_tstring(_T("在使用基本的填色功能，根據參考圖片的內容進行簡單的著色後，")) +
        _T("不同顏色的區域間會產生粗略的交界，但與圖片的真實內容較不相符。\r\n") +
        _T("如圖，藍色與綠色的半形空白交界，與參考圖片的內容有明顯差異。\r\n") +
        _T("「邊緣調整」可用簡單的操作修改這些邊界，使其與參考圖片更加相似。\r\n") +
        _T("當游標移至交界處附近時，將根據現在的ANSI內容判斷是否可進行調整。\r\n") +
        _T("允許調整時，以黃色正方型方框表示調整時會改動到的全形方塊範圍，\r\n") +
        _T("紅色線條則代表將被調整的邊界，其可為橫向或縱向。");
    g_all_usage_data[IC_USAGE_BOUNDARY][1].image_resource_id = IDB_USAGE_BOUNDARY_ADJUST_START;

    g_all_usage_data[IC_USAGE_BOUNDARY][2].description =
        std_tstring(_T("當黃色方框與紅色線條出現時，點下滑鼠左鍵即會改變ANSI內容。\r\n")) +
        _T("您可以直接在目標位置點擊滑鼠，或是經由拖曳來微調至理想位置。\r\n") +
        _T("放開滑鼠後，橫向或縱向的階梯狀方塊將被放入方框所在位置，") +
        _T("其顏色則依周圍內容而決定。您可移動到其他需要調整的區域，進行類似的操作。\r\n") +
        _T("對於已存在的階梯狀方格，也可隨時用此功能移動其內部邊界的位置。\r\n") + 
        _T("如對先前調整過的內容不滿意，可重複進行多次修改。\r\n") +
        _T("此範例中，藍／綠色邊界在調整後，即與參考圖片的內容大致相似。");
    g_all_usage_data[IC_USAGE_BOUNDARY][2].image_resource_id = IDB_USAGE_BOUNDARY_ADJUST_ACTION;

    g_all_usage_data[IC_USAGE_BOUNDARY][3].description =
        std_tstring(_T("在一些場合中，可調整的邊緣或其移動方向會有數種不同可能性。\r\n")) +
        _T("在按下滑鼠左鍵之前，可略為移動游標，可以觀察能夠進行的各種調整。\r\n") +
        _T("黃色方框與紅色線條會隨游標位置變化，以表示欲進行的調整類型：\r\n") +
        _T("(1)同時可向上或向下調整時，依游標位置的方向而定。\r\n") +
        _T("(2)同時可調整橫向與縱向邊緣時，以較接近游標者為優先。\r\n") +
        _T("(3)調整連續半形空白之間的縱向邊緣時，根據游標靠左／中／右的程度，可以在三種不同的位置放入全形方格。");
    g_all_usage_data[IC_USAGE_BOUNDARY][3].image_resource_id = IDB_USAGE_BOUNDARY_ADJUST_MULTIPLE;

    g_all_usage_data[IC_USAGE_BOUNDARY][4].description =
        std_tstring(_T("也有一些彼此相似的場合，會因ANSI內容而影響調整的可行性：\r\n")) +
        _T("(1)對於某些全形方格，僅能將其具有一致顏色的邊緣向外延伸。\r\n") +
        _T("(2)亮色只能出現在ANSI字元中的前景區域。當全形方格的邊緣為亮色時，只有位於上方和右側的邊緣可以移動。\r\n") +
        _T("(3)當周圍已有其他全形字元，剩餘空間無法放入全形方格時，無法調整。") +
        _T("若全形字元在一整格內的顏色完全相同（此種字元通常產生自先前的調整動作），則會被視為兩個半形空格，並可能因此允許周圍區域的調整。");
    g_all_usage_data[IC_USAGE_BOUNDARY][4].image_resource_id = IDB_USAGE_BOUNDARY_ADJUST_VALIDATION;

    g_all_usage_data[IC_USAGE_BOUNDARY][5].description =
        std_tstring(_T("一種較為複雜，但在美工圖案中常用的組成，也可直接用此功能調整。\r\n")) +
        _T("如圖，以半形空格構成的斜線，被用來分隔兩側較大範圍的純色區域。\r\n") +
        _T("將游標移到斜線的水平交界處，並稍微偏離中央垂直交界時，可進行上下調整。（若接近中央，則會改為對中央邊界進行左右調整。）\r\n") +
        _T("移動游標時會插入顏色相符的全形方格，略為改變原本斜線的位置。\r\n") +
        _T("此功能可用來產生傾斜度有變化的斜線，以更加符合參考圖片的內容。\r\n") +
        _T("在實際作畫中，斜線的可為單一顏色，兩側區域的顏色也可以相同。");
    g_all_usage_data[IC_USAGE_BOUNDARY][5].image_resource_id = IDB_USAGE_BOUNDARY_SHEAR_LINE;

    g_all_usage_data[IC_USAGE_BOUNDARY][6].description =
        std_tstring(_T("當想要調整的距離較長時，可以按住滑鼠並拖曳較長距離，\r\n")) +
        _T("使邊緣移動超過一個全形方格，而不需要多次調整其間的每個方格。\r\n") +
        _T("游標所在的區域將會放入適當的全形字元，使邊界符合游標的位置。\r\n") +
        _T("所經過的區域之內容，則會自動轉換為顏色相符的半形空白或全形方格。\r\n") +
        _T("（亮色時必須使用全形方格才能顯示，暗色則會使用單純的空白。）\r\n") +
        _T("然而，此功能僅在原始邊界的兩側為單純的兩種顏色時才可使用。\r\n") +
        _T("顏色組成較複雜時，將無法拖曳超過一個方格，以避免破壞原本內容。");
    g_all_usage_data[IC_USAGE_BOUNDARY][6].image_resource_id = IDB_USAGE_BOUNDARY_EXTENDED_ADJUST;

    g_all_usage_data[IC_USAGE_BOUNDARY][7].description =
        std_tstring(_T("在美工圖案的細部，常常會需要僅針對半形字元的範圍做修改。\r\n")) +
        _T("然而基本的邊緣調整會改變到整個全形字元，無法直接完成操作。\r\n") +
        _T("調整水平邊緣時，可以按住Shift鍵切換為「部分調整」模式。\r\n") +
        _T("此時繪圖區域中仍會產生類似的提示內容，但水平邊緣將被分為兩段：\r\n") +
        _T("較亮的紅色線條表示將被調整的邊緣，暗紅色線條則會保持固定。\r\n") +
        _T("此時若按下或拖曳滑鼠，將只有亮紅色線條所屬的半形區域產生改變。\r\n") +
        _T("本功能僅適用於水平邊緣的上下調整，垂直邊緣無法產生此類變化。");
    g_all_usage_data[IC_USAGE_BOUNDARY][7].image_resource_id = IDB_USAGE_BOUNDARY_PARTIAL_ADJUST;

    g_all_usage_data[IC_USAGE_BOUNDARY][8].description =
        std_tstring(_T("前例中，同一個位置可以進行基本的全形範圍調整，或是部分調整。\r\n")) +
        _T("另有一些場合，因顏色或形狀組成較為複雜，無法進行基本調整。\r\n") +
        _T("在按住Shift後，才會出現方框與線條，以提示能夠進行的部分調整。\r\n") +
        _T("此狀況通常出現在色塊的角落或突起處，因其具有較短的水平邊緣。\r\n") +
        _T("實際作畫時，可以先按住Shift，再移動游標來尋找可以調整的區域。\r\n") +
        _T("若在一個區域附近有數種不同可能的調整方式，則同樣會根據游標位置決定最適合者，並以提示內容來表示將要進行的調整方格與邊緣。");
    g_all_usage_data[IC_USAGE_BOUNDARY][8].image_resource_id = IDB_USAGE_BOUNDARY_PARTIAL_ONLY;

    g_all_usage_data[IC_USAGE_BOUNDARY][9].description =
        std_tstring(_T("部分調整也可以直接應用在水平階梯狀方格上，但有一些限制：\r\n")) +
        _T("單一全形方格的水平邊界無法設置於不同高度，一側的邊界只能移動至方格的最上、最下端、或與另一側的邊界等高。\r\n") +
        _T("這個動作也可以直接使用「改變顏色」功能來達成。此一設計是為了省去切換功能的動作，適合在連續調整多處邊緣時利用。\r\n") +
        _T("若字元的一側已經為純色，另一側即可不受限制地進行調整。\r\n") + 
        _T("此狀況下一般調整與部分調整的效果相同，純色部分均會保持不變。");
    g_all_usage_data[IC_USAGE_BOUNDARY][9].image_resource_id = IDB_USAGE_BOUNDARY_PARTIAL_RESTRICT;

    g_all_usage_data[IC_USAGE_BOUNDARY][10].description =
        std_tstring(_T("邊緣調整可以實現許多功能，也使其操作上容易有混淆之處。\r\n")) +
        _T("游標附近呈現的提示內容，即是為了讓使用者快速理解與做出判斷。\r\n") +
        _T("在此舉一較為複雜的常見案例：中央區域有兩相鄰的半形空白，\r\n") +
        _T("其兩側皆被全形方格所佔據，但皆具有顏色一致的垂直邊緣。\r\n") +
        _T("在按下滑鼠前，藉由移動游標位置，在中央處有三種調整可選擇。\r\n") +
        _T("雖然進行的動作都是在相同位置插入全形方格，但使用的顏色會根據周圍內容而定，以實現移動不同邊緣的目標。");
    g_all_usage_data[IC_USAGE_BOUNDARY][10].image_resource_id = IDB_USAGE_BOUNDARY_ADJUST_MULTIPLE_EXAMPLE;

    g_all_usage_data[IC_USAGE_BOUNDARY][11].description =
        std_tstring(_T("在某些適當的場合下，邊緣調整可以同時改變全形字元的結構。\r\n")) +
        _T("此圖中，綠色突起部分為一水平階梯狀方格，其左側被設為純綠色。\r\n") +
        _T("直接對其進行調整時，會在左側保持純色的狀態下移動右側邊緣。\r\n") +
        _T("若在按下滑鼠前，將游標略為往右側移動，黃色方框也會往右滑動半格。\r\n") +
        _T("此現象表示黃色方框區域可被視為一全形方格，並對其進行調整。\r\n") + 
        _T("在確定執行調整後，黃色方格左側的區域會轉為綠色的半形空白。\r\n") +
        _T("此動作的運作原理，在「合併方格」章節中有較詳盡的說明。");
    g_all_usage_data[IC_USAGE_BOUNDARY][11].image_resource_id = IDB_USAGE_BOUNDARY_AUTO_MERGE;

    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE].resize(6);

    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][0].description =
        std_tstring(_T("本章節將針對邊緣調整中「三角形模式」的部分進行說明。\r\n")) +
        _T("當右側選單中「三角形模式」被勾選時，邊緣調整的判斷依據將改變。\r\n") +
        _T("其將利用直角三角形或正三角形字元，與顏色設定來產生傾斜的邊界。\r\n") +
        _T("使用者可將半形空格間的邊界調整為三角形的斜邊，或將斜邊變回直角狀的水平和垂直邊界，但無法對一般模式中關注的階梯狀方格進行調整。\r\n") + 
        _T("圖中上下兩部分顯示了兩種三角形字元的運用，其中正三角形的使用必須按住Shift來操作，與一般模式使用「部分調整」的方式相同。");
    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][0].image_resource_id = IDB_USAGE_BOUNDARY_TRIANGLE_INTRO;

    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][1].description =
        std_tstring(_T("三角形模式的運作原理仍與一般模式相似：根據原有的內容判斷調整的可行性，並利用適合的字元與前景／背景色表現邊緣的移動。\r\n")) +
        _T("最基本的一種類型為純色的全形區域所產生的邊界。若互相垂直的兩相臨邊界在另一側具有相同的顏色，") +
        _T("則可插入一方向相符的直角三角形，並給予適當的配色以將原本的直角狀邊界「移動」至斜邊。\r\n") +
        _T("惟當邊界的兩端皆為亮色時，會因為背景色的限制而無法調整。\r\n") +
        _T("若在同一位置有多種可能的調整方式，則可移動游標來選擇想要進行者。");
    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][1].image_resource_id = IDB_USAGE_BOUNDARY_TRIANGLE_BASIC;

    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][2].description =
        std_tstring(_T("進行調整時，點擊與拖曳滑鼠的操作方式類似於邊緣調整的一般模式。\r\n")) +
        _T("按住滑鼠左鍵後，可以在直角狀邊界、斜邊、與另一端的直角狀邊界之間進行選擇。也可以利用連續的操作來逐漸推移邊界的位置。\r\n") +
        _T("若原本的內容已是由直角三角形產生的斜邊，且字元本身的顏色一致時，則無論周圍的顏色組合為何，都可進行調整。\r\n") +
        _T("然而，若在這種情況下調整成為直角狀邊界，則原本位於斜邊某一側的顏色將會消失。後續的調整將經由新的內容來進行判斷。");
    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][2].image_resource_id = IDB_USAGE_BOUNDARY_TRIANGLE_MOVING;

    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][3].description =
        std_tstring(_T("在適當的配色下，三角形方框也可表現出斜線與垂直線的混合邊界。\r\n")) +
        _T("本功能也可用於產生這些內容，而不需要實際設定細部區域的顏色。\r\n") +
        _T("本圖中列出三種相似的類型，其原始內容均完全相同。") +
        _T("１的調整動作產生了基本的三角形斜邊，２和３則形成了兩種不同形狀的混合邊界。\r\n") +
        _T("圖片左側部分為調整前所顯示的提示訊息。１的目標方格與２和３不同，具有半形寬度的偏移。２和３所欲調整的邊緣位置也略有差異。\r\n") +
        _T("在按下滑鼠左鍵前，略為移動游標的位置即可挑選想要採用的類型。");
    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][3].image_resource_id = IDB_USAGE_BOUNDARY_TRIANGLE_HALF;

    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][4].description =
        std_tstring(_T("除了直角三角形外，正三角形字元也常被用運於美工作品中。\r\n")) +
        _T("在三角形模式的邊緣調整功能中，操作時若按住Shift鍵，便會根據正三角形字元的特性進行調整，反之則以直角三角形字元來調整。\r\n") +
        _T("正三角形字元的斜邊在提示訊息中將被標示為半形區域的對角線，但其與字元內容並不完全相符。") +
        _T("在調整結束後，繪圖區域顯示的內容將為實際的ANSI字元表現。產生的效果是否適合，可能需要額外的考量來決定。");
    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][4].image_resource_id = IDB_USAGE_BOUNDARY_TRIANGLE_REGULAR;

    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][5].description =
        std_tstring(_T("正三角形字元的調整比直角三角形單純，不會出現混合型的邊界。\r\n")) +
        _T("僅在一種情況下，對於相同的內容，具有兩種可能的調整方式。\r\n") +
        _T("圖中為一簡單的直角狀邊界，若要將最外端的綠色凸起部分轉變為斜邊，則可在兩個不同的位置插入正三角形字元。\r\n") +
        _T("由於正三角形字元本身的誤差，所產生的結果外觀也會略有不同。\r\n") +
        _T("何者的視覺效果較佳，則與美工作品想要表現的內容有關。\r\n") +
        _T("在進行調整前，同樣可以藉由略為移動游標來決定所採用的類型。");
    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][5].image_resource_id = IDB_USAGE_BOUNDARY_TRIANGLE_REGULAR_CASE;

    g_all_usage_data[IC_USAGE_MERGE].resize(7);

    g_all_usage_data[IC_USAGE_MERGE][0].description =
        std_tstring(_T("「合併全形方格」（熱鍵Ｄ）為一種用途較特殊的功能。\r\n")) +
        _T("其可在保持某一全形區域的外觀不變的條件下，改變字元的結構。\r\n") +
        _T("在構圖初期，通常不會有使用此功能的需求。\r\n") +
        _T("當多次調整過邊緣，或已插入許多全形方格後，ANSI字元的配置會變得較雜亂。") +
        _T("本功能可以重新安排某些區域的ANSI字元，以利後續的編輯動作。\r\n") +
        _T("目前的程式版本中，大部分的合併動作皆會自動執行，請見本章節最後一頁說明。使用者仍可使用本功能手動合併內容。");
    g_all_usage_data[IC_USAGE_MERGE][0].image_resource_id = IDB_USAGE_MERGE_INTRO;

    g_all_usage_data[IC_USAGE_MERGE][1].description =
        std_tstring(_T("在此舉一範例，說明「合併全形方格」的可能應用場合。\r\n")) +
        _T("圖中，紅色線條與數字表示半形字元的範圍。在01，34與67的位置各有一全形字元，2和5的位置則為半形空白。\r\n") +
        _T("假設需要在位置4放入文字，若直接將文字填入，位置3的內容將被破壞。\r\n") +
        _T("若想維持其外觀，則可使用「填入全形字元」在23的位置放入適當的縱向階梯狀方格，再使用「改變顏色」調整各部分的顏色，使其與原本一致。\r\n") +
        _T("這個過程需要執行許多步驟，而「合併全形方格」可大幅簡化其操作。");
    g_all_usage_data[IC_USAGE_MERGE][1].image_resource_id = IDB_USAGE_MERGE_EXAMPLE;

    g_all_usage_data[IC_USAGE_MERGE][2].description =
        std_tstring(_T("本功能下，在可以合併為單一全形字元的位置，將出現提示內容。\r\n")) +
        _T("黃色方框表示可合併為新全形字元的範圍，粉紅色方框表示原有的字元。\r\n") +
        _T("（若無提示，則代表無法進行合併，或者該處已經是一全形字元。）\r\n") +
        _T("按下滑鼠左鍵後，適當的全形字元將被放入黃色方框的位置，超出的部分則轉為半形空白，字元的前景／背景色皆被修改，使外觀保持不變。\r\n") +
        _T("此例中，新字元為一縱向階梯狀方格。當其右端的天藍色區域變成半形空白後，便能以「填入文字」功能安全地放入文字。");
    g_all_usage_data[IC_USAGE_MERGE][2].image_resource_id = IDB_USAGE_MERGE_ACTION;

    g_all_usage_data[IC_USAGE_MERGE][3].description =
        std_tstring(_T("另外有一種狀況，通常在多次重複修改內容與調整顏色後出現。\r\n")) +
        _T("圖中，0和5的位置為半形空白，12和34處則為水平階梯狀全形方格。\r\n") +
        _T("在一連串的編輯後，1和4處被設定為純色，2和3處則包含多種顏色。\r\n") +
        _T("此設置雖然可在23的位置處表現想要的內容，卻浪費了1和4的位置。\r\n") + 
        _T("若想在01或45處直接放入新的全形方格，將會破壞2或3處的內容。\r\n") +
        _T("由於水平邊界的高度相同，可以將23處的內容合併為單一全形方格。\r\n") + 
        _T("提示內容中的兩個粉紅色方格，表示兩端皆有部分被轉為半形空白。");
    g_all_usage_data[IC_USAGE_MERGE][3].image_resource_id = IDB_USAGE_MERGE_DOUBLE;

    g_all_usage_data[IC_USAGE_MERGE][4].description =
        std_tstring(_T("較複雜的場合中，需要同時合併一橫列中的多個字元才能保持外觀不變。\r\n")) +
        _T("本圖舉出一些範例，其中滑鼠游標的位置代表使用者想要合併的位置。\r\n") +
        _T("如果單純只將該位置合併為一字元，並將超出的部分以半形空白替換，則必定會破壞原本的內容。必須將該部分繼續與更外側的內容合併。\r\n") +
        _T("本程式會自動進行判斷，若可能經由連續的合併維持整體外觀，則會在提示內容中畫出多個方格以表示此狀況。") +
        _T("按下滑鼠左鍵時，將會同時合併所有標示為亮黃色的方框，無需使用者在各個位置一一點擊。");
    g_all_usage_data[IC_USAGE_MERGE][4].image_resource_id = IDB_USAGE_MERGE_MULTIPLE;

    g_all_usage_data[IC_USAGE_MERGE][5].description =
        std_tstring(_T("如果游標位置處的內容可以合併為一全形字元，但超出範圍會被破壞，且無法經由前述的連續合併來化解，則可以按住Shift鍵執行「強制合併」。\r\n")) +
        _T("本範例中，若想要合併12位置的內容，則必定會破壞左側的階梯狀結構。\r\n") +
        _T("此時提示內容不會主動出現，必須在按住Shift鍵後才會顯示。\r\n") +
        _T("若按下滑鼠左鍵確定進行合併，則黃色方框內的部分仍會被完整保留，\r\n") + 
        _T("超出的部分將會轉為最適合的半形空白，但其外觀仍會產生改變。\r\n") +
        _T("如圖，位置0處的內容轉變為白色的半形空白，原本綠色的區域則消失。");
    g_all_usage_data[IC_USAGE_MERGE][5].image_resource_id = IDB_USAGE_MERGE_FORCED;

    g_all_usage_data[IC_USAGE_MERGE][6].description =
        std_tstring(_T("合併方格的原理與判斷，也被間接地運用在其他繪圖功能上。\r\n")) +
        _T("在「邊緣調整」與「填入小正方格」功能中，提示訊息內的正方形區域，代表編輯動作中將要放入全形字元的位置。") +
        _T("然而，該位置原本可能並未具有全形字元，或是與已存在的全形字元有半形寬度的偏移。\r\n") +
        _T("只要該位置能夠合併方格且不改變外觀（意即並非進行「強制合併」），\r\n") +
        _T("就會顯示提示訊息，並在確定進行編輯時先行合併方格以重新安排字元。\r\n") +
        _T("這個動作將由程式自動執行，無需使用者切換至「合併全形方格」功能。");
    g_all_usage_data[IC_USAGE_MERGE][6].image_resource_id = IDB_USAGE_MERGE_AUTO;

    g_all_usage_data[IC_USAGE_ACTION].resize(7);

    g_all_usage_data[IC_USAGE_ACTION][0].description =
        std_tstring(_T("使用者在繪圖時，程式會記錄每一個改變ANSI內容的步驟。\r\n")) +
        _T("若對於某一個步驟反悔，可隨時倒回至先前的狀態重新編輯。\r\n") +
        _T("步驟的詳細資訊位於右側選單中的最下端。在此可查看各個步驟的種類，步驟總數與目前位置，或進行復原／重做等操作。\r\n") +
        _T("程式開起時步驟總數為０。先前的使用過程並不會被記錄下來。\r\n") +
        _T("新增專案、讀取專案、或讀取ANSI檔後，將清除所有已記錄的步驟。\r\n") +
        _T("但儲存專案、儲存／輸出ANSI檔、或讀入圖片則不會影響現有的步驟。");
    g_all_usage_data[IC_USAGE_ACTION][0].image_resource_id = IDB_USAGE_ACTION_INTRO;

    g_all_usage_data[IC_USAGE_ACTION][1].description =
        std_tstring(_T("在進行了一些操作後，步驟資訊中的列表將包含所有已進行的步驟。\r\n")) +
        _T("將其展開後，由上到下分別為時間上最新至最舊的步驟。\r\n") +
        _T("您可在這些步驟中挑選欲倒回的程度，目前的步驟將以特殊顏色表示。\r\n") +
        _T("若按下滑鼠左鍵選擇某一步驟，繪圖內容將會倒回至「剛執行完該步驟」的狀態。意即，將該步驟之後的所有步驟復原。\r\n") +
        _T("列表最下端的「復原所有步驟」則可將已記錄的所有步驟全部復原。\r\n") + 
        _T("若不想改變現有的狀態，點擊列表外部的區域即可取消。");
    g_all_usage_data[IC_USAGE_ACTION][1].image_resource_id = IDB_USAGE_ACTION_STEPS;

    g_all_usage_data[IC_USAGE_ACTION][2].description =
        std_tstring(_T("一般使用時，「目前步驟」的數值必定等於「步驟總數」。\r\n")) +
        _T("只有在某一部分的步驟被復原後，「目前步驟」會具有較小的數值。\r\n") +
        _T("當未有新的步驟被加入時，可以在步驟選單中選擇順序較後的步驟，\r\n") +
        _T("以將先前復原的步驟依序重做，直到所選擇的步驟被完成。\r\n") +
        _T("若在此狀態下直接進行新的步驟，位於目前步驟之後的所有步驟將從記錄中被刪除，新的步驟則被加入緊鄰目前步驟之後的位置。\r\n") +
        _T("以此方式刪除的步驟將無法回復，在來回操作時須特別注意。");
    g_all_usage_data[IC_USAGE_ACTION][2].image_resource_id = IDB_USAGE_ACTION_OVERWRITE;

    g_all_usage_data[IC_USAGE_ACTION][3].description =
        std_tstring(_T("許多繪圖功能可以按住並拖曳滑鼠來進行，其將被記錄為單一步驟。\r\n")) +
        _T("這些功能包含了「填入半形空白」、「填入小正方格」、「大型筆刷」與") +
        _T("「改變顏色」等上色功能，以及經由拖曳滑鼠來微調的「邊緣調整」。\r\n") +
        _T("復原／重做步驟時，整個拖曳過程所造成的改變都會一併進行。\r\n") +
        _T("已被記錄的連續動作將無法分割，若您想要分解步驟以使復原／重做時具有更大的彈性，則可以使用點放滑鼠的方式分別在不同的位置操作。");
    g_all_usage_data[IC_USAGE_ACTION][3].image_resource_id = IDB_USAGE_ACTION_CONTINUOUS;
    
    g_all_usage_data[IC_USAGE_ACTION][4].description =
        std_tstring(_T("復原／重做少量步驟時可使用按鈕與熱鍵，而不需點開列表來選擇。\r\n")) +
        _T("點擊步驟列表下端的「復原一步」與「重複一步」按鈕，\r\n") +
        _T("或使用熱鍵Alt+Z與Alt+X，可進行單步的復原／重做。\r\n") +
        _T("若目前步驟的位置已無法復原或重做，按鈕將顯示為淺灰色。\r\n") +
        _T("一般操作時，若較常使用調色盤或全形字元列表，則步驟資訊通常會位於右方選單的顯示範圍之外，此時使用熱鍵操作較為方便。");
    g_all_usage_data[IC_USAGE_ACTION][4].image_resource_id = IDB_USAGE_ACTION_ONE_STEP;

    g_all_usage_data[IC_USAGE_ACTION][5].description =
        std_tstring(_T("本程式目前可記錄最多500個操作步驟，其於大部份的情況下已足夠。\r\n")) +
        _T("長期使用後，記錄數量仍可能到達上限。此後新進行的步驟仍然會被持續記錄，但最舊的步驟將從記錄中清除，以維持總數不超過500。\r\n") +
        _T("若在此時選擇步驟列表最下端的「復原所有步驟」，則只會復原目前被記錄的500個步驟，更舊的步驟將無法復原。");
    g_all_usage_data[IC_USAGE_ACTION][5].image_resource_id = IDB_USAGE_ACTION_MAX_COUNT;

    g_all_usage_data[IC_USAGE_ACTION][6].description =
        std_tstring(_T("除了各種繪圖功能外，另外有兩種動作也會被記錄為操作步驟。\r\n")) +
        _T("右方功能選單中，位於步驟資訊區域上方的「清除畫板」按鈕可用來清除所有現存的ANSI內容，此動作會被記錄成名為「清除畫板」的步驟。\r\n") +
        _T("另外，當按下PageDown、向下的方向鍵、或直接以功能選單中的按鈕改變繪圖區域所對應的橫列時，") +
        _T("如果繪圖區域最下端的橫列編號超過原本ANSI內容所具有的橫列數量，則會自動補上空白橫列以滿足所需數量。\r\n") +
        _T("當新的橫列被自動加入時，將產生名為「延伸底端範圍」的步驟。");
    g_all_usage_data[IC_USAGE_ACTION][6].image_resource_id = IDB_USAGE_ACTION_SPECIAL;


    return true;
}

static bool g_dummy = InitAllUsageData();