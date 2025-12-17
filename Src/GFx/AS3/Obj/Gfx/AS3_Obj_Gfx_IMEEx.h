//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Gfx_IMEEx.h
Content     :   
Created     :   Jan, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Gfx_IMEEx_H
#define INC_AS3_Obj_Gfx_IMEEx_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
#include "AS3_Obj_Gfx_IMECandidateListStyle.h"
#include "../../../AS3/Obj/Display/AS3_Obj_Display_Sprite.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_gfx
{
    extern const TypeInfo IMEExTI;
    extern const ClassInfo IMEExCI;
    extern const TypeInfo IMECandidateListStyleTI;
    extern const ClassInfo IMECandidateListStyleCI;
} // namespace fl_gfx
namespace fl_display
{
    extern const TypeInfo SpriteTI;
    extern const ClassInfo SpriteCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_gfx
{
    class IMEEx;
}}

namespace InstanceTraits { namespace fl_gfx
{
    class IMEEx;
}}

namespace Classes { namespace fl_gfx
{
    class IMEEx;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_gfx
{
    class IMEEx : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::IMEEx"; }
#endif
    public:
        typedef Classes::fl_gfx::IMEEx ClassType;
        typedef InstanceTraits::fl_events::EventDispatcher InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        IMEEx(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 5 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[9];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_gfx
{
    class IMEEx : public Class
    {
        friend class ClassTraits::fl_gfx::IMEEx;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::IMEExTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::IMEEx"; }
#endif
    public:
        typedef IMEEx SelfType;
        typedef IMEEx ClassType;
        
    private:
        IMEEx(ClassTraits::Traits& t);
       
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
            mid_getIMECandidateListStyle, 
            mid_setIMECandidateListStyle, 
            mid_SendLangBarMessage, 
            mid_GetOSVersion, 
            mid_GetCompositionString, 
        };
        void getIMECandidateListStyle(SPtr<Instances::fl_gfx::IMECandidateListStyle>& result);
        void setIMECandidateListStyle(const Value& result, Instances::fl_gfx::IMECandidateListStyle* style);
        void SendLangBarMessage(const Value& result, Instances::fl_display::Sprite* mc, const ASString& command, const ASString& message);
        void GetOSVersion(ASString& result);
        void GetCompositionString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_gfx::IMECandidateListStyle> getIMECandidateListStyle();
        void setIMECandidateListStyle(Instances::fl_gfx::IMECandidateListStyle* style)
        {
            setIMECandidateListStyle(Value::GetUndefined(), style);
        }
        void SendLangBarMessage(Instances::fl_display::Sprite* mc, const ASString& command, const ASString& message)
        {
            SendLangBarMessage(Value::GetUndefined(), mc, command, message);
        }
        ASString GetOSVersion()
        {
            ASString result(GetStringManager().CreateEmptyString());
            GetOSVersion(result);
            return result;
        }
        ASString GetCompositionString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            GetCompositionString(result);
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

