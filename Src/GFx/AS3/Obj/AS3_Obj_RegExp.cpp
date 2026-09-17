//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_RegExp.cpp
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

#include "AS3_Obj_RegExp.h"
#include "../AS3_VM.h"
#include "../AS3_Marshalling.h"
//##protect##"includes"
#ifdef SF_ENABLE_PCRE
#include "AS3_Obj_Array.h"
// 引入 PCRE2 8位头文件（匹配char*）
#include <pcre2.h>
#endif
//##protect##"includes"

namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl::RegExp, Instances::fl::RegExp::mid_sourceGet, ASString> TFunc_Instances_RegExp_sourceGet;
typedef ThunkFunc0<Instances::fl::RegExp, Instances::fl::RegExp::mid_globalGet, bool> TFunc_Instances_RegExp_globalGet;
typedef ThunkFunc0<Instances::fl::RegExp, Instances::fl::RegExp::mid_ignoreCaseGet, bool> TFunc_Instances_RegExp_ignoreCaseGet;
typedef ThunkFunc0<Instances::fl::RegExp, Instances::fl::RegExp::mid_multilineGet, bool> TFunc_Instances_RegExp_multilineGet;
typedef ThunkFunc0<Instances::fl::RegExp, Instances::fl::RegExp::mid_lastIndexGet, SInt32> TFunc_Instances_RegExp_lastIndexGet;
typedef ThunkFunc1<Instances::fl::RegExp, Instances::fl::RegExp::mid_lastIndexSet, const Value, SInt32> TFunc_Instances_RegExp_lastIndexSet;
typedef ThunkFunc0<Instances::fl::RegExp, Instances::fl::RegExp::mid_dotallGet, bool> TFunc_Instances_RegExp_dotallGet;
typedef ThunkFunc0<Instances::fl::RegExp, Instances::fl::RegExp::mid_extendedGet, bool> TFunc_Instances_RegExp_extendedGet;
typedef ThunkFunc1<Instances::fl::RegExp, Instances::fl::RegExp::mid_AS3exec, SPtr<Instances::fl::Object>, const ASString&> TFunc_Instances_RegExp_AS3exec;
typedef ThunkFunc1<Instances::fl::RegExp, Instances::fl::RegExp::mid_AS3test, bool, const ASString&> TFunc_Instances_RegExp_AS3test;

template <> const TFunc_Instances_RegExp_sourceGet::TMethod TFunc_Instances_RegExp_sourceGet::Method = &Instances::fl::RegExp::sourceGet;
template <> const TFunc_Instances_RegExp_globalGet::TMethod TFunc_Instances_RegExp_globalGet::Method = &Instances::fl::RegExp::globalGet;
template <> const TFunc_Instances_RegExp_ignoreCaseGet::TMethod TFunc_Instances_RegExp_ignoreCaseGet::Method = &Instances::fl::RegExp::ignoreCaseGet;
template <> const TFunc_Instances_RegExp_multilineGet::TMethod TFunc_Instances_RegExp_multilineGet::Method = &Instances::fl::RegExp::multilineGet;
template <> const TFunc_Instances_RegExp_lastIndexGet::TMethod TFunc_Instances_RegExp_lastIndexGet::Method = &Instances::fl::RegExp::lastIndexGet;
template <> const TFunc_Instances_RegExp_lastIndexSet::TMethod TFunc_Instances_RegExp_lastIndexSet::Method = &Instances::fl::RegExp::lastIndexSet;
template <> const TFunc_Instances_RegExp_dotallGet::TMethod TFunc_Instances_RegExp_dotallGet::Method = &Instances::fl::RegExp::dotallGet;
template <> const TFunc_Instances_RegExp_extendedGet::TMethod TFunc_Instances_RegExp_extendedGet::Method = &Instances::fl::RegExp::extendedGet;
template <> const TFunc_Instances_RegExp_AS3exec::TMethod TFunc_Instances_RegExp_AS3exec::Method = &Instances::fl::RegExp::AS3exec;
template <> const TFunc_Instances_RegExp_AS3test::TMethod TFunc_Instances_RegExp_AS3test::Method = &Instances::fl::RegExp::AS3test;

