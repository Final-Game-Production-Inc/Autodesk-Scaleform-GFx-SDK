//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_uint.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_uint_H
#define INC_AS3_Obj_uint_H

#include "AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl
{
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl
{
    class uint;
}}

namespace InstanceTraits { namespace fl
{
    class uint;
}}

namespace Classes { namespace fl
{
    class uint;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace InstanceTraits { namespace fl
{
    class uint : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::uint"; }
#endif
    public:
        uint(VM& vm, const ClassInfo& ci);

    public: 
        virtual void MakeObject(Value& result, Traits& t);

        static void AS3toExponential(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void AS3toFixed(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void AS3toPrecision(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void AS3toString(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void AS3valueOf(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void toStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void toLocaleStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void valueOfProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void toExponentialProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void toFixedProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void toPrecisionProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        enum { ThunkInfoNum = 5 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[9];
        static const Abc::ConstValue dva[1];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl
{
    class uint : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::uint"; }
#endif
    public:
        typedef Classes::fl::uint ClassType;
        typedef InstanceTraits::fl::uint InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        uint(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
        virtual bool Coerce(const Value& value, Value& result) const;
        bool CoerceValue(const Value& value, Value& result) const;
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl
{
    class uint : public Class
    {
        friend class ClassTraits::fl::uint;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::uintTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::uint"; }
#endif
    public:
        typedef uint SelfType;
        typedef uint ClassType;
        
    private:
        uint(ClassTraits::Traits& t);
    public:
        virtual void InitPrototype(AS3::Object& obj) const;
        enum { ThunkInfoNum = 6 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[11];
        static const Abc::ConstValue dva[2];
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
    public:
        virtual void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);
        virtual void Construct(Value& result, unsigned argc, const Value* argv, bool extCall = false);
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const UInt32 MIN_VALUE;
        const UInt32 MAX_VALUE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

