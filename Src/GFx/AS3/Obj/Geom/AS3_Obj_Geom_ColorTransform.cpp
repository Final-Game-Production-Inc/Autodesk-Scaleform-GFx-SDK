//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Geom_ColorTransform.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Geom_ColorTransform.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_geom::ColorTransform, Instances::fl_geom::ColorTransform::mid_colorGet, UInt32> TFunc_Instances_ColorTransform_colorGet;
typedef ThunkFunc1<Instances::fl_geom::ColorTransform, Instances::fl_geom::ColorTransform::mid_colorSet, const Value, UInt32> TFunc_Instances_ColorTransform_colorSet;
typedef ThunkFunc1<Instances::fl_geom::ColorTransform, Instances::fl_geom::ColorTransform::mid_concat, const Value, Instances::fl_geom::ColorTransform*> TFunc_Instances_ColorTransform_concat;
typedef ThunkFunc0<Instances::fl_geom::ColorTransform, Instances::fl_geom::ColorTransform::mid_toString, ASString> TFunc_Instances_ColorTransform_toString;

template <> const TFunc_Instances_ColorTransform_colorGet::TMethod TFunc_Instances_ColorTransform_colorGet::Method = &Instances::fl_geom::ColorTransform::colorGet;
template <> const TFunc_Instances_ColorTransform_colorSet::TMethod TFunc_Instances_ColorTransform_colorSet::Method = &Instances::fl_geom::ColorTransform::colorSet;
template <> const TFunc_Instances_ColorTransform_concat::TMethod TFunc_Instances_ColorTransform_concat::Method = &Instances::fl_geom::ColorTransform::concat;
template <> const TFunc_Instances_ColorTransform_toString::TMethod TFunc_Instances_ColorTransform_toString::Method = &Instances::fl_geom::ColorTransform::toString;

