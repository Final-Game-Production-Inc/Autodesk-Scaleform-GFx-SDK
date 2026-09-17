//##protect##"disclaimer"
/**********************************************************************

Filename    :   AS3_Obj_Display_GraphicsPath.cpp
Content     :   
Created     :   Apr, 2013
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_GraphicsPath.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_display::GraphicsPath, Instances::fl_display::GraphicsPath::mid_windingGet, ASString> TFunc_Instances_GraphicsPath_windingGet;
typedef ThunkFunc1<Instances::fl_display::GraphicsPath, Instances::fl_display::GraphicsPath::mid_windingSet, const Value, const ASString&> TFunc_Instances_GraphicsPath_windingSet;
typedef ThunkFunc6<Instances::fl_display::GraphicsPath, Instances::fl_display::GraphicsPath::mid_cubicCurveTo, const Value, Value::Number, Value::Number, Value::Number, Value::Number, Value::Number, Value::Number> TFunc_Instances_GraphicsPath_cubicCurveTo;
typedef ThunkFunc4<Instances::fl_display::GraphicsPath, Instances::fl_display::GraphicsPath::mid_curveTo, const Value, Value::Number, Value::Number, Value::Number, Value::Number> TFunc_Instances_GraphicsPath_curveTo;
typedef ThunkFunc2<Instances::fl_display::GraphicsPath, Instances::fl_display::GraphicsPath::mid_lineTo, const Value, Value::Number, Value::Number> TFunc_Instances_GraphicsPath_lineTo;
typedef ThunkFunc2<Instances::fl_display::GraphicsPath, Instances::fl_display::GraphicsPath::mid_moveTo, const Value, Value::Number, Value::Number> TFunc_Instances_GraphicsPath_moveTo;
typedef ThunkFunc2<Instances::fl_display::GraphicsPath, Instances::fl_display::GraphicsPath::mid_wideLineTo, const Value, Value::Number, Value::Number> TFunc_Instances_GraphicsPath_wideLineTo;
typedef ThunkFunc2<Instances::fl_display::GraphicsPath, Instances::fl_display::GraphicsPath::mid_wideMoveTo, const Value, Value::Number, Value::Number> TFunc_Instances_GraphicsPath_wideMoveTo;

template <> const TFunc_Instances_GraphicsPath_windingGet::TMethod TFunc_Instances_GraphicsPath_windingGet::Method = &Instances::fl_display::GraphicsPath::windingGet;
template <> const TFunc_Instances_GraphicsPath_windingSet::TMethod TFunc_Instances_GraphicsPath_windingSet::Method = &Instances::fl_display::GraphicsPath::windingSet;
template <> const TFunc_Instances_GraphicsPath_cubicCurveTo::TMethod TFunc_Instances_GraphicsPath_cubicCurveTo::Method = &Instances::fl_display::GraphicsPath::cubicCurveTo;
template <> const TFunc_Instances_GraphicsPath_curveTo::TMethod TFunc_Instances_GraphicsPath_curveTo::Method = &Instances::fl_display::GraphicsPath::curveTo;
template <> const TFunc_Instances_GraphicsPath_lineTo::TMethod TFunc_Instances_GraphicsPath_lineTo::Method = &Instances::fl_display::GraphicsPath::lineTo;
template <> const TFunc_Instances_GraphicsPath_moveTo::TMethod TFunc_Instances_GraphicsPath_moveTo::Method = &Instances::fl_display::GraphicsPath::moveTo;
template <> const TFunc_Instances_GraphicsPath_wideLineTo::TMethod TFunc_Instances_GraphicsPath_wideLineTo::Method = &Instances::fl_display::GraphicsPath::wideLineTo;
template <> const TFunc_Instances_GraphicsPath_wideMoveTo::TMethod TFunc_Instances_GraphicsPath_wideMoveTo::Method = &Instances::fl_display::GraphicsPath::wideMoveTo;

namespace Instances { namespace fl_display
{
    GraphicsPath::GraphicsPath(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , commands()
    , data()
//##protect##"instance::GraphicsPath::GraphicsPath()$data"
    , Winding(GetVM().GetStringManager().CreateString("evenOdd"))
//##protect##"instance::GraphicsPath::GraphicsPath()$data"
    {
//##protect##"instance::GraphicsPath::GraphicsPath()$code"
//##protect##"instance::GraphicsPath::GraphicsPath()$code"
    }

    void GraphicsPath::windingGet(ASString& result)
    {
//##protect##"instance::GraphicsPath::windingGet()"
        result = Winding;
//##protect##"instance::GraphicsPath::windingGet()"
    }
    void GraphicsPath::windingSet(const Value& result, const ASString& value)
    {
//##protect##"instance::GraphicsPath::windingSet()"
        SF_UNUSED(result);
        if(value == "evenOdd" || value == "nonZero" )
            Winding = value;
        else
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Parameter winding must be one of the accepted values.")));
//##protect##"instance::GraphicsPath::windingSet()"
    }
    void GraphicsPath::cubicCurveTo(const Value& result, Value::Number controlX1, Value::Number controlY1, Value::Number controlX2, Value::Number controlY2, Value::Number anchorX, Value::Number anchorY)
    {
//##protect##"instance::GraphicsPath::cubicCurveTo()"
        SF_UNUSED(result);
        commands->PushBack(6); //there is no practical way to get values from GraphicsPathCommand class, so command constants are hardcoded
        data->PushBack(controlX1);
        data->PushBack(controlX2);
        data->PushBack(controlY1);
        data->PushBack(controlY2);
        data->PushBack(anchorX);
        data->PushBack(anchorY);
//##protect##"instance::GraphicsPath::cubicCurveTo()"
    }
    void GraphicsPath::curveTo(const Value& result, Value::Number controlX, Value::Number controlY, Value::Number anchorX, Value::Number anchorY)
    {
//##protect##"instance::GraphicsPath::curveTo()"
        SF_UNUSED(result);
        commands->PushBack(3); //there is no practical way to get values from GraphicsPathCommand class, so command constants are hardcoded
        data->PushBack(controlX);
        data->PushBack(controlY);
        data->PushBack(anchorX);
        data->PushBack(anchorY);
//##protect##"instance::GraphicsPath::curveTo()"
    }
    void GraphicsPath::lineTo(const Value& result, Value::Number x, Value::Number y)
    {
//##protect##"instance::GraphicsPath::lineTo()"
        SF_UNUSED(result);
        commands->PushBack(2); //there is no practical way to get values from GraphicsPathCommand class, so command constants are hardcoded
        data->PushBack(x);
        data->PushBack(y);
//##protect##"instance::GraphicsPath::lineTo()"
    }
    void GraphicsPath::moveTo(const Value& result, Value::Number x, Value::Number y)
    {
//##protect##"instance::GraphicsPath::moveTo()"
        SF_UNUSED(result);
        commands->PushBack(2); //there is no practical way to get values from GraphicsPathCommand class, so command constants are hardcoded
        data->PushBack(x);
        data->PushBack(y);
//##protect##"instance::GraphicsPath::moveTo()"
    }
    void GraphicsPath::wideLineTo(const Value& result, Value::Number x, Value::Number y)
    {
//##protect##"instance::GraphicsPath::wideLineTo()"
        SF_UNUSED(result);
        commands->PushBack(5); //there is no practical way to get values from GraphicsPathCommand class, so command constants are hardcoded
        data->PushBack(x);
        data->PushBack(y);
//##protect##"instance::GraphicsPath::wideLineTo()"
    }
    void GraphicsPath::wideMoveTo(const Value& result, Value::Number x, Value::Number y)
    {
//##protect##"instance::GraphicsPath::wideMoveTo()"
        SF_UNUSED(result);
        commands->PushBack(4); //there is no practical way to get values from GraphicsPathCommand class, so command constants are hardcoded
        data->PushBack(x);
        data->PushBack(y);
//##protect##"instance::GraphicsPath::wideMoveTo()"
    }

//##protect##"instance$methods"
    void GraphicsPath::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc > 3)
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, GetVM() SF_DEBUG_ARG("GraphicsPath::AS3Constructor") SF_DEBUG_ARG(1) SF_DEBUG_ARG(1) SF_DEBUG_ARG(argc)));
            return;
        }
        
        if (argc > 0)
        {
            if (argv[0].IsObject() && (argv[0].GetObject() == 0 || argv[0].GetObject()->GetName() == "Vector$int"))
            {
                commands = static_cast<fl_vec::Vector_int*>(argv[0].GetObject());
            }
            else
            {
                GetVM().ThrowArgumentError(VM::Error(VM::eCheckTypeFailedError, GetVM() 
                    SF_DEBUG_ARG(GetVM().GetClassTraits(argv[0]).GetName().ToCStr())
                    SF_DEBUG_ARG("Vector.<int>")));
                return;
            }

        }
        if (argc > 1)
        {
            if (argv[1].IsObject() && (argv[1].GetObject() == 0 || argv[1].GetObject()->GetName() == "Vector$double"))
            {
                data = static_cast<fl_vec::Vector_double*>(argv[1].GetObject());
            }
            else
            {
                GetVM().ThrowArgumentError(VM::Error(VM::eCheckTypeFailedError, GetVM() 
                    SF_DEBUG_ARG(GetVM().GetClassTraits(argv[0]).GetName().ToCStr())
                    SF_DEBUG_ARG("Vector.<Number>")));
                return;
            }
        }
        if (argc > 2)
        {
            if (!argv[2].Convert2String(Winding))
                return;
            
            if (Winding != "evenOdd" && Winding != "nonZero")
            {
                GetVM().ThrowArgumentError(VM::Error(VM::eInvalidEnumError, GetVM() SF_DEBUG_ARG("winding")));
                return;
            }
        }
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 GraphicsPath::tito[GraphicsPath::ThunkInfoNum] = {
    //    0, 1, 3, 10, 15, 18, 21, 24, 
    // };
    const TypeInfo* GraphicsPath::tit[27] = {
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
    };
    const ThunkInfo GraphicsPath::ti[GraphicsPath::ThunkInfoNum] = {
        {TFunc_Instances_GraphicsPath_windingGet::Func, &GraphicsPath::tit[0], "winding", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GraphicsPath_windingSet::Func, &GraphicsPath::tit[1], "winding", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GraphicsPath_cubicCurveTo::Func, &GraphicsPath::tit[3], "cubicCurveTo", NULL, Abc::NS_Public, CT_Method, 6, 6, 0, 0, NULL},
        {TFunc_Instances_GraphicsPath_curveTo::Func, &GraphicsPath::tit[10], "curveTo", NULL, Abc::NS_Public, CT_Method, 4, 4, 0, 0, NULL},
        {TFunc_Instances_GraphicsPath_lineTo::Func, &GraphicsPath::tit[15], "lineTo", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_GraphicsPath_moveTo::Func, &GraphicsPath::tit[18], "moveTo", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_GraphicsPath_wideLineTo::Func, &GraphicsPath::tit[21], "wideLineTo", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_GraphicsPath_wideMoveTo::Func, &GraphicsPath::tit[24], "wideMoveTo", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
    };
    const MemberInfo GraphicsPath::mi[GraphicsPath::MemberInfoNum] = {
        {"commands", NULL, OFFSETOF(Instances::fl_display::GraphicsPath, commands), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
        {"data", NULL, OFFSETOF(Instances::fl_display::GraphicsPath, data), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
    };


    GraphicsPath::GraphicsPath(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::GraphicsPath::GraphicsPath()"
//##protect##"InstanceTraits::GraphicsPath::GraphicsPath()"

    }

    void GraphicsPath::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<GraphicsPath&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    GraphicsPath::GraphicsPath(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::GraphicsPath::GraphicsPath()"
//##protect##"ClassTraits::GraphicsPath::GraphicsPath()"

    }

    Pickable<Traits> GraphicsPath::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) GraphicsPath(vm, AS3::fl_display::GraphicsPathCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::GraphicsPathCI));
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
    const TypeInfo* GraphicsPathImplements[] = {
        &fl_display::IGraphicsPathTI,
        &fl_display::IGraphicsDataTI,
        NULL
    };

    const TypeInfo GraphicsPathTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::GraphicsPath::InstanceType),
        0,
        0,
        InstanceTraits::fl_display::GraphicsPath::ThunkInfoNum,
        InstanceTraits::fl_display::GraphicsPath::MemberInfoNum,
        "GraphicsPath", "flash.display", &fl::ObjectTI,
        GraphicsPathImplements
    };

    const ClassInfo GraphicsPathCI = {
        &GraphicsPathTI,
        ClassTraits::fl_display::GraphicsPath::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_display::GraphicsPath::ti,
        InstanceTraits::fl_display::GraphicsPath::mi,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

