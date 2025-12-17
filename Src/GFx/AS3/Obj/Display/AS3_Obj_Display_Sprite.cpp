//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_Sprite.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_Sprite.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Display_Graphics.h"
#include "../Media/AS3_Obj_Media_SoundTransform.h"
#include "../Geom/AS3_Obj_Geom_Rectangle.h"

#include "GFx/AS3/AS3_AvmSprite.h"
#include "Kernel/SF_Array.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_Resource.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_display::Sprite, Instances::fl_display::Sprite::mid_buttonModeGet, bool> TFunc_Instances_Sprite_buttonModeGet;
typedef ThunkFunc1<Instances::fl_display::Sprite, Instances::fl_display::Sprite::mid_buttonModeSet, const Value, bool> TFunc_Instances_Sprite_buttonModeSet;
typedef ThunkFunc0<Instances::fl_display::Sprite, Instances::fl_display::Sprite::mid_dropTargetGet, SPtr<Instances::fl_display::DisplayObject> > TFunc_Instances_Sprite_dropTargetGet;
typedef ThunkFunc0<Instances::fl_display::Sprite, Instances::fl_display::Sprite::mid_graphicsGet, SPtr<Instances::fl_display::Graphics> > TFunc_Instances_Sprite_graphicsGet;
typedef ThunkFunc0<Instances::fl_display::Sprite, Instances::fl_display::Sprite::mid_hitAreaGet, SPtr<Instances::fl_display::Sprite> > TFunc_Instances_Sprite_hitAreaGet;
typedef ThunkFunc1<Instances::fl_display::Sprite, Instances::fl_display::Sprite::mid_hitAreaSet, const Value, Instances::fl_display::Sprite*> TFunc_Instances_Sprite_hitAreaSet;
typedef ThunkFunc0<Instances::fl_display::Sprite, Instances::fl_display::Sprite::mid_soundTransformGet, SPtr<Instances::fl_media::SoundTransform> > TFunc_Instances_Sprite_soundTransformGet;
typedef ThunkFunc1<Instances::fl_display::Sprite, Instances::fl_display::Sprite::mid_soundTransformSet, const Value, Instances::fl_media::SoundTransform*> TFunc_Instances_Sprite_soundTransformSet;
typedef ThunkFunc0<Instances::fl_display::Sprite, Instances::fl_display::Sprite::mid_useHandCursorGet, bool> TFunc_Instances_Sprite_useHandCursorGet;
typedef ThunkFunc1<Instances::fl_display::Sprite, Instances::fl_display::Sprite::mid_useHandCursorSet, const Value, bool> TFunc_Instances_Sprite_useHandCursorSet;
typedef ThunkFunc2<Instances::fl_display::Sprite, Instances::fl_display::Sprite::mid_startDrag, const Value, bool, Instances::fl_geom::Rectangle*> TFunc_Instances_Sprite_startDrag;
typedef ThunkFunc0<Instances::fl_display::Sprite, Instances::fl_display::Sprite::mid_stopDrag, const Value> TFunc_Instances_Sprite_stopDrag;
typedef ThunkFunc3<Instances::fl_display::Sprite, Instances::fl_display::Sprite::mid_startTouchDrag, const Value, SInt32, bool, Instances::fl_geom::Rectangle*> TFunc_Instances_Sprite_startTouchDrag;
typedef ThunkFunc1<Instances::fl_display::Sprite, Instances::fl_display::Sprite::mid_stopTouchDrag, const Value, SInt32> TFunc_Instances_Sprite_stopTouchDrag;

