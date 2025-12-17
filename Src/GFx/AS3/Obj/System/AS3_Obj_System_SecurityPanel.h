//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_System_SecurityPanel.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_System_SecurityPanel_H
#define INC_AS3_Obj_System_SecurityPanel_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_system
{
    extern const TypeInfo SecurityPanelTI;
    extern const ClassInfo SecurityPanelCI;
} // namespace fl_system

namespace ClassTraits { namespace fl_system
{
    class SecurityPanel;
}}

namespace InstanceTraits { namespace fl_system
{
    class SecurityPanel;
}}

namespace Classes { namespace fl_system
{
    class SecurityPanel;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_system
{
    class SecurityPanel : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SecurityPanel"; }
#endif
    public:
        typedef Classes::fl_system::SecurityPanel ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SecurityPanel(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 7 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_system
{
    class SecurityPanel : public Class
    {
        friend class ClassTraits::fl_system::SecurityPanel;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_system::SecurityPanelTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::SecurityPanel"; }
#endif
    public:
        typedef SecurityPanel SelfType;
        typedef SecurityPanel ClassType;
        
    private:
        SecurityPanel(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* CAMERA;
        const char* DEFAULT;
        const char* DISPLAY;
        const char* LOCAL_STORAGE;
        const char* MICROPHONE;
        const char* PRIVACY;
        const char* SETTINGS_MANAGER;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

