//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Accessibility_AccessibilityProperties.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Accessibility_AccessibilityProperties_H
#define INC_AS3_Obj_Accessibility_AccessibilityProperties_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_accessibility
{
    extern const TypeInfo AccessibilityPropertiesTI;
    extern const ClassInfo AccessibilityPropertiesCI;
} // namespace fl_accessibility

namespace ClassTraits { namespace fl_accessibility
{
    class AccessibilityProperties;
}}

namespace InstanceTraits { namespace fl_accessibility
{
    class AccessibilityProperties;
}}

namespace Classes { namespace fl_accessibility
{
    class AccessibilityProperties;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_accessibility
{
    class AccessibilityProperties : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_accessibility::AccessibilityProperties;
        
    public:
        typedef AccessibilityProperties SelfType;
        typedef Classes::fl_accessibility::AccessibilityProperties ClassType;
        typedef InstanceTraits::fl_accessibility::AccessibilityProperties TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_accessibility::AccessibilityPropertiesTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_accessibility::AccessibilityProperties"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        AccessibilityProperties(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API members.
        ASString description;
        bool forceSimple;
        ASString name;
        bool noAutoLabeling;
        ASString shortcut;
        bool silent;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_accessibility
{
    class AccessibilityProperties : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::AccessibilityProperties"; }
#endif
    public:
        typedef Instances::fl_accessibility::AccessibilityProperties InstanceType;

    public:
        AccessibilityProperties(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_accessibility::AccessibilityProperties> MakeInstance(AccessibilityProperties& t)
        {
            return Pickable<Instances::fl_accessibility::AccessibilityProperties>(new(t.Alloc()) Instances::fl_accessibility::AccessibilityProperties(t));
        }
        SPtr<Instances::fl_accessibility::AccessibilityProperties> MakeInstanceS(AccessibilityProperties& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 6 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_accessibility
{
    class AccessibilityProperties : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::AccessibilityProperties"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_accessibility::AccessibilityProperties InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        AccessibilityProperties(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

