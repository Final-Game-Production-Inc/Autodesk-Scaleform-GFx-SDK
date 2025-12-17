//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Accessibility_AccessibilityImplementation.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Accessibility_AccessibilityImplementation_H
#define INC_AS3_Obj_Accessibility_AccessibilityImplementation_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_accessibility
{
    extern const TypeInfo AccessibilityImplementationTI;
    extern const ClassInfo AccessibilityImplementationCI;
} // namespace fl_accessibility
namespace fl
{
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl
namespace fl_geom
{
    extern const TypeInfo RectangleTI;
    extern const ClassInfo RectangleCI;
} // namespace fl_geom

namespace ClassTraits { namespace fl_accessibility
{
    class AccessibilityImplementation;
}}

namespace InstanceTraits { namespace fl_accessibility
{
    class AccessibilityImplementation;
}}

namespace Classes { namespace fl_accessibility
{
    class AccessibilityImplementation;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class Rectangle;
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_accessibility
{
    class AccessibilityImplementation : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_accessibility::AccessibilityImplementation;
        
    public:
        typedef AccessibilityImplementation SelfType;
        typedef Classes::fl_accessibility::AccessibilityImplementation ClassType;
        typedef InstanceTraits::fl_accessibility::AccessibilityImplementation TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_accessibility::AccessibilityImplementationTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_accessibility::AccessibilityImplementation"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        AccessibilityImplementation(InstanceTraits::Traits& t);

//##protect##"instance$methods"
#ifdef errno
#undef errno
#endif
//##protect##"instance$methods"

    public:

    public:
        // AS3 API members.
        UInt32 errno;
        bool stub;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_accessibility
{
    class AccessibilityImplementation : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::AccessibilityImplementation"; }
#endif
    public:
        typedef Instances::fl_accessibility::AccessibilityImplementation InstanceType;

    public:
        AccessibilityImplementation(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_accessibility::AccessibilityImplementation> MakeInstance(AccessibilityImplementation& t)
        {
            return Pickable<Instances::fl_accessibility::AccessibilityImplementation>(new(t.Alloc()) Instances::fl_accessibility::AccessibilityImplementation(t));
        }
        SPtr<Instances::fl_accessibility::AccessibilityImplementation> MakeInstanceS(AccessibilityImplementation& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 12 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[22];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_accessibility
{
    class AccessibilityImplementation : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::AccessibilityImplementation"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_accessibility::AccessibilityImplementation InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        AccessibilityImplementation(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

