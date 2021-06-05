
#pragma once

#if defined(MAC_OSX) || defined(IOS_ARM)
#include <sys/sysctl.h>
#include <sys/statvfs.h>
#else
#include <linux/sysctl.h>
#endif
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <wchar.h>
#include <dlfcn.h>
#include <assert.h>
#include <string>
#ifdef MAC_OSX
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <pwd.h>
#include <unistd.h>

#ifdef LINUX_SERVER
#include <string.h>
#include <limits.h>
#endif

#include <algorithm>
using std::min;
using std::max;

#define IS_ANDROID defined(ANDROID_X86) || defined (ANDROID_ARM)

#if IS_ANDROID
#include <cpu-features.h>
#endif

/* 
 * Base type definition 
 */

typedef char                                    CHAR;
typedef char                                    int8;
typedef unsigned char                   byte;
typedef unsigned char                   BYTE;
typedef unsigned char*                  LPBYTE;
typedef unsigned char                   uint8;

typedef unsigned short                  USHORT; 
typedef int                                             int32;
typedef int                                             INT32;
typedef int                                             INT;
typedef unsigned int                    uint;
typedef unsigned int                    uint32;
typedef unsigned int                    UINT32;
typedef unsigned int                    UINT;

typedef long                                    LONG;

typedef long long                               int64;
typedef long long                               LONGLONG;
typedef long long                               _int64;

#ifndef INTEL_COMPILER_MAC
typedef long long                               __int64;
#endif

typedef unsigned long                   DWORD;
typedef unsigned long                   ulong;
typedef unsigned long                   ULONG;
typedef unsigned long long              uint64;

typedef unsigned short                  WORD;
typedef DWORD*                                  PDWORD;
typedef DWORD*                                  LPDWORD;

typedef void                                    VOID;
typedef void*                                   LPVOID;
typedef int                     HRESULT;
typedef void*                                   HWND;
typedef void*                                   HKEY;
typedef HKEY*                                   PHKEY;
typedef void*                                   HANDLE;
typedef void*                                   HMODULE;
typedef void*                                   HINSTANCE;

typedef wchar_t                                 WCHAR;
typedef wchar_t*                                LPWSTR;
typedef wchar_t*                                LPWSTR;
typedef const wchar_t*                  LPCWSTR;

typedef const char*                             LPCSTR;

typedef char                            BOOL;

#ifndef _TCHAR_DEFINED
#define _TCHAR_DEFINED
#ifdef _UNICODE
typedef wchar_t                         _TCHAR;
typedef wchar_t                         TCHAR;
typedef wchar_t*                        LPTSTR;
typedef const wchar_t*                  LPCTSTR;
#define _T(x) L ## x
#define std_tstring std_tstring
#else
typedef char                            _TCHAR;
typedef char                            TCHAR;
typedef char*                           LPTSTR;
typedef const char*                     LPCTSTR;
#define _T(x) x
#define std_tstring std::string
#endif
#endif

#define TRUE                    1
#define FALSE                   0

struct RECT
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};

inline bool IntersectRect(RECT &inter_rect, const RECT &rect1, const RECT &rect2)
{
    inter_rect.left = (rect1.left > rect2.left) ? rect1.left : rect2.left;
    inter_rect.right = (rect1.right < rect2.right) ? rect1.right : rect2.right;
    inter_rect.top = (rect1.top > rect2.top) ? rect1.top : rect2.top;
    inter_rect.bottom = (rect1.bottom < rect2.bottom) ? rect1.bottom : rect2.bottom;

    bool has_intersect = true;
    if (inter_rect.left >= inter_rect.right || inter_rect.top >= inter_rect.bottom)
        has_intersect = false;

    return has_intersect;
}

/*
 * Common Constant
 */

typedef intptr_t INT_PTR;
#ifndef BYTE_MAX
#define BYTE_MAX 255
#endif

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

#ifndef _I64_MIN
#define _I64_MIN LLONG_MIN
#endif

// limits defined in <intsafe.h>
#ifndef SHORT_MIN
#define SHORT_MIN   (-32768)
#endif

#ifndef SHORT_MAX
#define SHORT_MAX   32767
#endif

#ifndef USHORT_MAX
#define USHORT_MAX  0xffff
#endif

/* 
 * Attributes 
 */

#ifndef __forceinline
#define __forceinline   __inline__ __attribute__((always_inline))
#endif
#define FORCEINLINE __forceinline

#ifndef STDCALL
#define STDCALL 
#define __stdcall 
#endif

#define WINAPI

#ifndef CDECL
#define CDECL 
#define __cdecl 
#endif

#define __declspec(x)   __attribute__((x))

#define APIENTRY WINAPI

/* 
 * Common Function
 */

