//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_NativeDragOptions.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Desktop_NativeDragOptions_H
#define INC_AS3_Obj_Desktop_NativeDragOptions_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_desktop
{
    extern const TypeInfo NativeDragOptionsTI;
    extern const ClassInfo NativeDragOptionsCI;
} // namespace fl_desktop
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_desktop
{
    class NativeDragOptions;
}}

namespace InstanceTraits { namespace fl_desktop
{
    class NativeDragOptions;
}}

namespace Classes { namespace fl_desktop
{
    class NativeDragOptions;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_desktop
{
    class NativeDragOptions : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_desktop::NativeDragOptions;
        
    public:
        typedef NativeDragOptions SelfType;
        typedef Classes::fl_desktop::NativeDragOptions ClassType;
        typedef InstanceTraits::fl_desktop::NativeDragOptions TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_desktop::NativeDragOptionsTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_desktop::NativeDragOptions"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        NativeDragOptions(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:

    public:
        // AS3 API members.
        bool allowCopy;
        bool allowLink;
        bool allowMove;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_desktop
{
    class NativeDragOptions : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::NativeDragOptions"; }
#endif
    public:
        typedef Instances::fl_desktop::NativeDragOptions InstanceType;

    public:
        NativeDragOptions(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_desktop::NativeDragOptions> MakeInstance(NativeDragOptions& t)
        {
            return Pickable<Instances::fl_desktop::NativeDragOptions>(new(t.Alloc()) Instances::fl_desktop::NativeDragOptions(t));
        }
        SPtr<Instances::fl_desktop::NativeDragOptions> MakeInstanceS(NativeDragOptions& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[1];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_desktop
{
    class NativeDragOptions : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::NativeDragOptions"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_desktop::NativeDragOptions InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        NativeDragOptions(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

