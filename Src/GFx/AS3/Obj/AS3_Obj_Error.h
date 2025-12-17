//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Error.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Error_H
#define INC_AS3_Obj_Error_H

#include "AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl
{
    extern const TypeInfo ErrorTI;
    extern const ClassInfo ErrorCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo ClassTI;
    extern const ClassInfo ClassCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
} // namespace fl

namespace ClassTraits { namespace fl
{
    class Error;
}}

namespace InstanceTraits { namespace fl
{
    class Error;
}}

namespace Classes { namespace fl
{
    class Error;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl
{
    class Error : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl::Error;
        
    public:
        typedef Error SelfType;
        typedef Classes::fl::Error ClassType;
        typedef InstanceTraits::fl::Error TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::ErrorTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl::Error"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Error(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        Error(InstanceTraits::Traits& t, int id SF_DEBUG_ARG(const ASString& msg));

    public:
        virtual void AS3Constructor(unsigned argc, const Value* argv);

        const ASString& GetMessage() const
        {
            return message;
        }
        const ASString& GetErrorName() const
        {
            return name;
        }
        int GetID() const
        {
            return ID;
        }
        
    private:
        //
        void SetMessage(ASStringNode* id)
        {
            message.AssignNode(id);
        }
        void SetMessage(const ASString& id)
        {
            message = id;
        }
        //
        void SetErrorName(ASStringNode* id)
        {
            name.AssignNode(id);
        }
        void SetErrorName(const ASString& id)
        {
            name = id;
        }
        void SetID(int id)
        {
            ID = id;
        }
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_errorIDGet, 
            mid_getStackTrace, 
            mid_toStringProto, 
        };
        void errorIDGet(SInt32& result);
        void getStackTrace(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        SInt32 errorIDGet()
        {
            SInt32 result;
            errorIDGet(result);
            return result;
        }
        ASString getStackTrace()
        {
            ASString result(GetStringManager().CreateEmptyString());
            getStackTrace(result);
            return result;
        }
        void toStringProto(ASString& result);

    public:
        // AS3 API members.
        ASString message;
        ASString name;

//##protect##"instance$data"
        int ID;
#ifdef GFX_AS3_VERBOSE
        ASString StackTrace;
#endif
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    class Error : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Error"; }
#endif
    public:
        typedef Instances::fl::Error InstanceType;

    public:
        Error(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl::Error> MakeInstance(Error& t)
        {
            return Pickable<Instances::fl::Error>(new(t.Alloc()) Instances::fl::Error(t));
        }
        SPtr<Instances::fl::Error> MakeInstanceS(Error& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 2 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[2];
//##protect##"instance_traits$methods"
        Pickable<Instances::fl::Error> MakeInstance(Traits& t, int id SF_DEBUG_ARG(const ASString& msg))
        {
            return Pickable<Instances::fl::Error>(new(t.Alloc()) Instances::fl::Error(*this, id SF_DEBUG_ARG(msg)));
        }
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl
{
    class Error : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Error"; }
#endif
    public:
        typedef Classes::fl::Error ClassType;
        typedef InstanceTraits::fl::Error InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Error(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 2 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[5];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl
{
    class Error : public Class
    {
        friend class ClassTraits::fl::Error;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::ErrorTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Error"; }
#endif
    public:
        typedef Error SelfType;
        typedef Error ClassType;
        
    private:
        Error(ClassTraits::Traits& t);
    public:
        virtual void InitPrototype(AS3::Object& obj) const;
        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[1];
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
        virtual void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);
        virtual Pickable<AS3::Object> MakePrototype() const;
//##protect##"class_$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_getErrorMessage, 
            mid_throwError, 
        };
        void getErrorMessage(ASString& result, SInt32 index);
        void throwError(Value& result, unsigned argc, const Value* const argv);

        // C++ friendly wrappers for AS3 methods.
        ASString getErrorMessage(SInt32 index)
        {
            ASString result(GetStringManager().CreateEmptyString());
            getErrorMessage(result, index);
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

