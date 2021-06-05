#ifndef PTHREADTOOL_H_
#define PTHREADTOOL_H_

#include <pthread.h>
#include "Common.h"

const int INFINITE = -1;
enum PSyncObjectWaitResult
{
  WAIT_ERROR    = -1,
  WAIT_OBJECT_0 =  0,
  WAIT_TIMEOUT  =  1
};

// wrapper for whatever critical section we have
class PCritSec
{
    // make copy constructor and assignment operator inaccessible
    PCritSec(const PCritSec &refCritSec);
    PCritSec &operator=(const PCritSec &refCritSec);

    pthread_mutex_t m_mutex_section;

public:
    PCritSec()
    {
        pthread_mutex_init(&m_mutex_section, 0);
    };

    ~PCritSec()
    {
        pthread_mutex_destroy(&m_mutex_section);
    };

    void Lock()
    {
        pthread_mutex_lock(&m_mutex_section);
    };

    void Unlock()
    {
        pthread_mutex_unlock(&m_mutex_section);
    };

    bool TryLock()
    {
        // pthread_mutex_trylock will return 0, if luck successful.
        return (pthread_mutex_trylock(&m_mutex_section) == 0);
    };
};

// locks a critical section, and unlocks it automatically
// when the lock goes out of scope
class PAutoLock
{
    // make copy constructor and assignment operator inaccessible
    PAutoLock(const PAutoLock &refAutoLock);
    PAutoLock &operator=(const PAutoLock &refAutoLock);

protected:
    PCritSec *m_pLock;

public:
    PAutoLock(PCritSec *plock)
    {
        m_pLock = plock;
        m_pLock->Lock();
    };

    ~PAutoLock()
    {
        m_pLock->Unlock();
    };
};

class PEvent
{
public:
    PEvent(bool manual_reset, bool initial_status)
    {
        m_manual_reset = manual_reset;
        m_triggered = initial_status;
        pthread_cond_init(&m_condition, 0);
        pthread_mutex_init(&m_mutex, 0);
    }

    ~PEvent()
    {
        pthread_cond_destroy(&m_condition);
        pthread_mutex_destroy(&m_mutex);
    }

    bool m_triggered;
    bool m_manual_reset;
    pthread_cond_t m_condition;
    pthread_mutex_t m_mutex;
};

typedef PEvent* PEventHandle;

PEventHandle CreatePEvent(bool manual_reset, bool initial_status);
void ClosePEvent(PEventHandle event_handle);
void SetPEvent(PEventHandle event_handle);
void ResetPEvent(PEventHandle event_handle);
PSyncObjectWaitResult WaitForPEvent(PEventHandle event_handle, int time_out = INFINITE);
PSyncObjectWaitResult WaitForSingleObject(PEventHandle event_handle, int time_out = INFINITE);

inline HANDLE CreateEvent(void *p_event_attributes, bool manual_reset, bool initial_status, void *p_event_name)
{
    return (HANDLE)CreatePEvent(manual_reset, initial_status);
}
inline void CloseHandle(HANDLE event) { ClosePEvent((PEventHandle)event);}
inline void SetEvent(HANDLE event) { SetPEvent((PEventHandle)event);}
inline void ResetEvent(HANDLE event) { ResetPEvent((PEventHandle)event);}
inline PSyncObjectWaitResult WaitForEvent(HANDLE event, int time_out = INFINITE) { return WaitForPEvent((PEventHandle)event, time_out);}
inline PSyncObjectWaitResult WaitForSingleObject(HANDLE event_handle, int time_out = INFINITE)
{
    return WaitForSingleObject((PEventHandle)event_handle, time_out);
}

class PThreadControl
{
    pthread_cond_t m_condition_begin;
    pthread_cond_t m_condition_complete;
    bool m_begin_triggered;
    pthread_mutex_t m_mutex_begin;
    pthread_mutex_t m_mutex_complete;
    bool m_complete_triggered;