template <> const TFunc_Instances_Sprite_buttonModeGet::TMethod TFunc_Instances_Sprite_buttonModeGet::Method = &Instances::fl_display::Sprite::buttonModeGet;
template <> const TFunc_Instances_Sprite_buttonModeSet::TMethod TFunc_Instances_Sprite_buttonModeSet::Method = &Instances::fl_display::Sprite::buttonModeSet;
template <> const TFunc_Instances_Sprite_dropTargetGet::TMethod TFunc_Instances_Sprite_dropTargetGet::Method = &Instances::fl_display::Sprite::dropTargetGet;
template <> const TFunc_Instances_Sprite_graphicsGet::TMethod TFunc_Instances_Sprite_graphicsGet::Method = &Instances::fl_display::Sprite::graphicsGet;
template <> const TFunc_Instances_Sprite_hitAreaGet::TMethod TFunc_Instances_Sprite_hitAreaGet::Method = &Instances::fl_display::Sprite::hitAreaGet;
template <> const TFunc_Instances_Sprite_hitAreaSet::TMethod TFunc_Instances_Sprite_hitAreaSet::Method = &Instances::fl_display::Sprite::hitAreaSet;
template <> const TFunc_Instances_Sprite_soundTransformGet::TMethod TFunc_Instances_Sprite_soundTransformGet::Method = &Instances::fl_display::Sprite::soundTransformGet;
template <> const TFunc_Instances_Sprite_soundTransformSet::TMethod TFunc_Instances_Sprite_soundTransformSet::Method = &Instances::fl_display::Sprite::soundTransformSet;
template <> const TFunc_Instances_Sprite_useHandCursorGet::TMethod TFunc_Instances_Sprite_useHandCursorGet::Method = &Instances::fl_display::Sprite::useHandCursorGet;
template <> const TFunc_Instances_Sprite_useHandCursorSet::TMethod TFunc_Instances_Sprite_useHandCursorSet::Method = &Instances::fl_display::Sprite::useHandCursorSet;
template <> const TFunc_Instances_Sprite_startDrag::TMethod TFunc_Instances_Sprite_startDrag::Method = &Instances::fl_display::Sprite::startDrag;
template <> const TFunc_Instances_Sprite_stopDrag::TMethod TFunc_Instances_Sprite_stopDrag::Method = &Instances::fl_display::Sprite::stopDrag;
template <> const TFunc_Instances_Sprite_startTouchDrag::TMethod TFunc_Instances_Sprite_startTouchDrag::Method = &Instances::fl_display::Sprite::startTouchDrag;
template <> const TFunc_Instances_Sprite_stopTouchDrag::TMethod TFunc_Instances_Sprite_stopTouchDrag::Method = &Instances::fl_display::Sprite::stopTouchDrag;

namespace Instances { namespace fl_display
{
    Sprite::Sprite(InstanceTraits::Traits& t)
    : Instances::fl_display::DisplayObjectContainer(t)
//##protect##"instance::Sprite::Sprite()$data"
//##protect##"instance::Sprite::Sprite()$data"
    {
//##protect##"instance::Sprite::Sprite()$code"
//##protect##"instance::Sprite::Sprite()$code"
    }

