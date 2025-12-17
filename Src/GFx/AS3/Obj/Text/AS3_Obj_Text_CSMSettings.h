//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_CSMSettings.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Text_CSMSettings_H
#define INC_AS3_Obj_Text_CSMSettings_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_text
{
    extern const TypeInfo CSMSettingsTI;
    extern const ClassInfo CSMSettingsCI;
} // namespace fl_text

namespace ClassTraits { namespace fl_text
{
    class CSMSettings;
}}

namespace InstanceTraits { namespace fl_text
{
    class CSMSettings;
}}

namespace Classes { namespace fl_text
{
    class CSMSettings;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_text
{
    class CSMSettings : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_text::CSMSettings;
        
    public:
        typedef CSMSettings SelfType;
        typedef Classes::fl_text::CSMSettings ClassType;
        typedef InstanceTraits::fl_text::CSMSettings TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_text::CSMSettingsTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_text::CSMSettings"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        CSMSettings(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API members.
        Value::Number fontSize;
        Value::Number insideCutoff;
        Value::Number outsideCutoff;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_text
{
    class CSMSettings : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::CSMSettings"; }
#endif
    public:
        typedef Instances::fl_text::CSMSettings InstanceType;

    public:
        CSMSettings(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_text::CSMSettings> MakeInstance(CSMSettings& t)
        {
            return Pickable<Instances::fl_text::CSMSettings>(new(t.Alloc()) Instances::fl_text::CSMSettings(t));
        }
        SPtr<Instances::fl_text::CSMSettings> MakeInstanceS(CSMSettings& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_text
{
    class CSMSettings : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::CSMSettings"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_text::CSMSettings InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        CSMSettings(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

