/**************************************************************************

Filename    :   FxGameDelegate.h
Content     :   Communication logic for CLIK GameDelegate
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxGameDelegateHandler_H
#define INC_FxGameDelegateHandler_H

#include "GFx/GFx_Player.h"
#include "Kernel/SF_Memory.h"
#include "Kernel/SF_Debug.h"

class FxDelegate;
class FxDelegateArgs;

namespace SF = Scaleform;
using namespace Scaleform;
using namespace GFx;

//
// Interface implemented by all callback handlers. These handlers register 
// callbacks with the FxGameDelegate.
//
class FxDelegateHandler : public RefCountBase<FxDelegateHandler, Stat_Default_Mem>
{
public:
    virtual ~FxDelegateHandler() {}

    //
    // All callback methods must have the following signature. To produce a response
    // to the callback, push parameters to the game delegate.
    //
    typedef void (*CallbackFn)(const FxDelegateArgs& params);

    //
    // Interface implemented by callback registrars. The handler should 
    // pass the appropriate parameters to the Visit method.
    //
    class CallbackProcessor
    {
    public:
        virtual ~CallbackProcessor() {}
        virtual void    Process(const String& methodName, CallbackFn method) = 0;
    };

    //
    // Callback registrar visitor method
    // Implementations are expected to call the registrar's Process method
    // for all callbacks.
    //
    virtual void        Accept(CallbackProcessor* cbreg) = 0;
};


//////////////////////////////////////////////////////////////////////////


//
// Callback response parameters
// The 
//
class FxResponseArgsBase
{
public:
    virtual ~FxResponseArgsBase() {}
    virtual unsigned GetValues(Value** pparams) = 0;
};

//
// Callback response that uses stack based storage
//
template <int N>
class FxResponseArgs : public FxResponseArgsBase
{
public:
    FxResponseArgs() : Index(1) {}  
    void    Add(const Value& v)
    {
        if (Index > N) 
        {
            SF_DEBUG_WARNING(1, "Adding parameter out of bounds!");
            return;
        }
        Values[Index++] = v;
    }
    unsigned    GetValues(Value** pparams) { *pparams = Values; return Index; }
private:
    Value    Values[N+1];    // Space for response data
    unsigned        Index;
};

//
// Callback response that uses dynamically allocated storage
//
class FxResponseArgsList : public FxResponseArgsBase
{
public:
    FxResponseArgsList()                    { Args.PushBack(Value::VT_Null); }   // Space for response data
    void    Add(const Value& v)          { Args.PushBack(v); }
    unsigned    GetValues(Value** pparams)    { *pparams = &Args[0]; return (unsigned)Args.GetSize(); }
private:
    Array<Value>        Args;
};


//////////////////////////////////////////////////////////////////////////


//
// Parameters passed to the callback handler
//
class FxDelegateArgs
{
public:
    FxDelegateArgs(FxDelegateHandler* pthis, Movie* pmovie, 
        const Value* vals, unsigned nargs) : pThis(pthis), 
        pMovieView(pmovie), Args(vals), NArgs(nargs) {}

    FxDelegateHandler*  GetHandler() const      { return pThis; }
    Movie*       GetMovie() const        { return pMovieView; }

    const Value&     operator[](UPInt i) const
    { 
        SF_ASSERT(i < NArgs);
        return Args[i]; 
    }
    unsigned                GetArgCount() const     { return NArgs; }

private:
    FxDelegateHandler*      pThis;
    Movie*           pMovieView;
    const Value*         Args;
    unsigned                    NArgs;
};


//////////////////////////////////////////////////////////////////////////


//
// Callback manager that marshals calls from ActionScript 
//
class FxDelegate : public ExternalInterface
{
public:
    //
    // Callback target
    //
    struct CallbackDefn
    {
        Ptr<FxDelegateHandler>         pThis;
        FxDelegateHandler::CallbackFn   pCallback;
    };

    //
    // Callback hash
    //
    struct CallbackHashFunctor
    {
        UPInt  operator()(const char* data) const
        {
            UPInt  size = SFstrlen(data);
            return String::BernsteinHashFunction(data, size);
        }
    };
    typedef Hash<String, CallbackDefn, CallbackHashFunctor> CallbackHash;


    FxDelegate();

    //
    // Install and uninstall callbacks
    //
    void            RegisterHandler(FxDelegateHandler* callback);
    void            UnregisterHandler(FxDelegateHandler* callback);

    //
    // Call a method registered with the AS2 GameDelegate instance
    //
    static void    Invoke(Movie* pmovieView, const char* methodName, 
                            FxResponseArgsBase& args);

    //
    // ExternalInterface callback entry point
    //
    void            Callback(Movie* pmovieView, const char* methodName, 
                                const Value* args, unsigned argCount);

private:
    //
    // Callbacks installed with the game delegate
    //
    CallbackHash    Callbacks;
};





#endif // INC_FxGameDelegateHandler_H