    bool m_kill_me;
    pthread_t m_thread;

    bool m_thread_created;

public:
    PThreadControl();
    virtual ~PThreadControl();

    int CreateThreadRun(void *(*start_routine)(void *), void *argument);
    int CreateThread_Run(void *(*start_routine)(void *), void *argument) 
    { 
        return CreateThreadRun(start_routine, argument); 
    }
    void CloseThread();

    void SignalBegin()
    {
        pthread_mutex_lock(&m_mutex_begin);
        m_begin_triggered = true;
        pthread_cond_signal(&m_condition_begin);
        pthread_mutex_unlock(&m_mutex_begin);
    }

    void WaitBegin()
    {
        pthread_mutex_lock(&m_mutex_begin);
        while(false == m_begin_triggered)
            pthread_cond_wait(&m_condition_begin, &m_mutex_begin);
        m_begin_triggered = false; //auto-reset
        pthread_mutex_unlock(&m_mutex_begin);
    }

    void SignalComplete()
    {
        pthread_mutex_lock(&m_mutex_complete);
        m_complete_triggered = true;
        pthread_cond_signal(&m_condition_complete);
        pthread_mutex_unlock(&m_mutex_complete);
    }

    void SignalEnd() { SignalComplete(); }

    void WaitComplete()
    {
        pthread_mutex_lock(&m_mutex_complete);
        while(false == m_complete_triggered)
            pthread_cond_wait(&m_condition_complete, &m_mutex_complete);
        m_complete_triggered = false; //auto-reset
        pthread_mutex_unlock(&m_mutex_complete);
    }

    void WaitComplete(int time_out_ms);

    bool TimeToDie()
    {
        return m_kill_me;
    }

    bool SetPriority(int priority) 
    {
        int original_policy;
        struct sched_param param;
        if (pthread_getschedparam(m_thread, &original_policy, &param) != 0) return false;

        int max_priority = sched_get_priority_max(original_policy);
        int min_priority = sched_get_priority_min(original_policy);

        if (max_priority < 0 || min_priority < 0) return false;
        float priority_granularity = (max_priority > min_priority) 
                                     ? (float)(max_priority - min_priority) / 4
                                     : 0.0f;

        switch (priority)
        {
            case THREAD_PRIORITY_IDLE: // ignore IDLE and set to lowest
            case THREAD_PRIORITY_LOWEST:
                param.sched_priority = min_priority;
                break;

            case THREAD_PRIORITY_BELOW_NORMAL:
                param.sched_priority = ch_Round(min_priority + priority_granularity * 1);
                break;

            case THREAD_PRIORITY_NORMAL:
                param.sched_priority = ch_Round(min_priority + priority_granularity * 2);
                break;

            case THREAD_PRIORITY_ABOVE_NORMAL:
                param.sched_priority = ch_Round(min_priority + priority_granularity * 3);
                break;

            case THREAD_PRIORITY_HIGHEST:
                param.sched_priority = max_priority;
                break;

            default:
                break;
        }

        if (pthread_setschedparam(m_thread, SCHED_OTHER, &param) != 0) return false;

        return true;
    }

    void ResetEnd()
    {
        pthread_mutex_lock(&m_mutex_complete);
        m_complete_triggered = false;
        pthread_mutex_unlock(&m_mutex_complete);
    }
};

// AutoTryLock is the none blocking auto lock,
// which will try to Lock "CritSec2" and use "IsLocked" to check is "Lock" success or not
class PAutoTryLock
{
    // make copy constructor and assignment operator inaccessible
    PAutoTryLock(const PAutoTryLock &ref_auto_lock);
    PAutoTryLock &operator=(const PAutoTryLock &ref_auto_lock);

protected:
    PCritSec *mp_lock;
    BOOL m_is_locked;

public:
    PAutoTryLock(PCritSec *p_lock)
    {
        _MYASSERT(p_lock);
        mp_lock = p_lock;
        m_is_locked = mp_lock->TryLock();
    };

