//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_ContextMenuBuiltInItems.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Ui_ContextMenuBuiltInItems_H
#define INC_AS3_Obj_Ui_ContextMenuBuiltInItems_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_ui
{
    extern const TypeInfo ContextMenuBuiltInItemsTI;
    extern const ClassInfo ContextMenuBuiltInItemsCI;
} // namespace fl_ui

namespace ClassTraits { namespace fl_ui
{
    class ContextMenuBuiltInItems;
}}

namespace InstanceTraits { namespace fl_ui
{
    class ContextMenuBuiltInItems;
}}

namespace Classes { namespace fl_ui
{
    class ContextMenuBuiltInItems;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_ui
{
    class ContextMenuBuiltInItems : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_ui::ContextMenuBuiltInItems;
        
    public:
        typedef ContextMenuBuiltInItems SelfType;
        typedef Classes::fl_ui::ContextMenuBuiltInItems ClassType;
        typedef InstanceTraits::fl_ui::ContextMenuBuiltInItems TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_ui::ContextMenuBuiltInItemsTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_ui::ContextMenuBuiltInItems"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        ContextMenuBuiltInItems(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API members.
        bool forwardAndBack;
        bool loop;
        bool play;
        bool print;
        bool quality;
        bool rewind;
        bool save;
        bool zoom;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_ui
{
    class ContextMenuBuiltInItems : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::ContextMenuBuiltInItems"; }
#endif
    public:
        typedef Instances::fl_ui::ContextMenuBuiltInItems InstanceType;

    public:
        ContextMenuBuiltInItems(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_ui::ContextMenuBuiltInItems> MakeInstance(ContextMenuBuiltInItems& t)
        {
            return Pickable<Instances::fl_ui::ContextMenuBuiltInItems>(new(t.Alloc()) Instances::fl_ui::ContextMenuBuiltInItems(t));
        }
        SPtr<Instances::fl_ui::ContextMenuBuiltInItems> MakeInstanceS(ContextMenuBuiltInItems& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 8 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_ui
{
    class ContextMenuBuiltInItems : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ContextMenuBuiltInItems"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_ui::ContextMenuBuiltInItems InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ContextMenuBuiltInItems(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

