//##protect##"disclaimer"
/**********************************************************************

Filename    :   .h
Content     :   
Created     :   Mar, 2013
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_External_ExtensionContext_H
#define INC_AS3_Obj_External_ExtensionContext_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_external
{
    extern const TypeInfo ExtensionContextTI;
    extern const ClassInfo ExtensionContextCI;
} // namespace fl_external
namespace fl
{
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_external
{
    class ExtensionContext;
}}

namespace InstanceTraits { namespace fl_external
{
    class ExtensionContext;
}}

namespace Classes { namespace fl_external
{
    class ExtensionContext;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_external
{
    class ExtensionContext : public Instances::fl_events::EventDispatcher
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_external::ExtensionContext;
        
    public:
        typedef ExtensionContext SelfType;
        typedef Classes::fl_external::ExtensionContext ClassType;
        typedef InstanceTraits::fl_external::ExtensionContext TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_external::ExtensionContextTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_external::ExtensionContext"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        ExtensionContext(InstanceTraits::Traits& t);

//##protect##"instance$methods"
		~ExtensionContext();
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_actionScriptDataGet, 
            mid_actionScriptDataSet, 
            mid_call, 
            mid_dispose, 
        };
        void actionScriptDataGet(SPtr<Instances::fl::Object>& result);
        void actionScriptDataSet(const Value& result, const Value& value);
        void call(Value& result, unsigned argc, const Value* const argv);
        void dispose(const Value& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::Object> actionScriptDataGet()
        {
            SPtr<Instances::fl::Object> result;
            actionScriptDataGet(result);
            return result;
        }
        void actionScriptDataSet(const Value& value)
        {
            actionScriptDataSet(Value::GetUndefined(), value);
        }
        void dispose()
        {
            dispose(Value::GetUndefined());
        }

//##protect##"instance$data"
		ASString ExtensionID;
		ASString ContextID;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_external
{
    class ExtensionContext : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::ExtensionContext"; }
#endif
    public:
        typedef Instances::fl_external::ExtensionContext InstanceType;

    public:
        ExtensionContext(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_external::ExtensionContext> MakeInstance(ExtensionContext& t)
        {
            return Pickable<Instances::fl_external::ExtensionContext>(new(t.Alloc()) Instances::fl_external::ExtensionContext(t));
        }
        SPtr<Instances::fl_external::ExtensionContext> MakeInstanceS(ExtensionContext& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 4 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[6];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_external
{
    class ExtensionContext : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ExtensionContext"; }
#endif
    public:
        typedef Classes::fl_external::ExtensionContext ClassType;
        typedef InstanceTraits::fl_external::ExtensionContext InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ExtensionContext(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 2 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[5];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_external
{
    class ExtensionContext : public Class
    {
        friend class ClassTraits::fl_external::ExtensionContext;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_external::ExtensionContextTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::ExtensionContext"; }
#endif
    public:
        typedef ExtensionContext SelfType;
        typedef ExtensionContext ClassType;
        
    private:
        ExtensionContext(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_createExtensionContext, 
            mid_getExtensionDirectory, 
        };
        void createExtensionContext(SPtr<Instances::fl_external::ExtensionContext>& result, const ASString& extensionID, const ASString& contextType);
        void getExtensionDirectory(ASString& result, const ASString& extensionID);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_external::ExtensionContext> createExtensionContext(const ASString& extensionID, const ASString& contextType)
        {
            SPtr<Instances::fl_external::ExtensionContext> result;
            createExtensionContext(result, extensionID, contextType);
            return result;
        }
        ASString getExtensionDirectory(const ASString& extensionID)
        {
            ASString result(GetStringManager().CreateEmptyString());
            getExtensionDirectory(result, extensionID);
            return result;
        }

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

