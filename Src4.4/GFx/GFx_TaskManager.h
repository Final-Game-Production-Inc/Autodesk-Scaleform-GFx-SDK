/**************************************************************************

PublicHeader:   None
Filename    :   GFx_TaskManager.h
Content     :   Defines a Task and threaded TaskManager
Created     :   May 31, 2007
Authors     :   Michael Antonov

Notes       :   A Task is a unit of work that can be executed
                on several threads.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_TaskManager_H
#define INC_SF_GFX_TaskManager_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
// Include loader because task manager is a state.
#include "GFx/GFx_Loader.h"

namespace Scaleform { namespace GFx {

// ***** Task

// Task describes a task that can be queued up for potential execution
// on a different thread. Users can create new tasks by overriding this
// class and implementing its Execute virtual function.
// Tasks are added for execution by a call to FxTaskManager::AddTask, at which
// point they become pending execution. Task objects will be AddRefed
// by the task manager and then by the container thread; once the task
// has completed it will be released.

class Task : public RefCountBase<Task, Stat_Default_Mem>
{
public:

    // TaskType classifies the kind of work that task represents.
    // Type bits are stores as a part of task id and are designed to
    // allow for intelligent distribution of tasks to dedicated threads.
    enum TaskType
    {
        // General computation that needs to be carried out on data,
        // it will not block on any resources.
        Type_Computation    = 0x00010000,
        // I/O task that can block based on availability of data
        // in its input file stream(s).
        Type_IO             = 0x00020000,        
        // We may want to add SPU - oriented processing tasks in the future.       
        Type_Mask           = 0x00FF0000,
    };

    // Defines a constant for every type of task we can queue up;
    // these should include the TaskType bits.
    enum TaskId
    {
        Id_Unknown          = Type_Computation | 1,
        Id_MovieDecoding    = Type_Computation | 2,
        // Right now we make use of IO related tasks only.
        Id_MovieDataLoad    = Type_IO | 1,
        Id_MovieImageLoad   = Type_IO | 2,
        Id_MovieBind        = Type_IO | 3,
    };

    enum TaskState
    {
        State_Idle,        // Task is idle before it has been added to task manager.
        State_Pending,     // Task becomes pending once it is added to task manager.
        State_Running,     // Task is running - while being executed.
        State_Abandoned,   // Task is abandoned if AbandonTask was called while 
                           // it is in task manager.
        State_Finished,
    };


protected:
    TaskId               ThisTaskId;
    volatile TaskState   CurrentState;
public:

    // Creates a task initializing it with the correct id.
    Task(TaskId id = Id_Unknown)
        : ThisTaskId(id), CurrentState(State_Idle)
    { }

    virtual ~Task() { }
    
    // Obtains Id describing this task.    
    inline TaskId      GetTaskId() const    { return ThisTaskId; }
    inline TaskType    GetTaskType() const  { return (TaskType)(GetTaskId() & Type_Mask); }
    inline TaskState   GetTaskState() const { return CurrentState; }


    // *** Task Virual Overrides

    // Override Execute function to perform the main work of the task.
    virtual void    Execute()  = 0;

    // Override OnAbandon to detect when the task was removed from the
    // task manager by a call to AbandonTask. This call can take place
    // under two circumstances:
    //  - If the task wan't started yet, in this case started will be
    //    false. If task controls synchronization resources waited on
    //    by any threads, it should signal them with abandon code.
    //  - If the task was started, it can choose to either follow a
    //    custom cancel work protocol, or just run to completion.
    virtual void    OnAbandon(bool started) { SF_UNUSED(started); }
};


// ***** TaskManager

// Task manager is an abstract class that can be implemented as a thread
// pool or a task queue. Tasks that need to be executed are added to the
// manager by calls to AddTask.

class TaskManager : public State
{
public:
    TaskManager() : State(State_TaskManager)   
    { }

    // Task manager destructor should be responsible for abandoning
    // all of the tasks in the queue.
    virtual ~TaskManager() { }

    // Adds a task for execution; the manager is responsible
    // for executing a task and keeping a reference count on it
    // until it has completed or was abandoned.
    virtual bool AddTask(Task* ptask)     = 0;

    // Instructs the task manager to abandon the task that was added
    // earlier. This function will abandon the task if it was not
    // yet dispatched for execution and return true. If task was already dispatched for
    // execution or never added to the manager, the method should return false.
    // Implementation of OnAbandon will also call ptask->OnAbandon
    // with an appropriate argument.    
    virtual bool AbandonTask(Task* ptask) = 0;

    // Instructs the task manager to stop accepting any new task and
    // call OnAbandon method for all currently running tasks
    virtual void RequestShutdown() = 0;
};


// GFxStateBag state access for TaskManager
inline  void  StateBag::SetTaskManager(TaskManager *ptr)
{
    SetState(State::State_TaskManager, ptr);
}
inline  Ptr<TaskManager> StateBag::GetTaskManager() const
{
    return *(TaskManager*) GetStateAddRef(State::State_TaskManager);
}

// ***** ThreadedTaskManager
#ifdef SF_ENABLE_THREADS

// ***** GFxThreadTaskManager

// Implementation of TaskManager interface based on thread pool
// Worker threads for a particular task type can be add by call to AddWorkerThreads
// method. 
// Then a task is added to the task manager:
//   1. if no worker threads with a given task type were added to the manager then 
//      a new temporary thread is created for running only this task. 
//   2. if there are worker nodes for a give task in the thread pool but all of them
//      are busy with running other tasks then this task will be added to the task queue

class ThreadedTaskManagerImpl;
class ThreadedTaskManager : public TaskManager
{
public:
    // Constructs a task manager and specifies thread
    // stack size required for each task.
    ThreadedTaskManager(UPInt stackSize = 128 * 1024);    
    ~ThreadedTaskManager();

    // Adds a specified number of worker threads working on a given processor.
    // If this is not called, threads will be created for each task.
    bool AddWorkerThreads(unsigned taskMask, unsigned count, 
                          UPInt stackSize = 128 * 1024, int processor = -1);

    // *** Task Manager implementation
    bool AddTask    (Task* ptask);   
    bool AbandonTask(Task* ptask);
    void RequestShutdown();

private:
    ThreadedTaskManagerImpl*    pImpl;
    UPInt                       ThreadStackSize;
};

#endif // SF_ENABLE_THREADS

}} // Scaleform::GFx

#endif
