#include "Common.h"

#ifdef UNIX_OS
#if IS_ANDROID
#include <android/log.h>
#else
#include <syslog.h>
#endif
#endif

#pragma comment (lib, "psapi.lib")

#ifdef _WINRT_METRO
CpuCount g_cpu_count;
bool cpu_count_dummy = g_cpu_count.detect();
#endif

CHAR szErrorMessage[256];

CMyConfig g_my_config;

CMyConfig::CMyConfig()
{
    m_dump_info = 0;

#ifdef UNIX_OS
    LPCTSTR p_dumpinfo_str = _tgetenv(_T("DumpInfo"));
    if (p_dumpinfo_str)
    {
        m_dump_info = (DWORD)_tstoi(p_dumpinfo_str);
    }
#else

#ifndef _WINRT_METRO
    CRegKey reg_key;
    if (reg_key.Open(HKEY_CURRENT_USER, _T("SOFTWARE\\CyberLink\\Magic"), KEY_READ) == ERROR_SUCCESS)
    {
        reg_key.QueryDWORDValue(_T("DumpInfo"), m_dump_info);
    }
#endif

#endif
}

CMyConfig::~CMyConfig()
{
}

//////////////////////////////////////////////////////////////////////////
int g_assign_cpu_count = 0;
void SetLogicalCPUCount(int cpu_count)
{
    if (cpu_count > 0)
    {
        g_assign_cpu_count = cpu_count;
    }
}

//////////////////////////////////////////////////////////////////////////

ChPoint operator+(const ChPoint& a, const ChPoint& b)
{
    ChPoint r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    return r;
}

ChPoint operator-(const ChPoint& a, const ChPoint& b)
{
    ChPoint r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    return r;
}

//////////////////////////////////////////////////////////////////////////

int ch_FileExist(LPCTSTR fname)
{
    if(fname == NULL || !_tcscmp(fname, _T("")) || _tcslen(fname) == 0)
        return 0;
    FILE *fp = _tfopen(fname, _T("rb"));
    if(fp == NULL)
        return 0;
    fclose(fp);
    return 1;
}

int ch_GetFileSize(LPCTSTR fname)
{
    FILE *fp = _tfopen(fname, _T("rb"));
    if(fp == NULL)
        return -1;

    fseek(fp, 0L, SEEK_END);
    int fsz = (int) ftell(fp);
    fclose(fp);
    return fsz;
}

int ch_dprintf(LPCTSTR fmt, ...)
{
    _TCHAR buffer[4096];

    va_list marker;
    va_start(marker, fmt);
    int nArgs = _vstprintf(buffer, fmt, marker);
#ifdef UNIX_OS
#if IS_ANDROID
    __android_log_print(ANDROID_LOG_DEBUG, _DBG_MSG_PREFIX_TAG, "%s", buffer);
#else
    syslog(LOG_NOTICE, _T("%s %s"), _DBG_MSG_PREFIX_TAG, buffer);
    printf(_T("%s %s\n"), _DBG_MSG_PREFIX_TAG, buffer);
#endif
#else
     _TCHAR buffer2[4096];

    _stprintf(buffer2, _T("%s %s"), _DBG_MSG_PREFIX_TAG, buffer);
    if(buffer2[_tcslen(buffer2) - 1] != '\n')
        _tcscat(buffer2, _T("\n"));
    OutputDebugString(buffer2);
#endif
    va_end (marker);		
    return nArgs;
}

LPTSTR ch_TimeFormat(LONGLONG tmCode, LPTSTR szTimeString)
{
	int nSec = (int)(tmCode / 10000000);
	int nMin = nSec / 60;
	nSec = nSec % 60;
	int nHr = nMin / 60;
	nMin = nMin % 60;
	_stprintf(szTimeString, _T("%02d:%02d:%02d"), nHr, nMin, nSec);
	return szTimeString;
}

void ch_GetNowTimeStr(LPTSTR strTime)
{
	time_t tm_now;
	time(&tm_now);
	struct tm *currtime = localtime(&tm_now);
	_stprintf(strTime, _T("%04d%02d%02d_%02d%02d%02d"), 
		currtime->tm_year + 1900, currtime->tm_mon + 1, currtime->tm_mday, 
		currtime->tm_hour, currtime->tm_min, currtime->tm_sec);
}

int ch_CopyFile(LPCTSTR src_name, LPCTSTR dest_name)
{
    FILE *fp = _tfopen(src_name, _T("rb"));
    if(fp == NULL)
        return -1;
    int fsz = ch_GetFileSize(src_name);
    BYTE *pBuf = new BYTE[fsz];
    fread(pBuf, sizeof(BYTE), fsz, fp);
    fclose(fp);
    fp = _tfopen(dest_name, _T("wb"));
    if(fp != NULL) {
        fwrite(pBuf, sizeof(BYTE), fsz, fp);
        fclose(fp);
    }
    delete [] pBuf;
    return 0;
}

