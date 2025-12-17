/**************************************************************************

Filename    :   GFxClikWidget.h
Content     :  Wrapper for GFx::Value that provides functionality for 
                dynamically adding/removing C++ EventListeners for CLIK
                events.

Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_ClikWidget_H
#define INC_ClikWidget_H

// Includes
#include "GFx/GFx_Player.h"

using namespace Scaleform;

class GFxClikWidget
{
public:
    GFxClikWidget() : VMVersion(0) {}
    GFxClikWidget(GFx::Value val) : WidgetRef(val), VMVersion(0) {}
    virtual ~GFxClikWidget() {}

    void        SetWidget(GFx::Value val)
    {
        WidgetRef = val;
    }

    bool        SetMember(const char* name, const GFx::Value& val)
    {
        if (!IsValid()) return false;
        return WidgetRef.SetMember(name, val);
    }

    bool        GetMember(const char* name, GFx::Value* pval) const
    {
        if (!IsValid()) return false;
        return WidgetRef.GetMember(name, pval);
    }

    void AddEventListener(GFx::Movie* pmovie, String eventType, GFx::FunctionHandler* eventListener)
    {
        if(pmovie && SFstrcmp(eventType, ""))
        {
            VMVersion = pmovie->GetAVMVersion();

            GFx::Value tempObject;
            pmovie->CreateObject(&tempObject, "Object");
            ASSetFunctionOn(pmovie, tempObject, eventType, eventListener);
            ASAddEventListener(eventType, tempObject, eventType);
        }
    }

    void    ASRemoveAllEventListeners(String eventType) 
    {
        GFx::Value tempVal;
        tempVal.SetString(eventType);
        if (VMVersion == 1)
        {
            WidgetRef.Invoke("removeAllEventListeners", NULL, &tempVal, 1);
        }
        else if (VMVersion == 2)
        {
            // WidgetRef.Invoke("removeAllEventListeners", NULL, &tempVal, 1);
        }
        
    }

    bool    IsValid() const     { return !WidgetRef.IsNull() && !WidgetRef.IsUndefined(); }
    GFx::Value&    GetRef()     { return WidgetRef; }

protected:
    void    ASSetFunctionOn(GFx::Movie* pmovie, GFx::Value tempObject, String memberName, GFx::FunctionHandler* eventListener) 
    { 
        if (pmovie)
        {
            GFx::Value tempFunc;
            pmovie->CreateFunction(&tempFunc, eventListener, NULL);
            tempObject.SetMember(memberName, tempFunc);
        }
    }

    void    ASAddEventListener(String eventType, GFx::Value objectContext, String listenerFuncName) 
    {
        GFx::Value params[5];
        params[0] = GFx::Value(eventType);

        unsigned paramCount = 0;

        if (VMVersion == 1) 
        {
            params[1] = GFx::Value(objectContext);
            params[2] = GFx::Value(listenerFuncName);
            paramCount = 3;
        } 
        else 
        {
            GFx::Value func;
            objectContext.GetMember(listenerFuncName, &func);
            params[1] = func;
            params[2] = GFx::Value(false);
            params[3] = GFx::Value(0.);
            params[4] = GFx::Value(false);
            paramCount = 5;

            // WidgetRef.SetMember(listenerFuncName, objectContext);
        }
        
        WidgetRef.Invoke("addEventListener", NULL, params, paramCount);
    }

    GFx::Value      WidgetRef;
    unsigned        VMVersion;
};
#endif  // INC_ClikWidget_H
