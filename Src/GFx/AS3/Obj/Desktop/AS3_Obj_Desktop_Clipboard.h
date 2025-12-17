//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_Clipboard.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Desktop_Clipboard_H
#define INC_AS3_Obj_Desktop_Clipboard_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_desktop
{
    extern const TypeInfo ClipboardTI;
    extern const ClassInfo ClipboardCI;
} // namespace fl_desktop
namespace fl
{
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo FunctionTI;
    extern const ClassInfo FunctionCI;
} // namespace fl

namespace ClassTraits { namespace fl_desktop
{
    class Clipboard;
}}

namespace InstanceTraits { namespace fl_desktop
{
    class Clipboard;
}}

namespace Classes { namespace fl_desktop
{
    class Clipboard;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_desktop
{
    class Clipboard : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_desktop::Clipboard;
        
    public:
        typedef Clipboard SelfType;
        typedef Classes::fl_desktop::Clipboard ClassType;
        typedef InstanceTraits::fl_desktop::Clipboard TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_desktop::ClipboardTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_desktop::Clipboard"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Clipboard(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_formatsGet, 
            mid_clear, 
            mid_clearData, 
            mid_getData, 
            mid_hasFormat, 
            mid_setData, 
            mid_setDataHandler, 
        };
        void formatsGet(SPtr<Instances::fl::Array>& result);
        void clear(const Value& result);
        void clearData(const Value& result, const ASString& format);
        void getData(Value& result, const ASString& format, const ASString& transferMode);
        void hasFormat(bool& result, const ASString& format);
        void setData(bool& result, const ASString& format, const Value& data, bool serializable);
        void setDataHandler(bool& result, const ASString& format, const Value& handler, bool serializable);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::Array> formatsGet();
        void clear()
        {
            clear(Value::GetUndefined());
        }
        void clearData(const ASString& format)
        {
            clearData(Value::GetUndefined(), format);
        }
        Value getData(const ASString& format, const ASString& transferMode)
        {
            Value result;
            getData(result, format, transferMode);
            return result;
        }
        bool hasFormat(const ASString& format)
        {
            bool result;
            hasFormat(result, format);
            return result;
        }
        bool setData(const ASString& format, const Value& data, bool serializable)
        {
            bool result;
            setData(result, format, data, serializable);
            return result;
        }
        bool setDataHandler(const ASString& format, const Value& handler, bool serializable)
        {
            bool result;
            setDataHandler(result, format, handler, serializable);
            return result;
        }

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_desktop
{
    class Clipboard : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Clipboard"; }
#endif
    public:
        typedef Instances::fl_desktop::Clipboard InstanceType;

    public:
        Clipboard(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_desktop::Clipboard> MakeInstance(Clipboard& t)
        {
            return Pickable<Instances::fl_desktop::Clipboard>(new(t.Alloc()) Instances::fl_desktop::Clipboard(t));
        }
        SPtr<Instances::fl_desktop::Clipboard> MakeInstanceS(Clipboard& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 7 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[17];
        static const Abc::ConstValue dva[3];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_desktop
{
    class Clipboard : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Clipboard"; }
#endif
    public:
        typedef Classes::fl_desktop::Clipboard ClassType;
        typedef InstanceTraits::fl_desktop::Clipboard InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Clipboard(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[1];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_desktop
{
    class Clipboard : public Class
    {
        friend class ClassTraits::fl_desktop::Clipboard;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_desktop::ClipboardTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Clipboard"; }
#endif
    public:
        typedef Clipboard SelfType;
        typedef Clipboard ClassType;
        
    private:
        Clipboard(ClassTraits::Traits& t);
       
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
            mid_generalClipboardGet, 
        };
        void generalClipboardGet(SPtr<Instances::fl_desktop::Clipboard>& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_desktop::Clipboard> generalClipboardGet()
        {
            SPtr<Instances::fl_desktop::Clipboard> result;
            generalClipboardGet(result);
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