int ch_NeedScaleFix(int& new_x, int& new_y, int width, int height)
{
    if(new_x == width && new_y == height)
        return 0;

    float r1 = float(width) / height;
    float r2 = float(new_x) / new_y;
    if(r2 > r1) {
        float ratio = (float) width / new_x;
        new_x = width;
        new_y = ch_Max(ch_Round(new_y * ratio), 1);
    }
    else {
        float ratio = (float) height / new_y;
        new_x = ch_Max(ch_Round(new_x * ratio), 1);
        new_y = height;
    }
    return 1;
}

bool ch_NeedScale(int& new_x, int& new_y, int width, int height)
{
    if(new_x <= width && new_y <= height)
        return false;

    float r1 = float(width) / height;
    float r2 = float(new_x) / new_y;
    if(r2 > r1) {
        float ratio = (float) width / new_x;
        new_x = width;
        new_y = ch_Max(ch_Round(new_y * ratio), 1);
    }
    else {
        float ratio = (float) height / new_y;
        new_x = ch_Max(ch_Round(new_x * ratio), 1);
        new_y = height;
    }
    return true;
}

#ifndef UNIX_OS

#ifndef _WINRT_METRO
int ch_ExploreFolder(LPCTSTR filter, LPCTSTR path, std::vector<std_tstring>& file_list, bool bDirOnly, bool bRecursive)
{
    WIN32_FIND_DATA fda;
    HANDLE hFind;

    std_tstring find_path = ch_MakeFileFullPath(path, filter);
    if((hFind = FindFirstFile(find_path.c_str(), &fda)) != INVALID_HANDLE_VALUE) {
        do {
            BOOL bCurrDir = fda.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
            BOOL bIsDot = !_tcscmp(fda.cFileName, _T(".")) || !_tcscmp(fda.cFileName, _T(".."));
            if(bDirOnly) {
                if(!bCurrDir || bIsDot)
                    continue;
            }
            else {
                if(bCurrDir)
                    continue;
            }
            file_list.push_back(ch_MakeFileFullPath(path, fda.cFileName));
        } while(FindNextFile(hFind, &fda));
        FindClose(hFind);
    }

    if(bRecursive) {
        find_path = ch_MakeFileFullPath(path, _T("*"));
        if((hFind = FindFirstFile(find_path.c_str(), &fda)) != INVALID_HANDLE_VALUE) {
            do {
                // only for directory
                BOOL bCurrDir = fda.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
                BOOL bIsDot = !_tcscmp(fda.cFileName, _T(".")) || !_tcscmp(fda.cFileName, _T(".."));
                if(!bCurrDir || bIsDot) 
                    continue;
                std_tstring sub_folder = ch_MakeFileFullPath(path, fda.cFileName);
                ch_ExploreFolder(filter, sub_folder.c_str(), file_list, bDirOnly, bRecursive);
            } while(FindNextFile(hFind, &fda));
        }
        FindClose(hFind);
    }

    return 0;
}

int ch_GetSelectedFolder(HWND hWnd, LPTSTR szFolderOut)
{
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));

	bi.hwndOwner = hWnd;
	bi.ulFlags   = BIF_RETURNONLYFSDIRS;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if(pidl == NULL)
		return -1;

	BOOL bRet = FALSE;

	TCHAR szFolder[MAX_PATH * 2];
	szFolder[0] = _T('\0');

	if(pidl) {
		if(SHGetPathFromIDList(pidl, szFolder))
			bRet = TRUE;

		IMalloc *pMalloc = NULL; 
		if(SUCCEEDED(SHGetMalloc(&pMalloc)) && pMalloc) {  
			pMalloc->Free(pidl);  
			pMalloc->Release(); 
		}
	}

	_tcscpy(szFolderOut, szFolder);

	return 0;
}
#endif

std::string unicode_to_utf8(const wchar_t *p_wide_string)
{
    int byte_count;
    char *p_string;
    std::string out_string = "\0";

    byte_count = WideCharToMultiByte(CP_UTF8, 0, p_wide_string, -1, 0, 0, 0, 0);
    p_string = (char *)malloc(byte_count);
    if (p_string == NULL)
        return out_string;

    byte_count = WideCharToMultiByte(CP_UTF8, 0, p_wide_string, -1, p_string, byte_count, 0, 0);
    if (byte_count != 0)
       out_string = p_string;

    free(p_string);

    return out_string;
}

std::string unicodeToAnsi(const wchar_t *p_wide_string)
{
	return unicode_to_ansi(p_wide_string);
}

std::string unicode_to_ansi(const wchar_t *p_wide_string)
{
    int byte_count;
    char *p_string;
    std::string out_string = "\0";

    byte_count = WideCharToMultiByte(CP_ACP, 0, p_wide_string, -1, 0, 0, 0, 0);
    p_string = (char *)malloc(byte_count);
    if (p_string == NULL)
        return out_string;

    byte_count = WideCharToMultiByte(CP_ACP, 0, p_wide_string, -1, p_string, byte_count, 0, 0);
    if (byte_count != 0)
       out_string = p_string;

    free(p_string);

    return out_string;
}

