//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_FrameLabel.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_FrameLabel_H
#define INC_AS3_Obj_Display_FrameLabel_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "GFx/GFx_MovieDef.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo FrameLabelTI;
    extern const ClassInfo FrameLabelCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_display
{
    class FrameLabel;
}}

namespace InstanceTraits { namespace fl_display
{
    class FrameLabel;
}}

namespace Classes { namespace fl_display
{
    class FrameLabel;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class FrameLabel : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::FrameLabel;
        
    public:
        typedef FrameLabel SelfType;
        typedef Classes::fl_display::FrameLabel ClassType;
        typedef InstanceTraits::fl_display::FrameLabel TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::FrameLabelTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::FrameLabel"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        FrameLabel(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        void Set(UInt32 sceneOff, const MovieDataDef::FrameLabelInfo* frameLabelInfo)
        {
            FrameNumber = frameLabelInfo->Number + 1 - sceneOff; // 1-based
            FrameName   = frameLabelInfo->Name;
        }
        void Set(UInt32 frameNum, const ASString& frameName)
        {
            FrameNumber = frameNum;
            FrameName   = frameName;
        }
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_frameGet, 
            mid_nameGet, 
        };
        void frameGet(SInt32& result);
        void nameGet(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        SInt32 frameGet()
        {
            SInt32 result;
            frameGet(result);
            return result;
        }
        ASString nameGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            nameGet(result);
            return result;
        }

//##protect##"instance$data"
        UInt32                              FrameNumber;
        ASString                            FrameName;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class FrameLabel : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::FrameLabel"; }
#endif
    public:
        typedef Instances::fl_display::FrameLabel InstanceType;

    public:
        FrameLabel(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::FrameLabel> MakeInstance(FrameLabel& t)
        {
            return Pickable<Instances::fl_display::FrameLabel>(new(t.Alloc()) Instances::fl_display::FrameLabel(t));
        }
        SPtr<Instances::fl_display::FrameLabel> MakeInstanceS(FrameLabel& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 2 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[2];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class FrameLabel : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::FrameLabel"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::FrameLabel InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        FrameLabel(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