    void Sprite::buttonModeGet(bool& result)
    {
//##protect##"instance::Sprite::buttonModeGet()"
        SF_UNUSED1(result);
        if (!pDispObj)
            result = false;
        else
            result = static_cast<AvmSprite*>(ToAvmDisplayObj(pDispObj))->IsButtonMode();
//##protect##"instance::Sprite::buttonModeGet()"
    }
    void Sprite::buttonModeSet(const Value& result, bool value)
    {
//##protect##"instance::Sprite::buttonModeSet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj);
        static_cast<AvmSprite*>(ToAvmDisplayObj(pDispObj))->SetButtonMode(value);
//##protect##"instance::Sprite::buttonModeSet()"
    }
    void Sprite::dropTargetGet(SPtr<Instances::fl_display::DisplayObject>& result)
    {
//##protect##"instance::Sprite::dropTargetGet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj);

        MovieImpl* proot = pDispObj->GetMovieImpl();
        Render::PointF mousePos = proot->GetMouseState(0)->GetLastPosition();
        // only mouseIndex 0 is supported.
        GFx::InteractiveObject* ptopCh = proot->GetTopMostEntity(mousePos, 0, true, pDispObj->CharToInteractiveObject_Unsafe());
        
        if (ptopCh)
        {
            ToAvmInteractiveObj(ptopCh)->CreateASInstance(true);
            result = ToAvmInteractiveObj(ptopCh)->GetAS3Obj();
        }
        else
        {
            //@TODO
            //GetVM().ThrowError();
            result = NULL;
        }
//##protect##"instance::Sprite::dropTargetGet()"
    }
    void Sprite::graphicsGet(SPtr<Instances::fl_display::Graphics>& result)
    {
//##protect##"instance::Sprite::graphicsGet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj);
        if (!pGraphics)
        {
            ASVM& asvm = static_cast<ASVM&>(GetVM());
            if (asvm.ConstructInstance(pGraphics, asvm.GraphicsClass))
            {
                pGraphics->pDrawing = pDispObj->GetDrawingContext();
                pGraphics->pDispObj = pDispObj;
            }
        }
        result = pGraphics;
//##protect##"instance::Sprite::graphicsGet()"
    }
    void Sprite::hitAreaGet(SPtr<Instances::fl_display::Sprite>& result)
    {
//##protect##"instance::Sprite::hitAreaGet()"
        if (pDispObj)
        {
            GFx::Sprite* hitAreaSprite = pDispObj->CharToSprite_Unsafe()->GetHitArea();
            if (hitAreaSprite)
            {
                // sprite should have create AS instance already (AB). 
                //ToAvmSprite(hitAreaSprite)->CreateASInstance(true);
                Instances::fl_display::DisplayObject* ido = ToAvmSprite(hitAreaSprite)->GetAS3Obj();
                if (ido != NULL && IsSprite(ido->GetTraitsType()))
                {
                    result = static_cast<Instances::fl_display::Sprite*>(ido);
                    return;
                }
            }
        }

        result = NULL;
//##protect##"instance::Sprite::hitAreaGet()"
    }
    void Sprite::hitAreaSet(const Value& result, Instances::fl_display::Sprite* value)
    {
//##protect##"instance::Sprite::hitAreaSet()"
        SF_UNUSED2(result, value);
        SF_ASSERT(pDispObj);

        if (value)
        {
            if (!value->pDispObj) value->CreateStageObject();
            SF_ASSERT(value->pDispObj);
            pDispObj->CharToSprite_Unsafe()->SetHitArea(value->pDispObj->CharToSprite_Unsafe());
        }
        else 
        {
            pDispObj->CharToSprite_Unsafe()->SetHitArea(NULL);
        }
//##protect##"instance::Sprite::hitAreaSet()"
    }
    void Sprite::soundTransformGet(SPtr<Instances::fl_media::SoundTransform>& result)
    {
//##protect##"instance::Sprite::soundTransformGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("Sprite::soundTransformGet()");
//##protect##"instance::Sprite::soundTransformGet()"
    }
    void Sprite::soundTransformSet(const Value& result, Instances::fl_media::SoundTransform* value)
    {
//##protect##"instance::Sprite::soundTransformSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("Sprite::soundTransformSet()");
//##protect##"instance::Sprite::soundTransformSet()"
    }
    void Sprite::useHandCursorGet(bool& result)
    {
//##protect##"instance::Sprite::useHandCursorGet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj && pDispObj->IsInteractiveObject());
        result = pDispObj->CharToInteractiveObject_Unsafe()->IsUseHandCursorFlagTrue();
//##protect##"instance::Sprite::useHandCursorGet()"
    }
    void Sprite::useHandCursorSet(const Value& result, bool value)
    {
//##protect##"instance::Sprite::useHandCursorSet()"
        SF_UNUSED2(result, value);
        SF_ASSERT(pDispObj && pDispObj->IsInteractiveObject());
        pDispObj->CharToInteractiveObject_Unsafe()->SetUseHandCursorFlag(value);
//##protect##"instance::Sprite::useHandCursorSet()"
    }
    void Sprite::startDrag(const Value& result, bool lockCenter, Instances::fl_geom::Rectangle* bounds)
    {
//##protect##"instance::Sprite::startDrag()"
        SF_UNUSED3(result, lockCenter, bounds);
        
        SF_ASSERT(pDispObj);
        MovieImpl* proot = pDispObj->GetMovieImpl();
        if (proot->IsDragging(0))
            return;
        MovieImpl::DragState st;

        // If there are arguments, init appropriate values.
        if (bounds)
        {
            st.Bound = true;
            st.BoundLT.x = SF_PIXELS_TO_TWIPS((float) bounds->GetX());
            st.BoundLT.y = SF_PIXELS_TO_TWIPS((float) bounds->GetY());
            st.BoundRB.x = SF_PIXELS_TO_TWIPS((float) bounds->GetX() + bounds->GetWidth());
            st.BoundRB.y = SF_PIXELS_TO_TWIPS((float) bounds->GetY() + bounds->GetHeight());
        }

        SF_ASSERT(pDispObj->IsInteractiveObject());
        st.pCharacter = pDispObj->CharToInteractiveObject_Unsafe();
        // Init mouse offsets based on LockCenter flag.
        st.InitCenterDelta(lockCenter);

        // Begin dragging.
        proot->SetDragState(st);  
        pDispObj->CharToInteractiveObject_Unsafe()->ModifyOptimizedPlayList();
//##protect##"instance::Sprite::startDrag()"
    }
    void Sprite::stopDrag(const Value& result)
    {
//##protect##"instance::Sprite::stopDrag()"
        SF_UNUSED1(result);

        SF_ASSERT(pDispObj);

        MovieImpl* proot = pDispObj->GetMovieImpl();
        proot->StopDrag(0);    
        SF_ASSERT(pDispObj->IsInteractiveObject());
        pDispObj->CharToInteractiveObject_Unsafe()->ModifyOptimizedPlayList();
//##protect##"instance::Sprite::stopDrag()"
    }
    void Sprite::startTouchDrag(const Value& result, SInt32 touchPointID, bool lockCenter, Instances::fl_geom::Rectangle* bounds)
    {
//##protect##"instance::Sprite::startTouchDrag()"
        SF_UNUSED4(result, touchPointID, lockCenter, bounds);
#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
        SF_ASSERT(pDispObj);
        MovieImpl* proot = pDispObj->GetMovieImpl();

        int mouseIndex = proot->FindMouseStateIndexByTouchID(unsigned(touchPointID));
        if (proot->IsDraggingMouseIndex(mouseIndex))
            return;
        MovieImpl::DragState st;

        // If there are arguments, init appropriate values.
        if (bounds)
        {
            st.Bound = true;
            st.BoundLT.x = SF_PIXELS_TO_TWIPS((float) bounds->GetX());
            st.BoundLT.y = SF_PIXELS_TO_TWIPS((float) bounds->GetY());
            st.BoundRB.x = SF_PIXELS_TO_TWIPS((float) bounds->GetX() + bounds->GetWidth());
            st.BoundRB.y = SF_PIXELS_TO_TWIPS((float) bounds->GetY() + bounds->GetHeight());
        }

        SF_ASSERT(pDispObj->IsInteractiveObject());
        st.pCharacter = pDispObj->CharToInteractiveObject_Unsafe();

        if (mouseIndex >= 0)
        {
            // Init mouse offsets based on LockCenter flag.
            st.InitCenterDelta(lockCenter, unsigned(mouseIndex));

            // Begin dragging.
            proot->SetDragState(st);  
            pDispObj->CharToInteractiveObject_Unsafe()->ModifyOptimizedPlayList();
        }
#else
        WARN_NOT_IMPLEMENTED("Sprite::startTouchDrag() - multitouch support is OFF");
#endif
//##protect##"instance::Sprite::startTouchDrag()"
    }
    void Sprite::stopTouchDrag(const Value& result, SInt32 touchPointID)
    {
//##protect##"instance::Sprite::stopTouchDrag()"
        SF_UNUSED2(result, touchPointID);
#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
        SF_ASSERT(pDispObj);

        MovieImpl* proot = pDispObj->GetMovieImpl();

        int mouseIndex = proot->FindMouseStateIndexByTouchID(unsigned(touchPointID));
        if (mouseIndex >= 0 && proot->IsDraggingMouseIndex(unsigned(mouseIndex)))
        {
            proot->StopDrag(mouseIndex);    
            SF_ASSERT(pDispObj->IsInteractiveObject());
            pDispObj->CharToInteractiveObject_Unsafe()->ModifyOptimizedPlayList();
        }
#else
        WARN_NOT_IMPLEMENTED("Sprite::stopTouchDrag() - multitouch support is OFF");
#endif
//##protect##"instance::Sprite::stopTouchDrag()"
    }

