//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Printing_PrintJobOptions.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Printing_PrintJobOptions_H
#define INC_AS3_Obj_Printing_PrintJobOptions_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_printing
{
    extern const TypeInfo PrintJobOptionsTI;
    extern const ClassInfo PrintJobOptionsCI;
} // namespace fl_printing

namespace ClassTraits { namespace fl_printing
{
    class PrintJobOptions;
}}

namespace InstanceTraits { namespace fl_printing
{
    class PrintJobOptions;
}}

namespace Classes { namespace fl_printing
{
    class PrintJobOptions;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_printing
{
    class PrintJobOptions : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_printing::PrintJobOptions;
        
    public:
        typedef PrintJobOptions SelfType;
        typedef Classes::fl_printing::PrintJobOptions ClassType;
        typedef InstanceTraits::fl_printing::PrintJobOptions TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_printing::PrintJobOptionsTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_printing::PrintJobOptions"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        PrintJobOptions(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API members.
        bool printAsBitmap;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_printing
{
    class PrintJobOptions : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::PrintJobOptions"; }
#endif
    public:
        typedef Instances::fl_printing::PrintJobOptions InstanceType;

    public:
        PrintJobOptions(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_printing::PrintJobOptions> MakeInstance(PrintJobOptions& t)
        {
            return Pickable<Instances::fl_printing::PrintJobOptions>(new(t.Alloc()) Instances::fl_printing::PrintJobOptions(t));
        }
        SPtr<Instances::fl_printing::PrintJobOptions> MakeInstanceS(PrintJobOptions& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_printing
{
    class PrintJobOptions : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::PrintJobOptions"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_printing::PrintJobOptions InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        PrintJobOptions(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

