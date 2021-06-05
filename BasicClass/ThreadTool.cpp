
#include "stdafx.h"
#include "ThreadTool.h"
#include "Common.h"

CThreadControl::CThreadControl() 
{
    bKillMe = false;

#ifndef _WINRT_METRO
    nThreadId = (DWORD) -1;
    hThread = INVALID_HANDLE_VALUE;
    hEvent_Begin = CreateEvent(NULL, FALSE, FALSE, NULL);
    hEvent_End = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
    mp_std_thread = NULL;
    hThread = INVALID_HANDLE_VALUE;
    hEvent_Begin = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
    hEvent_End = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
#endif
}

CThreadControl::~CThreadControl() 
{
    CloseThread();
    CloseHandle(hEvent_Begin);
    CloseHandle(hEvent_End);
}

int CThreadControl::CreateThread_Run(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter) 
{
    bKillMe = false;

#ifndef _WINRT_METRO    
    hThread = ::CreateThread(NULL, NULL, lpStartAddress, lpParameter, NULL, &nThreadId);
    if(hThread == INVALID_HANDLE_VALUE)
        return -1;
#else
    mp_std_thread = new std::thread(lpStartAddress, lpParameter);
    if (NULL == mp_std_thread)
        return -1;

    hThread = mp_std_thread->native_handle();
    nThreadId = mp_std_thread->get_id();
#endif

    return 0;
}

void CThreadControl::CloseThread()
{
    if(hThread == INVALID_HANDLE_VALUE)
        return;

#ifndef _WINRT_METRO
    bKillMe = true;
    SetEvent(hEvent_Begin);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    hThread = INVALID_HANDLE_VALUE;
    bKillMe = false;
#else
    bKillMe = true;
    SignalBegin();
    WaitForSingleObjectEx(hThread, INFINITE, TRUE);
    hThread = NULL;
    bKillMe = false;
    if (NULL != mp_std_thread)
    {
        mp_std_thread->join();
        delete mp_std_thread;
        mp_std_thread = NULL;
    }
#endif
}

void CThreadControl::ResetEnd()
{
    ResetEvent(hEvent_End);
}

void CThreadControl::ResetBegin()
{
    ResetEvent(hEvent_Begin);
}

void CThreadControl::SignalComplete()
{
    SetEvent(hEvent_End);
}

DWORD CThreadControl::WaitBegin(DWORD tmTimeOut)
{
#ifndef _WINRT_METRO
    return WaitForSingleObject(hEvent_Begin, tmTimeOut);
#else
    return WaitForSingleObjectEx(hEvent_Begin, tmTimeOut, true);
#endif
}

void CThreadControl::SignalBegin()
{
    SetEvent(hEvent_Begin);
}

DWORD CThreadControl::WaitComplete(DWORD tmTimeOut)
{
#ifndef _WINRT_METRO
    return WaitForSingleObject(hEvent_End, tmTimeOut);
#else
    return WaitForSingleObjectEx(hEvent_End, tmTimeOut, true);
#endif   
}

bool CThreadControl::TimeToDie() 
{ 
    return bKillMe; 
}

bool CThreadControl::SetPriority(const int nPriority)
{
#ifndef _WINRT_METRO
    return ::SetThreadPriority(hThread, nPriority) ? true: false;
#else
    if (hThread == NULL)
        return 0;

    return TRUE;
#endif
}

#ifndef _WINRT_METRO
DWORD_PTR CThreadControl::SetAffinityMask(const DWORD_PTR mask)
{
    return ::SetThreadAffinityMask(hThread, mask);
}                        
#endif

//////////////////////////////////////////////////////////////////////////

CThreadRunnable::CThreadRunnable()
{

}

CThreadRunnable::~CThreadRunnable()
{

}

//////////////////////////////////////////////////////////////////////////
#ifndef _WINRT_METRO
struct Worker_Param
{
    int nID;
    int session_id;
    CThreadServer *pCtx;
    CThreadRunnable *pTask;
    int nCmd;
    int tid;
    bool bActive;
};

CThreadServer::CThreadServer()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    m_nCPU = info.dwNumberOfProcessors;

    m_nNextSession = 0;
}

CThreadServer::~CThreadServer()
{
    for(int i = 0; i < GetThreadCnt(); i++) {
        m_threads[i]->CloseThread();
        delete m_params[i];
        delete m_threads[i];
    }
}

int CThreadServer::GetCPUNum()
{
    return m_nCPU;
}

int CThreadServer::GetThreadCnt()
{
    return (int) m_threads.size();
}

