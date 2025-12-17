//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Media_Microphone.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Media_Microphone_H
#define INC_AS3_Obj_Media_Microphone_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_media
{
    extern const TypeInfo MicrophoneTI;
    extern const ClassInfo MicrophoneCI;
    extern const TypeInfo SoundTransformTI;
    extern const ClassInfo SoundTransformCI;
} // namespace fl_media
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_media
{
    class Microphone;
}}

namespace InstanceTraits { namespace fl_media
{
    class Microphone;
}}

namespace Classes { namespace fl_media
{
    class Microphone;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class SoundTransform;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_media
{
    class Microphone : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Microphone"; }
#endif
    public:
        typedef Classes::fl_media::Microphone ClassType;
        typedef InstanceTraits::fl_events::EventDispatcher InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Microphone(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_media
{
    class Microphone : public Class
    {
        friend class ClassTraits::fl_media::Microphone;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_media::MicrophoneTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Microphone"; }
#endif
    public:
        typedef Microphone SelfType;
        typedef Microphone ClassType;
        
    private:
        Microphone(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

