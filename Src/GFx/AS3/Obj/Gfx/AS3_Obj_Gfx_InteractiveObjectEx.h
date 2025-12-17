//##protect##"disclaimer"
/**********************************************************************

Filename    :   AS3_Obj_Gfx_InteractiveObjectEx.h
Content     :   
Created     :   Jan, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Gfx_InteractiveObjectEx_H
#define INC_AS3_Obj_Gfx_InteractiveObjectEx_H

#include "AS3_Obj_Gfx_DisplayObjectEx.h"
//##protect##"includes"
#include "../Display/AS3_Obj_Display_InteractiveObject.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_gfx
{
    extern const TypeInfo InteractiveObjectExTI;
    extern const ClassInfo InteractiveObjectExCI;
} // namespace fl_gfx
namespace fl_display
{
    extern const TypeInfo InteractiveObjectTI;
    extern const ClassInfo InteractiveObjectCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl

namespace ClassTraits { namespace fl_gfx
{
    class InteractiveObjectEx;
}}

namespace InstanceTraits { namespace fl_gfx
{
    class InteractiveObjectEx;
}}

namespace Classes { namespace fl_gfx
{
    class InteractiveObjectEx;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_gfx
{
    class InteractiveObjectEx : public fl_gfx::DisplayObjectEx
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::InteractiveObjectEx"; }
#endif
    public:
        typedef Classes::fl_gfx::InteractiveObjectEx ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        InteractiveObjectEx(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 4 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[10];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_gfx
{
    class InteractiveObjectEx : public Class
    {
        friend class ClassTraits::fl_gfx::InteractiveObjectEx;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::InteractiveObjectExTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::InteractiveObjectEx"; }
#endif
    public:
        typedef InteractiveObjectEx SelfType;
        typedef InteractiveObjectEx ClassType;
        
    private:
        InteractiveObjectEx(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_setHitTestDisable, 
            mid_getHitTestDisable, 
            mid_setTopmostLevel, 
            mid_getTopmostLevel, 
        };
        void setHitTestDisable(const Value& result, Instances::fl_display::InteractiveObject* o, bool f);
        void getHitTestDisable(bool& result, Instances::fl_display::InteractiveObject* o);
        void setTopmostLevel(const Value& result, Instances::fl_display::InteractiveObject* o, bool f);
        void getTopmostLevel(bool& result, Instances::fl_display::InteractiveObject* o);

        // C++ friendly wrappers for AS3 methods.
        void setHitTestDisable(Instances::fl_display::InteractiveObject* o, bool f)
        {
            setHitTestDisable(Value::GetUndefined(), o, f);
        }
        bool getHitTestDisable(Instances::fl_display::InteractiveObject* o)
        {
            bool result;
            getHitTestDisable(result, o);
            return result;
        }
        void setTopmostLevel(Instances::fl_display::InteractiveObject* o, bool f)
        {
            setTopmostLevel(Value::GetUndefined(), o, f);
        }
        bool getTopmostLevel(Instances::fl_display::InteractiveObject* o)
        {
            bool result;
            getTopmostLevel(result, o);
            return result;
        }

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