namespace Instances { namespace fl_geom
{
    ColorTransform::ColorTransform(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , alphaMultiplier()
    , alphaOffset()
    , blueMultiplier()
    , blueOffset()
    , greenMultiplier()
    , greenOffset()
    , redMultiplier()
    , redOffset()
//##protect##"instance::ColorTransform::ColorTransform()$data"
//##protect##"instance::ColorTransform::ColorTransform()$data"
    {
//##protect##"instance::ColorTransform::ColorTransform()$code"
        alphaMultiplier = blueMultiplier = greenMultiplier = redMultiplier =  1.0;
        alphaOffset = blueOffset = greenOffset = redOffset = 0;
//##protect##"instance::ColorTransform::ColorTransform()$code"
    }

    void ColorTransform::colorGet(UInt32& result)
    {
//##protect##"instance::ColorTransform::colorGet()"
        UInt8 rc = (UInt8)redOffset;
		UInt8 bc = (UInt8)blueOffset;
		UInt8 gc = (UInt8)greenOffset;

		result = (rc << 16) | (gc << 8) | bc;
//##protect##"instance::ColorTransform::colorGet()"
    }
    void ColorTransform::colorSet(const Value& result, UInt32 value)
    {
//##protect##"instance::ColorTransform::colorSet()"
		// When you set this property, it changes the three color offset values 
		// (redOffset, greenOffset, and blueOffset) accordingly, and it sets the 
		// three color multiplier values (redMultiplier, greenMultiplier, and 
		// blueMultiplier) to 0. The alpha transparency multiplier and offset 
		// values do not change.
        // first extract rgb components out of value
		UInt32 rc = 0, gc = 0, bc = 0;
		rc = (value >> 16) & 0x000000ff;
		gc = (value >> 8) & 0x000000ff;
		bc = value & 0x000000ff;
		
		redMultiplier = greenMultiplier = blueMultiplier = 0;
		redOffset = static_cast<float>(rc);
		greenOffset = static_cast<float>(gc);
		blueOffset = static_cast<float>(bc);

		SF_UNUSED1(result);
//##protect##"instance::ColorTransform::colorSet()"
    }
    void ColorTransform::concat(const Value& result, Instances::fl_geom::ColorTransform* second)
    {
//##protect##"instance::ColorTransform::concat()"
		redOffset += redMultiplier*second->redOffset;
		blueOffset += blueMultiplier*second->blueOffset;
		greenOffset += greenMultiplier*second->greenOffset;
		alphaOffset += alphaMultiplier*second->alphaOffset;

		redMultiplier *= second->redMultiplier;
		greenMultiplier *= second->greenMultiplier;
		blueMultiplier *= second->blueMultiplier;
		alphaMultiplier *= second->alphaMultiplier;

        SF_UNUSED1(result);
//##protect##"instance::ColorTransform::concat()"
    }
    void ColorTransform::toString(ASString& result)
    {
//##protect##"instance::ColorTransform::toString()"
		result += GetStringManager().CreateConstString("(redMultiplier=") + GetVM().AsString(Value(redMultiplier)) + 
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("greenMultiplier=") + 
			GetVM().AsString(Value(greenMultiplier)) +
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("blueMultiplier=") + 
			GetVM().AsString(Value(blueMultiplier)) +
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("alphaMultiplier=") + 
			GetVM().AsString(Value(alphaMultiplier)) +
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("redOffset=") + 
			GetVM().AsString(Value(redOffset)) +
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("greenOffset=") + 
			GetVM().AsString(Value(greenOffset)) +
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("blueOffset=") + 
			GetVM().AsString(Value(blueOffset)) +
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("alphaOffset=") + 
			GetVM().AsString(Value(alphaOffset)) +
			GetStringManager().CreateConstString(")");
//##protect##"instance::ColorTransform::toString()"
    }

//##protect##"instance$methods"
	void ColorTransform::AS3Constructor(unsigned argc, const Value* argv)
	{
		// Initialize to default values

		if (argc > 0)
		{
			// According to doc, x will not be modified if there is an exception
			if (!argv[0].Convert2Number(redMultiplier))
			{
				return;
			}
			if (argc > 1)
			{
				if (!argv[1].Convert2Number(greenMultiplier))
				{
					return;
				}
				if (argc > 2)
				{
					if (!argv[2].Convert2Number(blueMultiplier))
					{
						return;
					}
					if (argc > 3)
					{
						if (!argv[3].Convert2Number(alphaMultiplier))
						{
							return;
						}
						if (argc > 4)
						{
							if (!argv[4].Convert2Number(redOffset))
							{
								return;
							}
							if (argc > 5)
							{
								if (!argv[5].Convert2Number(greenOffset))
								{
									return;
								}
								if (argc > 6)
								{
									if (!argv[6].Convert2Number(blueOffset))
									{
										return;
									}
									if (argc > 7)
									{
										if (!argv[7].Convert2Number(alphaOffset))
										{
											return;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_geom
{
    // const UInt16 ColorTransform::tito[ColorTransform::ThunkInfoNum] = {
    //    0, 1, 3, 5, 
    // };
    const TypeInfo* ColorTransform::tit[6] = {
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        NULL, &AS3::fl_geom::ColorTransformTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo ColorTransform::ti[ColorTransform::ThunkInfoNum] = {
        {TFunc_Instances_ColorTransform_colorGet::Func, &ColorTransform::tit[0], "color", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ColorTransform_colorSet::Func, &ColorTransform::tit[1], "color", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ColorTransform_concat::Func, &ColorTransform::tit[3], "concat", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ColorTransform_toString::Func, &ColorTransform::tit[5], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };
    const MemberInfo ColorTransform::mi[ColorTransform::MemberInfoNum] = {
        {"alphaMultiplier", NULL, OFFSETOF(Instances::fl_geom::ColorTransform, alphaMultiplier), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"alphaOffset", NULL, OFFSETOF(Instances::fl_geom::ColorTransform, alphaOffset), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"blueMultiplier", NULL, OFFSETOF(Instances::fl_geom::ColorTransform, blueMultiplier), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"blueOffset", NULL, OFFSETOF(Instances::fl_geom::ColorTransform, blueOffset), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"greenMultiplier", NULL, OFFSETOF(Instances::fl_geom::ColorTransform, greenMultiplier), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"greenOffset", NULL, OFFSETOF(Instances::fl_geom::ColorTransform, greenOffset), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"redMultiplier", NULL, OFFSETOF(Instances::fl_geom::ColorTransform, redMultiplier), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"redOffset", NULL, OFFSETOF(Instances::fl_geom::ColorTransform, redOffset), Abc::NS_Public, SlotInfo::BT_Number, 0},
    };


    ColorTransform::ColorTransform(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::ColorTransform::ColorTransform()"
//##protect##"InstanceTraits::ColorTransform::ColorTransform()"

    }

    void ColorTransform::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<ColorTransform&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_geom
{

    ColorTransform::ColorTransform(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::ColorTransform::ColorTransform()"
//##protect##"ClassTraits::ColorTransform::ColorTransform()"

    }

    Pickable<Traits> ColorTransform::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ColorTransform(vm, AS3::fl_geom::ColorTransformCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_geom::ColorTransformCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
    Cxform ColorTransform::GetCxformFromColorTransform(Instances::fl_geom::ColorTransform* value)
    {
        Cxform cxform;
        if (value)
        {
            cxform.M[0][0] = (float)value->redMultiplier;
            cxform.M[0][1] = (float)value->greenMultiplier;
            cxform.M[0][2] = (float)value->blueMultiplier;
            cxform.M[0][3] = (float)value->alphaMultiplier;
            cxform.M[1][0] = (float)value->redOffset;
            cxform.M[1][1] = (float)value->greenOffset;
            cxform.M[1][2] = (float)value->blueOffset;
            cxform.M[1][3] = (float)value->alphaOffset;
            cxform.Normalize();
        }
        return cxform;
    }
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_geom
{
    const TypeInfo ColorTransformTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_geom::ColorTransform::InstanceType),
        0,
        0,
        InstanceTraits::fl_geom::ColorTransform::ThunkInfoNum,
        InstanceTraits::fl_geom::ColorTransform::MemberInfoNum,
        "ColorTransform", "flash.geom", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo ColorTransformCI = {
        &ColorTransformTI,
        ClassTraits::fl_geom::ColorTransform::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_geom::ColorTransform::ti,
        InstanceTraits::fl_geom::ColorTransform::mi,
    };
}; // namespace fl_geom


}}} // namespace Scaleform { namespace GFx { namespace AS3

