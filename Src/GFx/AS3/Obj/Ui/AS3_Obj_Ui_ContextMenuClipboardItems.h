//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_ContextMenuClipboardItems.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Ui_ContextMenuClipboardItems_H
#define INC_AS3_Obj_Ui_ContextMenuClipboardItems_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_ui
{
    extern const TypeInfo ContextMenuClipboardItemsTI;
    extern const ClassInfo ContextMenuClipboardItemsCI;
} // namespace fl_ui

namespace ClassTraits { namespace fl_ui
{
    class ContextMenuClipboardItems;
}}

namespace InstanceTraits { namespace fl_ui
{
    class ContextMenuClipboardItems;
}}

namespace Classes { namespace fl_ui
{
    class ContextMenuClipboardItems;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_ui
{
    class ContextMenuClipboardItems : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_ui::ContextMenuClipboardItems;
        
    public:
        typedef ContextMenuClipboardItems SelfType;
        typedef Classes::fl_ui::ContextMenuClipboardItems ClassType;
        typedef InstanceTraits::fl_ui::ContextMenuClipboardItems TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_ui::ContextMenuClipboardItemsTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_ui::ContextMenuClipboardItems"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        ContextMenuClipboardItems(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API members.
        bool clear;
        bool copy;
        bool cut;
        bool paste;
        bool selectAll;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_ui
{
    class ContextMenuClipboardItems : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::ContextMenuClipboardItems"; }
#endif
    public:
        typedef Instances::fl_ui::ContextMenuClipboardItems InstanceType;

    public:
        ContextMenuClipboardItems(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_ui::ContextMenuClipboardItems> MakeInstance(ContextMenuClipboardItems& t)
        {
            return Pickable<Instances::fl_ui::ContextMenuClipboardItems>(new(t.Alloc()) Instances::fl_ui::ContextMenuClipboardItems(t));
        }
        SPtr<Instances::fl_ui::ContextMenuClipboardItems> MakeInstanceS(ContextMenuClipboardItems& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 5 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_ui
{
    class ContextMenuClipboardItems : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ContextMenuClipboardItems"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_ui::ContextMenuClipboardItems InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ContextMenuClipboardItems(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