namespace Instances { namespace fl
{
    RegExp::RegExp(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::RegExp::RegExp()$data"
#ifdef SF_ENABLE_PCRE
    , CompRegExp(nullptr)
    , MatchOffset(-1)
    , MatchLength(0)
    , IsGlobal(false)
    , LastIndex(0)
    , OptionFlags(PCRE2_UTF)  // PCRE2 8位 UTF 标志
    , HasNamedGroups(false)
    , MatchData(nullptr)      // 新增：PCRE2 匹配数据指针
#endif
//##protect##"instance::RegExp::RegExp()$data"
    {
//##protect##"instance::RegExp::RegExp()$code"
//##protect##"instance::RegExp::RegExp()$code"
    }

    RegExp::~RegExp()
    {
#ifdef SF_ENABLE_PCRE
        // 修复：释放 PCRE2 资源（匹配数据 + 编译后的正则）
        if (MatchData)
            pcre2_match_data_free(MatchData);
        if (CompRegExp)
            pcre2_code_free(CompRegExp);
        CompRegExp = nullptr;
        MatchData = nullptr;
#endif
    }

    void RegExp::sourceGet(ASString& result)
    {
//##protect##"instance::RegExp::sourceGet()"
#ifdef SF_ENABLE_PCRE
        result = GetStringManager().CreateString(Pattern);
#else
        SF_UNUSED1(result);
        NOT_IMPLEMENTED("RegExp::sourceGet()");
#endif
//##protect##"instance::RegExp::sourceGet()"
    }
    void RegExp::globalGet(bool& result)
    {
//##protect##"instance::RegExp::globalGet()"
#ifdef SF_ENABLE_PCRE
        result = IsGlobal;
#else
        result = AS3::DefaultValue<bool>(GetStringManager());
#endif
//##protect##"instance::RegExp::globalGet()"
    }
    void RegExp::ignoreCaseGet(bool& result)
    {
//##protect##"instance::RegExp::ignoreCaseGet()"
#ifdef SF_ENABLE_PCRE
        result = hasOption(PCRE2_CASELESS);
#else
        result = AS3::DefaultValue<bool>(GetStringManager());
#endif
//##protect##"instance::RegExp::ignoreCaseGet()"
    }
    void RegExp::multilineGet(bool& result)
    {
//##protect##"instance::RegExp::multilineGet()"
#ifdef SF_ENABLE_PCRE
        result = hasOption(PCRE2_MULTILINE);
#else
        result = AS3::DefaultValue<bool>(GetStringManager());
#endif
//##protect##"instance::RegExp::multilineGet()"
    }
    void RegExp::lastIndexGet(SInt32& result)
    {
//##protect##"instance::RegExp::lastIndexGet()"
#ifdef SF_ENABLE_PCRE
        result = LastIndex;
#else
        result = AS3::DefaultValue<int>(GetStringManager());
#endif
//##protect##"instance::RegExp::lastIndexGet()"
    }
    void RegExp::lastIndexSet(const Value& result, SInt32 i)
    {
//##protect##"instance::RegExp::lastIndexSet()"
#ifdef SF_ENABLE_PCRE
        SF_UNUSED(result);
        LastIndex = i;
#else
        SF_UNUSED2(result, i);
        NOT_IMPLEMENTED("RegExp::lastIndexSet()");
#endif
//##protect##"instance::RegExp::lastIndexSet()"
    }
    void RegExp::dotallGet(bool& result)
    {
//##protect##"instance::RegExp::dotallGet()"
#ifdef SF_ENABLE_PCRE
        result = hasOption(PCRE2_DOTALL);
#else
        result = AS3::DefaultValue<bool>(GetStringManager());
#endif
//##protect##"instance::RegExp::dotallGet()"
    }
    void RegExp::extendedGet(bool& result)
    {
//##protect##"instance::RegExp::extendedGet()"
#ifdef SF_ENABLE_PCRE
        result = hasOption(PCRE2_EXTENDED);
#else
        result = AS3::DefaultValue<bool>(GetStringManager());
#endif
//##protect##"instance::RegExp::extendedGet()"
    }
    static int pcre2_utf8_char_check(uint32_t* code_point, PCRE2_SPTR8 s, PCRE2_SIZE max_len) {
        uint8_t c = s[0];
        uint32_t cp = 0;
        int len = 0;

        // 单字节 UTF-8 (0xxxxxxx)
        if ((c & 0x80) == 0) {
            len = 1;
            cp = c;
        }
        // 双字节 (110xxxxx 10xxxxxx)
        else if ((c & 0xE0) == 0xC0) {
            len = 2;
            if (max_len < 2) return PCRE2_ERROR_UTF8_ERR1; // 字节不足
            if ((s[1] & 0xC0) != 0x80) return PCRE2_ERROR_UTF8_ERR2; // 续字节非法
            cp = ((c & 0x1F) << 6) | (s[1] & 0x3F);
            if (cp < 0x80) return PCRE2_ERROR_UTF8_ERR3; // 过度编码
        }
        // 三字节 (1110xxxx 10xxxxxx 10xxxxxx)
        else if ((c & 0xF0) == 0xE0) {
            len = 3;
            if (max_len < 3) return PCRE2_ERROR_UTF8_ERR4;
            if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return PCRE2_ERROR_UTF8_ERR5;
            cp = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
            if (cp < 0x800) return PCRE2_ERROR_UTF8_ERR6; // 过度编码
            if ((cp >= 0xD800) && (cp <= 0xDFFF)) return PCRE2_ERROR_UTF8_ERR7; // 代理区非法
        }
        // 四字节 (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
        else if ((c & 0xF8) == 0xF0) {
            len = 4;
            if (max_len < 4) return PCRE2_ERROR_UTF8_ERR8;
            if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return PCRE2_ERROR_UTF8_ERR9;
            cp = ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
            if (cp < 0x10000) return PCRE2_ERROR_UTF8_ERR10; // 过度编码
            if (cp > 0x10FFFF) return PCRE2_ERROR_UTF8_ERR11; // 超出 Unicode 范围
        }
        // 无效首字节
        else {
            return PCRE2_ERROR_UTF8_ERR12;
        }

        // 输出码点（若需要）
        if (code_point != NULL) {
            *code_point = cp;
        }
        return len;
    }

    static PCRE2_SIZE pcre2_utf8_strlen(PCRE2_SPTR8 s, PCRE2_SIZE length, int* error) {
        // 初始化错误码
        if (error != NULL) *error = 0;
        // 空指针校验
        if (s == NULL) {
            if (error != NULL) *error = PCRE2_ERROR_NULL;
            return PCRE2_UNSET;
        }

        PCRE2_SIZE char_count = 0;  // UTF-8 字符数
        PCRE2_SIZE byte_offset = 0; // 当前字节偏移
        // PCRE2_UNSET 表示「以 0 终止」，设置最大偏移为 SIZE_MAX 避免越界
        PCRE2_SIZE max_byte = (length == PCRE2_UNSET) ? PCRE2_SIZE_MAX : length;

        while (1) {
            // 终止条件：达到指定长度 或 遇到字符串结束符
            if (byte_offset >= max_byte || s[byte_offset] == 0) {
                break;
            }

            // 校验当前 UTF-8 字符并获取字节长度
            int ret = pcre2_utf8_char_check(NULL, s + byte_offset, max_byte - byte_offset);
            if (ret < 0) { // 校验失败（无效 UTF-8）
                if (error != NULL) *error = ret;
                return PCRE2_UNSET;
            }

            char_count++;          // 字符数+1
            byte_offset += ret;    // 偏移到下一个 UTF-8 字符
        }

        return char_count;
    }

    void RegExp::AS3exec(SPtr<Instances::fl::Object>& result, const ASString& s)
    {
//##protect##"instance::RegExp::AS3exec()"
#ifdef SF_ENABLE_PCRE
        SInt32 oldLastIndex = LastIndex;
        SInt32 startIndex = (globalGet() ? LastIndex : 0);
        MatchOffset = MatchLength = 0;

        // 修复1：类型转换（const char* -> PCRE2_SPTR8）
        PCRE2_SPTR8 subject = reinterpret_cast<PCRE2_SPTR8>(s.ToCStr());
        PCRE2_SIZE subjectLen = static_cast<PCRE2_SIZE>(s.GetSize());
        PCRE2_SIZE matchOffset = static_cast<PCRE2_SIZE>(startIndex);
        int matchCount = 0;

        // 修复2：初始化匹配数据（PCRE2 必须先创建 match_data）
        if (MatchData)
            pcre2_match_data_free(MatchData);
        MatchData = pcre2_match_data_create_from_pattern(CompRegExp, nullptr);

        // 修复3：pcre2_match 8位 API 调用（参数个数/类型匹配）
        matchCount = pcre2_match(
            CompRegExp,               // 编译后的正则表达式
            subject,                  // 待匹配字符串
            subjectLen,               // 字符串长度
            matchOffset,              // 起始匹配位置
            OptionFlags,              // 匹配标志
            MatchData,                // 匹配结果存储
            nullptr                   // 匹配上下文（nullptr 用默认）
        );

        if (matchCount < 0)
        {
            MatchOffset = matchCount;
            result = nullptr;
            return;
        }

        // 获取匹配结果的偏移和长度
        PCRE2_SIZE* outputVector = pcre2_get_ovector_pointer(MatchData);
        MatchOffset = static_cast<SInt32>(outputVector[0]);
        MatchLength = static_cast<SInt32>(outputVector[1] - outputVector[0]);

        VM& vm = GetVM();
        StringManager& sm = vm.GetStringManager();

        // 构建匹配结果数组
        SPtr<Instances::fl::Array> parr = vm.MakeArray();
        for (SInt32 i = 0; i < matchCount; i++)
        {
            if (outputVector[i*2] != PCRE2_UNSET)
            {
                String match(
                    reinterpret_cast<const char*>(subject + outputVector[i*2]),
                    static_cast<size_t>(outputVector[i*2 + 1] - outputVector[i*2])
                );
                parr->PushBack(Value(sm.CreateString(match)));
            }
            else
                parr->PushBack(Value::GetUndefined());
        }
        parr->AddDynamicSlotValuePair(sm.CreateString("index"), Value(MatchOffset));
        parr->AddDynamicSlotValuePair(sm.CreateString("input"), Value(s));

        // 处理命名分组
        if (HasNamedGroups)
        {
            uint32_t nameCount;
            uint32_t nameEntrySize;
            PCRE2_SPTR8 nameTable;

            // 修复4：pcre2_pattern_info 8位 API 调用（参数个数/类型匹配）
            pcre2_pattern_info(
                CompRegExp,
                PCRE2_INFO_NAMECOUNT,
                &nameCount
            );
            pcre2_pattern_info(
                CompRegExp,
                PCRE2_INFO_NAMEENTRYSIZE,
                &nameEntrySize
            );
            pcre2_pattern_info(
                CompRegExp,
                PCRE2_INFO_NAMETABLE,
                &nameTable
            );

            for (uint32_t i = 0; i < nameCount; i++)
            {
                // 解析命名分组名称和索引
                uint32_t nameIdx = (nameTable[0] << 8) | nameTable[1];
                String name(
                    reinterpret_cast<const char*>(nameTable + 2),
                    static_cast<size_t>(pcre2_utf8_strlen(nameTable + 2, PCRE2_UNSET, 0))
                );
                
                if (outputVector[nameIdx*2] != PCRE2_UNSET)
                {
                    String value(
                        reinterpret_cast<const char*>(subject + outputVector[nameIdx*2]),
                        static_cast<size_t>(outputVector[nameIdx*2 + 1] - outputVector[nameIdx*2])
                    );
                    parr->AddDynamicSlotValuePair(sm.CreateString(name), sm.CreateString(value));
                }
                nameTable += nameEntrySize;
            }
        }

        // 更新全局匹配的 lastIndex
        if (globalGet())
            LastIndex = MatchOffset + MatchLength;
        if (LastIndex == oldLastIndex)
            LastIndex++;

        result = parr;
#else
        SF_UNUSED1(s);
        result = AS3::DefaultValue<Instances::fl::Object*>(GetStringManager());
#endif
//##protect##"instance::RegExp::AS3exec()"
    }

    void RegExp::AS3test(bool& result, const ASString& s)
    {
//##protect##"instance::RegExp::AS3test()"
#ifdef SF_ENABLE_PCRE
        SPtr<Instances::fl::Object> pobj;
        AS3exec(pobj, s);
        result = (pobj != nullptr);
#else
        SF_UNUSED1(s);
        result = AS3::DefaultValue<bool>(GetStringManager());
#endif
//##protect##"instance::RegExp::AS3test()"
    }

//##protect##"instance$methods"
#ifdef SF_ENABLE_PCRE
    void RegExp::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc >= 1 && !argv[0].IsNullOrUndefined())
        {
            VM& vm = GetVM();
            StringManager& sm = vm.GetStringManager();

            ASString src = sm.CreateEmptyString();
            ASString opt = sm.CreateEmptyString();

            // 处理 RegExp 实例作为参数的情况
            if (argv[0].IsObject() && vm.IsOfType(argv[0], "RegExp", vm.GetCurrentAppDomain()))
            {
                RegExp* pobj = static_cast<Instances::fl::RegExp*>(argv[0].GetObject());
                src = pobj->sourceGet();
                opt = pobj->optionFlagsGet();

                // 错误：从另一个 RegExp 构造时不能传 flags
                if (argc >= 2 && !argv[1].IsNullOrUndefined())
                {
                    vm.ThrowTypeError(VM::Error(VM::eRegExpFlagsArgumentError,vm));
                    return;
                }
            }
            else 
            {
                // 解析 pattern 和 flags 参数
                if (!argv[0].Convert2String(src))
                    return;
                if (argc >= 2 && !argv[1].IsNullOrUndefined())
                {
                    if (!argv[1].Convert2String(opt))
                        return;
                }
            }

            const char* patternStr = src.ToCStr();
            const char* flagsStr = opt.ToCStr();
            Pattern = patternStr;

            // 检查 pattern 中的命名分组
            const char* p = patternStr;
            while (*p)
            {
                if (p[0] == '(' && p[1] == '?' && p[2] == 'P' && p[3] == '<')
                {
                    HasNamedGroups = true;
                }
                p++;
            }

            // 解析 flags
            if (flagsStr)
            {		
                for (; *flagsStr; flagsStr++)
                {
                    switch(*flagsStr)
                    {
                    case 'g': IsGlobal = true; break;
                    case 'i': OptionFlags |= PCRE2_CASELESS; break;
                    case 'm': OptionFlags |= PCRE2_MULTILINE; break;
                    case 's': OptionFlags |= PCRE2_DOTALL;    break;
                    case 'x': OptionFlags |= PCRE2_EXTENDED;  break;
                    }
                }
            }

            // 修复5：pcre2_compile 8位 API 调用（参数个数/类型匹配）
            PCRE2_SPTR8 compilePattern = reinterpret_cast<PCRE2_SPTR8>(Pattern.ToCStr());
            int errorCode;
            PCRE2_SIZE errorOffset;
            
            // 释放旧的编译结果
            if (CompRegExp)
            {
                pcre2_code_free(CompRegExp);
                CompRegExp = nullptr;
            }

            // 编译正则表达式
            CompRegExp = pcre2_compile(
                compilePattern,          // 正则表达式字符串（8位）
                PCRE2_ZERO_TERMINATED,   // 字符串以 \0 结尾
                OptionFlags,             // 编译标志
                &errorCode,              // 错误码
                &errorOffset,            // 错误位置
                nullptr                  // 编译上下文（nullptr 用默认）
            );

            // 编译失败时 CompRegExp 为 nullptr
            if (!CompRegExp)
            {
                // 可选：添加错误日志
                PCRE2_UCHAR8 errorMsg[256];
                pcre2_get_error_message(errorCode, errorMsg, sizeof(errorMsg));
                // vm.ThrowTypeError(...) // 可抛出编译错误
            }
        }
    }

