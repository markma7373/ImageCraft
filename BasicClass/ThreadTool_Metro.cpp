
#include "stdafx.h"
#include "ThreadTool_Metro.h"
#include "Common.h"

CThreadControl::CThreadControl() 
{
    bKillMe = false;
    //nThreadId = (DWORD) -1;
    mp_std_thread = NULL;
    hThread = INVALID_HANDLE_VALUE;
    hEvent_Begin = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
    hEvent_End = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
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
    mp_std_thread = new std::thread(lpStartAddress, lpParameter);

    if (NULL == mp_std_thread)
        return -1;

    hThread = mp_std_thread->native_handle();
    nThreadId = mp_std_thread->get_id();

    return 0;
}

void CThreadControl::CloseThread()
{
    if (hThread == INVALID_HANDLE_VALUE)
        return;
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
    return WaitForSingleObjectEx(hEvent_Begin, tmTimeOut, true);
}

void CThreadControl::SignalBegin()
{
    SetEvent(hEvent_Begin);
}

DWORD CThreadControl::WaitComplete(DWORD tmTimeOut)
{
    return WaitForSingleObjectEx(hEvent_End, tmTimeOut, true);
}

bool CThreadControl::TimeToDie() 
{ 
    return bKillMe; 
}

bool CThreadControl::SetPriority(const int nPriority)
{
    if (hThread == NULL)
        return 0;

    return TRUE;
}              

//////////////////////////////////////////////////////////////////////////

CThreadRunnable::CThreadRunnable()
{

}

CThreadRunnable::~CThreadRunnable()
{

}

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
, mp_std_thread(NULL)
{}

ThreadPool::~ThreadPool()
{
    FreeThread();
}

void ThreadPool::CreateThread(int num_thread /*= 0*/, int num_work_item /*= 0*/)
{
    FreeThread();

    if (num_thread > 0)
        m_num_thread = num_thread;
    else
        m_num_thread = g_cpu_count.GetSuggestedThreadNumber();
    if (num_work_item > 0)
        m_num_work_item = num_work_item;
    else
        m_num_work_item = m_num_thread * 2;

    m_kill_me = false;
    m_avaliable_work_item = CreateSemaphoreEx(NULL, 0, m_num_work_item, NULL, 0, SEMAPHORE_ALL_ACCESS);
    m_avaliable_work_item_slot = CreateSemaphoreEx(NULL, m_num_work_item, m_num_work_item, NULL, 0, SEMAPHORE_ALL_ACCESS);
    mp_thread_handle = new HANDLE[m_num_thread];
    mp_thread_id = new std::thread::id[m_num_thread];
    mp_std_thread = new std::thread *[m_num_thread];

    for (int i = 0; i < m_num_thread; i++)
    {
		mp_std_thread[i] = new std::thread((LPTHREAD_START_ROUTINE) ThreadPool::ThreadProcess, this);
        mp_thread_handle[i] = mp_std_thread[i]->native_handle();
        mp_thread_id[i] = mp_std_thread[i]->get_id();
    }

    m_work_item_queue.Initialize(m_num_work_item);
}

bool ThreadPool::AddWorkItem(ThreadWorkItem &work_item, DWORD time_out)
{
    DWORD result = WaitForSingleObjectEx(m_avaliable_work_item_slot, time_out, true);
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
        _MYASSERT(FORCE_FALSE);
        return false;
    }
}

bool ThreadPool::GetWorkItem(ThreadWorkItem **pp_work_item)
{
    WaitForSingleObjectEx(m_avaliable_work_item, INFINITE, true);

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

    for (int i = 0; i < m_num_thread; i++)
    {
        if (NULL != mp_std_thread[i])
        {
            mp_std_thread[i]->join();
            delete mp_std_thread[i];
        }
    }

    if (NULL != mp_std_thread)
    {
        delete mp_std_thread;
        mp_std_thread = NULL;
    }
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
    m_work_item.m_event_complete = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
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
    WaitForSingleObjectEx(m_work_item.m_event_complete, tmTimeOut, true);
}
