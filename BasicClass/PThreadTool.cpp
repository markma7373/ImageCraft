#include "PThreadTool.h"
#include <sys/timeb.h>
#include <time.h>

PEventHandle CreatePEvent(bool manual_reset, bool initial_status)
{
    PEventHandle event_handle = (PEventHandle) new PEvent(manual_reset, initial_status);
    return event_handle;
}

void ClosePEvent(PEventHandle event_handle)
{
    delete event_handle;
}

void SetPEvent(PEventHandle event_handle)
{
    pthread_mutex_lock(&event_handle->m_mutex);
    event_handle->m_triggered = true;
    if (event_handle->m_manual_reset)
        pthread_cond_broadcast(&event_handle->m_condition);
    else
        pthread_cond_signal(&event_handle->m_condition);
    pthread_mutex_unlock(&event_handle->m_mutex);
}

void ResetPEvent(PEventHandle event_handle)
{
    pthread_mutex_lock(&event_handle->m_mutex);
    event_handle->m_triggered = false;
    pthread_mutex_unlock(&event_handle->m_mutex);
}

PSyncObjectWaitResult WaitForPEvent(PEventHandle event_handle, int time_out)
{
    pthread_mutex_lock(&event_handle->m_mutex);
    if (time_out == INFINITE)
    {
        while(false == event_handle->m_triggered)
            pthread_cond_wait(&event_handle->m_condition, &event_handle->m_mutex);
        if (!event_handle->m_manual_reset)
            event_handle->m_triggered = false; //auto-reset
    }
    else
    {
        int rc = 0;
        struct timespec tm;
        struct timeb tp;
        long sec, millisec;

        sec = time_out / 1000;
        millisec = time_out % 1000;
        ftime( &tp );
        tp.time += sec;
        tp.millitm += millisec;

        if (tp.millitm > 999)
        {
            tp.millitm -= 1000;
            tp.time++;
        }

        tm.tv_sec = tp.time;
        tm.tv_nsec = tp.millitm * 1000000;

        while(false == event_handle->m_triggered)
        {
            rc = pthread_cond_timedwait(&event_handle->m_condition, &event_handle->m_mutex, &tm);

            if ((0 != rc) && (errno != EINTR))
                break;
        }

        if (0 != rc)
        {
            pthread_mutex_unlock(&event_handle->m_mutex);
            if (rc == ETIMEDOUT)
                return WAIT_TIMEOUT;
                
            return WAIT_ERROR;    
        }

        if (!event_handle->m_manual_reset)
            event_handle->m_triggered = false; //auto-reset
    }
    pthread_mutex_unlock(&event_handle->m_mutex);
    
    return WAIT_OBJECT_0;
}

PSyncObjectWaitResult WaitForSingleObject(PEventHandle event_handle, int time_out)
{
    return WaitForPEvent(event_handle, time_out);
}

PThreadControl::PThreadControl()
{
    m_begin_triggered = false;
    m_complete_triggered = false;
    pthread_cond_init(&m_condition_begin, 0);
    pthread_cond_init(&m_condition_complete, 0);
    pthread_mutex_init(&m_mutex_begin, 0);
    pthread_mutex_init(&m_mutex_complete, 0);
    m_kill_me = false;

    m_thread_created = false;
}

PThreadControl::~PThreadControl()
{
    CloseThread();
    pthread_cond_destroy(&m_condition_begin);
    pthread_cond_destroy(&m_condition_complete);
    pthread_mutex_destroy(&m_mutex_begin);
    pthread_mutex_destroy(&m_mutex_complete);
}

int PThreadControl::CreateThreadRun(void *(*start_routine)(void *), void *argument)
{
    if (true == m_thread_created)
        CloseThread();

    m_kill_me = false;
    int return_value = pthread_create(&m_thread, 0, start_routine, (void *)argument);

    if (0 == return_value)
        m_thread_created = true;
    else
        m_thread_created = false;

    return return_value;
}

void PThreadControl::CloseThread()
{
    if (true == m_thread_created)
    {
        m_kill_me = true;
        SignalBegin();
        pthread_join(m_thread, 0);
        m_kill_me = false;

        m_begin_triggered = false;
        m_complete_triggered = false;

        m_thread_created = false;
    }
}