std::wstring utf8_to_unicode(const char *p_string)
{
    int char_count;
    wchar_t *p_wide_string;
    std::wstring out_wstring = _T("\0");

    char_count = MultiByteToWideChar(CP_UTF8, 0, p_string, -1, NULL, 0);
    p_wide_string = (wchar_t *)malloc(char_count * sizeof(wchar_t));
    if (p_wide_string == NULL)
        return out_wstring;

    char_count = MultiByteToWideChar(CP_UTF8, 0, p_string, -1, p_wide_string, char_count);
    if (char_count != 0)
        out_wstring = p_wide_string;

    free(p_wide_string);

    return out_wstring;
}

std::wstring AnsiToUnicode(const char *p_string)
{
	return ansi_to_unicode(p_string);
}

std::wstring ansi_to_unicode(const char *p_string)
{
    int char_count;
    wchar_t *p_wide_string;
    std::wstring out_wstring = _T("\0");

    char_count = MultiByteToWideChar(CP_ACP, 0, p_string, -1, NULL, 0);
    p_wide_string = (wchar_t *)malloc(char_count * sizeof(wchar_t));
    if (p_wide_string == NULL)
        return out_wstring;

    char_count = MultiByteToWideChar(CP_ACP, 0, p_string, -1, p_wide_string, char_count);
    if (char_count != 0)
        out_wstring = p_wide_string;

    free(p_wide_string);

    return out_wstring;
}
#endif

void *ch_GetMem(int size_of_element, const std::vector<int> &dim)
{
    if (dim.size() == 0)
        return NULL;

    int n = 0;      // number of dimensions
    int ne = 1;     // number of elements needed
    int np = 1;     // number of pointers needed
    while ((n + 1) < (int)dim.size())
    {
        ne *= dim[n];
        np *= (1 + dim[n]);
        ++n;
    }
    ne = ne * dim[n];
    np--;
    ++n;

    int cbPointers = ((((np*sizeof(void *)) + 15) >> 4) << 4);  // size of pointers in bytes

    void **m;

    m = (void **)malloc(cbPointers + ne *size_of_element);
    if(m == NULL)
        return NULL;

    void **p = m;
    int npi = 1;    // number of pointers at this level

    for (int i = 0; i < n - 1; i++)
    {
        npi = npi * dim[i];
        int d = dim[i + 1];
        void **q = p + npi;
        if (i < n - 2)
        {
            for (int j = 0; j < npi; j++)
                *p++ = q + d * j;
        }
        else
        {
            q = (void * *) (((char *) m) + cbPointers);

            for (int j = 0; j < npi; j++)
                *p++ = (void *) ((char *) q + d * j * size_of_element);
        }
        p = q;
    }

    return m;
}

void *ch_GetMem1D(int size_of_element, int d1)
{
    std::vector<int> dim;
    dim.push_back(d1);
    return ch_GetMem(size_of_element, dim);
}

void *ch_GetMem2D(int size_of_element, int d1, int d2)
{
    std::vector<int> dim;
    dim.push_back(d1);
    dim.push_back(d2);
    return ch_GetMem(size_of_element, dim);
}

void *ch_GetMem3D(int size_of_element, int d1, int d2, int d3)
{
    std::vector<int> dim;
    dim.push_back(d1);
    dim.push_back(d2);
    dim.push_back(d3);
    return ch_GetMem(size_of_element, dim);
}

void ch_FreeMem(void *p)
{
    free(p);
}

void ShowMemoryUsage(LPCTSTR title/* = NULL*/)
{
#ifdef UNIX_OS
#ifdef MAC_OSX
    struct task_basic_info curr_task_info;
    mach_msg_type_number_t curr_task_info_count = TASK_BASIC_INFO_COUNT;

    if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&curr_task_info, &curr_task_info_count) == 
        KERN_SUCCESS)
    {
        float memory_usage_in_mb = (float)curr_task_info.resident_size / 1024.0f / 1024.0f;
        if (title)
            _DPRINTF((_T("%s: %.2f MB"), title, memory_usage_in_mb));
        else
            _DPRINTF((_T("Page File Usage = %.2f MB"), memory_usage_in_mb));
    }
#endif
#else

#ifndef _WINRT_METRO
    PROCESS_MEMORY_COUNTERS mem_info;
    GetProcessMemoryInfo(GetCurrentProcess(), &mem_info, sizeof(PROCESS_MEMORY_COUNTERS));

    float memory_usage_in_mb = (float)(mem_info.PagefileUsage) / 1024.0f / 1024.0f;

    if (title)
        _DPRINTF((_T("%s: %.2f MB"), title, memory_usage_in_mb));
    else
        _DPRINTF((_T("Page File Usage = %.2f MB"), memory_usage_in_mb));
#endif

#endif
}
