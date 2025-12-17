/**************************************************************************

Filename    :   UIView.cpp
Content     :   C++ interface for UI views in Menu Kit
Authors     :   Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "UIView.h"
#include "ManagerView.h"

void    UIView::ReturnToPreviousViewImpl()
{
    if (pManager != NULL)
    {
        PlayCloseAnimation();
        pManager->PopView();
    }
}

void    UIView::OnTopMostView(bool bPlayOpenAnimation)
{
    // Enable this view's components.
    DisableSubComponents(false);
    if (bPlayOpenAnimation)
    {
        PlayOpenAnimation();
    }
}

GFx::Double  UIView::toNumber(const GFx::Value& val)
{
    switch (val.GetType())
    {
    case GFx::Value::VT_Number:     return GFx::Double(val.GetNumber());
    case GFx::Value::VT_UInt:       return GFx::Double(val.GetUInt());
    case GFx::Value::VT_Int:        return GFx::Double(val.GetInt());
    default:
        {
            SF_ASSERT(0);
            return 0;
        }
    }

}