void PThreadControl::WaitComplete(int time_out_ms)
{
    int rc = 0;
    struct timespec tm;
    struct timeb tp;
    long sec, millisec;

    sec = time_out_ms / 1000;
    millisec = time_out_ms % 1000;
    ftime( &tp );
    tp.time += sec;
    tp.millitm += millisec;
    
    if (tp.millitm > 999)
    {
        tp.millitm -= 1000;
        tp.time++;
    }

    tm.tv_sec = tp.time;
    tm.tv_nsec = tp.millitm * 1000000;

    pthread_mutex_lock(&m_mutex_complete);
    while(false == m_complete_triggered)
    {
        rc = pthread_cond_timedwait(&m_condition_complete, &m_mutex_complete, &tm);
        if ((0 != rc) && (errno != EINTR))
            break;
    }
    m_complete_triggered = false; //auto-reset
    pthread_mutex_unlock(&m_mutex_complete);
}

PSemaphoreHandle CreateSemaphore(void *p_semaphore_attribute, long initial_count, long max_count, void *p_name)
{
    // no support for this two parameters
    _MYASSERT(p_semaphore_attribute == NULL);
    _MYASSERT(p_name == NULL);

    PSemaphoreHandle semaphore_handle = (PSemaphoreHandle) new PSemaphore(initial_count, max_count);
    return semaphore_handle;
}

void CloseHandle(PSemaphoreHandle semaphore_handle)
{
    delete semaphore_handle;
}

void ReleaseSemaphore(PSemaphoreHandle semaphore_handle, long release_count, long *previous_count)
{
    _MYASSERT(previous_count == NULL);

    pthread_mutex_lock(&semaphore_handle->m_mutex);
    semaphore_handle->m_semaphore_count += release_count;
    semaphore_handle->m_semaphore_count = min(semaphore_handle->m_semaphore_count, semaphore_handle->m_max_count);
    pthread_mutex_unlock(&semaphore_handle->m_mutex);
    pthread_cond_broadcast(&semaphore_handle->m_condition);
}

PSyncObjectWaitResult WaitForPSemaphore(PSemaphoreHandle semaphore_handle, int time_out)
{
    pthread_mutex_lock(&semaphore_handle->m_mutex);
    if (time_out == INFINITE)
    {
        while (semaphore_handle->m_semaphore_count <= 0)
        {
            pthread_cond_wait(&semaphore_handle->m_condition, &semaphore_handle->m_mutex);
        }
        semaphore_handle->m_semaphore_count--;
    }
    else
    {
        int rc = 0;
        struct timespec	 tm;
        struct timeb tp;
        long sec, millisec;

        sec = time_out / 1000;
        millisec = time_out % 1000;
        ftime( &tp );
        tp.time += sec;
        tp.millitm += millisec;

        if (tp.millitm > 999)
        {
            tp.millitm -= 1000;
            tp.time++;
        }

        tm.tv_sec = tp.time;
        tm.tv_nsec = tp.millitm * 1000000;

        while (semaphore_handle->m_semaphore_count <= 0)
        {
            rc = pthread_cond_timedwait(&semaphore_handle->m_condition, &semaphore_handle->m_mutex, &tm);

            if ((0 != rc) && (errno != EINTR))
                break;
        }

        if (0 != rc)
        {
            pthread_mutex_unlock(&semaphore_handle->m_mutex);
            if (rc == ETIMEDOUT)
                return WAIT_TIMEOUT;

            return WAIT_ERROR;
        }
        semaphore_handle->m_semaphore_count--;
    }
    pthread_mutex_unlock(&semaphore_handle->m_mutex);

    return WAIT_OBJECT_0;
}

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
    mpp_work_item = new PThreadWorkItem*[m_num_work_item];

    for (int i = 0; i < m_num_work_item; i++)
    {
        mpp_work_item[i] = NULL;
    }
    m_head_index = 0;
    m_tail_index = 0;
    m_work_count = 0;
}

bool WorkItemQueue::Push(PThreadWorkItem *p_work_item)
{
    if (m_work_count >= m_num_work_item)
        return false;

    mpp_work_item[m_tail_index] = p_work_item;
    m_tail_index++;
    m_work_count++;

    if (m_tail_index >= m_num_work_item)
        m_tail_index = 0;

    return true;
}

