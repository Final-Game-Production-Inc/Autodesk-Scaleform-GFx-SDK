//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_Shape.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_Shape.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_Resource.h"
#include "AS3_Obj_Display_Graphics.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_display::Shape, Instances::fl_display::Shape::mid_graphicsGet, SPtr<Instances::fl_display::Graphics> > TFunc_Instances_Shape_graphicsGet;

template <> const TFunc_Instances_Shape_graphicsGet::TMethod TFunc_Instances_Shape_graphicsGet::Method = &Instances::fl_display::Shape::graphicsGet;

namespace Instances { namespace fl_display
{
    Shape::Shape(InstanceTraits::Traits& t)
    : Instances::fl_display::DisplayObject(t)
//##protect##"instance::Shape::Shape()$data"
//##protect##"instance::Shape::Shape()$data"
    {
//##protect##"instance::Shape::Shape()$code"
//##protect##"instance::Shape::Shape()$code"
    }

    void Shape::graphicsGet(SPtr<Instances::fl_display::Graphics>& result)
    {
//##protect##"instance::Shape::graphicsGet()"
        SF_UNUSED1(result);
        if (!pDispObj) CreateStageObject();
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
//##protect##"instance::Shape::graphicsGet()"
    }

    SPtr<Instances::fl_display::Graphics> Shape::graphicsGet()
    {
        SPtr<Instances::fl_display::Graphics> result;
        graphicsGet(result);
        return result;
    }
//##protect##"instance$methods"
    Shape::~Shape()
    {
        //pGraphics = NULL;
    }

    void Shape::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        DisplayObject::ForEachChild_GC(prcc, op);
        AS3::ForEachChild_GC<Graphics, Mem_Stat>(prcc, pGraphics, op SF_DEBUG_ARG(*this));
    }

    void Shape::InitInstance(bool extCall)
    {
        if (!extCall)
        {
            // create stage display object if the movieclip was created by
            // "new".
            Shape::CreateStageObject();
            SF_ASSERT(pDispObj);
        }
    }

    GFx::DisplayObject* Shape::CreateStageObject()
    {
        if (!pDispObj)
        {
            MovieRoot* proot = static_cast<const ASVM&>(GetVM()).GetMovieRoot();

            CharacterCreateInfo ccinfo = 
                proot->GetMovieDefImpl()->GetCharacterCreateInfo(ResourceId(CharacterDef::CharId_EmptyShape));
            SF_ASSERT(ccinfo.pCharDef);
            pDispObj = *static_cast<GFx::DisplayObject*>(proot->GetASSupport()->CreateCharacterInstance(
                proot->GetMovieImpl(), ccinfo, NULL, 
                ResourceId(CharacterDef::CharId_EmptyMovieClip), CharacterDef::Shape));
            AvmDisplayObj* pAvmObj = ToAvmDisplayObj(pDispObj);
            pAvmObj->AssignAS3Obj(this);
            pAvmObj->SetAppDomain(GetInstanceTraits().GetAppDomain());
        }
        return pDispObj;
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 Shape::tito[Shape::ThunkInfoNum] = {
    //    0, 
    // };
    const TypeInfo* Shape::tit[1] = {
        &AS3::fl_display::GraphicsTI, 
    };
    const ThunkInfo Shape::ti[Shape::ThunkInfoNum] = {
        {TFunc_Instances_Shape_graphicsGet::Func, &Shape::tit[0], "graphics", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

    Shape::Shape(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObject(vm, ci)
    {
//##protect##"InstanceTraits::Shape::Shape()"
//##protect##"InstanceTraits::Shape::Shape()"

    }

    void Shape::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Shape&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    Shape::Shape(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObject(vm, ci)
    {
//##protect##"ClassTraits::Shape::Shape()"
//##protect##"ClassTraits::Shape::Shape()"

    }

    Pickable<Traits> Shape::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Shape(vm, AS3::fl_display::ShapeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::ShapeCI));
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
    const TypeInfo ShapeTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_display::Shape::InstanceType),
        0,
        0,
        InstanceTraits::fl_display::Shape::ThunkInfoNum,
        0,
        "Shape", "flash.display", &fl_display::DisplayObjectTI,
        TypeInfo::None
    };

    const ClassInfo ShapeCI = {
        &ShapeTI,
        ClassTraits::fl_display::Shape::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_display::Shape::ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

