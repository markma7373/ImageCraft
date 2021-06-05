#pragma once

#define CheckPointer(p, return_value) { if (p == NULL) return return_value; }

#define DECLARE_IUNKNOWN ;
#define STDMETHODCALLTYPE       __stdcall
#define WINAPI                  __stdcall
#define STDMETHODIMP            HRESULT STDMETHODCALLTYPE

#define S_OK            ((HRESULT)0L)
#define S_FALSE         ((HRESULT)1L)
#define SUCCEEDED(hr)   (((HRESULT)(hr)) >= 0)
#define FAILED(hr)      (((HRESULT)(hr)) < 0)

#define DEFAULT_SPIN_COUNT 4000

struct CUnknown {
        CUnknown(TCHAR *name, LPUNKNOWN p_unknown, HRESULT *p_hresult) {};
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void *pp_interface) { return E_NOTIMPL; }
};

inline HRESULT GetInterface(LPUNKNOWN p_unknown, void **pp_interface)
{
    if (NULL == pp_interface)
        return E_FAIL;

    *pp_interface = p_unknown;
    return S_OK;
}

typedef RECT * LPRECT;

inline bool IntersectRect(LPRECT result, const RECT *rect1, const RECT* rect2)
{
    if (NULL == result)
        return false;

    if (NULL == rect1)
        return false;

    if (NULL == rect2)
        return false;

    result->left = max(rect1->left, rect2->left);
    result->right = min(rect1->right, rect2->right);
    result->top = max(rect1->top, rect2->top);
    result->bottom = min(rect1->bottom, rect2->bottom);

    bool is_intersect = ((result->right > result->left) && (result->bottom > result->top));

    if (!is_intersect)
        result->left = result->top = result->right = result->bottom = 0;

    return is_intersect;
}

inline bool UnionRect(LPRECT result, const RECT *rect1, const RECT* rect2)
{
    if (NULL == result)
        return false;

    if (NULL == rect1)
        return false;

    if (NULL == rect2)
        return false;

    result->left = min(rect1->left, rect2->left);
    result->right = max(rect1->right, rect2->right);
    result->top = min(rect1->top, rect2->top);
    result->bottom = max(rect1->bottom, rect2->bottom);

    LONG result_width = result->right - result->left;
    LONG result_height = result->bottom - result->top;

    if ((result_width != 0) || (result_height != 0))
        return true;
    else
        return false;
}

inline HANDLE WINAPI CreateEvent(LPSECURITY_ATTRIBUTES event_attributes, BOOL manual_reset, BOOL initial_state, LPCTSTR name)
{
    DWORD flags = (DWORD)((initial_state << 1) + manual_reset);
    return CreateEventEx(event_attributes, name, flags, EVENT_ALL_ACCESS);
}

inline DWORD WINAPI WaitForSingleObject(HANDLE handle, DWORD milliseconds)
{
    return WaitForSingleObjectEx(handle, milliseconds, TRUE);
}

inline void WINAPI InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
    InitializeCriticalSectionEx(lpCriticalSection, DEFAULT_SPIN_COUNT, 0);
}

inline HANDLE WINAPI CreateSemaphore(LPSECURITY_ATTRIBUTES attributes, LONG initial_count, LONG max_count, LPCTSTR name)
{
    return CreateSemaphoreEx(attributes, initial_count, max_count, name, 0, EVENT_ALL_ACCESS);
}

inline void Sleep(DWORD millisecnods)
{
    HANDLE nonsignal_event = NULL;
    nonsignal_event = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
    WaitForSingleObjectEx(nonsignal_event, millisecnods, EVENT_ALL_ACCESS);
    CloseHandle(nonsignal_event);
}

inline DWORD timeGetTime()
{
    SYSTEMTIME system_time;
    GetSystemTime(&system_time);
    return (((system_time.wHour * 60) + system_time.wMinute) * 60 + system_time.wSecond) * 1000 + system_time.wMilliseconds;
}

inline void GetSystemInfo(SYSTEM_INFO *info)
{
    GetNativeSystemInfo(info);
}

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