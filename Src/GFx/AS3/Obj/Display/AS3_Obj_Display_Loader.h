//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_Loader.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_Loader_H
#define INC_AS3_Obj_Display_Loader_H

#include "AS3_Obj_Display_DisplayObjectContainer.h"
//##protect##"includes"
#include "AS3_Obj_Display_DisplayObjectContainer.h"
#include "../Net/AS3_Obj_Net_URLRequest.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo LoaderTI;
    extern const ClassInfo LoaderCI;
    extern const TypeInfo DisplayObjectTI;
    extern const ClassInfo DisplayObjectCI;
    extern const TypeInfo LoaderInfoTI;
    extern const ClassInfo LoaderInfoCI;
} // namespace fl_display
namespace fl_net
{
    extern const TypeInfo URLRequestTI;
    extern const ClassInfo URLRequestCI;
} // namespace fl_net
namespace fl_system
{
    extern const TypeInfo LoaderContextTI;
    extern const ClassInfo LoaderContextCI;
} // namespace fl_system
namespace fl_utils
{
    extern const TypeInfo ByteArrayTI;
    extern const ClassInfo ByteArrayCI;
} // namespace fl_utils
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl

namespace ClassTraits { namespace fl_display
{
    class Loader;
}}

namespace InstanceTraits { namespace fl_display
{
    class Loader;
}}

namespace Classes { namespace fl_display
{
    class Loader;
}}

//##protect##"forward_declaration"
class AvmLoader;

namespace Instances
{
    namespace fl_net
    {
        class URLRequest;
    }
    namespace fl_utils
    {
        class ByteArray;
    }
    namespace fl_system
    {
        class LoaderContext;
    }
}

class NotifyLoadInitC;

//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class Loader : public Instances::fl_display::DisplayObjectContainer
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::Loader;
        
    public:
        typedef Loader SelfType;
        typedef Classes::fl_display::Loader ClassType;
        typedef InstanceTraits::fl_display::Loader TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::LoaderTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::Loader"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Loader(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
	public:
        ~Loader();
        virtual void InitInstance(bool extCall);
        virtual GFx::DisplayObject* CreateStageObject();

        AvmLoader* GetAvmLoader() const;

        LoaderInfo* GetContentLoaderInfo() const;
    public:
        void ExecuteProgressEvent(UPInt bytesLoaded, UPInt totalBytes);
        void ExecuteCompleteEvent();
        void QueueCompleteEvent();
        void QueueInitEvent(GFx::DisplayObject* obj, Ptr<NotifyLoadInitC> ptr);
        void ExecuteErrorEvent(const char* url);
        void ExecuteOpenEvent();
        void ExecuteInitEvent(GFx::DisplayObject* obj);
        void ResetContent();
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_contentGet, 
            mid_contentLoaderInfoGet, 
            mid_close, 
            mid_load, 
            mid_loadBytes, 
            mid_unload, 
            mid_unloadAndStop, 
        };
        void contentGet(SPtr<Instances::fl_display::DisplayObject>& result);
        void contentLoaderInfoGet(SPtr<Instances::fl_display::LoaderInfo>& result);
        void close(const Value& result);
        void load(const Value& result, Instances::fl_net::URLRequest* request, Instances::fl_system::LoaderContext* context = NULL);
        void loadBytes(const Value& result, Instances::fl_utils::ByteArray* bytes, Instances::fl_system::LoaderContext* context = NULL);
        void unload(const Value& result);
        void unloadAndStop(const Value& result, bool gc = true);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_display::DisplayObject> contentGet();
        SPtr<Instances::fl_display::LoaderInfo> contentLoaderInfoGet();
        void close()
        {
            close(Value::GetUndefined());
        }
        void load(Instances::fl_net::URLRequest* request, Instances::fl_system::LoaderContext* context = NULL)
        {
            load(Value::GetUndefined(), request, context);
        }
        void loadBytes(Instances::fl_utils::ByteArray* bytes, Instances::fl_system::LoaderContext* context = NULL)
        {
            loadBytes(Value::GetUndefined(), bytes, context);
        }
        void unload()
        {
            unload(Value::GetUndefined());
        }
        void unloadAndStop(bool gc = true)
        {
            unloadAndStop(Value::GetUndefined(), gc);
        }

//##protect##"instance$data"
    protected:
        SPtr<LoaderInfo> pContentLoaderInfo;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class Loader : public fl_display::DisplayObjectContainer
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Loader"; }
#endif
    public:
        typedef Instances::fl_display::Loader InstanceType;

    public:
        Loader(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::Loader> MakeInstance(Loader& t)
        {
            return Pickable<Instances::fl_display::Loader>(new(t.Alloc()) Instances::fl_display::Loader(t));
        }
        SPtr<Instances::fl_display::Loader> MakeInstanceS(Loader& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 7 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[12];
        static const Abc::ConstValue dva[1];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class Loader : public fl_display::DisplayObjectContainer
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Loader"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::Loader InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Loader(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

