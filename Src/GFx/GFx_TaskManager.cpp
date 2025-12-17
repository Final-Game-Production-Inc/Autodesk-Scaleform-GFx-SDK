/**************************************************************************

Filename    :   GFx_TaskManager.cpp
Content     :   Threaded task manager implementatoin
Created     :   May 31, 2007
Authors     :   Michael Antonov

Notes       :   A Task is a unit of work that can be executed
                on several threads.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_TaskManager.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_Debug.h"

#ifdef SF_ENABLE_THREADS

namespace Scaleform { namespace GFx {

// ***** TasksContainer

// This class is used to keep track of currently running tasks by a task manager.
// and it is needed for TaskManager to send an abandon signal for running tasks.
class TasksContainer
{
public:
    TasksContainer(Mutex* ptaskMutex) : pTasksMutex(ptaskMutex) {}
    ~TasksContainer() 
    {
        AbandonAllTask();
    }

    void AddTask(Task* ptask) 
    {
        SF_ASSERT(ptask);
        if (!ptask) return;
        Mutex::Locker guard(pTasksMutex);
        Tasks.PushBack(ptask);
    }
    bool RemoveTask(Task* ptask)
    {
        SF_ASSERT(ptask);
        if (!ptask) return false;
        Mutex::Locker guard(pTasksMutex);
        for(unsigned i=0; i<Tasks.GetSize(); i++)
        {
            if (Tasks[i] == ptask)
            {
                Tasks.RemoveAt(i);
                return true;
            }
        }
        return false;
    }
    bool FindTask(Task* ptask)
    {
        SF_ASSERT(ptask);
        if (!ptask) return false;
        Mutex::Locker guard(pTasksMutex);
        for(unsigned i=0; i<Tasks.GetSize(); i++)
        {
            if (Tasks[i] == ptask)
                return true;
        }
        return false;
    }
    bool AbandonTask(Task* ptask)
    {
        SF_ASSERT(ptask);
        if (!ptask) return false;
        Mutex::Locker guard(pTasksMutex);
        for(unsigned i=0; i<Tasks.GetSize(); i++)
        {
            if (Tasks[i] == ptask)
            {
                Tasks[i]->OnAbandon(true);
                return true;
            }
        }
        return false;
    }
    void AbandonAllTask() 
    {
        Mutex::Locker guard(pTasksMutex);
        for(unsigned i=0; i<Tasks.GetSize(); i++)
            Tasks[i]->OnAbandon(true);
    }
private:
    Mutex*             pTasksMutex;
    Array<Task*>    Tasks;
};

// ***** GFxThreadTaskManagerImpl

// Threaded Task Manager is divided in two part to avoid circular dependences. 
// a task can created another tasks and can hold a strong pointer to ThreadedTaskManager
// a task thread also need task manager so it holds a strong pointer to ThreadedTaskManagerImpl
class TaskThreadPool;
class ThreadedTaskManagerImpl: public RefCountBase<ThreadedTaskManagerImpl, Stat_Default_Mem>
{
public:
    ThreadedTaskManagerImpl();
    ~ThreadedTaskManagerImpl();

    bool               AddWorkerThreads(unsigned taskMask, unsigned count, UPInt stackSize, int processor = -1);
    TasksContainer* GetRunningTasks () { return &RunningTasks; }
    TaskThreadPool* GetThreadPool   ()   { return pThreadPool; }
    bool               AbandonTask     (Task* ptask);

    void               RequestShutdown ();
private:
    friend class TaskThreadPool;
    Mutex             TasksMutex;
    TasksContainer  RunningTasks;
    TaskThreadPool* pThreadPool;
};

// ***** TaskThread

// an object of this class execute a tasks in a separate thread
// upon task completion the thread object gets delete

class TaskThread : public Thread
{
public:
    TaskThread(Task* ptask, ThreadedTaskManagerImpl* ptm, UPInt stackSize = 128 * 1024, int processor = -1)
        : Thread(stackSize,processor), pTask(ptask), pTaskManager(ptm)
    { 
    //    printf("TaskThread::TaskThread : %x, thread: %d\n", this, GetCurrentThreadId()); 
    }
    ~TaskThread();

    virtual int  Run();
protected:
    Ptr<Task>                    pTask;
    Ptr<ThreadedTaskManagerImpl> pTaskManager;
};

// ***** TaskThreadInPool

// objects of this call should be added into a thread pool. 
// after tasks completion thread stays in the pool waiting 
// for anther task

class TaskThreadInPool : public TaskThread
{
public:
    TaskThreadInPool(unsigned taskMask, ThreadedTaskManagerImpl* ptm, UPInt stackSize = 128 * 1024, int processor = -1)
       : TaskThread(0, ptm, stackSize, processor), TaskMask(taskMask)
    {}
    ~TaskThreadInPool() {}

    bool         SetTask(Task* ptask);
    virtual int  Run();

    unsigned     GetTaskType() const { return TaskMask; }

private:
    unsigned TaskMask;    
};

// ***** TaskThreadPool

// Threaded pool implementation
class TaskThreadPool: public NewOverrideBase<Stat_Default_Mem>
{
public:
    TaskThreadPool(ThreadedTaskManagerImpl* ptm);
    ~TaskThreadPool();

    bool     AddTask      (Task* ptask);
    bool     AbandonTask  (Task* ptask);
    bool     AddThreads   (unsigned taskMask, unsigned count, UPInt stackSize, int processor = -1);
    bool     RemoveThread (TaskThread*);

    // adds a task's ref count and returns it from the tasks queue. 
    // if there is no task in the queue waits on a condition variable until 
    // queue is not empty. Returns 0 if shutdown is requested. 
    Task* GetTaskAddRef(unsigned taskMask);

    // instruct the pool to stop accepting new task and send abandon signal 
    // for all tasks (in the task queue and currently running)
    void     RequestShutdown();

private:
    Task* FindTaskByMask(unsigned taskMask);

    Array<Ptr<Task> >          TasksQueue;
    Array<TaskThreadInPool*>    Threads;
    Lock                           ThreadsLock;
    WaitCondition                  TaskWaitCondition;
    volatile bool                   ShutdownRequested;
    ThreadedTaskManagerImpl*     pTaskManager;
};

/******************************************************************/
ThreadedTaskManagerImpl::ThreadedTaskManagerImpl()
    : RunningTasks(&TasksMutex), pThreadPool(0)
{
    //printf("ThreadedTaskManagerImpl::ThreadedTaskManagerImpl : %x, thread : %d\n", this, GetCurrentThreadId());    
}