    SPtr<Instances::fl_display::DisplayObject> Sprite::dropTargetGet()
    {
        SPtr<Instances::fl_display::DisplayObject> result;
        dropTargetGet(result);
        return result;
    }
    SPtr<Instances::fl_display::Graphics> Sprite::graphicsGet()
    {
        SPtr<Instances::fl_display::Graphics> result;
        graphicsGet(result);
        return result;
    }
    SPtr<Instances::fl_media::SoundTransform> Sprite::soundTransformGet()
    {
        SPtr<Instances::fl_media::SoundTransform> result;
        soundTransformGet(result);
        return result;
    }
//##protect##"instance$methods"
    Sprite::~Sprite()
    {

    }

    void Sprite::AS3Constructor(unsigned argc, const Value* argv)
    {
        SF_UNUSED2(argc, argv);
        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);

        ToAvmSprite(spr)->CreateChildren();
    }

    void Sprite::InitInstance(bool extCall)
    {
        if (!extCall)
        {
            // create stage display object if the movieclip was created by
            // "new".
            Sprite::CreateStageObject();
            SF_ASSERT(pDispObj && pDispObj->IsSprite());
            GFx::Sprite* pspr = pDispObj->CharToSprite_Unsafe();
            AvmSprite* avmSpr = ToAvmSprite(pspr);
            // add in playlist
            MovieRoot* as3root = avmSpr->GetAS3Root();
            
            as3root->AddScriptableMovieClip(pspr);
        }
        SF_ASSERT(pDispObj);
        if (IsMovieClip())
        {
            // check if this is a Sprite or a MovieClip.
            // If this is a movieclip, set the appropriate flag and add
            // to the list of scriptable clips.
            pDispObj->SetMovieClipFlag();
        }
    }

