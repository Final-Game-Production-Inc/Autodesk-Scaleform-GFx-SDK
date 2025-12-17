//##protect##"disclaimer"
/**********************************************************************

Filename    :   AS3_Obj_Gfx_SystemEx.h
Content     :   
Created     :   Jan, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Gfx_SystemEx_H
#define INC_AS3_Obj_Gfx_SystemEx_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_gfx
{
    extern const TypeInfo SystemExTI;
    extern const ClassInfo SystemExCI;
} // namespace fl_gfx
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo anyTI;
    extern const ClassInfo anyCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
} // namespace fl

namespace ClassTraits { namespace fl_gfx
{
    class SystemEx;
}}

namespace InstanceTraits { namespace fl_gfx
{
    class SystemEx;
}}

namespace Classes { namespace fl_gfx
{
    class SystemEx;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_gfx
{
    class SystemEx : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SystemEx"; }
#endif
    public:
        typedef Classes::fl_gfx::SystemEx ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SystemEx(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 8 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[14];
        static const Abc::ConstValue dva[3];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_gfx
{
    class SystemEx : public Class
    {
        friend class ClassTraits::fl_gfx::SystemEx;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::SystemExTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::SystemEx"; }
#endif
    public:
        typedef SystemEx SelfType;
        typedef SystemEx ClassType;
        
    private:
        SystemEx(ClassTraits::Traits& t);
       
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
            mid_actionVerboseSet, 
            mid_actionVerboseGet, 
            mid_setBackgroundAlpha, 
            mid_getStackTrace, 
            mid_getCodeFileName, 
            mid_getCodeFileNames, 
            mid_describeType, 
            mid_printObjectsReport, 
        };
        void actionVerboseSet(const Value& result, bool verbose);
        void actionVerboseGet(bool& result);
        void setBackgroundAlpha(const Value& result, Value::Number value);
        void getStackTrace(ASString& result);
        void getCodeFileName(ASString& result);
        void getCodeFileNames(SPtr<Instances::fl::Array>& result);
        void describeType(ASString& result, const Value& v);
        void printObjectsReport(const Value& result, bool runGarbageCollector, UInt32 reportFlags, const ASString& swfFilter);

        // C++ friendly wrappers for AS3 methods.
        void actionVerboseSet(bool verbose)
        {
            actionVerboseSet(Value::GetUndefined(), verbose);
        }
        bool actionVerboseGet()
        {
            bool result;
            actionVerboseGet(result);
            return result;
        }
        void setBackgroundAlpha(Value::Number value)
        {
            setBackgroundAlpha(Value::GetUndefined(), value);
        }
        ASString getStackTrace()
        {
            ASString result(GetStringManager().CreateEmptyString());
            getStackTrace(result);
            return result;
        }
        ASString getCodeFileName()
        {
            ASString result(GetStringManager().CreateEmptyString());
            getCodeFileName(result);
            return result;
        }
        SPtr<Instances::fl::Array> getCodeFileNames();
        ASString describeType(const Value& v)
        {
            ASString result(GetStringManager().CreateEmptyString());
            describeType(result, v);
            return result;
        }
        void printObjectsReport(bool runGarbageCollector, UInt32 reportFlags, const ASString& swfFilter)
        {
            printObjectsReport(Value::GetUndefined(), runGarbageCollector, reportFlags, swfFilter);
        }

    public:
        // AS3 API members.
        const UInt32 REPORT_SHORT_FILENAMES;
        const UInt32 REPORT_NO_CIRCULAR_REFERENCES;
        const UInt32 REPORT_SUPPRESS_OVERALL_STATS;
        const UInt32 REPORT_ONLY_ANON_OBJ_ADDRESSES;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

