//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_AVM1Movie.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_AVM1Movie_H
#define INC_AS3_Obj_Display_AVM1Movie_H

#include "AS3_Obj_Display_DisplayObject.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo AVM1MovieTI;
    extern const ClassInfo AVM1MovieCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class AVM1Movie;
}}

namespace InstanceTraits { namespace fl_display
{
    class AVM1Movie;
}}

namespace Classes { namespace fl_display
{
    class AVM1Movie;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class AVM1Movie : public Instances::fl_display::DisplayObject
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::AVM1Movie;
        
    public:
        typedef AVM1Movie SelfType;
        typedef Classes::fl_display::AVM1Movie ClassType;
        typedef InstanceTraits::fl_display::AVM1Movie TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::AVM1MovieTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::AVM1Movie"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        AVM1Movie(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class AVM1Movie : public fl_display::DisplayObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::AVM1Movie"; }
#endif
    public:
        typedef Instances::fl_display::AVM1Movie InstanceType;

    public:
        AVM1Movie(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::AVM1Movie> MakeInstance(AVM1Movie& t)
        {
            return Pickable<Instances::fl_display::AVM1Movie>(new(t.Alloc()) Instances::fl_display::AVM1Movie(t));
        }
        SPtr<Instances::fl_display::AVM1Movie> MakeInstanceS(AVM1Movie& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class AVM1Movie : public fl_display::DisplayObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::AVM1Movie"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::AVM1Movie InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        AVM1Movie(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