bool WorkItemQueue::Pop(PThreadWorkItem **pp_work_item)
{
    if (m_work_count <= 0)
        return false;

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

PThreadPool::PThreadPool()
    : m_num_thread(0)
    , m_num_work_item(0)
    , m_avaliable_work_item(NULL)
    , m_avaliable_work_item_slot(NULL)
    , m_kill_me(false)
    , mp_thread(NULL)
{}

PThreadPool::~PThreadPool()
{
    FreeThread();
}

void PThreadPool::CreateThread(int num_thread /*= 0*/, int num_work_item /*= 0*/)
{
    FreeThread();


    if (num_thread > 0)
        m_num_thread = num_thread;
    else
        m_num_thread = GetLogicalCPUCount();

    if (num_work_item > 0)
        m_num_work_item = num_work_item;
    else
        m_num_work_item = m_num_thread * 2;

    m_kill_me = false;
    m_avaliable_work_item = CreateSemaphore(NULL, 0, m_num_work_item, NULL);
    m_avaliable_work_item_slot = CreateSemaphore(NULL, m_num_work_item, m_num_work_item, NULL);

    mp_thread = new pthread_t[m_num_thread];

    for (int i = 0; i < m_num_thread; i++)
    {
        pthread_create(&mp_thread[i], 0, PThreadPool::ThreadProcess, (void*)this);
    }

    m_work_item_queue.Initialize(m_num_work_item);
}

bool PThreadPool::AddWorkItem(PThreadWorkItem &work_item, int time_out)
{
    PSyncObjectWaitResult result = WaitForPSemaphore(m_avaliable_work_item_slot, time_out);
    if (result == WAIT_OBJECT_0)
    {
        {
            PAutoLock work_item_queue_lock(&m_work_item_queue_section);
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
        return false;
    }
}

bool PThreadPool::GetWorkItem(PThreadWorkItem **pp_work_item)
{
    WaitForPSemaphore(m_avaliable_work_item, INFINITE);

    {
        {
            PAutoLock work_item_queue_lock(&m_work_item_queue_section);
            if (!m_work_item_queue.Pop(pp_work_item))
                return false;
        }
        ReleaseSemaphore(m_avaliable_work_item_slot, 1, NULL);
        return true;
    }
}

void PThreadPool::FreeThread()
{
    if (mp_thread != NULL)
    {
        m_kill_me = true;
        ReleaseSemaphore(m_avaliable_work_item, m_num_thread, NULL);

        for (int i = 0; i < m_num_thread; i++)
        {
             pthread_join(mp_thread[i], 0);
        }

        m_kill_me = false;

        delete [] mp_thread;
        mp_thread = NULL;
    }

    if (m_avaliable_work_item != NULL)
    {
        CloseHandle(m_avaliable_work_item);
        m_avaliable_work_item = NULL;
    }

    if (m_avaliable_work_item_slot != NULL)
    {
        CloseHandle(m_avaliable_work_item_slot);
        m_avaliable_work_item_slot = NULL;
    }
}

void *PThreadPool::ThreadProcess(void *p_param)
{
    PThreadPool *p_thread_pool = (PThreadPool*)p_param;
    PThreadWorkItem *p_work_item;

    for (;;)
    {
        if (p_thread_pool->TimeToDie())
            break;

        if (p_thread_pool->GetWorkItem(&p_work_item))
        {
            p_work_item->mp_function(p_work_item->mp_param);
            SetPEvent(p_work_item->m_event_complete);
        }
    }
    return 0;
}

PThreadControlShell::PThreadControlShell()
    : mp_thread_pool(NULL)
    , mp_internal_thread_pool(NULL)
{}

PThreadControlShell::~PThreadControlShell()
{
    if (mp_internal_thread_pool != NULL)
        delete mp_internal_thread_pool;
    if (m_work_item.m_event_complete != NULL)
        ClosePEvent(m_work_item.m_event_complete);
}

void PThreadControlShell::CreateThreadRun(ThreadFunctionReturnType (*p_start_address)(void *), void *p_parameter)
{
    m_work_item.mp_function = p_start_address;
    m_work_item.mp_param = p_parameter;

    if (m_work_item.m_event_complete != NULL)
        ClosePEvent(m_work_item.m_event_complete);

    m_work_item.m_event_complete = CreatePEvent(false, false);
}

void PThreadControlShell::SetThreadPool(PThreadPool *p_thread_pool)
{
    mp_thread_pool = p_thread_pool;
}

void PThreadControlShell::SignalBegin()
{
    // enable creating thread pool if mp_thread_pool is null
    if (mp_thread_pool == NULL)
    {
        mp_internal_thread_pool = new PThreadPool;
        // Create one thread for each ThreadControl
        mp_internal_thread_pool->CreateThread(1, 1);
        mp_thread_pool = mp_internal_thread_pool;
    }

    mp_thread_pool->AddWorkItem(m_work_item);
}

void PThreadControlShell::WaitComplete(int time_out)
{
    WaitForPEvent(m_work_item.m_event_complete, time_out);
}
