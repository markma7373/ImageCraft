
#pragma once

#include <wtypes.h>
#include <vector>
#include <thread>

#define DEFAULT_SPIN_COUNT 4000

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
        InitializeCriticalSectionEx(&m_CritSec, DEFAULT_SPIN_COUNT, 0);
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
    std::thread::id nThreadId;
    std::thread *mp_std_thread;

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
    std::thread::id *mp_thread_id;
    HANDLE *mp_thread_handle;
    ChCritSec m_work_item_queue_section;

    std::thread **mp_std_thread;

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