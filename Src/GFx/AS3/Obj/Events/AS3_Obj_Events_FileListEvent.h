//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_FileListEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_FileListEvent_H
#define INC_AS3_Obj_Events_FileListEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo FileListEventTI;
    extern const ClassInfo FileListEventCI;
} // namespace fl_events

namespace ClassTraits { namespace fl_events
{
    class FileListEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class FileListEvent;
}}

namespace Classes { namespace fl_events
{
    class FileListEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class FileListEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::FileListEvent;
        
    public:
        typedef FileListEvent SelfType;
        typedef Classes::fl_events::FileListEvent ClassType;
        typedef InstanceTraits::fl_events::FileListEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::FileListEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::FileListEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        FileListEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API members.
        SPtr<Instances::fl::Array> files;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class FileListEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::FileListEvent"; }
#endif
    public:
        typedef Instances::fl_events::FileListEvent InstanceType;

    public:
        FileListEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::FileListEvent> MakeInstance(FileListEvent& t)
        {
            return Pickable<Instances::fl_events::FileListEvent>(new(t.Alloc()) Instances::fl_events::FileListEvent(t));
        }
        SPtr<Instances::fl_events::FileListEvent> MakeInstanceS(FileListEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class FileListEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::FileListEvent"; }
#endif
    public:
        typedef Classes::fl_events::FileListEvent ClassType;
        typedef InstanceTraits::fl_events::FileListEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        FileListEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class FileListEvent : public Class
    {
        friend class ClassTraits::fl_events::FileListEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::FileListEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::FileListEvent"; }
#endif
    public:
        typedef FileListEvent SelfType;
        typedef FileListEvent ClassType;
        
    private:
        FileListEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* DIRECTORY_LISTING;
        const char* SELECT_MULTIPLE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

