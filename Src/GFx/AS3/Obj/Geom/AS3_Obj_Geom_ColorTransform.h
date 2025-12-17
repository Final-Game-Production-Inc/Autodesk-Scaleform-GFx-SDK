//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Geom_ColorTransform.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Geom_ColorTransform_H
#define INC_AS3_Obj_Geom_ColorTransform_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_geom
{
    extern const TypeInfo ColorTransformTI;
    extern const ClassInfo ColorTransformCI;
} // namespace fl_geom
namespace fl
{
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_geom
{
    class ColorTransform;
}}

namespace InstanceTraits { namespace fl_geom
{
    class ColorTransform;
}}

namespace Classes { namespace fl_geom
{
    class ColorTransform;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_geom
{
    class ColorTransform : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_geom::ColorTransform;
        
    public:
        typedef ColorTransform SelfType;
        typedef Classes::fl_geom::ColorTransform ClassType;
        typedef InstanceTraits::fl_geom::ColorTransform TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_geom::ColorTransformTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_geom::ColorTransform"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        ColorTransform(InstanceTraits::Traits& t);

//##protect##"instance$methods"
		virtual void AS3Constructor(unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_colorGet, 
            mid_colorSet, 
            mid_concat, 
            mid_toString, 
        };
        void colorGet(UInt32& result);
        void colorSet(const Value& result, UInt32 value);
        void concat(const Value& result, Instances::fl_geom::ColorTransform* second);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        UInt32 colorGet()
        {
            UInt32 result;
            colorGet(result);
            return result;
        }
        void colorSet(UInt32 value)
        {
            colorSet(Value::GetUndefined(), value);
        }
        void concat(Instances::fl_geom::ColorTransform* second)
        {
            concat(Value::GetUndefined(), second);
        }
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }

    public:
        // AS3 API members.
        Value::Number alphaMultiplier;
        Value::Number alphaOffset;
        Value::Number blueMultiplier;
        Value::Number blueOffset;
        Value::Number greenMultiplier;
        Value::Number greenOffset;
        Value::Number redMultiplier;
        Value::Number redOffset;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_geom
{
    class ColorTransform : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::ColorTransform"; }
#endif
    public:
        typedef Instances::fl_geom::ColorTransform InstanceType;

    public:
        ColorTransform(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_geom::ColorTransform> MakeInstance(ColorTransform& t)
        {
            return Pickable<Instances::fl_geom::ColorTransform>(new(t.Alloc()) Instances::fl_geom::ColorTransform(t));
        }
        SPtr<Instances::fl_geom::ColorTransform> MakeInstanceS(ColorTransform& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 8 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 4 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[6];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_geom
{
    class ColorTransform : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ColorTransform"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_geom::ColorTransform InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ColorTransform(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
        static Cxform GetCxformFromColorTransform(Instances::fl_geom::ColorTransform* value);
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

