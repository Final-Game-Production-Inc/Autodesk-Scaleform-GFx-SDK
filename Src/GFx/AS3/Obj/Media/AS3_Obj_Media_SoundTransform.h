//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Media_SoundTransform.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Media_SoundTransform_H
#define INC_AS3_Obj_Media_SoundTransform_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_media
{
    extern const TypeInfo SoundTransformTI;
    extern const ClassInfo SoundTransformCI;
} // namespace fl_media
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
} // namespace fl

namespace ClassTraits { namespace fl_media
{
    class SoundTransform;
}}

namespace InstanceTraits { namespace fl_media
{
    class SoundTransform;
}}

namespace Classes { namespace fl_media
{
    class SoundTransform;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_media
{
    class SoundTransform : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_media::SoundTransform;
        
    public:
        typedef SoundTransform SelfType;
        typedef Classes::fl_media::SoundTransform ClassType;
        typedef InstanceTraits::fl_media::SoundTransform TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_media::SoundTransformTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_media::SoundTransform"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        SoundTransform(InstanceTraits::Traits& t);

//##protect##"instance$methods"
#ifdef GFX_ENABLE_SOUND
        virtual void AS3Constructor(unsigned argc, const Value* argv);
#endif
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_leftToLeftGet, 
            mid_leftToLeftSet, 
            mid_leftToRightGet, 
            mid_leftToRightSet, 
            mid_panGet, 
            mid_panSet, 
            mid_rightToLeftGet, 
            mid_rightToLeftSet, 
            mid_rightToRightGet, 
            mid_rightToRightSet, 
            mid_volumeGet, 
            mid_volumeSet, 
        };
        void leftToLeftGet(Value::Number& result);
        void leftToLeftSet(const Value& result, Value::Number value);
        void leftToRightGet(Value::Number& result);
        void leftToRightSet(const Value& result, Value::Number value);
        void panGet(Value::Number& result);
        void panSet(const Value& result, Value::Number value);
        void rightToLeftGet(Value::Number& result);
        void rightToLeftSet(const Value& result, Value::Number value);
        void rightToRightGet(Value::Number& result);
        void rightToRightSet(const Value& result, Value::Number value);
        void volumeGet(Value::Number& result);
        void volumeSet(const Value& result, Value::Number value);

        // C++ friendly wrappers for AS3 methods.
        Value::Number leftToLeftGet()
        {
            Value::Number result;
            leftToLeftGet(result);
            return result;
        }
        void leftToLeftSet(Value::Number value)
        {
            leftToLeftSet(Value::GetUndefined(), value);
        }
        Value::Number leftToRightGet()
        {
            Value::Number result;
            leftToRightGet(result);
            return result;
        }
        void leftToRightSet(Value::Number value)
        {
            leftToRightSet(Value::GetUndefined(), value);
        }
        Value::Number panGet()
        {
            Value::Number result;
            panGet(result);
            return result;
        }
        void panSet(Value::Number value)
        {
            panSet(Value::GetUndefined(), value);
        }
        Value::Number rightToLeftGet()
        {
            Value::Number result;
            rightToLeftGet(result);
            return result;
        }
        void rightToLeftSet(Value::Number value)
        {
            rightToLeftSet(Value::GetUndefined(), value);
        }
        Value::Number rightToRightGet()
        {
            Value::Number result;
            rightToRightGet(result);
            return result;
        }
        void rightToRightSet(Value::Number value)
        {
            rightToRightSet(Value::GetUndefined(), value);
        }
        Value::Number volumeGet()
        {
            Value::Number result;
            volumeGet(result);
            return result;
        }
        void volumeSet(Value::Number value)
        {
            volumeSet(Value::GetUndefined(), value);
        }

//##protect##"instance$data"
#ifdef GFX_ENABLE_SOUND
        Value::Number Volume;
        Value::Number Pan;
#endif
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_media
{
    class SoundTransform : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::SoundTransform"; }
#endif
    public:
        typedef Instances::fl_media::SoundTransform InstanceType;

    public:
        SoundTransform(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_media::SoundTransform> MakeInstance(SoundTransform& t)
        {
            return Pickable<Instances::fl_media::SoundTransform>(new(t.Alloc()) Instances::fl_media::SoundTransform(t));
        }
        SPtr<Instances::fl_media::SoundTransform> MakeInstanceS(SoundTransform& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 12 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[18];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_media
{
    class SoundTransform : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SoundTransform"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_media::SoundTransform InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SoundTransform(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