ThreadedTaskManagerImpl::~ThreadedTaskManagerImpl()
{
    //printf("ThreadedTaskManagerImpl::~ThreadedTaskManagerImpl : %x, thread : %d\n", this,GetCurrentThreadId());
    RunningTasks.AbandonAllTask();
    delete pThreadPool;
}

bool ThreadedTaskManagerImpl::AddWorkerThreads(unsigned taskMask, unsigned count, UPInt stackSize, int processor)
{
    if (!pThreadPool)
        pThreadPool = new TaskThreadPool(this);
    return pThreadPool->AddThreads(taskMask,count,stackSize,processor);
}

void ThreadedTaskManagerImpl::RequestShutdown() 
{ 
    RunningTasks.AbandonAllTask();
    if (pThreadPool) pThreadPool->RequestShutdown(); 
}

bool ThreadedTaskManagerImpl::AbandonTask(Task* ptask)
{
    if (pThreadPool) 
        return pThreadPool->AbandonTask(ptask);
    return RunningTasks.AbandonTask(ptask);
}

/******************************************************************/
TaskThread::~TaskThread()
{
    //printf("TaskThread::TaskThread : %x, thread : %d\n", this, GetCurrentThreadId());
    if (pTask)
    {
        pTask->OnAbandon(true);
        pTaskManager->GetRunningTasks()->RemoveTask(pTask);
    }
    TaskThreadPool* pool = pTaskManager->GetThreadPool();
    if (pool) pool->RemoveThread(this);
}

int  TaskThread::Run()
{
    SF_ASSERT(pTask);
    if (pTask) 
    {
        //pTaskManager->GetRunningTasks()->AddTask(pTask);
        pTask->Execute();        
        pTaskManager->GetRunningTasks()->RemoveTask(pTask);
        pTask = 0; // Release task's resources.
    }
    return 1;
}

bool TaskThreadInPool::SetTask(Task* ptask) 
{
    SF_ASSERT(!pTask);
    SF_ASSERT(ptask);
    if (pTask) return false;
    pTask = ptask;
    return true;
}

int  TaskThreadInPool::Run()
{   
    Ptr<Task> ptask;
    while ((ptask = *pTaskManager->GetThreadPool()->GetTaskAddRef(TaskMask)))
    {
        SetTask(ptask);
        TaskThread::Run();
        ptask = 0; // if ptask is not release here it would lead to memory leak.
    }
    return 1;
}

/******************************************************************/
TaskThreadPool::TaskThreadPool(ThreadedTaskManagerImpl* ptm) 
    : ShutdownRequested(false), pTaskManager(ptm)
{
    SF_ASSERT(pTaskManager);
}

TaskThreadPool::~TaskThreadPool() 
{
    RequestShutdown();
    for (unsigned i=0; i<Threads.GetSize(); i++)
        Threads[i]->Wait();
}

bool TaskThreadPool::AddTask(Task* ptask) 
{
    SF_ASSERT(ptask);
    if (ShutdownRequested) 
        return false;
    {
        Lock::Locker guard(&ThreadsLock);
        unsigned i;
        for (i=0; i<Threads.GetSize(); i++)
        {
            if ((unsigned)ptask->GetTaskType() == Threads[i]->GetTaskType())
                break;
        }
        SF_DEBUG_ERROR(i == Threads.GetSize(), "There is no a thread in thread pool which can process a given task");
        if (i == Threads.GetSize())
            return false;
    }
    Mutex::Locker guard(&pTaskManager->TasksMutex);
    TasksQueue.PushBack(ptask);
    TaskWaitCondition.NotifyAll();
    return true;
}


