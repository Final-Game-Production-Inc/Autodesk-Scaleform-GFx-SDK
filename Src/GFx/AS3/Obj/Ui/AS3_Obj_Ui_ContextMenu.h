//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_ContextMenu.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Ui_ContextMenu_H
#define INC_AS3_Obj_Ui_ContextMenu_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
#include "AS3_Obj_Ui_ContextMenuBuiltInItems.h"
#include "../AS3_Obj_Array.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_ui
{
    extern const TypeInfo ContextMenuTI;
    extern const ClassInfo ContextMenuCI;
    extern const TypeInfo ContextMenuBuiltInItemsTI;
    extern const ClassInfo ContextMenuBuiltInItemsCI;
} // namespace fl_ui
namespace fl
{
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
} // namespace fl
namespace fl_display
{
    extern const TypeInfo NativeMenuTI;
    extern const ClassInfo NativeMenuCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_ui
{
    class ContextMenu;
}}

namespace InstanceTraits { namespace fl_ui
{
    class ContextMenu;
}}

namespace Classes { namespace fl_ui
{
    class ContextMenu;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_display
    {
        class NativeMenu;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_ui
{
    class ContextMenu : public Instances::fl_events::EventDispatcher
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_ui::ContextMenu;
        
    public:
        typedef ContextMenu SelfType;
        typedef Classes::fl_ui::ContextMenu ClassType;
        typedef InstanceTraits::fl_ui::ContextMenu TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_ui::ContextMenuTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_ui::ContextMenu"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        ContextMenu(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_builtInItemsGet, 
            mid_builtInItemsSet, 
            mid_customItemsGet, 
            mid_customItemsSet, 
            mid_clone, 
            mid_hideBuiltInItems, 
        };
        void builtInItemsGet(SPtr<Instances::fl_ui::ContextMenuBuiltInItems>& result);
        void builtInItemsSet(const Value& result, Instances::fl_ui::ContextMenuBuiltInItems* value);
        void customItemsGet(SPtr<Instances::fl::Array>& result);
        void customItemsSet(const Value& result, Instances::fl::Array* value);
        void clone(SPtr<Instances::fl_display::NativeMenu>& result);
        void hideBuiltInItems(const Value& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_ui::ContextMenuBuiltInItems> builtInItemsGet();
        void builtInItemsSet(Instances::fl_ui::ContextMenuBuiltInItems* value)
        {
            builtInItemsSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl::Array> customItemsGet();
        void customItemsSet(Instances::fl::Array* value)
        {
            customItemsSet(Value::GetUndefined(), value);
        }
        void hideBuiltInItems()
        {
            hideBuiltInItems(Value::GetUndefined());
        }

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_ui
{
    class ContextMenu : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::ContextMenu"; }
#endif
    public:
        typedef Instances::fl_ui::ContextMenu InstanceType;

    public:
        ContextMenu(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_ui::ContextMenu> MakeInstance(ContextMenu& t)
        {
            return Pickable<Instances::fl_ui::ContextMenu>(new(t.Alloc()) Instances::fl_ui::ContextMenu(t));
        }
        SPtr<Instances::fl_ui::ContextMenu> MakeInstanceS(ContextMenu& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 5 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[7];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_ui
{
    class ContextMenu : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ContextMenu"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_ui::ContextMenu InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ContextMenu(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

