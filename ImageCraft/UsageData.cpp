#include "stdafx.h"
#include "UsageData.h"
#include "resource.h"

std::vector<std::vector<UsagePageData> > g_all_usage_data;

bool InitAllUsageData()
{
    g_all_usage_data.resize(IC_USAGE_MENU_AMOUNT);

    g_all_usage_data[IC_USAGE_BASIC].resize(11);

    g_all_usage_data[IC_USAGE_BASIC][0].description =
        std_tstring(_T("���{���O�@�ӥHø�ϥ\�ର�D��ANSI�s��n��A�]�p�F�P�Ӧ۩�p�e�a�C\r\n")) +
        _T("�������������j���������uø�ϰϰ�v�A�Ω���ܻPø�sANSI���e�C\r\n") +
        _T("�k�����u�\����v�h��mø�ϹL�{���|�ϥΨ쪺�U�إ\��C\r\n") +
        _T("�b�}�l�ϥΫe�A�нT�{ø�ϰϰ쪺�j�p������ܨ䤤��ݨ�����u�A\r\n") +
        _T("�H�ά��u�k�����q�u���v��u�����v��������Ʀr�C\r\n") +
        _T("�p�G�o�Ǥ��e�Q�B�סA�нվ�����j�p�A�H��jø�ϰϰ�C\r\n") +
        _T("�Y�z���Ĥ@���}�ҥ��{���A�h�������|�㦳�A���w�]�ؤo�C");
    g_all_usage_data[IC_USAGE_BASIC][0].image_resource_id = IDB_USAGE_BASIC_INTRO;

    g_all_usage_data[IC_USAGE_BASIC][1].description =
        std_tstring(_T("�C��ANSI�@�~���s�@�L�{�A�q�`�|���H�ۤ@�i�Ϥ��@���ѦҡC\r\n")) +
        _T("�b���{�����AANSI���e�P�ѦҹϤ����զX�Q�٬��@�u�M�סv�C\r\n") +
        _T("�����`���N���бM�ת��򥻬[�c�A�H�Φs�ɡ�Ū�ɵ��`�Υ\��C\r\n") +
        _T("�����A�бq�W���檺�u�ɮסv���u�}�ҹϤ��v���AŪ�J�@�i�Ϥ��C\r\n") +
        _T("�z�i�H�����ϥΩҪ��a��xpbliss.jpg�A�άO��ܥ�����w���Ϥ��C\r\n") +
        _T("�bŪ�J��A�Ϥ��|�H�b�z�����A�X�{�bø�ϰϰ줤�A�Ω�bø�sANSI���e�ɧ@�Y�ɤ��A���Y���U�٪��u�Թϡv�\��C");
    g_all_usage_data[IC_USAGE_BASIC][1].image_resource_id = IDB_USAGE_BASIC_IMAGE;

    g_all_usage_data[IC_USAGE_BASIC][2].description =
        std_tstring(_T("�P�Ϥ��������ﶵ���\���檺�̤W�ݡA�䤤���u���\���ʹϤ��v���@�w����A�u���b�Ŀ�ɤ~����ܹϤ�����m�B�ؤo�P���סC\r\n")) +
        _T("�b�}�l�@�e�e�A�i�H���N�Q�nø�s�����e���ʨ�ø�ϰϰ줤�H��ݪ��u�򦨪��ϰ�A��۷���V80�b�ΡB�a�V23�檺ANSI���e�C\r\n") +
        _T("�@�e���h��ĳ�N�u���\���ʹϤ��v�����A�H�קK���ʨ�վ�n���Ϥ��C\r\n") +
        _T("�u�@�סv�Ʊ�i�H�վ�Ϥ������z���סA�H��K�PANSI���e�i����C\r\n") +
        _T("�]�i�Ρu���ùϤ��v�]����ѡ^���ֳt�����A�Ӥ����ܿ@�ת��]�w�ȡC");
    g_all_usage_data[IC_USAGE_BASIC][2].image_resource_id = IDB_USAGE_BASIC_IMAGE_CONTROL;

    g_all_usage_data[IC_USAGE_BASIC][3].description =
        std_tstring(_T("�Ϥ��������B�Y��P����i�H�ǥѷƹ��P�u���ާ@�A�ο�椤���u�Y��v�P�u����v�Ʊ�ӽվ�C�bø�ϰϰ줤�즲��СA�Ϥ��Y�|�H�������C\r\n")) +
        _T("�ϥηƹ��u���ɡA�Ϥ��N�H��Ц�m���T�w�I�i���Y��C�Y�վ�u�Y��v�Ʊ�A�Y�񪺩T�w�I�h�|�אּø�ϰϰ쪺���ߡC\r\n") +
        _T("����Ϥ��ɡA�i�H�����վ�u����v�Ʊ�ӧ��ܨ��סA�]�i����Shift���A�즲��СA���ɱN�ھ�ø�ϰϰ줤�ߩ���Ц�m����V�ӱ���Ϥ��C\r\n") +
        _T("�u��������v���s�h�i�N���ਤ���k�s�A���Ϥ��^��쥻����V�C");
    g_all_usage_data[IC_USAGE_BASIC][3].image_resource_id = IDB_USAGE_BASIC_IMAGE_MOVE;

    g_all_usage_data[IC_USAGE_BASIC][4].description =
        std_tstring(_T("�bø�ϰϰ줤�AANSI�r�����氪�P���Φr�����e�׹w�]��24�����C\r\n")) +
        _T("�W���檺�u�]�w�v���u�i���]�w�v���i�H��惡�ƭȡC�z�Q�ƭȬ�8��4�����ơA") +
        _T("�i��ֶ������������ܮɪ��~�t�C����ĳ�������򥻳]�w�C\r\n") +
        _T("24����������ѳ̦n����ܫ~��A�]������`����BBS������ܤؤo�C\r\n") +
        _T("�Y�z���ù��ѪR�׸��p�A�h�i����20��16�������]�w�ȡC\r\n") +
        _T("�Ъ`�N�A����ܥ��Τ�檺�ؤo�ɡA�ѦҹϤ����|�H���Y��C\r\n") +
        _T("�̦n��ø�ϫe�M�w�A�X���ؤo�A�ɶq���n��ø�ϹL�{�����]�w�C");
    g_all_usage_data[IC_USAGE_BASIC][4].image_resource_id = IDB_USAGE_BASIC_BLOCK_SIZE;

    g_all_usage_data[IC_USAGE_BASIC][5].description =
        std_tstring(_T("�̰򥻪�ø�ϥ\�ର�u��J�b�Ϊťաv�A�бq�k���uø�ϥ\��v����ܡC\r\n")) +
        _T("���\��H�γ�������Lø�ϥ\��A�㦳�������W���k�G\r\n") +
        _T("�H�ƹ������J�ثe��ܪ��C��A�ƹ��k��h�i�q��Ц�m�D���C��C\r\n") +
        _T("�z�]�i�ηƹ�����q�k�����զ�L���D���C��A���b���\��U�A�ѩ�b�Ϊťե����H�I�����{�Aø�ϮɨϥΪ��C��N�۰��ഫ���t��C\r\n") +
        _T("�Y�ݭn��J�j���ϰ쪺�ťաA�i���I������é즲���覡�i��C\r\n") +
        _T("�b�o��²�檺�d�Ҥ��A�ȥH�Ŧ�M��⪺�b�ΪŮ�񺡵e���C");
    g_all_usage_data[IC_USAGE_BASIC][5].image_resource_id = IDB_USAGE_BASIC_DRAW_SPACES;

    g_all_usage_data[IC_USAGE_BASIC][6].description =
        std_tstring(_T("�Y�Q�nø�s���ؼ��a�V�ؤo���j�A�i�H�X�jø�ϰϰ�H�[�\�h���C\r\n")) +
        _T("�k�����T���b�Y�P�u�W���v�u�U���v�i�H����ø�ϰϰ�ҹ�������C�C\r\n") +
        _T("���ɰѦҹϤ������e�|�۰ʥ����ܹ������ϰ�A�ú����쥻���Y���ҡC\r\n") +
        _T("��L�����W�U��V��BPgDn��PgUp�]�i�Ω�i��P�˪��ާ@�C\r\n") +
        _T("���d�Ҥ��Ȯɤ��ݭn�@���ާ@�A����i�A�Ω���j����ANSI�@�~�C\r\n") +
        _T("��ĳ�b�@�}�lŪ�J�Ϥ��ɡA�N�Q�nø�s���ϰ줧�̤W�ݹ�����1-23�C�A\r\n") +
        _T("�b�վ�n�A�X����m�M�ؤo��A�A�N��ƼW�[�ܨ������ƶq�C");
    g_all_usage_data[IC_USAGE_BASIC][6].image_resource_id = IDB_USAGE_BASIC_SCROLL;

    g_all_usage_data[IC_USAGE_BASIC][7].description =
        std_tstring(_T("ANSI�@�~�`�`�ݭn�h���ק�A�x�s��Ū���u�@���A���@�`�Ϊ����n�\��C\r\n")) +
        _T("�W���檺�u�ɮסv���u�x�s�M�סv�i�H�N�ثe���A�x�s����@�ɮסG\r\n") +
        _T("�]�tANSI���e�B�ѦҹϤ����e�B�H�ιϤ��b�e��������m�P�Y���ҡC\r\n") +
        _T("ANSI���e�̤U�ݪ��ťա]�i��]���վ��ƮɦӲ��͡^�|�Q�۰ʬٲ��C\r\n") +
        _T("�u�ɮסv���uŪ���M�סv�h�i�q���x�s���M���ɱN�����A�����٭�C\r\n") +
        _T("�N�Y�A�ѦҹϤ������e�|�Q�۰�Ū�J�A�é�m��w�վ�n���z�Q��m�C\r\n") +
        _T("�u�n�O�s�M���ɡA�Y�ϨS���쥻���Ϥ��]�i�H�b�P�˪��t�m�U�i��ø�ϡC\r\n");
    g_all_usage_data[IC_USAGE_BASIC][7].image_resource_id = IDB_USAGE_BASIC_PROJECT;

    g_all_usage_data[IC_USAGE_BASIC][8].description =
        std_tstring(_T("���F�D���x�s��Ū���M�ץ~�A���{���]���۰��x�s���\��C\r\n")) +
        _T("�C���}�ҥ��{���ɡA�|�۰�Ū�J�W�@�������e���ϥΪ��A�C\r\n") +
        _T("���ɵ������D��|�X�{�u(�۰��x�s)�v�A�H��ܥثe���A�i��]�t�@�ǥ��D���x�s����ʡA�ëD�P���e�x�s���M���ɤ��e�ۦP�C\r\n") +
        _T("�Y�b�����A�U�i��u�x�s�M�סv�A�h���e�|�Q��s�ܸӱM���ɤ��C\r\n") +
        _T("�z�i�H�q�{�s���A�~��s��A��Ū�J���e�x�s���M���ɥH���̷s��ʡC");
    g_all_usage_data[IC_USAGE_BASIC][8].image_resource_id = IDB_USAGE_BASIC_AUTO_SAVE;

    g_all_usage_data[IC_USAGE_BASIC][9].description =
        std_tstring(_T("�b�u�ɮסv��椤�A�]�i��W�B�zANSI���e�A�Ӥ��v�T�Ϥ��]�m�C\r\n")) +
        _T("���{��ø�s���ϧΦs��ANSI��(.ans)��A�i�b��LBBS�{�����}�ҡC\r\n") +
        _T("�Y�N��L�ӷ���ANSI��Ū�J���{����A�h���V�e�׷|���쭭��C\r\n") +
        _T("�C�@��C���A���{���̦h�u��B�z80�ӥb�Φr���A�W�L�������N�Q�����C\r\n") +
        _T("���a�V���׫h�S������A�i�Ϋe�z���W�U½���\��Ӳ�����ܤ��e�C\r\n") +
        _T("��ǳƭn��ø�s���@�~�o�����峹�ɡA�ѩ�BBS��ܤW������A\r\n") +
        _T("��ĳ�ϥΡu��XANSI�ɡv���ﶵ�A���]�N��U�@�������C");
    g_all_usage_data[IC_USAGE_BASIC][9].image_resource_id = IDB_USAGE_BASIC_ANSI;

    g_all_usage_data[IC_USAGE_BASIC][10].description =
        std_tstring(_T("�b�u�]�w�v���u�i���]�w�v���i�H�վ��X�ɮת��ؤo�]�m�C\r\n")) +
        _T("�b�@��PTT�{�����A���u�Ϯת���V�ؤo�Y�W�L79�ӥb�Φr���A") +
        _T("�h�b�ϥΤW�U�����s�����j�����u�Ϯ׮ɡA�|�X�{���~���ۦ���ܡC\r\n") +
        _T("�u��XANSI�ɡv�i�H�N���u�Ϯ׵��Ŭ����p���ؤo�H�קK�����p�C\r\n") +
        _T("���ū᪺���e��V�ؤo�N���78�ӥb�Φr���A�a�V�ؤo���v�T�C");
    g_all_usage_data[IC_USAGE_BASIC][10].image_resource_id = IDB_USAGE_BASIC_EXPORT;

    g_all_usage_data[IC_USAGE_DRAW].resize(7);

    g_all_usage_data[IC_USAGE_DRAW][0].description =
        std_tstring(_T("�����`�N�����ƺذ򥻪���J�\��A��Ҧ��k�����uø�ϥ\��v���C\r\n")) +
        _T("�u��J�p�����v�P�e�@�������Ъ��u��J�b�Ϊťաv�㦳�������ާ@�覡�A�N��w���C��H�S�w�ؤo��Jø�ϰϰ�C\r\n") +
        _T("�u��J���Φr���v�i�Ω��m�U��ANSI���u�@�~���`�Ϊ��r���C\r\n") +
        _T("�u���J���R���ťաv�H�������J���R����r���覡���ܤ@��C�����e�C\r\n") +
        _T("�u�˵����e�v�h�Ω�bø�ϹL�{���T�{�U��m���r���P�C��t�m�C\r\n") +
        _T("�b�ާ@�W�A�i�H�ϥΩҼХܪ��r����������ӧֳt����ø�ϥ\��C");
    g_all_usage_data[IC_USAGE_DRAW][0].image_resource_id = IDB_USAGE_DRAW_INTRO;

    g_all_usage_data[IC_USAGE_DRAW][1].description =
        std_tstring(_T("�u��J�p�����v����H���Τ�檺�@�b�j�pø�s���e�C\r\n")) +
        _T("��Q��ANSI��V�����椤���ץb�檺�u�e�v�ӹ�{�U���C��զX�C\r\n") +
        _T("�t��i�H�X�{�b���N��m�A���G��u��X�{�b�C�@�檺�U�b�����C\r\n") +
        _T("�Y���ձN�G��ø�s��@�檺�W�b���A��N�|�۰����ܬ��t��C\r\n") +
        _T("�z�i�H�q�k�����զ�L����C��A�άO�ηƹ��k��q��Ц�m�D��C\r\n") +
        _T("�b���\��U�A���Ц��ø�ϰϰ줤�ɡA�|�b���m��ܯ������ءC\r\n") +
        _T("��Ω��ܹ��ANSI���e���A�u�e�v���Q��J����m�C");
    g_all_usage_data[IC_USAGE_DRAW][1].image_resource_id = IDB_USAGE_DRAW_SQUARE;

    g_all_usage_data[IC_USAGE_DRAW][2].description =
        std_tstring(_T("�u��J���Φr���v���@���������\��A�i�b��Ц�m�B��J���Φr���C\r\n")) +
        _T("�b���\��U����C�⪺�覡�������P�A�|�P�ɨϥΨ�e���P�I����C\r\n") +
        _T("�b�զ�L���A�H�ƹ������ܫe����A�ƹ��k���ܭI����C\r\n") +
        _T("�bø�ϰϰ줺�A�P�˥i�H�ηƹ��k��q��Ц�m�D���C��C\r\n") +
        _T("���ɡA���Ӧ�m���r�����ݪ��e����P�I����|�P�ɳQ�@����ܡC\r\n") +
        _T("�զ�L�U�誺�C��i��ܱ���J���r���A�Q��ܪ̱N�H�����ؼХܡC\r\n") +
        _T("�t���@���i�����r���C�������A�������N��U�@�������C");
    g_all_usage_data[IC_USAGE_DRAW][2].image_resource_id = IDB_USAGE_DRAW_BLOCK;

    g_all_usage_data[IC_USAGE_DRAW][3].description =
        std_tstring(_T("�즸�ϥΥ��{���ɡA�r���C���]�t�Fø�ϱ`�Ϊ��X��Ϊ��C\r\n")) +
        _T("�ϥΪ̤]�i�q�C��W�誺���ӿ�ܦC������A�H��J��L�������r���C\r\n") +
        _T("�o�ǦC�����C�Ӧr�����i�ϥΡu��J��r�v�\��ӿ�J�A\r\n") +
        _T("�M�Ӧ]������`�ΡA�G�S�O�����C�X�A��K�ϥΪֳ̧t�D��C\r\n") +
        _T("�k�����u�e���v�u�᭶�v���s�h�i���ܿ�椤���e�@�ӡ���@�ӦC��C\r\n") 
        _T("��J���O�۪�����ܤ��P�����r���]�p���B���^�ɸ�����K�C\r\n") +
        _T("�b�u��J���Φr���v�\��U�A�]�i�μ���Shift��Ctrl�����e�����᭶�C");
    g_all_usage_data[IC_USAGE_DRAW][3].image_resource_id = IDB_USAGE_DRAW_BLOCK_SELECT;

    g_all_usage_data[IC_USAGE_DRAW][4].description =
        std_tstring(_T("���Τ��e���}�a�쥻��ANSI���e�A�b��J�ɶ��S�O�`�N�C\r\n")) +
        _T("��Ц�m�B��ܪ��������ءA�Ω󴣥ܷ|�Q��ʨ쪺�r���C\r\n") +
        _T("���d�Ҥ��A�Ҷ�J�����A�|�P�ɧ�ʨ쥪�k��ӥ��Φr���C\r\n") +
        _T("���U�ƹ������A�s�����Τ��|�Q�j���J�ܴ�Ц�m�C\r\n") +
        _T("�ª��T���Φr���N�����A���k�����Ŷ��h�Q�b�ΪŮ�Ҩ��N�C\r\n") +
        _T("�b���Ů檺�C��|�۰ʿ�̱ܳ���̡A�����|�ϭ쥻���e���ͧ��ܡC\r\n") +
        _T("�b��L�i��}�a���e��ø�ϥ\�त�A�]�㦳�P�˪����ܻP��ʾ���C");
    g_all_usage_data[IC_USAGE_DRAW][4].image_resource_id = IDB_USAGE_DRAW_BLOCK_OVERLAP;

    g_all_usage_data[IC_USAGE_DRAW][5].description =
        std_tstring(_T("�b�@�몺��r��J�������A�i�H�ϥΪť���PDel��Ӥ������ʤ��e�C\r\n")) +
        _T("���{���Hø�ϪO���Ҧ��]�p�A�]���H�u���J���R���ťաv�i��o�ǰʧ@�C\r\n") +
        _T("�b��Ц�m�B�A�H�ƹ����䴡�J�ťաAShift+����R���ťաC\r\n") +
        _T("�ťժ��C���ܤ覡�P�u��J�b�Ϊťաv�ۦP�A�]�i�ηƹ��k���ܡC\r\n") +
        _T("��Х��������e���|�ܤơA�k�������e�h�|���k�Ω����ưʡC\r\n") +
        _T("�Y��Ц����Φr���������A���ʧ@�N�}�a�ӥ��Φr���C\r\n") +
        _T("�t�~�A���J�ťծɡA���k���ưʶW�X��t�����e�N�|�����C");
    g_all_usage_data[IC_USAGE_DRAW][5].image_resource_id = IDB_USAGE_DRAW_INSERT_DELETE;

    g_all_usage_data[IC_USAGE_DRAW][6].description =
        std_tstring(_T("���ɭԡA�z�|�ݭn�[��S�w��m���r�����c�A�H��Ҧp�󲣥ͬ��u�ĪG�C\r\n")) +
        _T("�u�˵����e�v�i�H��ܴ�Ц�m�����ANSI�r���P�C��]�w�C\r\n") +
        _T("�b���\��U�A���F����N�~�A�I���ƹ����|���ܥ��󤺮e�P�C��C\r\n") +
        _T("��Ц�m���������ئP�˥ΨӪ�ܦr�����d��A�����ثh����˵����b�ΰϰ�C�]�Y�Ӧr�����b�ΡA�������رN�P�����جۦP�C�^\r\n") +
        _T("�P�ɡA��Ъ���|�X�{�@����ΰϰ�A��ܦr�����e�P�e�����I����C\r\n") +
        _T("�ϰ줺���Ʀr�N��BBS�C��X�A�p���d�Ҥ����C��۷��u*[0;37;46m�v�C");
    g_all_usage_data[IC_USAGE_DRAW][6].image_resource_id = IDB_USAGE_DRAW_VIEW;

    g_all_usage_data[IC_USAGE_TEXT].resize(4);

    g_all_usage_data[IC_USAGE_TEXT][0].description =
        std_tstring(_T("�u��J��r�v�]�����^�i�ΨӦb���w��m��J���N��r�C\r\n")) +
        _T("���{���H�����p�e�a���覡��J��r�A�ӫDBBS������r�s�褶���C\r\n") +
        _T("��r�|���N�Ҧb��m���쥻�r���A��r�k�������e�h���|���ʡC\r\n") +
        _T("�ϥΦ��\��ɡA�������q�զ�L����ܷQ�n�ϥΪ���r�C��C\r\n") +
        _T("���в���ø�ϰϰ줤�ɡA�|�X�{���ܰT���A��ܿ�J���_�l�r����m�C\r\n") +
        _T("���ɤ@�������ϰ�N�|�ܷt�H�Y�㴣�ܡA����ڤ��e�ä��|���ͧ��ܡC\r\n") +
        _T("��T�w��m��A�I�U�ƹ�����N���ͤ@�ӥi��J��r���p�������C");
    g_all_usage_data[IC_USAGE_TEXT][0].image_resource_id = IDB_USAGE_TEXT_INTRO;

    g_all_usage_data[IC_USAGE_TEXT][1].description =
        std_tstring(_T("�b�զ⪺��J�ϰ줤�A�z�i�H������J�k�ӿ�J��r�A�αq�L�B�ŤU���ƻs���N�r���öK�W�ܿ�J�ϰ�A�����e���w���@��C�C\r\n")) +
        _T("���r�i�J��J�ϰ��A�����|�V�k���i�A��r���N�P�{�b�ϥΪ����Τ��j�p�۲šA�i�̦��[���r�b�쥻���e���N�|�л\���ϰ�C\r\n") +
        _T("�]�Ъ`�N�A��J���e�N�|�Q��m��u��r�v�Ҧb����C�C�^\r\n") +
        _T("�H�ƹ��I���u�T�w�v���s�Ϋ��UEnter��ɡA�N�|��ڰ����J�ʧ@�C\r\n") +
        _T("�Y������J�A���I���u�����v���s�Ϋ��UEsc��C");
    g_all_usage_data[IC_USAGE_TEXT][1].image_resource_id = IDB_USAGE_TEXT_INPUT;

    g_all_usage_data[IC_USAGE_TEXT][2].description =
        std_tstring(_T("��J����r�N�ĥΥثe��ܪ��C��A���m���I����h�P�쥻���e�۲šC\r\n")) +
         _T("�M�ӡA�Y�ҿ�ܪ��C��P�쥻���I���⧹���ۦP�A��r�C��N�۰��ର�¦�Υզ�]�̭I����M�w���A�X�̡^�A�H�����ܱo�i���C\r\n") +
         _T("���]�p�Ω��קK�]����C��A�����H�b���e������r��m�����p�C\r\n") +
         _T("�b��J��r��A���i�Ρu�����C��v�\��վ��r�ϰ쪺�e���P�I����C\r\n") +
         _T("�аѦҥ����u�C��]�w�v�ؿ��������e�A�H�o���ϥΡu�����C��v�\��վ��r�C�⪺��k�A�H�����`�N���ƶ��C");
    g_all_usage_data[IC_USAGE_TEXT][2].image_resource_id = IDB_USAGE_TEXT_COLOR;

    g_all_usage_data[IC_USAGE_TEXT][3].description =
        std_tstring(_T("���\��i�H�Ψӿ�J�`��BBS�����]�pPCMan�^�i��ܪ���r�P�Ÿ��C\r\n")) +
         _T("�M�ӡA�����\�h�r���L�k�bBBS����ܡA��h�b�����֨�������r�C\r\n") +
         _T("���M�o�Ǧr���j��������b��J��r����������ܡA���b�x�s.ans�ɮɱN�L�k�ഫ���X�G�榡�����e�C�bBBS�������]�L�k�N��K�W�ܤ峹���C\r\n") +
         _T("�b�T�w�i���J��r���ʧ@�ɡA�p�G��J�ϰ줤�]�t�F�����r���A�h�|�X�{�@ĵ�i�����A�ñN�o�Ǧr���C�X�C") +
         _T("����J����r���A���Ħr�����������M�|�Q���`��J�A���L�Ħr������m�N�U�۳Q��ӥb�ΪťթҴ����C");
    g_all_usage_data[IC_USAGE_TEXT][3].image_resource_id = IDB_USAGE_TEXT_INVALID_CHAR;

    g_all_usage_data[IC_USAGE_BRUSH].resize(4);

    g_all_usage_data[IC_USAGE_BRUSH][0].description =
        std_tstring(_T("�ϥΥb�Ϊťն�J�j�d��ϰ�ɡA���M�i�H�즲�ƹ��s���J�h�Ӧ�m�A����󭱿n���j���ϰ�A�b�ާ@�W���M�۷��K�C\r\n")) +
        _T("�u�j������v�]����ϡ^���@�������ۦ�\��A���ѼƺدS�w�Ϊ��B�i�վ�ؤo������A�H�ֳt�B�z�j�d��ϰ�A�β��ͯS��Ϊ����ϼˡC\r\n") +
        _T("�ѩ�ANSI�r�����e�P�C�⪺�ѥͭ���A�j������H�b�μe�ת��p����欰�@�e�W���̤p���A�åB�u��ϥΫD�G�⪺�C��C\r\n") +
        _T("���Ϥ���C�ص���Ϊ��U�|�@�ҡA�èϥΤ��P���C��P�ؤo�C");
    g_all_usage_data[IC_USAGE_BRUSH][0].image_resource_id = IDB_USAGE_BRUSH_INTRO;

    g_all_usage_data[IC_USAGE_BRUSH][1].description =
        std_tstring(_T("�b�k�誺�\���椤�A�j����������U�ݪ������C\r\n")) +
        _T("��ﶵ�U�ݪ�������ܥثe������Ϊ��P�ؤo�A�åi�ѨϥΪ̽վ�C\r\n") +
        _T("���в���ø�ϰϰ줤�ɡA���ܰT���N��ܵ��ꪺ�Ϊ��A�P�������ANSI�r������ڦ�m�C�o��̪���ӵ{�׬Ҩ���p����浲�c������C\r\n") +
        _T("��λP�٧Ϊ�����Ѥp�����զ��̱��񪺧Ϊ��A��u�P���u����h�����׬����w�ؤo�A�ʫ׵��P�b�ΪŮ�e���Ӫ����ΡC\r\n") +
        _T("�ާ@�W�i��Ctrl�PShift������e����@�ص���A�Υηƹ��u�����ܤؤo�C");
    g_all_usage_data[IC_USAGE_BRUSH][1].image_resource_id = IDB_USAGE_BRUSH_SHAPE;

    g_all_usage_data[IC_USAGE_BRUSH][2].description =
        std_tstring(_T("�b��J���e���ާ@�W�A�j������P�u��J�b�Ϊťաv�����@�P�C\r\n")) +
        _T("�t�O�Ȧb�j������㦳���j���ؤo�P�S�w���Ϊ��A�u��J�b�Ϊťաv�h�ϥγ�@�b�Ϊťժ��d�򰵬���u����v�C\r\n") +
        _T("�����ƹ�����ɡA�N�̩ҿ��C��P����Ϊ����ܩҼХܪ��d��C\r\n") +
        _T("�Y����ƹ�����é즲�A�h�|�u�ۭy����ܵ���Ϊ��ҳq�L���ϰ�C\r\n") +
        _T("������ܪ��C��ɡA���F��զ�L�B�ާ@�A�]�i�b�{�����e�W�I���ƹ��k��H��ܸӳB���C��A����ܫG��ɱN�|���ܬ��t��C");
    g_all_usage_data[IC_USAGE_BRUSH][2].image_resource_id = IDB_USAGE_BRUSH_OPERATION;

    g_all_usage_data[IC_USAGE_BRUSH][3].description =
        std_tstring(_T("�j�����ꪺ��t�H�b�ΪťթΤp�����զX�Ӧ��A�e���P�즳�����Φr���Ĭ�C��J���e�ɡA���{���|�۰ʽվ�r���A�H���ͳ̤p�{�ת��}�a�C\r\n")) +
        _T("�p�G�쥻���e�]�O�b�ΪťջP�p����檺�զX�]�q�`�]�ѵ���Ҳ��͡^�A�K�৹��O�s����~�����e�C��L���Φr���h���e������v�T�C\r\n") +
        _T("���d�Ҥ��A�s��J����α�Ĳ�F�쥻���٧λP�T���α���C") +
        _T("�٧ΤW���@�Ӥp�����]�i�J��ε���d��ӳQ�אּ����A����L���e�����S���ܤơC") +
        _T("���䳡���h���F�t�X�����t�����c�A�Ӧ���ӤT���Φr���D��}�a�C");
    g_all_usage_data[IC_USAGE_BRUSH][3].image_resource_id = IDB_USAGE_BRUSH_CONFLICT;

    g_all_usage_data[IC_USAGE_COLOR].resize(9);

    g_all_usage_data[IC_USAGE_COLOR][0].description =
        std_tstring(_T("���{�����A����إD�n�覡�i�H���ܥثe�ϥΪ��C��G\r\n")) +
        _T("(1)�q�k�����զ�L���A�H�ƹ������ܫe����A�k���ܭI����C\r\n") +
        _T("(2)�b����ø�ϥ\��U�A�H�ƹ��k��qø�ϰϰ줤����Ц�m�D���C��C\r\n") +
        _T("�b�@��BBS�������A�i�H�w��C�Ӧr�����O�վ��e�����I����C\r\n") +
        _T("�M�ӡA�󥻵{�����A�\�h��ø�ϥ\��b�ާ@�ɥu�ݭn��ܳ�@�C��C�ھڹ�ڨϥΪ�ANSI�r�������A���C��i��|�H�e�����I���⪺�覡�e�{�C\r\n") +
        _T("���F���ɾާ@�Ĳv�A�b���P��ø�ϥ\��U�D���C��ɡA�����N�������P�C");
    g_all_usage_data[IC_USAGE_COLOR][0].image_resource_id = IDB_USAGE_COLOR_INTRO;

    g_all_usage_data[IC_USAGE_COLOR][1].description =
        std_tstring(_T("�b�u��J�b�Ϊťաv�B�u��J�p�����v�B�u���J���R���ťաv�B\r\n")) +
        _T("�u�j������v�P�u��J��r�v�\��U�A�L�צp�����C��A�ҷ|�P�ɧ��ܫe���P�I����C") +
        _T("��ܪ��C�⬰�G��ɡA�I���⤴�|�ഫ���������t��C\r\n") +
        _T("�u��J�p�����v�P�u��J��r�v�i�H�ϥΫG��A��L�@�ߨϥηt��C\r\n") +
        _T("�b�u��J���Φr���v�\��U�A�i�q�զ�L���H�ƹ�������k����O��ܫe�����I����A") +
        _T("�ΥH�ƹ��k��qø�ϰϰ줤�@���D��ӳB���e�����I����C\r\n") +
        _T("�b�y�ᤶ�Ъ��u�����C��v�\�त�A�������N�ھڦr�����e�Ӳ����ܤơC");
    g_all_usage_data[IC_USAGE_COLOR][1].image_resource_id = IDB_USAGE_COLOR_PICK;

    g_all_usage_data[IC_USAGE_COLOR][2].description =
        std_tstring(_T("�u�����C��v�Ω��ܧ�ANSI�r���������϶����C��A�Ӥ����ܦr�����e�C\r\n")) +
        _T("�b���\��U�A�N��в��ܥ��@�r���W�ɡA�|�H����������ܦr��������d��A�åH����Ϊ���ܷ|���ͧ��ܪ��ؼа϶��C\r\n") +
        _T("�Y�Ӧr�����b�ΪťաA�άO�ƺدS�w���X��Ϊ����Φr�����@�ɡA\r\n") +
        _T("�h����Ϊ����b�b�Φr���e�פ��A�P��Щҫ��B�C��ۦP���ϰ줧��ɡC\r\n") +
        _T("�I�U�ƹ������A����Ϊ������ϰ�N�|�Q�אּ�ثe�ҿ�ܪ��C��C\r\n") +
        _T("�Y�H�ƹ��k��q�o�Ǧr������C��A�����N�P�e�z���u����ܡv�ۦP�C");
    g_all_usage_data[IC_USAGE_COLOR][2].image_resource_id = IDB_USAGE_COLOR_CHANGE;

    g_all_usage_data[IC_USAGE_COLOR][3].description =
        std_tstring(_T("�u�����C��v�]�i�H�Ψӧ��ܤ�r�������e���C��C\r\n")) +
        _T("���ɨ�B�@�覡�P�bBBS���վ��X�ۦP�A�������w�e�����I����C\r\n") +
        _T("����Ϊ��N�e���b�Φr���e�סA�N���ӽd���C�ⳣ�|���ܡC\r\n") +
        _T("�b�u�����C��v�\��U�A�D���C�⪺�覡�N�����������ܤơG\r\n") +
        _T("(1)�b�k���զ�L���A�i�ηƹ�������k���ܤ��P���e�����I����C\r\n")
        _T("(2)�bø�ϰϰ줺�H�ƹ��k��D���C��ɡA���ӳB�r�������өw�C�Y���@���r�A�h�@���D���e�����I����A�Ϥ��h�ھڴ�Ц�m�D����C");
    g_all_usage_data[IC_USAGE_COLOR][3].image_resource_id = IDB_USAGE_COLOR_TEXT;

    g_all_usage_data[IC_USAGE_COLOR][4].description =
        std_tstring(_T("�b�`�Ϊ��X��Ϊ����Φr�����A���T���Φr���㦳�S�O���~�[�P�S�ʡC\r\n")) +
        _T("��i�ΨӪ�{�צV�q�L�b�ΰϰ쪺�﨤�u�A�H�Φ��S�w���ɱ���t�C\r\n") +
        _T("�o�P�r������ڤ��e�ä������۲šA�]���|���ͻ~�t�P�Ӫ������ŻءC\r\n") +
        _T("ø�ϰϰ줺�|��ܥ��T���Φr������ڥ~�ΡA��ø�Ϫ̧P�_�o�ǻ~�t�C\r\n") +
        _T("�M�ӡAø�ϥ\�त�����ܰT���h�|�N�r�����e�����I���ϰ짡��ܬ����Υb�ΰϰ쪺�����T���ΡC�b�����C��ɡA�i�H�ȧ����I�諸�����C\r\n") +
        _T("�Ϥ����ƺ����Τ覡�A�U�ݳ�����ܤF��ڤ��e�P���ܰϰ쪺�t���C");
    g_all_usage_data[IC_USAGE_COLOR][4].image_resource_id = IDB_USAGE_COLOR_REGULAR_TRIANGLE;

    g_all_usage_data[IC_USAGE_COLOR][5].description =
        std_tstring(_T("�u�����C��v�\�त�t���@�u�d��Ҧ��v�ﶵ�A��B�@�覡�P�p�e�a�����u�d����v�u��ۦ��A�i�H�q���w��m�@���N�۾F���P��ϰ�W��C\r\n")) + 
        _T("�b�ΪťաB�������������説���B�P�����T���Φr���A�b�ϰ춡���{�P�_���P�_�W�A�Pø�ϰϰ���ܪ��~�[�����۲šC") + 
        _T("���T���Φr����������t�h�����P���{�ϰ챵Ĳ�A�Y�Ϲ�ڤW�����㦳�I���C�⪺�������e�C\r\n") + 
        _T("��l�r���]�D�n�O�@���r�^�h�����Q�I����񺡾�Ӱϰ�C\r\n") + 
        _T("���{���ϰ쥲���b�����Ϋ�����V�W�s���A�������I���ä��������{�C"); 
    g_all_usage_data[IC_USAGE_COLOR][5].image_resource_id = IDB_USAGE_COLOR_AREA_CONNECT;

    g_all_usage_data[IC_USAGE_COLOR][6].description =
        std_tstring(_T("�u�d��Ҧ��v�i�q�k����椤�Ŀ�A�b�u�����C��v�\��U�]��H��L��Ctrl������@��Ҧ��P�d��Ҧ��C")) +
        _T("�b���Ҧ��U�A���ܰT�����|�X�{�¦V�~�����b�Y�A�����ϥΪ̦��ާ@�i��|���ܤj�d��ϰ쪺�C��C\r\n") + 
        _T("���U�ƹ���A�Ҧ��۾F���P��ϰ�ҷ|�Q���ܦ��ثe��ܪ��e����C\r\n") + 
        _T("�ѩ�G��bANSI��������A���\��ĥΪ��C��|���ܬ��@���C��C\r\n") + 
        _T("����r���D�Ů�δX��Ϊ���������e�A��e����|�O�����ܡA�ȭI����|�Q���ܡC�Y�C��۲šA�۾F�ϰ�i�H�q�L��r�����s����t�@�ݡC");
    g_all_usage_data[IC_USAGE_COLOR][6].image_resource_id = IDB_USAGE_COLOR_AREA_CHANGE;

    g_all_usage_data[IC_USAGE_COLOR][7].description =
        std_tstring(_T("��ϥΪ̥H�d��Ҧ������C��ɡA�p�G���۾F�ϰ�d�򪺧P�_�����~�A�K�i��b�ާ@���ʨ�w���H�~�������C\r\n")) +
        _T("���F�קK�o�ت��p�A���Ҧ��㦳�w���\��C�N��Щ�b�Q�n��ʪ���m�����Shift�䤣��A�K�i�bø�ϰϰ줤�ݨ��ʺA�w�����G�C") +
        _T("�P��Ц�m�۾F���P��ϰ�A���C��|�b�쥻�C��P�ثe��ܪ��C�⤧���Ӧ^�ܤơC\r\n") + 
        _T("�w������ܪ��ܤƨä��|���ܥ���ANSI���e�A�ȬO���F���ϥΪ̽T�{�۾F�ϰ쪺���T�d��A�H�M�w�O�_�n��ڶi���ʡC");
    g_all_usage_data[IC_USAGE_COLOR][7].image_resource_id = IDB_USAGE_COLOR_AREA_HINT;

    g_all_usage_data[IC_USAGE_COLOR][8].description =
        std_tstring(_T("�u�˵����e�v�B�u��t�վ�v�P�u�X�֥��Τ��v�o�ǥ\�઺�ĪG�P�ثe��ܪ��C��L���A�]�ëD�ΨӦbø�ϰϰ�W�ۦ�C\r\n")) +
        _T("��b�o�ǥ\��U�q�զ�L����C��ɡA���{���N�|��ø�ϥ\������ܨ�L�u�ݭn����C��v���\�त�A�̫�ϥιL���@�ءC\r\n") + 
        _T("�p�ϩҥܡA�b�u��t�վ�v�\��U��ܺ��ɡAø�ϥ\��]�|�۰ʧ��ܡC\r\n") + 
        _T("���]�p�i���ϥΪ̦b�i��@�ǽվ��A�b����C�⪺�P�ɧֳt�����ܱ`�Ϊ��ۦ�\����~��ø�ϡA�Ӥ��ݭn��ʧ���ø�ϥ\��C");
    g_all_usage_data[IC_USAGE_COLOR][8].image_resource_id = IDB_USAGE_COLOR_MODE_SWITCH;

    g_all_usage_data[IC_USAGE_BOUNDARY].resize(12);

    g_all_usage_data[IC_USAGE_BOUNDARY][0].description =
        std_tstring(_T("�u��t�վ�v�O���{���̭��n���\��A�Ψӽվ�ANSI���e�����C����ɡC\r\n")) +
        _T("�f�tø�ϰϰ줤���ѦҹϤ��A�i�H�ֳt���ͬۦ������u�ϮסC\r\n") +
        _T("���\��㦳��ؤ��P���Ҧ��A�u�@��Ҧ��v�Ω�վ��V�M�a�V����t�A�u�T���μҦ��v�h�M�`��T���Φr���i���ͪ��ܤơC\r\n") +
        _T("�Ŀ�Ψ����k���椤���u�T���μҦ��v�ﶵ�A�i��ܱ��ϥΪ��Ҧ��C\r\n") +
        _T("�b�u��t�վ�v�\��U�A�]�i�ϥ�Ctrl����ؼҦ����������C\r\n") +
        _T("�����`�N���Ф@��Ҧ��������A�T���μҦ��h��U�@���`���СC");
    g_all_usage_data[IC_USAGE_BOUNDARY][0].image_resource_id = IDB_USAGE_BOUNDARY_INTRO;

    g_all_usage_data[IC_USAGE_BOUNDARY][1].description =
        std_tstring(_T("�b�ϥΰ򥻪����\��A�ھڰѦҹϤ������e�i��²�檺�ۦ��A")) +
        _T("���P�C�⪺�ϰ춡�|���Ͳʲ�����ɡA���P�Ϥ����u�ꤺ�e�����۲šC\r\n") +
        _T("�p�ϡA�Ŧ�P��⪺�b�Ϊťե�ɡA�P�ѦҹϤ������e������t���C\r\n") +
        _T("�u��t�վ�v�i��²�檺�ާ@�ק�o����ɡA�Ϩ�P�ѦҹϤ���[�ۦ��C\r\n") +
        _T("���в��ܥ�ɳB����ɡA�N�ھڲ{�b��ANSI���e�P�_�O�_�i�i��վ�C\r\n") +
        _T("���\�վ�ɡA�H���⥿�諬��ت�ܽվ�ɷ|��ʨ쪺���Τ���d��A\r\n") +
        _T("����u���h�N��N�Q�վ㪺��ɡA��i����V���a�V�C");
    g_all_usage_data[IC_USAGE_BOUNDARY][1].image_resource_id = IDB_USAGE_BOUNDARY_ADJUST_START;

    g_all_usage_data[IC_USAGE_BOUNDARY][2].description =
        std_tstring(_T("������ػP����u���X�{�ɡA�I�U�ƹ�����Y�|����ANSI���e�C\r\n")) +
        _T("�z�i�H�����b�ؼЦ�m�I���ƹ��A�άO�g�ѩ즲�ӷL�զܲz�Q��m�C\r\n") +
        _T("��}�ƹ���A��V���a�V�����説����N�Q��J��ةҦb��m�A") +
        _T("���C��h�̩P�򤺮e�ӨM�w�C�z�i���ʨ��L�ݭn�վ㪺�ϰ�A�i���������ާ@�C\r\n") +
        _T("���w�s�b�����説���A�]�i�H�ɥΦ��\�ಾ�ʨ䤺����ɪ���m�C\r\n") + 
        _T("�p����e�վ�L�����e�����N�A�i���ƶi��h���ק�C\r\n") +
        _T("���d�Ҥ��A�š������ɦb�վ��A�Y�P�ѦҹϤ������e�j�P�ۦ��C");
    g_all_usage_data[IC_USAGE_BOUNDARY][2].image_resource_id = IDB_USAGE_BOUNDARY_ADJUST_ACTION;

    g_all_usage_data[IC_USAGE_BOUNDARY][3].description =
        std_tstring(_T("�b�@�ǳ��X���A�i�վ㪺��t�Ψ䲾�ʤ�V�|���ƺؤ��P�i��ʡC\r\n")) +
        _T("�b���U�ƹ����䤧�e�A�i�������ʴ�СA�i�H�[�����i�檺�U�ؽվ�C\r\n") +
        _T("�����ػP����u���|�H��Ц�m�ܤơA�H��ܱ��i�檺�վ������G\r\n") +
        _T("(1)�P�ɥi�V�W�ΦV�U�վ�ɡA�̴�Ц�m����V�өw�C\r\n") +
        _T("(2)�P�ɥi�վ��V�P�a�V��t�ɡA�H�������Ъ̬��u���C\r\n") +
        _T("(3)�վ�s��b�Ϊťդ������a�V��t�ɡA�ھڴ�оa���������k���{�סA�i�H�b�T�ؤ��P����m��J���Τ��C");
    g_all_usage_data[IC_USAGE_BOUNDARY][3].image_resource_id = IDB_USAGE_BOUNDARY_ADJUST_MULTIPLE;

    g_all_usage_data[IC_USAGE_BOUNDARY][4].description =
        std_tstring(_T("�]���@�ǩ����ۦ������X�A�|�]ANSI���e�Ӽv�T�վ㪺�i��ʡG\r\n")) +
        _T("(1)���Y�ǥ��Τ��A�ȯ�N��㦳�@�P�C�⪺��t�V�~�����C\r\n") +
        _T("(2)�G��u��X�{�bANSI�r�������e���ϰ�C����Τ�檺��t���G��ɡA�u�����W��M�k������t�i�H���ʡC\r\n") +
        _T("(3)��P��w����L���Φr���A�Ѿl�Ŷ��L�k��J���Τ��ɡA�L�k�վ�C") +
        _T("�Y���Φr���b�@��椺���C�⧹���ۦP�]���ئr���q�`���ͦۥ��e���վ�ʧ@�^�A�h�|�Q������ӥb�ΪŮ�A�åi��]�����\�P��ϰ쪺�վ�C");
    g_all_usage_data[IC_USAGE_BOUNDARY][4].image_resource_id = IDB_USAGE_BOUNDARY_ADJUST_VALIDATION;

    g_all_usage_data[IC_USAGE_BOUNDARY][5].description =
        std_tstring(_T("�@�ظ��������A���b���u�Ϯפ��`�Ϊ��զ��A�]�i�����Φ��\��վ�C\r\n")) +
        _T("�p�ϡA�H�b�ΪŮ�c�����׽u�A�Q�ΨӤ��j�ⰼ���j�d�򪺯¦�ϰ�C\r\n") +
        _T("�N��в���׽u��������ɳB�A�õy�L��������������ɮɡA�i�i��W�U�վ�C�]�Y���񤤥��A�h�|�אּ�襤����ɶi�楪�k�վ�C�^\r\n") +
        _T("���ʴ�Юɷ|���J�C��۲Ū����Τ��A�������ܭ쥻�׽u����m�C\r\n") +
        _T("���\��i�ΨӲ��Ͷɱ׫צ��ܤƪ��׽u�A�H��[�ŦX�ѦҹϤ������e�C\r\n") +
        _T("�b��ڧ@�e���A�׽u���i����@�C��A�ⰼ�ϰ쪺�C��]�i�H�ۦP�C");
    g_all_usage_data[IC_USAGE_BOUNDARY][5].image_resource_id = IDB_USAGE_BOUNDARY_SHEAR_LINE;

    g_all_usage_data[IC_USAGE_BOUNDARY][6].description =
        std_tstring(_T("��Q�n�վ㪺�Z�������ɡA�i�H����ƹ��é즲�����Z���A\r\n")) +
        _T("����t���ʶW�L�@�ӥ��Τ��A�Ӥ��ݭn�h���վ�䶡���C�Ӥ��C\r\n") +
        _T("��ЩҦb���ϰ�N�|��J�A�����Φr���A����ɲŦX��Ъ���m�C\r\n") +
        _T("�Ҹg�L���ϰ줧���e�A�h�|�۰��ഫ���C��۲Ū��b�ΪťթΥ��Τ��C\r\n") +
        _T("�]�G��ɥ����ϥΥ��Τ��~����ܡA�t��h�|�ϥγ�ª��ťաC�^\r\n") +
        _T("�M�ӡA���\��Ȧb��l��ɪ��ⰼ����ª�����C��ɤ~�i�ϥΡC\r\n") +
        _T("�C��զ��������ɡA�N�L�k�즲�W�L�@�Ӥ��A�H�קK�}�a�쥻���e�C");
    g_all_usage_data[IC_USAGE_BOUNDARY][6].image_resource_id = IDB_USAGE_BOUNDARY_EXTENDED_ADJUST;

    g_all_usage_data[IC_USAGE_BOUNDARY][7].description =
        std_tstring(_T("�b���u�Ϯת��ӳ��A�`�`�|�ݭn�Ȱw��b�Φr�����d�򰵭ק�C\r\n")) +
        _T("�M�Ӱ򥻪���t�վ�|���ܨ��ӥ��Φr���A�L�k���������ާ@�C\r\n") +
        _T("�վ������t�ɡA�i�H����Shift��������u�����վ�v�Ҧ��C\r\n") +
        _T("����ø�ϰϰ줤���|�������������ܤ��e�A��������t�N�Q������q�G\r\n") +
        _T("���G������u����ܱN�Q�վ㪺��t�A�t����u���h�|�O���T�w�C\r\n") +
        _T("���ɭY���U�Ω즲�ƹ��A�N�u���G����u�����ݪ��b�ΰϰ첣�ͧ��ܡC\r\n") +
        _T("���\��ȾA�Ω������t���W�U�վ�A������t�L�k���ͦ����ܤơC");
    g_all_usage_data[IC_USAGE_BOUNDARY][7].image_resource_id = IDB_USAGE_BOUNDARY_PARTIAL_ADJUST;

    g_all_usage_data[IC_USAGE_BOUNDARY][8].description =
        std_tstring(_T("�e�Ҥ��A�P�@�Ӧ�m�i�H�i��򥻪����νd��վ�A�άO�����վ�C\r\n")) +
        _T("�t���@�ǳ��X�A�]�C��ΧΪ��զ����������A�L�k�i��򥻽վ�C\r\n") +
        _T("�b����Shift��A�~�|�X�{��ػP�u���A�H���ܯ���i�檺�����վ�C\r\n") +
        _T("�����p�q�`�X�{�b����������ά�_�B�A�]��㦳���u��������t�C\r\n") +
        _T("��ڧ@�e�ɡA�i�H������Shift�A�A���ʴ�ШӴM��i�H�վ㪺�ϰ�C\r\n") +
        _T("�Y�b�@�Ӱϰ���񦳼ƺؤ��P�i�઺�վ�覡�A�h�P�˷|�ھڴ�Ц�m�M�w�̾A�X�̡A�åH���ܤ��e�Ӫ�ܱN�n�i�檺�վ���P��t�C");
    g_all_usage_data[IC_USAGE_BOUNDARY][8].image_resource_id = IDB_USAGE_BOUNDARY_PARTIAL_ONLY;

    g_all_usage_data[IC_USAGE_BOUNDARY][9].description =
        std_tstring(_T("�����վ�]�i�H�������Φb�������説���W�A�����@�ǭ���G\r\n")) +
        _T("��@���Τ�檺������ɵL�k�]�m�󤣦P���סA�@������ɥu�ಾ�ʦܤ�檺�̤W�B�̤U�ݡB�λP�t�@������ɵ����C\r\n") +
        _T("�o�Ӱʧ@�]�i�H�����ϥΡu�����C��v�\��ӹF���C���@�]�p�O���F�٥h�����\�઺�ʧ@�A�A�X�b�s��վ�h�B��t�ɧQ�ΡC\r\n") +
        _T("�Y�r�����@���w�g���¦�A�t�@���Y�i��������a�i��վ�C\r\n") + 
        _T("�����p�U�@��վ�P�����վ㪺�ĪG�ۦP�A�¦ⳡ�����|�O�����ܡC");
    g_all_usage_data[IC_USAGE_BOUNDARY][9].image_resource_id = IDB_USAGE_BOUNDARY_PARTIAL_RESTRICT;

    g_all_usage_data[IC_USAGE_BOUNDARY][10].description =
        std_tstring(_T("��t�վ�i�H��{�\�h�\��A�]�Ϩ�ާ@�W�e�����V�c���B�C\r\n")) +
        _T("��Ъ���e�{�����ܤ��e�A�Y�O���F���ϥΪֳ̧t�z�ѻP���X�P�_�C\r\n") +
        _T("�b���|�@�����������`���רҡG�����ϰ즳��۾F���b�ΪťաA\r\n") +
        _T("��ⰼ�ҳQ���Τ��Ҧ��ڡA���Ҩ㦳�C��@�P��������t�C\r\n") +
        _T("�b���U�ƹ��e�A�ǥѲ��ʴ�Ц�m�A�b�����B���T�ؽվ�i��ܡC\r\n") +
        _T("���M�i�檺�ʧ@���O�b�ۦP��m���J���Τ��A���ϥΪ��C��|�ھکP�򤺮e�өw�A�H��{���ʤ��P��t���ؼСC");
    g_all_usage_data[IC_USAGE_BOUNDARY][10].image_resource_id = IDB_USAGE_BOUNDARY_ADJUST_MULTIPLE_EXAMPLE;

    g_all_usage_data[IC_USAGE_BOUNDARY][11].description =
        std_tstring(_T("�b�Y�ǾA�����X�U�A��t�վ�i�H�P�ɧ��ܥ��Φr�������c�C\r\n")) +
        _T("���Ϥ��A����_�������@�������説���A�䥪���Q�]���º��C\r\n") +
        _T("�������i��վ�ɡA�|�b�����O���¦⪺���A�U���ʥk����t�C\r\n") +
        _T("�Y�b���U�ƹ��e�A�N��в������k�����ʡA�����ؤ]�|���k�ưʥb��C\r\n") +
        _T("���{�H��ܶ����ذϰ�i�Q�����@���Τ��A�ù��i��վ�C\r\n") + 
        _T("�b�T�w����վ��A�����楪�����ϰ�|�ର��⪺�b�ΪťաC\r\n") +
        _T("���ʧ@���B�@��z�A�b�u�X�֤��v���`�������Ժɪ������C");
    g_all_usage_data[IC_USAGE_BOUNDARY][11].image_resource_id = IDB_USAGE_BOUNDARY_AUTO_MERGE;

    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE].resize(6);

    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][0].description =
        std_tstring(_T("�����`�N�w����t�վ㤤�u�T���μҦ��v�������i�满���C\r\n")) +
        _T("��k����椤�u�T���μҦ��v�Q�Ŀ�ɡA��t�վ㪺�P�_�̾ڱN���ܡC\r\n") +
        _T("��N�Q�Ϊ����T���ΩΥ��T���Φr���A�P�C��]�w�Ӳ��Ͷɱת���ɡC\r\n") +
        _T("�ϥΪ̥i�N�b�ΪŮ涡����ɽվ㬰�T���Ϊ�����A�αN�����ܦ^�������������M������ɡA���L�k��@��Ҧ������`�����説���i��վ�C\r\n") + 
        _T("�Ϥ��W�U�ⳡ����ܤF��ؤT���Φr�����B�ΡA�䤤���T���Ϊ��ϥΥ�������Shift�Ӿާ@�A�P�@��Ҧ��ϥΡu�����վ�v���覡�ۦP�C");
    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][0].image_resource_id = IDB_USAGE_BOUNDARY_TRIANGLE_INTRO;

    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][1].description =
        std_tstring(_T("�T���μҦ����B�@��z���P�@��Ҧ��ۦ��G�ھڭ즳�����e�P�_�վ㪺�i��ʡA�çQ�ξA�X���r���P�e�����I�����{��t�����ʡC\r\n")) +
        _T("�̰򥻪��@���������¦⪺���ΰϰ�Ҳ��ͪ���ɡC�Y���۫���������{��ɦb�t�@���㦳�ۦP���C��A") +
        _T("�h�i���J�@��V�۲Ū������T���ΡA�õ����A���t��H�N�쥻����������ɡu���ʡv�ܱ���C\r\n") +
        _T("������ɪ���ݬҬ��G��ɡA�|�]���I���⪺����ӵL�k�վ�C\r\n") +
        _T("�Y�b�P�@��m���h�إi�઺�վ�覡�A�h�i���ʴ�Шӿ�ܷQ�n�i��̡C");
    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][1].image_resource_id = IDB_USAGE_BOUNDARY_TRIANGLE_BASIC;

    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][2].description =
        std_tstring(_T("�i��վ�ɡA�I���P�즲�ƹ����ާ@�覡��������t�վ㪺�@��Ҧ��C\r\n")) +
        _T("����ƹ������A�i�H�b��������ɡB����B�P�t�@�ݪ���������ɤ����i���ܡC�]�i�H�Q�γs�򪺾ާ@�ӳv��������ɪ���m�C\r\n") +
        _T("�Y�쥻�����e�w�O�Ѫ����T���β��ͪ�����A�B�r���������C��@�P�ɡA�h�L�שP���C��զX����A���i�i��վ�C\r\n") +
        _T("�M�ӡA�Y�b�o�ر��p�U�վ㦨����������ɡA�h�쥻������Y�@�����C��N�|�����C���򪺽վ�N�g�ѷs�����e�Ӷi��P�_�C");
    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][2].image_resource_id = IDB_USAGE_BOUNDARY_TRIANGLE_MOVING;

    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][3].description =
        std_tstring(_T("�b�A���t��U�A�T���Τ�ؤ]�i��{�X�׽u�P�����u���V�X��ɡC\r\n")) +
        _T("���\��]�i�Ω󲣥ͳo�Ǥ��e�A�Ӥ��ݭn��ڳ]�w�ӳ��ϰ쪺�C��C\r\n") +
        _T("���Ϥ��C�X�T�جۦ��������A���l���e�������ۦP�C") +
        _T("�����վ�ʧ@���ͤF�򥻪��T���α���A���M���h�Φ��F��ؤ��P�Ϊ����V�X��ɡC\r\n") +
        _T("�Ϥ������������վ�e����ܪ����ܰT���C�����ؼФ��P���M�����P�A�㦳�b�μe�ת������C���M���ұ��վ㪺��t��m�]�����t���C\r\n") +
        _T("�b���U�ƹ�����e�A�������ʴ�Ъ���m�Y�i�D��Q�n�ĥΪ������C");
    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][3].image_resource_id = IDB_USAGE_BOUNDARY_TRIANGLE_HALF;

    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][4].description =
        std_tstring(_T("���F�����T���Υ~�A���T���Φr���]�`�Q�ιB����u�@�~���C\r\n")) +
        _T("�b�T���μҦ�����t�վ�\�त�A�ާ@�ɭY����Shift��A�K�|�ھڥ��T���Φr�����S�ʶi��վ�A�Ϥ��h�H�����T���Φr���ӽվ�C\r\n") +
        _T("���T���Φr��������b���ܰT�����N�Q�Хܬ��b�ΰϰ쪺�﨤�u�A����P�r�����e�ä������۲šC") +
        _T("�b�վ㵲����Aø�ϰϰ���ܪ����e�N����ڪ�ANSI�r����{�C���ͪ��ĪG�O�_�A�X�A�i��ݭn�B�~���Ҷq�ӨM�w�C");
    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][4].image_resource_id = IDB_USAGE_BOUNDARY_TRIANGLE_REGULAR;

    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][5].description =
        std_tstring(_T("���T���Φr�����վ�񪽨��T���γ�¡A���|�X�{�V�X������ɡC\r\n")) +
        _T("�Ȧb�@�ر��p�U�A���ۦP�����e�A�㦳��إi�઺�վ�覡�C\r\n") +
        _T("�Ϥ����@²�檺��������ɡA�Y�n�N�̥~�ݪ����Y�_�������ܬ�����A�h�i�b��Ӥ��P����m���J���T���Φr���C\r\n") +
        _T("�ѩ󥿤T���Φr���������~�t�A�Ҳ��ͪ����G�~�[�]�|�������P�C\r\n") +
        _T("��̪���ı�ĪG���ΡA�h�P���u�@�~�Q�n��{�����e�����C\r\n") +
        _T("�b�i��վ�e�A�P�˥i�H�ǥѲ������ʴ�ШӨM�w�ұĥΪ������C");
    g_all_usage_data[IC_USAGE_BOUNDARY_TRIANGLE][5].image_resource_id = IDB_USAGE_BOUNDARY_TRIANGLE_REGULAR_CASE;

    g_all_usage_data[IC_USAGE_MERGE].resize(7);

    g_all_usage_data[IC_USAGE_MERGE][0].description =
        std_tstring(_T("�u�X�֥��Τ��v�]����ҡ^���@�إγ~���S���\��C\r\n")) +
        _T("��i�b�O���Y�@���ΰϰ쪺�~�[���ܪ�����U�A���ܦr�������c�C\r\n") +
        _T("�b�c�Ϫ���A�q�`���|���ϥΦ��\�઺�ݨD�C\r\n") +
        _T("��h���վ�L��t�A�Τw���J�\�h���Τ���AANSI�r�����t�m�|�ܱo�����áC") +
        _T("���\��i�H���s�w�ƬY�ǰϰ쪺ANSI�r���A�H�Q���򪺽s��ʧ@�C\r\n") +
        _T("�ثe���{���������A�j�������X�ְʧ@�ҷ|�۰ʰ���A�Ш������`�̫�@�������C�ϥΪ̤��i�ϥΥ��\���ʦX�֤��e�C");
    g_all_usage_data[IC_USAGE_MERGE][0].image_resource_id = IDB_USAGE_MERGE_INTRO;

    g_all_usage_data[IC_USAGE_MERGE][1].description =
        std_tstring(_T("�b���|�@�d�ҡA�����u�X�֥��Τ��v���i�����γ��X�C\r\n")) +
        _T("�Ϥ��A����u���P�Ʀr��ܥb�Φr�����d��C�b01�A34�P67����m�U���@���Φr���A2�M5����m�h���b�ΪťաC\r\n") +
        _T("���]�ݭn�b��m4��J��r�A�Y�����N��r��J�A��m3�����e�N�Q�}�a�C\r\n") +
        _T("�Y�Q������~�[�A�h�i�ϥΡu��J���Φr���v�b23����m��J�A���a�V���説���A�A�ϥΡu�����C��v�վ�U�������C��A�Ϩ�P�쥻�@�P�C\r\n") +
        _T("�o�ӹL�{�ݭn����\�h�B�J�A�ӡu�X�֥��Τ��v�i�j�T²�ƨ�ާ@�C");
    g_all_usage_data[IC_USAGE_MERGE][1].image_resource_id = IDB_USAGE_MERGE_EXAMPLE;

    g_all_usage_data[IC_USAGE_MERGE][2].description =
        std_tstring(_T("���\��U�A�b�i�H�X�֬���@���Φr������m�A�N�X�{���ܤ��e�C\r\n")) +
        _T("�����ت�ܥi�X�֬��s���Φr�����d��A�������ت�ܭ즳���r���C\r\n") +
        _T("�]�Y�L���ܡA�h�N��L�k�i��X�֡A�Ϊ̸ӳB�w�g�O�@���Φr���C�^\r\n") +
        _T("���U�ƹ������A�A�����Φr���N�Q��J�����ت���m�A�W�X�������h�ର�b�ΪťաA�r�����e�����I����ҳQ�ק�A�ϥ~�[�O�����ܡC\r\n") +
        _T("���Ҥ��A�s�r�����@�a�V���説���C���k�ݪ����Ŧ�ϰ��ܦ��b�Ϊťի�A�K��H�u��J��r�v�\��w���a��J��r�C");
    g_all_usage_data[IC_USAGE_MERGE][2].image_resource_id = IDB_USAGE_MERGE_ACTION;

    g_all_usage_data[IC_USAGE_MERGE][3].description =
        std_tstring(_T("�t�~���@�ت��p�A�q�`�b�h�����ƭק鷺�e�P�վ��C���X�{�C\r\n")) +
        _T("�Ϥ��A0�M5����m���b�ΪťաA12�M34�B�h���������説���Τ��C\r\n") +
        _T("�b�@�s�ꪺ�s���A1�M4�B�Q�]�w���¦�A2�M3�B�h�]�t�h���C��C\r\n") +
        _T("���]�m���M�i�b23����m�B��{�Q�n�����e�A�o���O�F1�M4����m�C\r\n") + 
        _T("�Y�Q�b01��45�B������J�s�����Τ��A�N�|�}�a2��3�B�����e�C\r\n") +
        _T("�ѩ������ɪ����׬ۦP�A�i�H�N23�B�����e�X�֬���@���Τ��C\r\n") + 
        _T("���ܤ��e������ӯ�������A��ܨ�ݬҦ������Q�ର�b�ΪťաC");
    g_all_usage_data[IC_USAGE_MERGE][3].image_resource_id = IDB_USAGE_MERGE_DOUBLE;

    g_all_usage_data[IC_USAGE_MERGE][4].description =
        std_tstring(_T("�����������X���A�ݭn�P�ɦX�֤@��C�����h�Ӧr���~��O���~�[���ܡC\r\n")) +
        _T("�����|�X�@�ǽd�ҡA�䤤�ƹ���Ъ���m�N��ϥΪ̷Q�n�X�֪���m�C\r\n") +
        _T("�p�G��¥u�N�Ӧ�m�X�֬��@�r���A�ñN�W�X�������H�b�Ϊťմ����A�h���w�|�}�a�쥻�����e�C�����N�ӳ����~��P��~�������e�X�֡C\r\n") +
        _T("���{���|�۰ʶi��P�_�A�Y�i��g�ѳs�򪺦X�ֺ�������~�[�A�h�|�b���ܤ��e���e�X�h�Ӥ��H��ܦ����p�C") +
        _T("���U�ƹ�����ɡA�N�|�P�ɦX�֩Ҧ��Хܬ��G���⪺��ءA�L�ݨϥΪ̦b�U�Ӧ�m�@�@�I���C");
    g_all_usage_data[IC_USAGE_MERGE][4].image_resource_id = IDB_USAGE_MERGE_MULTIPLE;

    g_all_usage_data[IC_USAGE_MERGE][5].description =
        std_tstring(_T("�p�G��Ц�m�B�����e�i�H�X�֬��@���Φr���A���W�X�d��|�Q�}�a�A�B�L�k�g�ѫe�z���s��X�֨ӤƸѡA�h�i�H����Shift�����u�j��X�֡v�C\r\n")) +
        _T("���d�Ҥ��A�Y�Q�n�X��12��m�����e�A�h���w�|�}�a���������説���c�C\r\n") +
        _T("���ɴ��ܤ��e���|�D�ʥX�{�A�����b����Shift���~�|��ܡC\r\n") +
        _T("�Y���U�ƹ�����T�w�i��X�֡A�h�����ؤ����������|�Q����O�d�A\r\n") + 
        _T("�W�X�������N�|�ର�̾A�X���b�ΪťաA����~�[���|���ͧ��ܡC\r\n") +
        _T("�p�ϡA��m0�B�����e���ܬ��զ⪺�b�ΪťաA�쥻��⪺�ϰ�h�����C");
    g_all_usage_data[IC_USAGE_MERGE][5].image_resource_id = IDB_USAGE_MERGE_FORCED;

    g_all_usage_data[IC_USAGE_MERGE][6].description =
        std_tstring(_T("�X�֤�檺��z�P�P�_�A�]�Q�����a�B�Φb��Lø�ϥ\��W�C\r\n")) +
        _T("�b�u��t�վ�v�P�u��J�p�����v�\�त�A���ܰT����������ΰϰ�A�N��s��ʧ@���N�n��J���Φr������m�C") +
        _T("�M�ӡA�Ӧ�m�쥻�i��å��㦳���Φr���A�άO�P�w�s�b�����Φr�����b�μe�ת������C\r\n") +
        _T("�u�n�Ӧ�m����X�֤��B�����ܥ~�[�]�N�Y�ëD�i��u�j��X�֡v�^�A\r\n") +
        _T("�N�|��ܴ��ܰT���A�æb�T�w�i��s��ɥ���X�֤��H���s�w�Ʀr���C\r\n") +
        _T("�o�Ӱʧ@�N�ѵ{���۰ʰ���A�L�ݨϥΪ̤����ܡu�X�֥��Τ��v�\��C");
    g_all_usage_data[IC_USAGE_MERGE][6].image_resource_id = IDB_USAGE_MERGE_AUTO;

    g_all_usage_data[IC_USAGE_ACTION].resize(7);

    g_all_usage_data[IC_USAGE_ACTION][0].description =
        std_tstring(_T("�ϥΪ̦bø�ϮɡA�{���|�O���C�@�ӧ���ANSI���e���B�J�C\r\n")) +
        _T("�Y���Y�@�ӨB�J�Ϯ��A�i�H�ɭ˦^�ܥ��e�����A���s�s��C\r\n") +
        _T("�B�J���ԲӸ�T���k����椤���̤U�ݡC�b���i�d�ݦU�ӨB�J�������A�B�J�`�ƻP�ثe��m�A�ζi��_����������ާ@�C\r\n") +
        _T("�{���}�_�ɨB�J�`�Ƭ����C���e���ϥιL�{�ä��|�Q�O���U�ӡC\r\n") +
        _T("�s�W�M�סBŪ���M�סB��Ū��ANSI�ɫ�A�N�M���Ҧ��w�O�����B�J�C\r\n") +
        _T("���x�s�M�סB�x�s����XANSI�ɡB��Ū�J�Ϥ��h���|�v�T�{�����B�J�C");
    g_all_usage_data[IC_USAGE_ACTION][0].image_resource_id = IDB_USAGE_ACTION_INTRO;

    g_all_usage_data[IC_USAGE_ACTION][1].description =
        std_tstring(_T("�b�i��F�@�Ǿާ@��A�B�J��T�����C��N�]�t�Ҧ��w�i�檺�B�J�C\r\n")) +
        _T("�N��i�}��A�ѤW��U���O���ɶ��W�̷s�ܳ��ª��B�J�C\r\n") +
        _T("�z�i�b�o�ǨB�J���D����˦^���{�סA�ثe���B�J�N�H�S���C���ܡC\r\n") +
        _T("�Y���U�ƹ������ܬY�@�B�J�Aø�Ϥ��e�N�|�˦^�ܡu����槹�ӨB�J�v�����A�C�N�Y�A�N�ӨB�J���᪺�Ҧ��B�J�_��C\r\n") +
        _T("�C��̤U�ݪ��u�_��Ҧ��B�J�v�h�i�N�w�O�����Ҧ��B�J�����_��C\r\n") + 
        _T("�Y���Q���ܲ{�������A�A�I���C��~�����ϰ�Y�i�����C");
    g_all_usage_data[IC_USAGE_ACTION][1].image_resource_id = IDB_USAGE_ACTION_STEPS;

    g_all_usage_data[IC_USAGE_ACTION][2].description =
        std_tstring(_T("�@��ϥήɡA�u�ثe�B�J�v���ƭȥ��w����u�B�J�`�ơv�C\r\n")) +
        _T("�u���b�Y�@�������B�J�Q�_���A�u�ثe�B�J�v�|�㦳���p���ƭȡC\r\n") +
        _T("�����s���B�J�Q�[�J�ɡA�i�H�b�B�J��椤��ܶ��Ǹ��᪺�B�J�A\r\n") +
        _T("�H�N���e�_�쪺�B�J�̧ǭ����A����ҿ�ܪ��B�J�Q�����C\r\n") +
        _T("�Y�b�����A�U�����i��s���B�J�A���ثe�B�J���᪺�Ҧ��B�J�N�q�O�����Q�R���A�s���B�J�h�Q�[�J��F�ثe�B�J���᪺��m�C\r\n") +
        _T("�H���覡�R�����B�J�N�L�k�^�_�A�b�Ӧ^�ާ@�ɶ��S�O�`�N�C");
    g_all_usage_data[IC_USAGE_ACTION][2].image_resource_id = IDB_USAGE_ACTION_OVERWRITE;

    g_all_usage_data[IC_USAGE_ACTION][3].description =
        std_tstring(_T("�\�hø�ϥ\��i�H����é즲�ƹ��Ӷi��A��N�Q�O������@�B�J�C\r\n")) +
        _T("�o�ǥ\��]�t�F�u��J�b�Ϊťաv�B�u��J�p�����v�B�u�j������v�P") +
        _T("�u�����C��v���W��\��A�H�θg�ѩ즲�ƹ��ӷL�ժ��u��t�վ�v�C\r\n") +
        _T("�_��������B�J�ɡA��ө즲�L�{�ҳy�������ܳ��|�@�ֶi��C\r\n") +
        _T("�w�Q�O�����s��ʧ@�N�L�k���ΡA�Y�z�Q�n���ѨB�J�H�ϴ_��������ɨ㦳��j���u�ʡA�h�i�H�ϥ��I��ƹ����覡���O�b���P����m�ާ@�C");
    g_all_usage_data[IC_USAGE_ACTION][3].image_resource_id = IDB_USAGE_ACTION_CONTINUOUS;
    
    g_all_usage_data[IC_USAGE_ACTION][4].description =
        std_tstring(_T("�_��������ֶq�B�J�ɥi�ϥΫ��s�P����A�Ӥ����I�}�C��ӿ�ܡC\r\n")) +
        _T("�I���B�J�C��U�ݪ��u�_��@�B�v�P�u���Ƥ@�B�v���s�A\r\n") +
        _T("�Ψϥμ���Alt+Z�PAlt+X�A�i�i���B���_��������C\r\n") +
        _T("�Y�ثe�B�J����m�w�L�k�_��έ����A���s�N��ܬ��L�Ǧ�C\r\n") +
        _T("�@��ާ@�ɡA�Y���`�ϥνզ�L�Υ��Φr���C��A�h�B�J��T�q�`�|���k���檺��ܽd�򤧥~�A���ɨϥμ���ާ@������K�C");
    g_all_usage_data[IC_USAGE_ACTION][4].image_resource_id = IDB_USAGE_ACTION_ONE_STEP;

    g_all_usage_data[IC_USAGE_ACTION][5].description =
        std_tstring(_T("���{���ثe�i�O���̦h500�Ӿާ@�B�J�A���j���������p�U�w�����C\r\n")) +
        _T("�����ϥΫ�A�O���ƶq���i���F�W���C����s�i�檺�B�J���M�|�Q����O���A�����ª��B�J�N�q�O�����M���A�H�����`�Ƥ��W�L500�C\r\n") +
        _T("�Y�b���ɿ�ܨB�J�C��̤U�ݪ��u�_��Ҧ��B�J�v�A�h�u�|�_��ثe�Q�O����500�ӨB�J�A���ª��B�J�N�L�k�_��C");
    g_all_usage_data[IC_USAGE_ACTION][5].image_resource_id = IDB_USAGE_ACTION_MAX_COUNT;

    g_all_usage_data[IC_USAGE_ACTION][6].description =
        std_tstring(_T("���F�U��ø�ϥ\��~�A�t�~����ذʧ@�]�|�Q�O�����ާ@�B�J�C\r\n")) +
        _T("�k��\���椤�A���B�J��T�ϰ�W�誺�u�M���e�O�v���s�i�ΨӲM���Ҧ��{�s��ANSI���e�A���ʧ@�|�Q�O�����W���u�M���e�O�v���B�J�C\r\n") +
        _T("�t�~�A����UPageDown�B�V�U����V��B�Ϊ����H�\���椤�����s����ø�ϰϰ�ҹ�������C�ɡA") +
        _T("�p�Gø�ϰϰ�̤U�ݪ���C�s���W�L�쥻ANSI���e�Ҩ㦳����C�ƶq�A�h�|�۰ʸɤW�ťվ�C�H�����һݼƶq�C\r\n") +
        _T("��s����C�Q�۰ʥ[�J�ɡA�N���ͦW���u�������ݽd��v���B�J�C");
    g_all_usage_data[IC_USAGE_ACTION][6].image_resource_id = IDB_USAGE_ACTION_SPECIAL;


    return true;
}

static bool g_dummy = InitAllUsageData();