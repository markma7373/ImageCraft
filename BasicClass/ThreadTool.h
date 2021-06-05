
#pragma once

#include <wtypes.h>
#include <vector>

#ifdef _WINRT_METRO
#include <thread>
#define DEFAULT_SPIN_COUNT 4000
#endif

// wrapper for whatever critical section we have
class ChCritSec 
{
    // make copy constructor and assignment operator inaccessible
    ChCritSec(const ChCritSec &refCritSec);
    ChCritSec &operator=(const ChCritSec &refCritSec);

    CRITICAL_SECTION m_CritSec;

public:
    ChCritSec() 
    {
#ifndef _WINRT_METRO
        InitializeCriticalSection(&m_CritSec);
#else
        InitializeCriticalSectionEx(&m_CritSec, DEFAULT_SPIN_COUNT, 0);
#endif
    };

    ~ChCritSec() 
    {
        DeleteCriticalSection(&m_CritSec);
    };

    void Lock() 
    {
        EnterCriticalSection(&m_CritSec);
    };

    void Unlock() 
    {
        LeaveCriticalSection(&m_CritSec);
    };

#ifndef _WINRT_METRO
    BOOL TryLock()
    {
        return TryEnterCriticalSection(&m_CritSec);
    };
#endif
};

// locks a critical section, and unlocks it automatically
// when the lock goes out of scope
class ChAutoLock 
{
    // make copy constructor and assignment operator inaccessible
    ChAutoLock(const ChAutoLock &refAutoLock);
    ChAutoLock &operator=(const ChAutoLock &refAutoLock);

protected:
    ChCritSec * m_pLock;

public:
    ChAutoLock(ChCritSec * plock)
    {
        m_pLock = plock;
        if(m_pLock != NULL)
            m_pLock->Lock();
    };

    ~ChAutoLock() 
    {
        if(m_pLock != NULL)
            m_pLock->Unlock();
    };
};

//////////////////////////////////////////////////////////////////////////

class CThreadControl 
{
    HANDLE hEvent_Begin;
    HANDLE hEvent_End;
    HANDLE hThread;
    bool bKillMe;
#ifndef _WINRT_METRO
    DWORD nThreadId;
#else
    std::thread::id nThreadId;
    std::thread *mp_std_thread;
#endif

public:

    CThreadControl();
    ~CThreadControl();

    int CreateThread_Run(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter);
    void CloseThread();
    void ResetEnd();
    void ResetBegin();
    void SignalComplete();
    DWORD WaitBegin(DWORD tmTimeOut = INFINITE);
    void SignalBegin();
    DWORD WaitComplete(DWORD tmTimeOut = INFINITE);
    bool TimeToDie();
    bool SetPriority(const int nPriority);
    DWORD_PTR SetAffinityMask(const DWORD_PTR mask);
};

//////////////////////////////////////////////////////////////////////////

class CThreadRunnable
{
public:
    CThreadRunnable();
    virtual ~CThreadRunnable();

    virtual int Run(const int cmd_id, const int tid) = 0;
};

struct Worker_Param;

#ifndef _WINRT_METRO
class CThreadServer
{
    int m_nCPU;
    int m_nNextSession;

    std::vector<Worker_Param *> m_params;
    std::vector<CThreadControl *> m_threads;

    void Proc_Worker();
    static DWORD WINAPI Thread_Worker(void *lpParam);

    void AllocThreads(const int th_num);

public: 
    CThreadServer();
    ~CThreadServer();

    int GetCPUNum();
    int GetThreadCnt();
    int GetFreeThreadCnt();

    int AcquireSession(int& session_id, const int thread_num);
    void ReleaseSession(const int session_id);

    int RunThreads(const int session_id, CThreadRunnable *pTask, const int cmd_id);
    int WaitThreads(const int session_id);
    bool IsThreadsActive(const int session_id);
    bool SetPriority(const int session_id, const int nPriority);
};
#endif

////////////////////////////////////////////////////////////////////////////
// the implementation of ThreadPool

struct ThreadWorkItem
{
    LPVOID mp_param;
    LPTHREAD_START_ROUTINE mp_function;
    HANDLE m_event_complete;
    ThreadWorkItem()
    {
        mp_param = NULL;
        mp_function = NULL;
        m_event_complete = INVALID_HANDLE_VALUE;
    }
};

class WorkItemQueue
{
public:
    WorkItemQueue();
    ~WorkItemQueue();
    void Initialize(int num_work_item);
    bool Push(ThreadWorkItem *p_work_item);
    bool Pop(ThreadWorkItem **pp_work_item);

private:
    int m_head_index;
    int m_tail_index;
    int m_work_count;
    int m_num_work_item;
    ThreadWorkItem **mpp_work_item;

    void FreeQueue();
};

class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();
    void CreateThread(int num_thread = 0, int num_work_item = 0);
    bool AddWorkItem(ThreadWorkItem &work_item, DWORD time_out = INFINITE);
    void FreeThread();
    bool IsThreadCreated()
    {
        return (m_num_thread != 0);
    }

private:
    int m_num_thread;
    int m_num_work_item;

    HANDLE m_avaliable_work_item;
    HANDLE m_avaliable_work_item_slot;

    WorkItemQueue m_work_item_queue;

    bool m_kill_me;
#ifndef _WINRT_METRO
    DWORD *mp_thread_id;
#else
    std::thread::id *mp_thread_id;
    std::thread **mp_std_thread;
#endif
    
    HANDLE *mp_thread_handle;
    ChCritSec m_work_item_queue_section;

    bool GetWorkItem(ThreadWorkItem **pp_work_item);

    bool TimeToDie()
    {
        return m_kill_me;
    }
    static DWORD WINAPI ThreadProcess(LPVOID lpParam);
};

class ThreadControlShell
{
public:
    ThreadControlShell();
    ~ThreadControlShell();

    void CreateThread_Run(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter);
    void SetThreadPool(ThreadPool *p_thread_pool);
    void SignalBegin();
    void WaitComplete(DWORD tmTimeOut = INFINITE);

private:
    ThreadWorkItem m_work_item;
    ThreadPool *mp_thread_pool;
    ThreadPool *mp_internal_thread_pool;
};

#ifndef _WINRT_METRO
// ChAutoTryLock is the none blocking auto lock,
// which will try to Lock "ChCritSec2" and use "IsLocked" to check is "Lock" success or not
class ChAutoTryLock
{
    // make copy constructor and assignment operator inaccessible
    ChAutoTryLock(const ChAutoTryLock &ref_auto_lock);
    ChAutoTryLock &operator=(const ChAutoTryLock &ref_auto_lock);

protected:
    ChCritSec *mp_lock;
    BOOL m_is_locked;

public:
    ChAutoTryLock(ChCritSec *p_lock)
    {
        _MYASSERT(p_lock);
        mp_lock = p_lock;
        m_is_locked = mp_lock->TryLock();
    };

    ~ChAutoTryLock()
    {
        _MYASSERT(mp_lock);
        if (m_is_locked)
            mp_lock->Unlock();
    };

    BOOL IsLocked()
    {
        return m_is_locked;
    };
};
#endif