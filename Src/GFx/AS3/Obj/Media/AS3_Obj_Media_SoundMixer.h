//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Media_SoundMixer.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Media_SoundMixer_H
#define INC_AS3_Obj_Media_SoundMixer_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_media
{
    extern const TypeInfo SoundMixerTI;
    extern const ClassInfo SoundMixerCI;
    extern const TypeInfo SoundTransformTI;
    extern const ClassInfo SoundTransformCI;
} // namespace fl_media
namespace fl
{
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl
namespace fl_utils
{
    extern const TypeInfo ByteArrayTI;
    extern const ClassInfo ByteArrayCI;
} // namespace fl_utils

namespace ClassTraits { namespace fl_media
{
    class SoundMixer;
}}

namespace InstanceTraits { namespace fl_media
{
    class SoundMixer;
}}

namespace Classes { namespace fl_media
{
    class SoundMixer;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class ByteArray;
    class SoundTransform;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_media
{
    class SoundMixer : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SoundMixer"; }
#endif
    public:
        typedef Classes::fl_media::SoundMixer ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SoundMixer(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_media
{
    class SoundMixer : public Class
    {
        friend class ClassTraits::fl_media::SoundMixer;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_media::SoundMixerTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::SoundMixer"; }
#endif
    public:
        typedef SoundMixer SelfType;
        typedef SoundMixer ClassType;
        
    private:
        SoundMixer(ClassTraits::Traits& t);
       
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