#ifdef MAC_OSX
// only in MAC OSX
static inline DWORD GetTickCount()
{
    return mach_absolute_time() * 1e-6;
}
#endif

#ifndef ZeroMemory
#define ZeroMemory(ptr, size) memset((ptr), 0, (size))
#endif

/*
 * TCHAR function definition
 */

#define _tsplitpath _splitpath
#define _stprintf sprintf
#define _stscanf sscanf
#define _tcslen strlen
#define _tgetenv getenv
#define _tcscmp strcmp
#define _tfopen fopen
#define _vstprintf vsprintf
#define _tcscat strcat
#define _tcstok strtok
#define _tstoi atoi
#define _tstof atof
#define _ttoi atoi
#define _ftprintf fprintf
#define _tstoi64 atoll
#define _tcstoul strtoul
#define _tcscpy strcpy
#define _tstat stat
#define _tcsncpy strncpy

class CStringForUnix
{
public:
    CStringForUnix(LPCTSTR p_str = _T("")) : m_data(p_str) {}

    inline operator LPCTSTR() const 
    {
        return m_data.c_str();
    }

    friend bool operator ==(const CStringForUnix &str1, LPCTSTR str2);
    friend bool operator ==(const CStringForUnix &str1, const CStringForUnix & str2);
    friend bool operator ==(const LPCTSTR str1, const CStringForUnix & str2);
    friend bool operator !=(const CStringForUnix &str1, LPCTSTR str2);
    friend bool operator !=(const CStringForUnix &str1, const CStringForUnix & str2);
    friend bool operator !=(const LPCTSTR str1, const CStringForUnix & str2);


    inline CStringForUnix &operator +=(LPCTSTR p_str)
    {
        m_data += p_str;
        return (*this);
    }

    friend CStringForUnix operator +(const CStringForUnix &str1, const CStringForUnix &str2);
    friend CStringForUnix operator +(LPCTSTR str1, const CStringForUnix &str2);

    inline size_t Length() const { return m_data.length(); }

    inline CStringForUnix &TrimLeft(LPCTSTR p_str = _T(" \t\n\r")) 
    {
        if (m_data.length() == 0) return (*this);

        size_t move_offset = m_data.find_first_not_of(p_str);
        if (move_offset != 0)
        {
            if (move_offset == std::string::npos)
            {
                m_data.clear();
            }
            else
            {
                m_data.erase(0, move_offset);
            }
        }

        return (*this);
    }

    inline CStringForUnix &TrimRight(LPCTSTR p_str = _T(" \t\n\r")) 
    {
        if (m_data.length() == 0) return (*this);

        size_t move_offset = m_data.find_last_not_of(p_str);
        if (move_offset != m_data.length() - 1)
        {
            if (move_offset == std::string::npos)
            {
                m_data.clear();
            }
            else
            {
                m_data.erase(move_offset + 1);
            }
        }

        return (*this);
    }

    inline CStringForUnix &TrimLeft(_TCHAR ch) { return TrimLeft(std_tstring(1, ch).c_str()); }
    inline CStringForUnix &TrimRight(_TCHAR ch) { return TrimRight(std_tstring(1, ch).c_str()); }

    inline CStringForUnix Mid(int offset) { return CStringForUnix(m_data.substr(offset)); }
    inline CStringForUnix Mid(int offset, size_t len) { return CStringForUnix(m_data.substr(offset, len)); }
    inline CStringForUnix Right(size_t len) { return (len < m_data.length()) ? CStringForUnix(m_data.substr(m_data.length() - len)) : (*this); }
    inline CStringForUnix Left(size_t len) { return (len < m_data.length()) ? CStringForUnix(m_data.substr(0, len)) : (*this); }

    void Format(LPCTSTR fmt, ...)
    {
        _TCHAR buffer[4096];
        va_list marker;
        va_start(marker, fmt);
        _vstprintf(buffer, fmt, marker);
        m_data = buffer;
    }

    bool IsEmpty() const { return m_data.length() == 0; }

private:
    CStringForUnix(const std_tstring &str) : m_data(str) {}

    std_tstring m_data;

};

inline CStringForUnix operator +(const CStringForUnix &str1, const CStringForUnix &str2)
{
    return CStringForUnix(str1.m_data + str2.m_data);
}

inline CStringForUnix operator +(LPCTSTR str1, const CStringForUnix &str2)
{
    return CStringForUnix(str1 + str2.m_data);
}