int CThreadServer::GetFreeThreadCnt()
{
    int cnt = 0;
    for(int i = 0; i < GetThreadCnt(); i++) 
        if(m_params[i]->session_id == -1)
            cnt++;
    return cnt;
}

int CThreadServer::AcquireSession(int& session_id, const int thread_num)
{
    int nFreeCnt = GetFreeThreadCnt();
    if(nFreeCnt < thread_num)
        AllocThreads(thread_num - nFreeCnt);

    int assigned_cnt = 0;
    for(int i = 0; i < GetThreadCnt(); i++) {
        if(m_params[i]->session_id == -1) {
            m_params[i]->session_id = m_nNextSession;
            if(++assigned_cnt == thread_num)
                break;
        }
    }
    session_id = m_nNextSession;
    m_nNextSession = (m_nNextSession + 1) % 1000000;
    return 0;
}

void CThreadServer::ReleaseSession(const int session_id)
{
    for(int i = 0; i < GetThreadCnt(); i++) {
        if(m_params[i]->session_id == session_id) 
            m_params[i]->session_id = -1;
    }
}

void CThreadServer::AllocThreads(const int th_num)
{
    for(int i = 0; i < th_num; i++) {
        m_params.push_back(new Worker_Param);
        m_threads.push_back(new CThreadControl);
        Worker_Param *p_param = m_params.back();
        p_param->nID = GetThreadCnt();
        p_param->pCtx = this;
        p_param->session_id = -1;
        p_param->bActive = false;
        m_threads.back()->CreateThread_Run(CThreadServer::Thread_Worker, p_param);
    }
}

DWORD WINAPI CThreadServer::Thread_Worker(void *lpParam)
{
    if (lpParam == NULL)
        return 0;

    Worker_Param *param = (Worker_Param *) lpParam;

    while(true) {
        param->pCtx->m_threads[param->nID]->WaitBegin();
        if(param->pCtx->m_threads[param->nID]->TimeToDie())
            break;
        param->bActive = true;
        param->pTask->Run(param->nCmd, param->tid);
        param->bActive = false;
        param->pCtx->m_threads[param->nID]->SignalComplete();
    }

    return 0;
}

int CThreadServer::RunThreads(const int session_id, CThreadRunnable *pTask, const int cmd_id)
{
    int th_cnt = 0;
    for(int i = 0; i < GetThreadCnt(); i++) {
        if(m_params[i]->session_id == session_id) {
            m_params[i]->pTask = pTask;
            m_params[i]->nCmd = cmd_id;
            m_params[i]->tid = th_cnt++;
            m_threads[i]->SignalBegin();
        }
    }

    return 0;
}

int CThreadServer::WaitThreads(const int session_id)
{
    for(int i = 0; i < GetThreadCnt(); i++) {
        if(m_params[i]->session_id == session_id) 
            m_threads[i]->WaitComplete();
    }

    return 0;
}

bool CThreadServer::IsThreadsActive(const int session_id)
{
    bool bRet = false;
    for(int i = 0; i < GetThreadCnt(); i++) {
        if(m_params[i]->session_id == session_id)
            bRet |= m_params[i]->bActive;
    }

    return bRet;
}

bool CThreadServer::SetPriority(const int session_id, const int nPriority)
{
    bool bRet = true;
    for(int i = 0; i < GetThreadCnt(); i++) {
        if(m_params[i]->session_id == session_id)
            bRet &= m_threads[i]->SetPriority(nPriority);
    }

    return bRet;
}
#endif

////////////////////////////////////////////////////////////////////////////
// the implementation of ThreadPool

WorkItemQueue::WorkItemQueue()
: m_head_index(0)
, m_tail_index(0)
, m_work_count(0)
, m_num_work_item(0)
, mpp_work_item(NULL)
{}

WorkItemQueue::~WorkItemQueue()
{
    FreeQueue();
}

void WorkItemQueue::Initialize(int num_work_item)
{
    FreeQueue();

    m_num_work_item = num_work_item;
    mpp_work_item = new ThreadWorkItem*[m_num_work_item];

    for (int i = 0; i < m_num_work_item; i++)
    {
        mpp_work_item[i] = NULL;
    }
    m_head_index = 0;
    m_tail_index = 0;
    m_work_count = 0;
}

bool WorkItemQueue::Push(ThreadWorkItem *p_work_item)
{
    if (m_work_count >= m_num_work_item)
        return false;

    _MYASSERT(p_work_item);
    _MYASSERT(mpp_work_item);

    mpp_work_item[m_tail_index] = p_work_item;
    m_tail_index++;
    m_work_count++;
    if (m_tail_index >= m_num_work_item)
        m_tail_index = 0;
    return true;
}

