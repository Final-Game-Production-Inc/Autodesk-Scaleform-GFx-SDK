/**************************************************************************

Filename    :   UIView.h
Content     :   C++ interface for UI views in Menu Kit
Authors     :   Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_UIView_H
#define INC_UIView_H

// Includes
#include "GFx/GFx_Player.h"

using namespace Scaleform;

// Forward declarations
class ManagerView;

// ********************************************************************
// This base class contains functionality common for all UI views.
// It provides a C++ interface as a bridge between the application
// and the SWF asset that defines the view.
//
class UIView
{
public:
    // UIView(GFx::Value val) : Val(val) {}
    UIView(GFx::Value val,  const char* viewName, ManagerView* pmanager) : 
        View(val), ViewName(viewName), pManager(pmanager) {}
    virtual ~UIView() {}

    struct EventData
    {
        GFx::Value    Target;
        String     Type;
        unsigned        Data;
        unsigned        MouseIndex;
        unsigned        Button;
        unsigned        Index;
        unsigned        LastIndex;
    };

    /** Unpacks the the parameters of a GFxClikWidget callback into a EventData structure. */
    static EventData    ConvertParamsToEventData(const GFx::FunctionHandler::Params params)
    {
        GFx::Value tempVal;
        EventData eventData;

        params.pArgs->GetMember("type", &tempVal);
        eventData.Type = tempVal.GetString();

        if (params.pArgs->HasMember("index"))
        {
            params.pArgs->GetMember("index", &tempVal);
            eventData.Index = (unsigned)toNumber(tempVal);
        }

        return eventData;
    }

    /** Retrieves a reference to the actual GFx::Value for this view. */
    const GFx::Value&               GetGFxValue() const         { return View; }
    /** Retrieves a reference to the ManagerView who loaded this view. */
    ManagerView*                    GetManagerView() const      { return pManager; }

    /** Called when the view is first loaded. One-time initialization can be done here. */
    virtual void                        OnViewLoaded() {}
    /** Called when the view is pushed on to the stack. Reoccurring view setup can be done here. */
    virtual void                        OnViewActivated() {}
    /** Called when the view is removed from the stack. View clean-up can be done here. */
    virtual void                        OnViewClosed()
    {
        DisableSubComponents(true);
    }
    virtual void                        OnTopMostView(bool bPlayOpenAnimation = false);
    /** Called just before view is popped from the stack. Custom logic for the view's shutdown should be added here. */
    virtual void                        ReturnToPreviousViewImpl();

    /**
     *  Enable/disable sub-components of the view. 
     *  Because almost everything in the menu takes focus on rollOver, this is necessary to avoid 
     *  undesirable focus changes when screens are tweened in and out. This could and should be replaced
     *  using a proper ActionScript extension once one is implemented.
     */
    virtual void                        DisableSubComponents(bool bDisableComponents) { SF_UNUSED(bDisableComponents); }
    /** Plays the open animation for the view. */
    virtual void                        PlayOpenAnimation() {}
    /** Plays the close animation for the view. */
    virtual void                        PlayCloseAnimation() {}

protected:
    // Holds a reference to the the view in the SWF (MovieClip for example)
    GFx::Value        View;
    /** The unique name of this view. Used to push this view on to the stack. */
    String          ViewName;
    /** Reference to the ManagerView for this MenuView. */
    ManagerView*    pManager;

    static GFx::Double toNumber(const GFx::Value& val);
};

class UIViewListenerProxy : public GFx::FunctionHandler
{
public:
    typedef void (*WidgetCallback)(UIView* pthis, const GFx::FunctionHandler::Params& params);

    UIViewListenerProxy (UIView* pthis, WidgetCallback callback) : pThis(pthis), Callback(callback) {}
    ~UIViewListenerProxy () {}

    virtual void Call(const Params& params)
    {
        Callback(pThis, params);
    }

protected:
    UIView*             pThis;
    WidgetCallback      Callback;
};


#endif  // INC_UIView_H