inline bool operator ==(const CStringForUnix &str1, LPCTSTR str2) { return str1.m_data.compare(str2) == 0; }
inline bool operator ==(const CStringForUnix &str1, const CStringForUnix & str2) { return str1.m_data.compare(str2.m_data) == 0; } 
inline bool operator ==(const LPCTSTR str1, const CStringForUnix & str2) { return str2.m_data.compare(str1) == 0; }
inline bool operator !=(const CStringForUnix &str1, LPCTSTR str2) { return !(str1 == str2); }
inline bool operator !=(const CStringForUnix &str1, const CStringForUnix & str2) { return !(str1 == str2); }
inline bool operator !=(const LPCTSTR str1, const CStringForUnix & str2) { return !(str1 == str2); }
//////////////////////////////////////////////////////////////////////////
// Operators not been implemented.
bool operator >=(const CStringForUnix &str1, LPCTSTR str2); 
bool operator >=(const CStringForUnix &str1, const CStringForUnix & str2);
bool operator >=(const LPCTSTR str1, const CStringForUnix & str2);
bool operator <=(const CStringForUnix &str1, LPCTSTR str2);
bool operator <=(const CStringForUnix &str1, const CStringForUnix & str2);
bool operator <=(const LPCTSTR str1, const CStringForUnix & str2);
bool operator >(const CStringForUnix &str1, LPCTSTR str2);
bool operator >(const CStringForUnix &str1, const CStringForUnix & str2);
bool operator >(const LPCTSTR str1, const CStringForUnix & str2);
bool operator <(const CStringForUnix &str1, LPCTSTR str2);
bool operator <(const CStringForUnix &str1, const CStringForUnix & str2);
bool operator <(const LPCTSTR str1, const CStringForUnix & str2);
//////////////////////////////////////////////////////////////////////////

typedef CStringForUnix                   CString;
typedef CStringForUnix                   CStringA;

inline void _splitpath(const char *p_path, char *p_drive, char *p_dir, char *p_fname, char *p_ext)
{
    if (!p_path || !p_drive || !p_dir || !p_fname || !p_ext) return;

    std::string pathstr = p_path;
    p_drive[0] = 0;
    p_dir[0] = 0;
    p_fname[0] = 0;
    p_ext[0] = 0;

    if (pathstr.length() == 0) return;

    size_t dir_offset_slash = pathstr.find_last_of("/");
    size_t dir_offset_backslash = pathstr.find_last_of("\\");

    size_t dir_offset = dir_offset_slash;
    if (dir_offset_backslash != std_tstring::npos &&
        (dir_offset == std_tstring::npos ||
         dir_offset < dir_offset_backslash))
    {
        dir_offset = dir_offset_backslash;
    }

    std::string filestr;
    if (dir_offset != std::string::npos)
    {
        strcpy(p_dir, pathstr.substr(0, dir_offset + 1).c_str());
        filestr = pathstr.substr(dir_offset + 1);
    }
    else
    {
        filestr = pathstr;
    }

    if (filestr.length() == 0) return;

    size_t dot_offset = filestr.find_last_of(".");
    if (dot_offset != std::string::npos)
    {
        if (dot_offset > 0) strcpy(p_fname, filestr.substr(0, dot_offset).c_str());
        if (dot_offset < filestr.length()) strcpy(p_ext, filestr.substr(dot_offset).c_str());
    }
    else
    {
        strcpy(p_fname, filestr.c_str());
    }
}

/* 
 * COM related definition 
 */

typedef void*                                   LPUNKNOWN;

#define E_NOTIMPL               ((HRESULT)0x80004001L)
#define E_NOINTERFACE   ((HRESULT)0x80004002L)
#define E_POINTER               ((HRESULT)0x80004003L)
#define E_ABORT                 ((HRESULT)0x80004004L)
//#define E_FAIL                ((HRESULT)0x80004005L)
#define E_UNEXPECTED    ((HRESULT)0x8000FFFFL)
#define E_ACCESSDENIED  ((HRESULT)0x80070005L)
#define E_HANDLE                ((HRESULT)0x80070006L)
#define E_OUTOFMEMORY   ((HRESULT)0x8007000EL)
#define E_INVALIDARG    ((HRESULT)0x80070057L)

#define E_FAIL          ((HRESULT)0x80000008L)
#define S_OK            ((HRESULT)0L)
#define S_FALSE         ((HRESULT)1L)

#define SUCCEEDED(hr)   (((HRESULT)(hr)) >= 0)
#define FAILED(hr)              (((HRESULT)(hr)) < 0)

#define WM_APP                  (0x8000)
#define WM_USER                 (0x0400)

#define DECLARE_IUNKNOWN ;

#define INITGUID
#include "guiddef_unix.h"

