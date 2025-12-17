//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Html_HTMLWindowCreateOptions.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Html_HTMLWindowCreateOptions_H
#define INC_AS3_Obj_Html_HTMLWindowCreateOptions_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_html
{
    extern const TypeInfo HTMLWindowCreateOptionsTI;
    extern const ClassInfo HTMLWindowCreateOptionsCI;
} // namespace fl_html

namespace ClassTraits { namespace fl_html
{
    class HTMLWindowCreateOptions;
}}

namespace InstanceTraits { namespace fl_html
{
    class HTMLWindowCreateOptions;
}}

namespace Classes { namespace fl_html
{
    class HTMLWindowCreateOptions;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_html
{
    class HTMLWindowCreateOptions : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_html::HTMLWindowCreateOptions;
        
    public:
        typedef HTMLWindowCreateOptions SelfType;
        typedef Classes::fl_html::HTMLWindowCreateOptions ClassType;
        typedef InstanceTraits::fl_html::HTMLWindowCreateOptions TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_html::HTMLWindowCreateOptionsTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_html::HTMLWindowCreateOptions"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        HTMLWindowCreateOptions(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API members.
        bool fullscreen;
        Value::Number height;
        bool locationBarVisible;
        bool menuBarVisible;
        bool resizable;
        bool scrollBarsVisible;
        bool statusBarVisible;
        bool toolBarVisible;
        Value::Number width;
        Value::Number x;
        Value::Number y;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_html
{
    class HTMLWindowCreateOptions : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::HTMLWindowCreateOptions"; }
#endif
    public:
        typedef Instances::fl_html::HTMLWindowCreateOptions InstanceType;

    public:
        HTMLWindowCreateOptions(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_html::HTMLWindowCreateOptions> MakeInstance(HTMLWindowCreateOptions& t)
        {
            return Pickable<Instances::fl_html::HTMLWindowCreateOptions>(new(t.Alloc()) Instances::fl_html::HTMLWindowCreateOptions(t));
        }
        SPtr<Instances::fl_html::HTMLWindowCreateOptions> MakeInstanceS(HTMLWindowCreateOptions& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 11 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_html
{
    class HTMLWindowCreateOptions : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::HTMLWindowCreateOptions"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_html::HTMLWindowCreateOptions InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        HTMLWindowCreateOptions(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