    bool RegExp::hasOption(int mask)
    {
        return (OptionFlags & mask) != 0;
    }

    ASString RegExp::optionFlagsGet()
    {
        StringManager& sm = GetVM().GetStringManager();

        ASString flags = sm.CreateEmptyString();
        flags += (IsGlobal)     ? "g" : "";
        flags += (ignoreCaseGet()) ? "i" : "";
        flags += (multilineGet())  ? "m" : "";
        flags += (dotallGet())     ? "s" : "";
        flags += (extendedGet())   ? "x" : "";
        return flags;
    }

    ASString RegExp::ToString()
    {
        StringManager& sm = GetVM().GetStringManager();
        return sm.CreateConstString("/") + sourceGet() +
               sm.CreateConstString("/") + optionFlagsGet();
    }
#endif

    void RegExp::toStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
#ifdef SF_ENABLE_PCRE
        SF_UNUSED4(ti, vm, argc, argv);
        Instances::fl::RegExp* pobj = static_cast<Instances::fl::RegExp*>(_this.GetObject());
        result = pobj->ToString();
#else
        SF_UNUSED4(ti, _this, argc, argv);
        result = vm.GetStringManager().CreateConstString("/(?:)/");
#endif
    }
//##protect##"instance$methods"

}} // namespace Instances