bool WorkItemQueue::Pop(ThreadWorkItem **pp_work_item)
{
    if (m_work_count <= 0)
        return false;

    _MYASSERT(pp_work_item);
    _MYASSERT(mpp_work_item);

    (*pp_work_item) = mpp_work_item[m_head_index];
    m_head_index++;
    m_work_count--;
    if (m_head_index >= m_num_work_item)
        m_head_index = 0;
    return true;
}

void WorkItemQueue::FreeQueue()
{
    if (mpp_work_item != NULL)
    {
        delete [] mpp_work_item;
        mpp_work_item = NULL;
    }
    m_num_work_item = 0;
    m_head_index = 0;
    m_tail_index = 0;
    m_work_count = 0;
}

ThreadPool::ThreadPool()
: mp_thread_handle(NULL)
, m_avaliable_work_item(INVALID_HANDLE_VALUE)
, m_avaliable_work_item_slot(INVALID_HANDLE_VALUE)
, m_kill_me(false)
, mp_thread_id(NULL)
, m_num_thread(0)
, m_num_work_item(0)
#ifdef _WINRT_METRO
, mp_std_thread(NULL)
#endif
{}

ThreadPool::~ThreadPool()
{
    FreeThread();
}

void ThreadPool::CreateThread(int num_thread /*= 0*/, int num_work_item /*= 0*/)
{
    FreeThread();

    if (num_thread > 0)
    {
        m_num_thread = num_thread;
    }
    else
    {
#ifndef _WINRT_METRO
        m_num_thread = max(1, GetLogicalCPUCount());
#else
        m_num_thread = g_cpu_count.GetSuggestedThreadNumber();
#endif
    }

    if (num_work_item > 0)
        m_num_work_item = num_work_item;
    else
        m_num_work_item = m_num_thread * 2;

    m_kill_me = false;
#ifndef _WINRT_METRO
    m_avaliable_work_item = CreateSemaphore(NULL, 0, m_num_work_item, NULL);
    m_avaliable_work_item_slot = CreateSemaphore(NULL, m_num_work_item, m_num_work_item, NULL);
    mp_thread_handle = new HANDLE[m_num_thread];
    mp_thread_id = new DWORD[m_num_thread];

    for (int i = 0; i < m_num_thread; i++)
    {
        mp_thread_handle[i] = ::CreateThread(NULL, NULL, ThreadPool::ThreadProcess, this, NULL, &mp_thread_id[i]);
    }
#else
    m_avaliable_work_item = CreateSemaphoreEx(NULL, 0, m_num_work_item, NULL, 0, SEMAPHORE_ALL_ACCESS);
    m_avaliable_work_item_slot = CreateSemaphoreEx(NULL, m_num_work_item, m_num_work_item, NULL, 0, SEMAPHORE_ALL_ACCESS);
    mp_thread_handle = new HANDLE[m_num_thread];
    mp_thread_id = new std::thread::id[m_num_thread];
    mp_std_thread = new std::thread *[m_num_thread];

    for (int i = 0; i < m_num_thread; i++)
    {
        mp_std_thread[i] = new std::thread(ThreadPool::ThreadProcess, this);
        mp_thread_handle[i] = mp_std_thread[i]->native_handle();
        mp_thread_id[i] = mp_std_thread[i]->get_id();
    }
#endif

    m_work_item_queue.Initialize(m_num_work_item);
}

bool ThreadPool::AddWorkItem(ThreadWorkItem &work_item, DWORD time_out)
{
#ifndef _WINRT_METRO
    DWORD result = WaitForSingleObject(m_avaliable_work_item_slot, time_out);
#else
    DWORD result = WaitForSingleObjectEx(m_avaliable_work_item_slot, time_out, true);
#endif

    if (result == WAIT_OBJECT_0)
    {
        {
            ChAutoLock work_item_queue_lock(&m_work_item_queue_section);
            if (!m_work_item_queue.Push(&work_item))
                return false;
        }
        ReleaseSemaphore(m_avaliable_work_item, 1, NULL);
        return true;
    }
    else if (result == WAIT_TIMEOUT)
    {
        return false;
    }
    else
    {
#ifndef _WINRT_METRO
        _MYASSERT(FALSE);
#else
        _MYASSERT(FORCE_FALSE);
#endif
        return false;
    }
}

