//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_RegExp.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_RegExp_H
#define INC_AS3_Obj_RegExp_H

#include "AS3_Obj_Object.h"
//##protect##"includes"
#ifdef SF_ENABLE_PCRE
#define PCRE_STATIC 1
#include "pcre2.h"
#endif
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

    // 替代 pcre2_utf_to_uc：校验单个 UTF-8 字符合法性，返回其字节长度（失败返回 PCRE2 错误码）
    // 参数：
    //   code_point: 输出解析后的 Unicode 码点（传 NULL 则不输出）
    //   s:          待校验的 UTF-8 字符起始指针
    //   max_len:    剩余可读取的字节数（防止越界）
    // 返回值：
    //   成功：返回该 UTF-8 字符的字节长度（1~4）
    //   失败：返回 PCRE2_ERROR_UTF8_ERR1 ~ PCRE2_ERROR_UTF8_ERR23 等错误码
    extern int pcre2_utf8_char_check(uint32_t* code_point, PCRE2_SPTR8 s, PCRE2_SIZE max_len);

    // 替代 pcre2_utf_strlen：统计 UTF-8 字符串的「字符数」（非字节数）
    // 参数：
    //   s:        待统计的 UTF-8 字符串（PCRE2_SPTR8 对应 const uint8_t*）
    //   length:   字节长度（PCRE2_UNSET 表示字符串以 0 终止）
    //   error:    输出错误码（NULL 则忽略，失败时返回 PCRE2 UTF8 错误码）
    // 返回值：
    //   成功：UTF-8 字符数
    //   失败：PCRE2_UNSET
    extern PCRE2_SIZE pcre2_utf8_strlen(PCRE2_SPTR8 s, PCRE2_SIZE length, int* error);


// Forward declaration.
namespace fl
{
    extern const TypeInfo RegExpTI;
    extern const ClassInfo RegExpCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
} // namespace fl

namespace ClassTraits { namespace fl
{
    class RegExp;
}}

namespace InstanceTraits { namespace fl
{
    class RegExp;
}}

namespace Classes { namespace fl
{
    class RegExp;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl
{
    class RegExp : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl::RegExp;
        
    public:
        typedef RegExp SelfType;
        typedef Classes::fl::RegExp ClassType;
        typedef InstanceTraits::fl::RegExp TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::RegExpTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl::RegExp"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        RegExp(InstanceTraits::Traits& t);

//##protect##"instance$methods"
#ifdef SF_ENABLE_PCRE
        virtual void AS3Constructor(unsigned argc, const Value* argv);

        bool        hasOption(int mask);
        ASString    optionFlagsGet();

    public:
        void        SetGlobal(bool global) { IsGlobal = global; }
        SInt32      GetMatchOffset() { return MatchOffset; }
        SInt32      GetMatchLength() { return MatchLength; }
        ASString    ToString();

        ~RegExp();
#else
    public:
        void        SetGlobal(bool)  {}
        SInt32      GetMatchOffset() { return -1; }
        SInt32      GetMatchLength() { return 0; }
#endif
    public:
        static void toStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_sourceGet, 
            mid_globalGet, 
            mid_ignoreCaseGet, 
            mid_multilineGet, 
            mid_lastIndexGet, 
            mid_lastIndexSet, 
            mid_dotallGet, 
            mid_extendedGet, 
            mid_AS3exec, 
            mid_AS3test, 
        };
        void sourceGet(ASString& result);
        void globalGet(bool& result);
        void ignoreCaseGet(bool& result);
        void multilineGet(bool& result);
        void lastIndexGet(SInt32& result);
        void lastIndexSet(const Value& result, SInt32 i);
        void dotallGet(bool& result);
        void extendedGet(bool& result);
        void AS3exec(SPtr<Instances::fl::Object>& result, const ASString& s);
        void AS3test(bool& result, const ASString& s);

        // C++ friendly wrappers for AS3 methods.
        ASString sourceGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            sourceGet(result);
            return result;
        }
        bool globalGet()
        {
            bool result;
            globalGet(result);
            return result;
        }
        bool ignoreCaseGet()
        {
            bool result;
            ignoreCaseGet(result);
            return result;
        }
        bool multilineGet()
        {
            bool result;
            multilineGet(result);
            return result;
        }
        SInt32 lastIndexGet()
        {
            SInt32 result;
            lastIndexGet(result);
            return result;
        }
        void lastIndexSet(SInt32 i)
        {
            lastIndexSet(Value::GetUndefined(), i);
        }
        bool dotallGet()
        {
            bool result;
            dotallGet(result);
            return result;
        }
        bool extendedGet()
        {
            bool result;
            extendedGet(result);
            return result;
        }
        SPtr<Instances::fl::Object> AS3exec(const ASString& s)
        {
            SPtr<Instances::fl::Object> result;
            AS3exec(result, s);
            return result;
        }
        bool AS3test(const ASString& s)
        {
            bool result;
            AS3test(result, s);
            return result;
        }

//##protect##"instance$data"
#ifdef SF_ENABLE_PCRE
        static const UInt32 OUTPUT_VECTOR_SIZE  = 99;   // Output 32 matches (32+1)*3

        pcre2_code*   CompRegExp;     // Compiled regexp pattern
        pcre2_match_data* MatchData; // Pcre2 In

        SInt32  MatchOffset;    // Last match offset
        SInt32  MatchLength;    // Last match length

        String  Pattern;        // Source pattern string
        bool    IsGlobal;       // Is global matching
        SInt32  LastIndex;      // Last processed offset
        int     OptionFlags;    // Options
        bool    HasNamedGroups; // Named groups flag
#endif
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    class RegExp : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::RegExp"; }
#endif
    public:
        typedef Instances::fl::RegExp InstanceType;

    public:
        RegExp(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl::RegExp> MakeInstance(RegExp& t)
        {
            return Pickable<Instances::fl::RegExp>(new(t.Alloc()) Instances::fl::RegExp(t));
        }
        SPtr<Instances::fl::RegExp> MakeInstanceS(RegExp& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 10 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[13];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl
{
    class RegExp : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::RegExp"; }
#endif
    public:
        typedef Classes::fl::RegExp ClassType;
        typedef InstanceTraits::fl::RegExp InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        RegExp(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl
{
    class RegExp : public Class
    {
        friend class ClassTraits::fl::RegExp;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::RegExpTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::RegExp"; }
#endif
    public:
        typedef RegExp SelfType;
        typedef RegExp ClassType;
        
    private:
        RegExp(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
        void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);
        virtual Pickable<AS3::Object> MakePrototype() const;
        virtual void InitPrototype(AS3::Object& obj) const;       
//##protect##"class_$methods"

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

