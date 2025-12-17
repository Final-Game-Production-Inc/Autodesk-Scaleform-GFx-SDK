//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Media_SoundLoaderContext.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Media_SoundLoaderContext_H
#define INC_AS3_Obj_Media_SoundLoaderContext_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_media
{
    extern const TypeInfo SoundLoaderContextTI;
    extern const ClassInfo SoundLoaderContextCI;
} // namespace fl_media

namespace ClassTraits { namespace fl_media
{
    class SoundLoaderContext;
}}

namespace InstanceTraits { namespace fl_media
{
    class SoundLoaderContext;
}}

namespace Classes { namespace fl_media
{
    class SoundLoaderContext;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_media
{
    class SoundLoaderContext : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_media::SoundLoaderContext;
        
    public:
        typedef SoundLoaderContext SelfType;
        typedef Classes::fl_media::SoundLoaderContext ClassType;
        typedef InstanceTraits::fl_media::SoundLoaderContext TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_media::SoundLoaderContextTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_media::SoundLoaderContext"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        SoundLoaderContext(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API members.
        Value::Number bufferTime;
        bool checkPolicyFile;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_media
{
    class SoundLoaderContext : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::SoundLoaderContext"; }
#endif
    public:
        typedef Instances::fl_media::SoundLoaderContext InstanceType;

    public:
        SoundLoaderContext(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_media::SoundLoaderContext> MakeInstance(SoundLoaderContext& t)
        {
            return Pickable<Instances::fl_media::SoundLoaderContext>(new(t.Alloc()) Instances::fl_media::SoundLoaderContext(t));
        }
        SPtr<Instances::fl_media::SoundLoaderContext> MakeInstanceS(SoundLoaderContext& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_media
{
    class SoundLoaderContext : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SoundLoaderContext"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_media::SoundLoaderContext InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SoundLoaderContext(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