// 以下代码保持不变（省略重复部分）
namespace InstanceTraits { namespace fl
{
    const TypeInfo* RegExp::tit[13] = {
        &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::ObjectTI, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, &AS3::fl::StringTI, 
    };
    const ThunkInfo RegExp::ti[RegExp::ThunkInfoNum] = {
        {TFunc_Instances_RegExp_sourceGet::Func, &RegExp::tit[0], "source", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_RegExp_globalGet::Func, &RegExp::tit[1], "global", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_RegExp_ignoreCaseGet::Func, &RegExp::tit[2], "ignoreCase", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_RegExp_multilineGet::Func, &RegExp::tit[3], "multiline", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_RegExp_lastIndexGet::Func, &RegExp::tit[4], "lastIndex", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_RegExp_lastIndexSet::Func, &RegExp::tit[5], "lastIndex", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_RegExp_dotallGet::Func, &RegExp::tit[7], "dotall", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_RegExp_extendedGet::Func, &RegExp::tit[8], "extended", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_RegExp_AS3exec::Func, &RegExp::tit[9], "exec", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_RegExp_AS3test::Func, &RegExp::tit[11], "test", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    RegExp::RegExp(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
    }

    void RegExp::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<RegExp&>(t));
    }
}} // namespace InstanceTraits