bool TaskThreadPool::AddThreads(unsigned taskMask, unsigned count, UPInt stackSize, int processor)
{
    if (ShutdownRequested)
        return false;
    // TODO: it may be useful to check and give an warring if stackSize is deferent from the one
    // that has already been set for threads with a given taskMask
    Lock::Locker guard(&ThreadsLock);
    while (count-- > 0)
    {
        Ptr<TaskThreadInPool> thread = *new TaskThreadInPool(taskMask, pTaskManager, stackSize, processor);
        thread->Start();
        Threads.PushBack(thread);
    }
    return true;
}
bool TaskThreadPool::RemoveThread(TaskThread* thread)
{
    Lock::Locker guard(&ThreadsLock);
    for (unsigned i=0; i<Threads.GetSize(); i++)
    {
        if (Threads[i] == thread) 
        {
            Threads.RemoveAt(i);
            return true;
        }
    }
    return false;
}

Task* TaskThreadPool::GetTaskAddRef(unsigned taskMask)
{
    if (ShutdownRequested)
        return 0;
    Mutex::Locker guard(&pTaskManager->TasksMutex);
    Task* ptask = 0;
    while (!ShutdownRequested && (ptask = FindTaskByMask(taskMask)) == 0)
        TaskWaitCondition.Wait(&pTaskManager->TasksMutex);
    if (ptask)
        pTaskManager->GetRunningTasks()->AddTask(ptask);
    return ptask;
}

Task* TaskThreadPool::FindTaskByMask(unsigned taskMask)
{
    Task *ptask = 0;
    for (unsigned i=0; i<TasksQueue.GetSize(); i++)
    {
        if (TasksQueue[i]->GetTaskType() & taskMask)
        {
            ptask = TasksQueue[i];
            ptask->AddRef();
            TasksQueue.RemoveAt(i);
            break;
        }
    }
    return ptask;
}

void TaskThreadPool::RequestShutdown() 
{ 
    Mutex::Locker guard(&pTaskManager->TasksMutex);
    if (ShutdownRequested)
        return;
    ShutdownRequested = true;
    for (unsigned i=0; i<TasksQueue.GetSize(); i++)
        TasksQueue[i]->OnAbandon(false);
    TasksQueue.Clear();
    TaskWaitCondition.NotifyAll();
}

bool TaskThreadPool::AbandonTask(Task* ptask)
{
    SF_ASSERT(ptask);
    if (!ptask) return false;
    Mutex::Locker guard(&pTaskManager->TasksMutex);
    //fist check in the task queue
    for (unsigned i=0; i<TasksQueue.GetSize(); i++)
    {
        if (TasksQueue[i] == ptask)
        {
            TasksQueue[i]->OnAbandon(false);
            TasksQueue.RemoveAt(i);
            return true;
        }
    }
    // if did not find there try running tasks
    return pTaskManager->GetRunningTasks()->AbandonTask(ptask);
}


/******************************************************************/
ThreadedTaskManager::ThreadedTaskManager(UPInt stackSize) 
    : ThreadStackSize(stackSize)
{
    //printf("ThreadedTaskManager::ThreadedTaskManager : %x, thread: %d\n", this, GetCurrentThreadId());
    pImpl = new ThreadedTaskManagerImpl;
}

ThreadedTaskManager::~ThreadedTaskManager()
{
    //printf("ThreadedTaskManager::~ThreadedTaskManager : %x, thread: %d\n", this, GetCurrentThreadId());
    if (pImpl) 
    {
        pImpl->RequestShutdown();
        pImpl->Release();
    }
}

bool ThreadedTaskManager::AddTask(Task* ptask)
{    
    SF_ASSERT(ptask != 0);
    if (!ptask) return false;
    TaskThreadPool* pthreadPool = pImpl->GetThreadPool();
    if (pthreadPool && pthreadPool->AddTask(ptask)) 
        return true;
    
    Ptr<TaskThread> ptaskThread = *new TaskThread(ptask, pImpl, ThreadStackSize);
    if (ptaskThread)
    {        
        pImpl->GetRunningTasks()->AddTask(ptask);
        ptaskThread->Start();
        // Thread will release itself once the task is finished.
        return true;
    }
    return false;
}

bool ThreadedTaskManager::AbandonTask(Task* ptask)
{
    return pImpl->AbandonTask(ptask);
}

bool ThreadedTaskManager::AddWorkerThreads(unsigned taskMask, unsigned count, UPInt stackSize, int processor)
{
    return pImpl->AddWorkerThreads(taskMask,count,stackSize,processor);
}
void ThreadedTaskManager::RequestShutdown()
{
    pImpl->RequestShutdown();
}

}} // namespace Scaleform { namespace GFx {

#endif
