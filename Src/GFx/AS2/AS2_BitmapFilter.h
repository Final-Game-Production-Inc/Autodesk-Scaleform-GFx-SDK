/**************************************************************************

Filename    :   AS2_BitmapFilter.h
Content     :   BitmapFilter reference class for ActionScript 2.0
Created     :   12/10/2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GASBitmapFilter_H
#define INC_GASBitmapFilter_H

#include "GFxConfig.h"

#ifdef GFX_AS2_ENABLE_FILTERS

#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_ObjectProto.h"

#include "GFx/GFx_FilterDesc.h"

namespace Scaleform { namespace GFx { namespace AS2 {

using Render::Filter;
using Render::FilterType;
using Render::BlurFilterImpl;
using Render::BlurFilterParams;


// ***** Declared Classes
class BitmapFilterObject;
class BitmapFilterProto;
class BitmapFilterCtorFunction;

// 
// Actionscript BitmapFilter object declaration
//

class BitmapFilterObject : public Object
{
    friend class BitmapFilterProto;
    friend class BitmapFilterCtorFunction;
private:
    BitmapFilterObject(const BitmapFilterObject&) : Object(GetCollector()) {} 
protected:

    Ptr<Filter> pFilter;    

//?    BitmapFilterObject(const BitmapFilterObject& obj) 
//?        : Object(obj->GetGC()), Filter(obj.Filter) {} //?AB: do we need it? no copy ctor for Object
    BitmapFilterObject(ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }

    virtual ~BitmapFilterObject();

    Filter* writableFilter()
    {
        if (pFilter && pFilter->IsFrozen())
            pFilter = *(Filter*)pFilter->Clone(Memory::GetHeapByAddress(this));
        return pFilter;
    }
    BlurFilterParams& writableFilterParams()
    {
        static BlurFilterParams unavailableParams;
        Filter* filter = writableFilter();
        if ( !filter || !(filter->GetFilterType() >= Filter_Blur && filter->GetFilterType() <= Filter_Blur_End))
            return unavailableParams;
        BlurFilterImpl* bf = static_cast<BlurFilterImpl*>(filter);
        return (bf->GetParams());
    }
    const BlurFilterParams& readonlyFilterParams() const
    {
        static BlurFilterParams unavailableParams;
        Filter* filter = pFilter;
        if ( !filter || !(filter->GetFilterType() >= Filter_Blur && filter->GetFilterType() <= Filter_Blur_End))
            return unavailableParams;
        BlurFilterImpl* bf = static_cast<BlurFilterImpl*>(filter);
        return (bf->GetParams());
    }

    virtual void Finalize_GC();
public:

    BitmapFilterObject(Environment* penv, FilterType ft);

    virtual ObjectType    GetObjectType() const { return Object_BitmapFilter; }

    void                  SetFilter(Filter* filter) { pFilter = filter; }
    const Filter*         GetFilter() const         { return pFilter; }
    Filter*               GetFilter()               { return pFilter; }


    // The offset distance for the shadow, in pixels.
    void            SetDistance(float d)
    {
        Filter* f = writableFilter();
        if ( !f || !(f->GetFilterType() >= Filter_Blur && f->GetFilterType() <= Filter_Blur_End))
            return;
        BlurFilterImpl* bf = static_cast<BlurFilterImpl*>(f);
        bf->SetAngleDistance(bf->GetAngle(), d);
    }

    // The angle of the shadow. Valid values are 0 to 360? (floating point).
    void            SetAngle(float a)
    {
        Filter* f = writableFilter();
        if ( !f || !(f->GetFilterType() >= Filter_Blur && f->GetFilterType() <= Filter_Blur_End))
            return;
        BlurFilterImpl* bf = static_cast<BlurFilterImpl*>(f);
        bf->SetAngleDistance(a, bf->GetDistance());
    }

    // The number of passes for blur and shadows (quality)
    void            SetPasses(UInt32 d)     
    { 
        writableFilterParams().Passes = Alg::Min<unsigned>(d, 15); 
    }

    // The color of the shadow. Valid values are in hexadecimal format 0xRRGGBB
    void            SetColor(UInt32 c)
    {
        BlurFilterParams& fp = writableFilterParams();
        fp.Colors[0].SetColor(c, fp.Colors[0].GetAlpha());
    }
    // The alpha transparency value for the shadow color. Valid values are 0 to 1.
    void            SetAlpha(float a)
    {
        writableFilterParams().Colors[0].SetAlphaFloat(a);
    }
    // The color of the highlight (for bevel). Valid values are in hexadecimal format 0xRRGGBB
    void            SetColor2(UInt32 c)      
    { 
        BlurFilterParams& fp = writableFilterParams();
        fp.Colors[1].SetColor(c, fp.Colors[1].GetAlpha());
    }
    // The alpha transparency value for the highlight color. Valid values are 0 to 1.
    void            SetAlpha2(float a)
    { 
        writableFilterParams().Colors[1].SetAlphaFloat(a);
    }
    // The amount of horizontal blur.
    void            SetBlurX(float b)       
    { 
        writableFilterParams().BlurX = PixelsToTwips(b); 
    }
    // The amount of vertical blur.
    void            SetBlurY(float b)       
    { 
        writableFilterParams().BlurY = PixelsToTwips(b); 
    }
    // The strength of the imprint or spread. The higher the value, 
    // the more color is imprinted and the stronger the contrast between 
    // the shadow and the background. Valid values are from 0 to 255.
    void            SetStrength(float s)    
    {  
        writableFilterParams().Strength = s; 
    }
    // Applies a knockout effect (true), which effectively makes the 
    // object's fill transparent and reveals the background color of the document.
    void            SetKnockOut(bool k) 
    { 
        if (k) 
            writableFilterParams().Mode |= BlurFilterParams::Mode_Knockout;
        else
            writableFilterParams().Mode &= ~BlurFilterParams::Mode_Knockout;
    }
    // Indicates whether or not the object is hidden. The value true indicates that 
    // the object itself is not drawn; only the shadow is visible.
    void            SetHideObject(bool h) 
    { 
        if (h) 
            writableFilterParams().Mode |= BlurFilterParams::Mode_HideObject;
        else
            writableFilterParams().Mode &= ~BlurFilterParams::Mode_HideObject;
    }
    void            SetInnerShadow(bool i) 
    { 
        if (i) 
            writableFilterParams().Mode |= BlurFilterParams::Mode_Inner;
        else 
            writableFilterParams().Mode &= ~BlurFilterParams::Mode_Inner;
    }
    void            SetBlurFlags(UInt32 f)
    {
        writableFilterParams().Mode = f;
    }

    UInt32          GetPasses()     const { return readonlyFilterParams().Passes; }
    UInt32          GetColor()      const { return readonlyFilterParams().Colors[0].ToColor32() & 0x00FFFFFF; }
    float           GetAlpha()      const { float a;  readonlyFilterParams().Colors[0].GetAlphaFloat(&a); return a; }
    UInt32          GetColor2()     const { return readonlyFilterParams().Colors[1].ToColor32() & 0x00FFFFFF; }
    float           GetAlpha2()     const { float a; readonlyFilterParams().Colors[1].GetAlphaFloat(&a); return a; }
    float           GetBlurX()      const { return TwipsToPixels(readonlyFilterParams().BlurX); }
    float           GetBlurY()      const { return TwipsToPixels(readonlyFilterParams().BlurY); }
    float           GetStrength()   const { return readonlyFilterParams().Strength; }
    bool            IsKnockOut()    const { return (readonlyFilterParams().Mode & BlurFilterParams::Mode_Knockout) != 0; }
    bool            IsHideObject()  const { return (readonlyFilterParams().Mode & BlurFilterParams::Mode_HideObject) != 0; }
    bool            IsInnerShadow() const { return (readonlyFilterParams().Mode & BlurFilterParams::Mode_Inner) != 0; }
    UInt32          GetBlurFlags()  const { return (readonlyFilterParams().Mode); }
    float           GetDistance()   const 
    { 
        Filter* f = pFilter;
        if ( !f || !(f->GetFilterType() >= Filter_Blur && f->GetFilterType() <= Filter_Blur_End))
            return 0.0f;
        BlurFilterImpl* bf = static_cast<BlurFilterImpl*>(f);
        return bf->GetDistance();
    }
    float           GetAngle()      const 
    { 
        Filter* f = pFilter;
        if ( !f || !(f->GetFilterType() >= Filter_Blur && f->GetFilterType() <= Filter_Blur_End))
            return 0.0f;
        BlurFilterImpl* bf = static_cast<BlurFilterImpl*>(f);
        return bf->GetAngle();
    }

    static BitmapFilterObject* CreateFromDesc(Environment* penv, const Filter& filter);
};


class BitmapFilterProto : public Prototype<BitmapFilterObject>
{
public:
    BitmapFilterProto(ASStringContext *psc, Object* prototype, 
        const FunctionRef& constructor);
};


class BitmapFilterCtorFunction : public CFunctionObject
{
public:
    BitmapFilterCtorFunction(ASStringContext *psc);

    //static void GlobalCtor(const GASFnCall& fn);

    static FunctionRef Register(GlobalContext* pgc);
};

}}} //SF::GFx::AS2

#endif  // SF_NO_FXPLAYER_AS_FILTERS

#endif  // INC_GASBitmapFilter_H