    ~PAutoTryLock()
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

class PSemaphore
{
public:
    PSemaphore(long initial_count, long max_count)
    {
        m_semaphore_count = initial_count;
        m_max_count = max_count;
        pthread_cond_init(&m_condition, 0);
        pthread_mutex_init(&m_mutex, 0);
    }

    ~PSemaphore()
    {
        pthread_cond_destroy(&m_condition);
        pthread_mutex_destroy(&m_mutex);
    }

    long m_semaphore_count;
    long m_max_count;
    pthread_cond_t m_condition;
    pthread_mutex_t m_mutex;
};

typedef PSemaphore* PSemaphoreHandle;
PSemaphoreHandle CreateSemaphore(void *p_semaphore_attribute, long initial_count, long max_count, void *p_name);
void CloseHandle(PSemaphoreHandle semaphore_handle);
void ReleaseSemaphore(PSemaphoreHandle semaphore_handle, long release_count, long *previous_count);
PSyncObjectWaitResult WaitForPSemaphore(PSemaphoreHandle semaphore_handle, int time_out = INFINITE);

struct PThreadWorkItem
{
    void *mp_param;
    ThreadFunctionReturnType (*mp_function)(void *);
    PEventHandle m_event_complete;
    PThreadWorkItem()
    {
        mp_param = NULL;
        mp_function = NULL;
        m_event_complete = NULL;
    }
};

class WorkItemQueue
{
public:
    WorkItemQueue();
    ~WorkItemQueue();
    void Initialize(int num_work_item);
    bool Push(PThreadWorkItem *p_work_item);
    bool Pop(PThreadWorkItem **pp_work_item);

private:
    int m_head_index;
    int m_tail_index;
    int m_work_count;
    int m_num_work_item;
    PThreadWorkItem **mpp_work_item;

    void FreeQueue();
};

class PThreadPool
{
public:
    PThreadPool();
    ~PThreadPool();
    void CreateThread(int num_thread = 0, int num_work_item = 0);
    bool AddWorkItem(PThreadWorkItem &work_item, int time_out = INFINITE);
    void FreeThread();
    int GetThreadNumber()
    {
        return m_num_thread;
    }
    bool IsThreadCreated()
    {
        return (m_num_thread != 0);
    }

private:
    int m_num_thread;
    int m_num_work_item;

    PSemaphoreHandle m_avaliable_work_item;
    PSemaphoreHandle m_avaliable_work_item_slot;

    WorkItemQueue m_work_item_queue;

    bool m_kill_me;
    pthread_t *mp_thread;
    PCritSec m_work_item_queue_section;

    bool GetWorkItem(PThreadWorkItem **pp_work_item);

    bool TimeToDie()
    {
        return m_kill_me;
    }
    static ThreadFunctionReturnType ThreadProcess(void *p_param);
};

class PThreadControlShell
{
public:
    PThreadControlShell();
    ~PThreadControlShell();

    void CreateThreadRun(ThreadFunctionReturnType (*p_start_address)(void *), void *p_parameter);
    void CreateThread_Run(ThreadFunctionReturnType (*p_start_address)(void *), void *p_parameter)
    {
        CreateThreadRun(p_start_address, p_parameter);
    }
    void SetThreadPool(PThreadPool *p_thread_pool);
    void SignalBegin();
    void WaitComplete(int time_out = INFINITE);

private:
    PThreadWorkItem m_work_item;
    PThreadPool *mp_thread_pool;
    PThreadPool *mp_internal_thread_pool;
};

#ifdef UNIX_OS
typedef PThreadControl CThreadControl;
typedef PCritSec ChCritSec;
typedef PAutoLock ChAutoLock;
typedef PAutoTryLock ChAutoTryLock;
typedef PThreadPool ThreadPool;
typedef PThreadControlShell ThreadControlShell;
#endif

#endif /* PTHREADTOOL_H_ */