    void Sprite::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        DisplayObjectContainer::ForEachChild_GC(prcc, op);
        AS3::ForEachChild_GC<Graphics, Mem_Stat>(prcc, pGraphics, op SF_DEBUG_ARG(*this));
    }

    GFx::DisplayObject* Sprite::CreateStageObject()
    {
        if (!pDispObj)
        {
            ASVM& asvm = static_cast<ASVM&>(GetVM());
            MovieRoot* proot = asvm.GetMovieRoot();

            MovieDefImpl* pdefImpl = asvm.GetResourceMovieDef(this);

            if (pdefImpl)
            {
                // it is possible that there is no corresponding stage object
                // for the class; use empty movie clip then.
                CharacterCreateInfo ccinfo;
                ccinfo.pCharDef = NULL;

                FindLibarySymbol(&ccinfo, pdefImpl);
                
                if (!ccinfo.pCharDef)
                {
                    // not found... Let check if there are symbols linked to the same class in other
                    // moviedefs. Seems like Flash does it.
                    if (!proot->FindLibrarySymbolInAllABCs(this, &ccinfo))
                    {
                        ccinfo = pdefImpl->GetCharacterCreateInfo(ResourceId(CharacterDef::CharId_EmptyMovieClip));
                        if (GetVM().GetCallStack().GetSize() > 0)
                        {
                            // figure out MovieDefImpl from current call frame.
                            const VMAbcFile& abcFile = GetVM().GetCurrCallFrame().GetFile();
                            ccinfo.pBindDefImpl = static_cast<const ASVM::AbcFileWithMovieDef&>(abcFile.GetAbcFile()).pDefImpl;
                        }
                        else
                            ccinfo.pBindDefImpl = pdefImpl;
                    }
                }

                SF_ASSERT(ccinfo.pCharDef);
                pDispObj = *static_cast<GFx::DisplayObject*>(proot->GetASSupport()->CreateCharacterInstance(
                    proot->GetMovieImpl(), ccinfo, NULL, 
                    ResourceId(), CharacterDef::Sprite));
                AvmDisplayObj* pAvmObj = ToAvmDisplayObj(pDispObj);
                pAvmObj->AssignAS3Obj(this);
                pAvmObj->SetAppDomain(GetInstanceTraits().GetAppDomain());

                SF_ASSERT(pDispObj->IsSprite());
            }
        }
        return pDispObj;
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 Sprite::tito[Sprite::ThunkInfoNum] = {
    //    0, 1, 3, 4, 5, 6, 8, 9, 11, 12, 14, 17, 18, 22, 
    // };
    const TypeInfo* Sprite::tit[24] = {
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl_display::GraphicsTI, 
        &AS3::fl_display::SpriteTI, 
        NULL, &AS3::fl_display::SpriteTI, 
        &AS3::fl_media::SoundTransformTI, 
        NULL, &AS3::fl_media::SoundTransformTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, &AS3::fl_geom::RectangleTI, 
        NULL, 
        NULL, &AS3::fl::int_TI, &AS3::fl::BooleanTI, &AS3::fl_geom::RectangleTI, 
        NULL, &AS3::fl::int_TI, 
    };
    const ThunkInfo Sprite::ti[Sprite::ThunkInfoNum] = {
        {TFunc_Instances_Sprite_buttonModeGet::Func, &Sprite::tit[0], "buttonMode", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Sprite_buttonModeSet::Func, &Sprite::tit[1], "buttonMode", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Sprite_dropTargetGet::Func, &Sprite::tit[3], "dropTarget", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Sprite_graphicsGet::Func, &Sprite::tit[4], "graphics", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Sprite_hitAreaGet::Func, &Sprite::tit[5], "hitArea", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Sprite_hitAreaSet::Func, &Sprite::tit[6], "hitArea", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Sprite_soundTransformGet::Func, &Sprite::tit[8], "soundTransform", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Sprite_soundTransformSet::Func, &Sprite::tit[9], "soundTransform", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Sprite_useHandCursorGet::Func, &Sprite::tit[11], "useHandCursor", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Sprite_useHandCursorSet::Func, &Sprite::tit[12], "useHandCursor", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Sprite_startDrag::Func, &Sprite::tit[14], "startDrag", NULL, Abc::NS_Public, CT_Method, 0, 2, 0, 0, NULL},
        {TFunc_Instances_Sprite_stopDrag::Func, &Sprite::tit[17], "stopDrag", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Sprite_startTouchDrag::Func, &Sprite::tit[18], "startTouchDrag", NULL, Abc::NS_Public, CT_Method, 1, 3, 0, 0, NULL},
        {TFunc_Instances_Sprite_stopTouchDrag::Func, &Sprite::tit[22], "stopTouchDrag", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    Sprite::Sprite(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObjectContainer(vm, ci)
    {
//##protect##"InstanceTraits::Sprite::Sprite()"
        SetTraitsType(Traits_Sprite);
//##protect##"InstanceTraits::Sprite::Sprite()"

    }

    void Sprite::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Sprite&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    Sprite::Sprite(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObjectContainer(vm, ci)
    {
//##protect##"ClassTraits::Sprite::Sprite()"
        SetTraitsType(Traits_Sprite);
//##protect##"ClassTraits::Sprite::Sprite()"

    }

    Pickable<Traits> Sprite::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Sprite(vm, AS3::fl_display::SpriteCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::SpriteCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_display
{
    const TypeInfo SpriteTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_display::Sprite::InstanceType),
        0,
        0,
        InstanceTraits::fl_display::Sprite::ThunkInfoNum,
        0,
        "Sprite", "flash.display", &fl_display::DisplayObjectContainerTI,
        TypeInfo::None
    };

    const ClassInfo SpriteCI = {
        &SpriteTI,
        ClassTraits::fl_display::Sprite::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_display::Sprite::ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

