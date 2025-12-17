//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_Scene.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_Scene_H
#define INC_AS3_Obj_Display_Scene_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "GFx/GFx_MovieDef.h"
#include "GFx/GFx_Sprite.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo SceneTI;
    extern const ClassInfo SceneCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
} // namespace fl

namespace ClassTraits { namespace fl_display
{
    class Scene;
}}

namespace InstanceTraits { namespace fl_display
{
    class Scene;
}}

namespace Classes { namespace fl_display
{
    class Scene;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class Scene : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::Scene;
        
    public:
        typedef Scene SelfType;
        typedef Classes::fl_display::Scene ClassType;
        typedef InstanceTraits::fl_display::Scene TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::SceneTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::Scene"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Scene(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        void Set(const MovieDataDef::SceneInfo* sceneInfo);
        void Set(GFx::Sprite* spr);

//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_labelsGet, 
            mid_nameGet, 
            mid_numFramesGet, 
        };
        void labelsGet(SPtr<Instances::fl::Array>& result);
        void nameGet(ASString& result);
        void numFramesGet(SInt32& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::Array> labelsGet();
        ASString nameGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            nameGet(result);
            return result;
        }
        SInt32 numFramesGet()
        {
            SInt32 result;
            numFramesGet(result);
            return result;
        }

//##protect##"instance$data"
        const MovieDataDef::SceneInfo* SceneInfo;
        Ptr<GFx::Sprite>               SpriteObj;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class Scene : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Scene"; }
#endif
    public:
        typedef Instances::fl_display::Scene InstanceType;

    public:
        Scene(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::Scene> MakeInstance(Scene& t)
        {
            return Pickable<Instances::fl_display::Scene>(new(t.Alloc()) Instances::fl_display::Scene(t));
        }
        SPtr<Instances::fl_display::Scene> MakeInstanceS(Scene& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 3 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[3];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class Scene : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Scene"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::Scene InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Scene(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

