//##protect##"disclaimer"
/**********************************************************************

Filename    :   .h
Content     :   
Created     :   Jul, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Gfx_GamePad_H
#define INC_AS3_Obj_Gfx_GamePad_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_gfx
{
    extern const TypeInfo GamePadTI;
    extern const ClassInfo GamePadCI;
} // namespace fl_gfx
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl

namespace ClassTraits { namespace fl_gfx
{
    class GamePad;
}}

namespace InstanceTraits { namespace fl_gfx
{
    class GamePad;
}}

namespace Classes { namespace fl_gfx
{
    class GamePad;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_gfx
{
    class GamePad : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::GamePad"; }
#endif
    public:
        typedef Classes::fl_gfx::GamePad ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        GamePad(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 29 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[1];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_gfx
{
    class GamePad : public Class
    {
        friend class ClassTraits::fl_gfx::GamePad;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::GamePadTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::GamePad"; }
#endif
    public:
        typedef GamePad SelfType;
        typedef GamePad ClassType;
        
    private:
        GamePad(ClassTraits::Traits& t);
       
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
            mid_supportsAnalogEvents, 
        };
        void supportsAnalogEvents(bool& result);

        // C++ friendly wrappers for AS3 methods.
        bool supportsAnalogEvents()
        {
            bool result;
            supportsAnalogEvents(result);
            return result;
        }

    public:
        // AS3 API members.
        Value PAD_NONE;
        Value PAD_BACK;
        Value PAD_START;
        Value PAD_A;
        Value PAD_B;
        Value PAD_X;
        Value PAD_Y;
        Value PAD_R1;
        Value PAD_L1;
        Value PAD_R2;
        Value PAD_L2;
        Value PAD_UP;
        Value PAD_DOWN;
        Value PAD_RIGHT;
        Value PAD_LEFT;
        Value PAD_PLUS;
        Value PAD_MINUS;
        Value PAD_1;
        Value PAD_2;
        Value PAD_H;
        Value PAD_C;
        Value PAD_Z;
        Value PAD_O;
        Value PAD_T;
        Value PAD_S;
        Value PAD_SELECT;
        Value PAD_HOME;
        Value PAD_RT;
        Value PAD_LT;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