# define STDMETHODCALLTYPE  __stdcall
# define STDMETHODVCALLTYPE __cdecl
# define STDAPICALLTYPE         __stdcall
# define STDAPIVCALLTYPE    __cdecl
# define STDAPI                         EXTERN_C HRESULT STDAPICALLTYPE
# define STDAPI_(t)             EXTERN_C t STDAPICALLTYPE
# define STDMETHODIMP           HRESULT STDMETHODCALLTYPE
# define STDMETHODIMP_(t)   t STDMETHODCALLTYPE
# define STDAPIV                EXTERN_C HRESULT STDAPIVCALLTYPE
# define STDAPIV_(t)            EXTERN_C t STDAPIVCALLTYPE
# define STDMETHODIMPV          HRESULT STDMETHODVCALLTYPE
# define STDMETHODIMPV_(t)  t STDMETHODVCALLTYPE

# if defined(__cplusplus) && !defined(CINTERFACE)
#  define STDMETHOD(m)          virtual HRESULT STDMETHODCALLTYPE m
#  define STDMETHOD_(t,m)   virtual       t STDMETHODCALLTYPE m
#  define PURE  =0
#endif

#define DECLARE_INTERFACE(i) struct i
#define DECLARE_INTERFACE_(i,b) struct i : public b

#define CheckPointer(p, return_value) { if (p == NULL) return return_value; }

struct IUnknown
{
    virtual unsigned long AddRef(void)  {return 0;};
    virtual unsigned long Release(void) {return 0;};
};
struct CUnknown {
        CUnknown(WCHAR* name, void*) {};
        CUnknown( CHAR* name, void*) {};
        CUnknown(_TCHAR *name, void *, void *) {}
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void *pp_interface) { return E_NOTIMPL; }
};

static inline HRESULT GetInterface(LPUNKNOWN p_unknown, void **pp_interface)
{
    *pp_interface = p_unknown;
    return S_OK;
}

/*
 *      LoadLibraryEx, FreeLibrary, GetProcAddress
 */

#ifndef LOAD_WITH_ALTERED_SEARCH_PATH
#define LOAD_WITH_ALTERED_SEARCH_PATH   0x00000008
#endif
inline void *LoadLibraryEx_Unix(LPCSTR lpFileName, 
                               HANDLE *hFile = NULL, 
                               DWORD dwFlags = LOAD_WITH_ALTERED_SEARCH_PATH)
{
	assert(dwFlags == LOAD_WITH_ALTERED_SEARCH_PATH);
    assert(hFile == NULL);
	return dlopen(lpFileName, RTLD_LAZY|RTLD_LOCAL);
}

#define LoadLibraryEx   LoadLibraryEx_Unix
#define LoadLibrary     LoadLibraryEx_Unix
#define FreeLibrary             dlclose
#define GetProcAddress  dlsym

inline bool PathFileExists(LPCTSTR p_filename)
{
    FILE *p_file = _tfopen(p_filename, _T("r"));

    if (p_file)
    {
        fclose(p_file);
        return true;
    }
    else
    {
        return false;
    }
}

inline void Sleep(int ms_count)
{
    sleep(ms_count / 1000.0f);
}

struct ULARGE_INTEGER
{
    unsigned long long QuadPart;
};

#ifdef MAC_OSX
inline bool GetDiskFreeSpaceEx(LPCTSTR p_path, ULARGE_INTEGER *p_free_byte_count, void *p1, void *p2)
{
    if (p_free_byte_count)
    {
        struct statvfs buf;
        if (!statvfs(p_path, &buf)) 
        {
            unsigned long long blksize = buf.f_bsize;
            unsigned long long freeblks = buf.f_bfree;
            unsigned long long free_size = freeblks * blksize;

            p_free_byte_count->QuadPart = free_size / 255;

            return true;
        } 
        else
        {
            return false;
        }
    }

    return true;
}
#endif

inline bool GetModuleFileName(void *p_handle, LPTSTR filename, int length)
{
    Dl_info info; 
    memset(&info, 0, sizeof(info));

    if (dladdr(p_handle, &info) == 0) return false;
    if (!info.dli_fname) return false;

    int copy_length = _tcslen(info.dli_fname);
    if (copy_length > length) copy_length = length; 
    _tcsncpy(filename, info.dli_fname, copy_length);

    return true;
}

inline std_tstring GetUnixHomeDir()
{
    const char *p_homedir = "";

    passwd *p_pwd = getpwuid(getuid());
    if (p_pwd) p_homedir = p_pwd->pw_dir;

    return std_tstring(p_homedir);
}

#define THREAD_PRIORITY_LOWEST -2
#define THREAD_PRIORITY_BELOW_NORMAL ((THREAD_PRIORITY_LOWEST) + 1)
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_HIGHEST 2
#define THREAD_PRIORITY_ABOVE_NORMAL ((THREAD_PRIORITY_HIGHEST) - 1)
#define THREAD_PRIORITY_IDLE -15