namespace Classes { namespace fl
{
    RegExp::RegExp(ClassTraits::Traits& t)
    : Class(t)
    {
    }

    void RegExp::Call(const Value& _this, Value& result, unsigned argc, const Value* const argv)
    {
        SF_UNUSED(_this);
        Construct(result, argc, argv);

#ifdef SF_ENABLE_PCRE
        Instances::fl::RegExp* pobj = static_cast<Instances::fl::RegExp*>(result.GetObject());
        result = pobj->ToString();                 
#endif
    }

    Pickable<AS3::Object> RegExp::MakePrototype() const
    {
        InstanceTraits::fl::RegExp& itr = static_cast<InstanceTraits::fl::RegExp&>(GetClassTraits().GetInstanceTraits());
        return itr.MakeInstance(itr);
    }

    void RegExp::InitPrototype(AS3::Object& obj) const
    {
        if (GetParentClass())
            GetParentClass()->InitPrototype(obj);

        InitPrototypeFromVTableCheckType(obj);

        static const TypeInfo* tit[] = {&AS3::fl::StringTI};
        static const ThunkInfo f[] = {
            {&Instances::fl::RegExp::toStringProto, &tit[0], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0},
        };
        for (unsigned i = 0; i < NUMBEROF(f); ++i)
            AddDynamicFunc(obj, f[i]);
    }
}} // namespace Classes

namespace ClassTraits { namespace fl
{
    RegExp::RegExp(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
    }

    Pickable<Traits> RegExp::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) RegExp(vm, AS3::fl::RegExpCI));
        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl::RegExpCI));
        ctr->SetInstanceTraits(itr);
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));
        return ctr;
    }
}} // namespace ClassTraits

namespace fl
{
    const TypeInfo RegExpTI = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject,
        sizeof(ClassTraits::fl::RegExp::InstanceType),
        0,
        0,
        InstanceTraits::fl::RegExp::ThunkInfoNum,
        0,
        "RegExp", "", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo RegExpCI = {
        &RegExpTI,
        ClassTraits::fl::RegExp::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl::RegExp::ti,
        NULL,
    };
}; // namespace fl

}}} // namespace Scaleform { namespace GFx { namespace AS3