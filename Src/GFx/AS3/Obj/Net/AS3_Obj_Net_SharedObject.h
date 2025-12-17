//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_SharedObject.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Net_SharedObject_H
#define INC_AS3_Obj_Net_SharedObject_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
#include "GFx/GFx_SharedObject.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_net
{
    extern const TypeInfo SharedObjectTI;
    extern const ClassInfo SharedObjectCI;
    extern const TypeInfo NetConnectionTI;
    extern const ClassInfo NetConnectionCI;
} // namespace fl_net
namespace fl
{
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl

namespace ClassTraits { namespace fl_net
{
    class SharedObject;
}}

namespace InstanceTraits { namespace fl_net
{
    class SharedObject;
}}

namespace Classes { namespace fl_net
{
    class SharedObject;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_net
    {
        class NetConnection;
        class SharedObject;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_net
{
    class SharedObject : public Instances::fl_events::EventDispatcher
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_net::SharedObject;
        
    public:
        typedef SharedObject SelfType;
        typedef Classes::fl_net::SharedObject ClassType;
        typedef InstanceTraits::fl_net::SharedObject TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_net::SharedObjectTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_net::SharedObject"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        SharedObject(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        const String& GetName() const { return Name; }
        const String& GetLocalPath() const { return LocalPath; }

        bool    SetNameAndLocalPath(const ASString& name, const ASString& localPath);
        // Special setter to initialize the 'data' member
        void    SetDataObject(Object* pobj) { DataObj = pobj; }

    protected:
#ifdef GFX_AS3_ENABLE_SHAREDOBJECT
        bool    FlushImpl(SharedObjectVisitor* writer);
#endif
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_clientGet, 
            mid_clientSet, 
            mid_dataGet, 
            mid_defaultObjectEncodingSet, 
            mid_fpsSet, 
            mid_objectEncodingGet, 
            mid_objectEncodingSet, 
            mid_sizeGet, 
            mid_clear, 
            mid_close, 
            mid_connect, 
            mid_flush, 
            mid_send, 
            mid_setDirty, 
            mid_setProperty, 
        };
        void clientGet(SPtr<Instances::fl::Object>& result);
        void clientSet(const Value& result, const Value& value);
        void dataGet(SPtr<Instances::fl::Object>& result);
        void defaultObjectEncodingSet(const Value& result, UInt32 value);
        void fpsSet(const Value& result, Value::Number value);
        void objectEncodingGet(UInt32& result);
        void objectEncodingSet(const Value& result, UInt32 value);
        void sizeGet(UInt32& result);
        void clear(const Value& result);
        void close(const Value& result);
        void connect(const Value& result, Instances::fl_net::NetConnection* myConnection, const ASString& params);
        void flush(ASString& result, SInt32 minDiskSpace = 0);
        void send(Value& result, unsigned argc, const Value* const argv);
        void setDirty(const Value& result, const ASString& propertyName);
        void setProperty(const Value& result, const ASString& propertyName, const Value& value);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::Object> clientGet()
        {
            SPtr<Instances::fl::Object> result;
            clientGet(result);
            return result;
        }
        void clientSet(const Value& value)
        {
            clientSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl::Object> dataGet()
        {
            SPtr<Instances::fl::Object> result;
            dataGet(result);
            return result;
        }
        void defaultObjectEncodingSet(UInt32 value)
        {
            defaultObjectEncodingSet(Value::GetUndefined(), value);
        }
        void fpsSet(Value::Number value)
        {
            fpsSet(Value::GetUndefined(), value);
        }
        UInt32 objectEncodingGet()
        {
            UInt32 result;
            objectEncodingGet(result);
            return result;
        }
        void objectEncodingSet(UInt32 value)
        {
            objectEncodingSet(Value::GetUndefined(), value);
        }
        UInt32 sizeGet()
        {
            UInt32 result;
            sizeGet(result);
            return result;
        }
        void clear()
        {
            clear(Value::GetUndefined());
        }
        void close()
        {
            close(Value::GetUndefined());
        }
        void connect(Instances::fl_net::NetConnection* myConnection, const ASString& params)
        {
            connect(Value::GetUndefined(), myConnection, params);
        }
        ASString flush(SInt32 minDiskSpace = 0)
        {
            ASString result(GetStringManager().CreateEmptyString());
            flush(result, minDiskSpace);
            return result;
        }
        void setDirty(const ASString& propertyName)
        {
            setDirty(Value::GetUndefined(), propertyName);
        }
        void setProperty(const ASString& propertyName, const Value& value)
        {
            setProperty(Value::GetUndefined(), propertyName, value);
        }

//##protect##"instance$data"
    protected:
        SPtr<Instances::fl::Object> DataObj;
        String                  Name;
        String                  LocalPath;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_net
{
    class SharedObject : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::SharedObject"; }
#endif
    public:
        typedef Instances::fl_net::SharedObject InstanceType;

    public:
        SharedObject(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_net::SharedObject> MakeInstance(SharedObject& t)
        {
            return Pickable<Instances::fl_net::SharedObject>(new(t.Alloc()) Instances::fl_net::SharedObject(t));
        }
        SPtr<Instances::fl_net::SharedObject> MakeInstanceS(SharedObject& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 15 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[25];
        static const Abc::ConstValue dva[1];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_net
{
    class SharedObject : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SharedObject"; }
#endif
    public:
        typedef Classes::fl_net::SharedObject ClassType;
        typedef InstanceTraits::fl_net::SharedObject InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SharedObject(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 2 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[9];
        static const Abc::ConstValue dva[5];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_net
{
    class SharedObject : public Class
    {
        friend class ClassTraits::fl_net::SharedObject;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_net::SharedObjectTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::SharedObject"; }
#endif
    public:
        typedef SharedObject SelfType;
        typedef SharedObject ClassType;
        
    private:
        SharedObject(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
    public:
        virtual void    ForEachChild_GC(Collector* prcc, GcOp op) const;
//##protect##"class_$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_getLocal, 
            mid_getRemote, 
        };
        void getLocal(SPtr<Instances::fl_net::SharedObject>& result, const ASString& name, const ASString& localPath, bool secure);
        void getRemote(SPtr<Instances::fl_net::SharedObject>& result, const ASString& name, const ASString& remotePath, const Value& persistence, bool secure);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_net::SharedObject> getLocal(const ASString& name, const ASString& localPath, bool secure)
        {
            SPtr<Instances::fl_net::SharedObject> result;
            getLocal(result, name, localPath, secure);
            return result;
        }
        SPtr<Instances::fl_net::SharedObject> getRemote(const ASString& name, const ASString& remotePath, const Value& persistence, bool secure)
        {
            SPtr<Instances::fl_net::SharedObject> result;
            getRemote(result, name, remotePath, persistence, secure);
            return result;
        }

//##protect##"class_$data"
    protected:
        typedef ASStringHash< SPtr<Instances::fl_net::SharedObject> >     SOLookupType;
        SOLookupType        SharedObjects;

//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