bool ThreadPool::GetWorkItem(ThreadWorkItem **pp_work_item)
{
#ifndef _WINRT_METRO
    WaitForSingleObject(m_avaliable_work_item, INFINITE);
#else
    WaitForSingleObjectEx(m_avaliable_work_item, INFINITE, true);
#endif

    {
        {
            ChAutoLock work_item_queue_lock(&m_work_item_queue_section);
            if (!m_work_item_queue.Pop(pp_work_item))
                return false;
        }
        ReleaseSemaphore(m_avaliable_work_item_slot, 1, NULL);
        return true;
    }
}

void ThreadPool::FreeThread()
{
    if (mp_thread_handle != NULL)
    {
        m_kill_me = true;
        ReleaseSemaphore(m_avaliable_work_item, m_num_thread, NULL);
        for (int i = 0; i < m_num_thread; i++)
        {
#ifndef _WINRT_METRO
            WaitForSingleObject(mp_thread_handle[i], INFINITE);
            CloseHandle(mp_thread_handle[i]);
#else
            WaitForSingleObjectEx(mp_thread_handle[i], INFINITE, TRUE);
#endif
        }
        m_kill_me = false;

        delete [] mp_thread_handle;
        mp_thread_handle = NULL;
    }
    if (mp_thread_id != NULL)
    {
        delete [] mp_thread_id;
        mp_thread_id = NULL;
    }
    if (m_avaliable_work_item != INVALID_HANDLE_VALUE)
        CloseHandle(m_avaliable_work_item);
    if (m_avaliable_work_item_slot != INVALID_HANDLE_VALUE)
        CloseHandle(m_avaliable_work_item_slot);

#ifdef _WINRT_METRO
    for (int i = 0; i < m_num_thread; i++)
    {
        if (NULL != mp_std_thread[i])
        {
            mp_std_thread[i]->join();
            delete mp_std_thread[i];
            mp_std_thread[i] = NULL;
        }
    }

    if (NULL != mp_std_thread)
    {
        delete [] mp_std_thread;
        mp_std_thread = NULL;
    }
#endif
}

DWORD WINAPI ThreadPool::ThreadProcess(LPVOID lpParam)
{
    _MYASSERT(lpParam);

    ThreadPool *p_thread_pool = (ThreadPool*)lpParam;
    ThreadWorkItem *p_work_item;

    for (;;)
    {
        if (p_thread_pool->TimeToDie())
            break;

        if (p_thread_pool->GetWorkItem(&p_work_item))
        {
            try
            {
                p_work_item->mp_function(p_work_item->mp_param);
            }
            catch (std::bad_alloc)
            {
            }
            SetEvent(p_work_item->m_event_complete);
        }
    }
    return 0;
}

ThreadControlShell::ThreadControlShell()
: mp_thread_pool(NULL)
, mp_internal_thread_pool(NULL)
{}

ThreadControlShell::~ThreadControlShell()
{
    if (mp_internal_thread_pool != NULL)
        delete mp_internal_thread_pool;
    if (m_work_item.m_event_complete != INVALID_HANDLE_VALUE)
        CloseHandle(m_work_item.m_event_complete);
}

void ThreadControlShell::CreateThread_Run(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter)
{
    m_work_item.mp_function = lpStartAddress;
    m_work_item.mp_param = lpParameter;

    if (m_work_item.m_event_complete != INVALID_HANDLE_VALUE)
        CloseHandle(m_work_item.m_event_complete);
#ifndef _WINRT_METRO
    m_work_item.m_event_complete = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
    m_work_item.m_event_complete = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
#endif
}

void ThreadControlShell::SetThreadPool(ThreadPool *p_thread_pool)
{
    _MYASSERT(p_thread_pool);
    mp_thread_pool = p_thread_pool;
}

void ThreadControlShell::SignalBegin()
{
    // enable creating thread pool if mp_thread_pool is null, let TT3D Photo can run normally
    if (mp_thread_pool == NULL)
    {
        mp_internal_thread_pool = new ThreadPool;
        // Create one thread for each ThreadControl
        mp_internal_thread_pool->CreateThread(1, 1);
        mp_thread_pool = mp_internal_thread_pool;
    }
    mp_thread_pool->AddWorkItem(m_work_item);
}

void ThreadControlShell::WaitComplete(DWORD tmTimeOut)
{
#ifndef _WINRT_METRO
    WaitForSingleObject(m_work_item.m_event_complete, tmTimeOut);
#else
    WaitForSingleObjectEx(m_work_item.m_event_complete, tmTimeOut, true);
#endif
}
