/**************************************************************************

Filename    :   AS3_Obj_Global.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_AS3_Obj_Global_H
#define INC_AS3_Obj_Global_H

#include "AS3_Obj_Object.h"
#include "AS3_Obj_XML.h"
#include "Net/AS3_Obj_Net_URLRequest.h"
#include "../AS3_VM.h" // This include is required by the WiiU compiler.

namespace Scaleform { namespace GFx { namespace AS3 
{
///////////////////////////////////////////////////////////////////////////////
// Forward declaration

class VMAbcFile;

namespace InstanceTraits
{
    class Function;

    namespace fl
    {
        class GlobalObjectScript;
    }
}

namespace AOT
{
    class File;
}

///////////////////////////////////////////////////////////////////////////
namespace Instances { namespace fl
{
    ///////////////////////////////////////////////////////////////////////////
    class GlobalObject : public Instances::fl::Object
    {
    protected:
        GlobalObject(InstanceTraits::Traits& t);
        virtual ~GlobalObject();

    public:
        virtual void Execute();

    protected:
        virtual void Construct(Value& _this, unsigned argc, const Value* argv, bool extCall = false);
        virtual void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);
        void Add2VT(const ClassInfo& ci, const ThunkInfo& func)
        {
            GetTraits().Add2VT(ci, func);
        }
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl::GlobalObject"; }
#endif
    };

    ///////////////////////////////////////////////////////////////////////////
    class GlobalObjectScript : public GlobalObject
    {
        friend class InstanceTraits::fl::GlobalObjectScript;
        friend class InstanceTraits::UserDefined; // Because of InitOnDemand().
        friend class InstanceTraits::Function; // Because of InitOnDemand().

    protected:
        // This constructor won't set up slot values.
        GlobalObjectScript(InstanceTraits::fl::GlobalObjectScript& t);

    public:
        virtual ~GlobalObjectScript();

    public:
        virtual void Execute();

    public:
        bool IsInitialized() const
        {
            return Initialized;
        }
        void InitUserDefinedClassTraits();

    public:
        VMAbcFile& GetFile() const;
        const Abc::ScriptInfo& GetScript() const;

    protected:
        virtual const SlotInfo* InitializeOnDemand(
            const SlotInfo* si,
            const ASString& name,
            const Namespace& ns,
            UPInt& index);

    private:
        void InitOnDemand();

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl::GlobalObjectScript"; }
        virtual Scaleform::String GetAS3ObjectName() const; 
#endif
    private:
        bool Initialized;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct ConstStringKey
    {
        const char *pChar;

        ConstStringKey(const char *pc)
        {
            pChar = pc;
        }
        bool operator == (const ConstStringKey& c) const
        {
            return strcmp(pChar, c.pChar) == 0;
        }
        bool operator != (const ConstStringKey& c) const
        {
            return strcmp(pChar, c.pChar) != 0;
        }
    };

    class ConstStringHashFn
    {
    public:

        size_t  operator()(const ConstStringKey& data) const
        { 
            size_t  size = strlen(data.pChar);
            return String::BernsteinHashFunction(data.pChar, size);
        }
    };

    template <class U>
    class ConstStringHash : public Hash<ConstStringKey, U, ConstStringHashFn, AllocatorGH<ConstStringKey, Mem_Stat> > {};

    ///////////////////////////////////////////////////////////////////////////
    class GlobalObjectCPP : public GlobalObject
    {
        friend class AS3::VM; // Because of exceptions ...
        friend class ThunkGenVT;

    public:
        typedef GlobalObjectCPP Self;
        
    public:
        GlobalObjectCPP(VM& vm, InstanceTraits::Traits& t);
        ~GlobalObjectCPP();

    public:
#ifdef SF_AS3_CLASS_AS_SLOT
        const Value& GetValueArray(AbsoluteIndex ind) const
        {
            return Values[ind.Get()];
        }
        void SetValueArray(AbsoluteIndex ind, const Value& v)
        {
            Values[ind.Get()] = v;
        }
#endif

#ifdef SF_AS3_CLASS_AS_SLOT
        // Add to fixed slots.
        const SlotInfo& AddFixedSlotValuePair(const ASString& prop_name, Pickable<const Instances::fl::Namespace> ns,
            const ClassTraits::Traits* ctr, const Value& v, UPInt& index);
        // This method will also register Traits of a class.
        const SlotInfo& AddFixedSlot(Class& cl, Pickable<const Namespace> ns, UPInt& index)
        {
            return AddFixedSlotValuePair(cl.GetName(), ns, &cl.GetClassTraits(), Value(&cl), index);
        }
        // Add class slot to public namespace; used for TopLevel classes.
        // This method will also register Traits of a class.
        void AddFixedSlot(Class& cl);

        virtual const SlotInfo* InitializeOnDemand(
            const SlotInfo* si,
            const ASString& name,
            const Namespace& ns,
            UPInt& index);
#endif

        ClassTraits::Traits* GetClassTraits(
            const ASString& name,
            const Namespace& ns);

        void RegisterClassInfoTable(const ClassInfo* table[]);

    public:
        //
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
        
    public:
        // AS3 API ...

//##markup##"obj_global_h$ConstPool"
//##begin##"obj_global_h$ConstPool"

        static SInt32 Ints[5];
        static UInt32 UInts[8];
        static Value::Number Numbers[9];
        static const char* Strings[12];

//##end##"obj_global_h$ConstPool"

//##markup##"obj_global_h$package_methods"
//##begin##"obj_global_h$package_methods"
        void decodeURI(ASString& result, const ASString& uri);
        ASString decodeURI(const ASString& uri)
        {
            ASString result(GetStringManager().CreateEmptyString());
            decodeURI(result, uri);
            return result;
        }
        void decodeURIComponent(ASString& result, const ASString& uri);
        ASString decodeURIComponent(const ASString& uri)
        {
            ASString result(GetStringManager().CreateEmptyString());
            decodeURIComponent(result, uri);
            return result;
        }
        void encodeURI(ASString& result, const ASString& uri);
        ASString encodeURI(const ASString& uri)
        {
            ASString result(GetStringManager().CreateEmptyString());
            encodeURI(result, uri);
            return result;
        }
        void encodeURIComponent(ASString& result, const ASString& uri);
        ASString encodeURIComponent(const ASString& uri)
        {
            ASString result(GetStringManager().CreateEmptyString());
            encodeURIComponent(result, uri);
            return result;
        }
        void isNaN(bool& result, Value::Number n = NumberUtil::NaN());
        bool isNaN(Value::Number n = NumberUtil::NaN())
        {
            bool result;
            isNaN(result, n);
            return result;
        }
        void isFinite(bool& result, Value::Number n = NumberUtil::NaN());
        bool isFinite(Value::Number n = NumberUtil::NaN())
        {
            bool result;
            isFinite(result, n);
            return result;
        }
        void parseInt(Value& result, unsigned argc, const Value* const argv);
        void parseFloat(Value::Number& result, const ASString& str);
        Value::Number parseFloat(const ASString& str)
        {
            Value::Number result;
            parseFloat(result, str);
            return result;
        }
        void escape(Value& result, unsigned argc, const Value* const argv);
        void unescape(Value& result, unsigned argc, const Value* const argv);
        void isXMLName(bool& result, const Value& str = Value(NumberUtil::NaN()));
        bool isXMLName(const Value& str = Value(NumberUtil::NaN()))
        {
            bool result;
            isXMLName(result, str);
            return result;
        }
        void trace(Value& result, unsigned argc, const Value* const argv);
        void print(Value& result, unsigned argc, const Value* const argv);
#ifndef SF_AS3_NO_FLASH
        void navigateToURL(const Value& result, Instances::fl_net::URLRequest* request, const ASString& window);
        void navigateToURL(Instances::fl_net::URLRequest* request, const ASString& window)
        {
            navigateToURL(Value::GetUndefined(), request, window);
        }
        void getClassByAlias(SPtr<Class>& result, const ASString& aliasName);
        SPtr<Class> getClassByAlias(const ASString& aliasName)
        {
            SPtr<Class> result;
            getClassByAlias(result, aliasName);
            return result;
        }
        void registerClassAlias(const Value& result, const ASString& aliasName, Class* classObject);
        void registerClassAlias(const ASString& aliasName, Class* classObject)
        {
            registerClassAlias(Value::GetUndefined(), aliasName, classObject);
        }
        void fscommand(Value& result, unsigned argc, const Value* const argv);
        void clearInterval(const Value& result, UInt32 id);
        void clearInterval(UInt32 id)
        {
            clearInterval(Value::GetUndefined(), id);
        }
        void clearTimeout(const Value& result, UInt32 id);
        void clearTimeout(UInt32 id)
        {
            clearTimeout(Value::GetUndefined(), id);
        }
        void describeType(SPtr<Instances::fl::XML>& result, const Value& value);
        SPtr<Instances::fl::XML> describeType(const Value& value);
        void escapeMultiByte(ASString& result, const ASString& value);
        ASString escapeMultiByte(const ASString& value)
        {
            ASString result(GetStringManager().CreateEmptyString());
            escapeMultiByte(result, value);
            return result;
        }
        void getDefinitionByName(Value& result, const ASString& name);
        Value getDefinitionByName(const ASString& name)
        {
            Value result;
            getDefinitionByName(result, name);
            return result;
        }
        void getQualifiedClassName(ASString& result, const Value& value);
        ASString getQualifiedClassName(const Value& value)
        {
            ASString result(GetStringManager().CreateEmptyString());
            getQualifiedClassName(result, value);
            return result;
        }
        void getQualifiedSuperclassName(Value& result, const Value& value);
        Value getQualifiedSuperclassName(const Value& value)
        {
            Value result;
            getQualifiedSuperclassName(result, value);
            return result;
        }
        void getTimer(SInt32& result);
        SInt32 getTimer()
        {
            SInt32 result;
            getTimer(result);
            return result;
        }
        void setInterval(Value& result, unsigned argc, const Value* const argv);
        void setTimeout(Value& result, unsigned argc, const Value* const argv);
        void unescapeMultiByte(ASString& result, const ASString& value);
        ASString unescapeMultiByte(const ASString& value)
        {
            ASString result(GetStringManager().CreateEmptyString());
            unescapeMultiByte(result, value);
            return result;
        }

#endif
//##end##"obj_global_h$package_methods"

        void trace()
        {
            Value result;
            trace(result, (unsigned)0, (const Value*)NULL);
        }
        template <typename T1>
        void trace(const T1& a1)
        {
            Value result;
            BoxArgV1<T1> argv(a1, GetStringManager());
            trace(result, static_cast<unsigned>(argv.AN), static_cast<const Value* const>(&argv.V));
        }
//         template <>
//         void trace<Value>(const Value& a1)
//         {
//             Value result;
//             trace(result, 1, &a1);
//         }
        template <typename T1, typename T2>
        void trace(const T1& a1, const T2& a2)
        {
            Value result;
            BoxArgV2<T1, T2> argv(a1, a2, GetStringManager());
            trace(result, static_cast<unsigned>(argv.AN), static_cast<const Value* const>(&argv.V));
        }
        template <typename T1, typename T2, typename T3>
        void trace(const T1& a1, const T2& a2, const T3& a3)
        {
            Value result;
            BoxArgV3<T1, T2, T3> argv(a1, a2, a3, GetStringManager());
            trace(result, static_cast<unsigned>(argv.AN), static_cast<const Value* const>(&argv.V));
        }

    private:
        void Add2VT(const ClassInfo& ci, const ThunkInfo& func)
        {
            GetTraits().Add2VT(ci, func);
        }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl::GlobalObjectCPP"; }
#endif
    private:
        ConstStringHash<const ClassInfo*> CIRegistrationHash;

        // AS3 API members ...
        const Value::Number PositiveInfinity;
        const Value::Number NegativeInfinity;
        const Value::Number NaN;
        const Value         undefined;
        const UInt32        HIDE_NSURI_METHODS;
        const UInt32        INCLUDE_BASES;
        const UInt32        INCLUDE_INTERFACES;
        const UInt32        INCLUDE_VARIABLES;
        const UInt32        INCLUDE_ACCESSORS;
        const UInt32        INCLUDE_METHODS;
        const UInt32        INCLUDE_METADATA;
        const UInt32        INCLUDE_CONSTRUCTOR;
        const UInt32        INCLUDE_TRAITS;
        const UInt32        USE_ITRAITS;
        const UInt32        HIDE_OBJECT;
        const UInt32        FLASH10_FLAGS;
        enum { MemberInfoNum = 16 };
        static MemberInfo mi[MemberInfoNum];

        // Values is used instead of a tail to store fixed properties.
#ifdef SF_AS3_CLASS_AS_SLOT
        ValueArray          Values;
#endif
        ArrayLH<SPtr<ClassTraits::Traits> > CTraits;
    };

}} // namespace Instances

///////////////////////////////////////////////////////////////////////////
namespace InstanceTraits { namespace fl
{
    ///////////////////////////////////////////////////////////////////////////
    class GlobalObject : public CTraits
    {
    public:
        typedef Instances::fl::GlobalObject InstanceType;

    public:
        GlobalObject(VM& vm);

    public:
        virtual void MakeObject(Value& result, Traits& t);
        virtual bool IsGlobal() const;

        static const TypeInfo   TInfo;
        static const ClassInfo  CInfo;
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::fl::GlobalObject"; }
#endif
    };

    ///////////////////////////////////////////////////////////////////////////
    class GlobalObjectScript : public GlobalObject
    {
    public:
        typedef Instances::fl::GlobalObjectScript InstanceType;

    public:
        GlobalObjectScript(
            VMAbcFile& file,
            VM& vm, 
            const Abc::ScriptInfo& script
            );

    public:
        virtual void ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const;
        virtual unsigned GetFixedMemSize() const;
        virtual VMAbcFile* GetFilePtr() const;
        VMAppDomain& GetAppDomain() const
        {
            return GetFile().GetAppDomain();
        }

        Pickable<Instances::fl::GlobalObjectScript> MakeInstance();

        VMAbcFile& GetFile() const
        {
            return *File;
        }
        const Abc::ScriptInfo& GetScript() const
        {
            return Script;
        }

    private:
        GlobalObjectScript& operator =(const GlobalObjectScript&);

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::fl::GlobalObjectScript"; }
#endif
    private:
        SPtr<VMAbcFile> File;
        const Abc::ScriptInfo& Script;
    };

}} // namespace InstanceTraits

namespace Instances { namespace fl
{
    inline
    VMAbcFile& GlobalObjectScript::GetFile() const
    {
        return static_cast<const InstanceTraits::fl::GlobalObjectScript&>(GetTraits()).GetFile();
    }

    inline
    const Abc::ScriptInfo& GlobalObjectScript::GetScript() const
    {
        return static_cast<const InstanceTraits::fl::GlobalObjectScript&>(GetTraits()).GetScript();
    }
}}

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif // INC_AS3_Obj_Global_H
