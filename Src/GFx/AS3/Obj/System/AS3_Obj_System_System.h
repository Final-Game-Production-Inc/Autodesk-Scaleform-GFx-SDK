//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_System_System.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_System_System_H
#define INC_AS3_Obj_System_System_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_system
{
    extern const TypeInfo SystemTI;
    extern const ClassInfo SystemCI;
} // namespace fl_system
namespace fl
{
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_system
{
    class System;
}}

namespace InstanceTraits { namespace fl_system
{
    class System;
}}

namespace Classes { namespace fl_system
{
    class System;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class IME;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_system
{
    class System : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::System"; }
#endif
    public:
        typedef Classes::fl_system::System ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        System(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 7 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[9];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_system
{
    class System : public Class
    {
        friend class ClassTraits::fl_system::System;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_system::SystemTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::System"; }
#endif
    public:
        typedef System SelfType;
        typedef System ClassType;
        
    private:
        System(ClassTraits::Traits& t);
       
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
            mid_imeGet, 
            mid_totalMemoryGet, 
            mid_exit, 
            mid_gc, 
            mid_pause, 
            mid_resume, 
            mid_setClipboard, 
        };
        void imeGet(Value& result);
        void totalMemoryGet(UInt32& result);
        void exit(const Value& result, UInt32 code);
        void gc(const Value& result);
        void pause(const Value& result);
        void resume(const Value& result);
        void setClipboard(const Value& result, const ASString& string);

        // C++ friendly wrappers for AS3 methods.
        Value imeGet()
        {
            Value result;
            imeGet(result);
            return result;
        }
        UInt32 totalMemoryGet()
        {
            UInt32 result;
            totalMemoryGet(result);
            return result;
        }
        void exit(UInt32 code)
        {
            exit(Value::GetUndefined(), code);
        }
        void gc()
        {
            gc(Value::GetUndefined());
        }
        void pause()
        {
            pause(Value::GetUndefined());
        }
        void resume()
        {
            resume(Value::GetUndefined());
        }
        void setClipboard(const ASString& string)
        {
            setClipboard(Value::GetUndefined(), string);
        }

    public:
        // AS3 API members.
        bool useCodePage;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

