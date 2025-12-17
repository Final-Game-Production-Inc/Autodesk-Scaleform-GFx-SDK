//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_XML.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy, Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_XML_H
#define INC_AS3_Obj_XML_H

#include "AS3_Obj_Object.h"
//##protect##"includes"
// This header is required by the 3DS compiler.
#include "AS3_Obj_Namespace.h"
// Expat related header.
#define XML_STATIC
#include <expat.h>
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl
{
    extern const TypeInfo XMLTI;
    extern const ClassInfo XMLCI;
    extern const TypeInfo XMLListTI;
    extern const ClassInfo XMLListCI;
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo anyTI;
    extern const ClassInfo anyCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo QNameTI;
    extern const ClassInfo QNameCI;
} // namespace fl

namespace ClassTraits { namespace fl
{
    class XML;
}}

namespace InstanceTraits { namespace fl
{
    class XML;
}}

namespace Classes { namespace fl
{
    class XML;
}}

//##protect##"forward_declaration"

namespace Instances
{
    namespace fl
    {
        class XMLList;
        class XMLElement;
        class XMLText;
        class XMLAttr;
        class XMLProcInstr;
        class XMLComment;
        class QName;
    }
}

#ifdef GFX_ENABLE_XML

class NamespaceArray
{
public:
    NamespaceArray(MemoryHeap* heap) : Namespaces(heap) {}
    ~NamespaceArray();

public:
    void Add(Instances::fl::Namespace& other, bool checkUnique = true);
    void AddUnique(const NamespaceArray& other);

    Instances::fl::Namespace& operator[](UPInt i) const { return *Namespaces[i]; }
    Instances::fl::Namespace& Get(UPInt i) const { return *Namespaces[i]; }

    UPInt GetSize() const { return Namespaces.GetSize(); }

    void Clear() { Namespaces.Clear(); }

    bool Find(const Instances::fl::Namespace& ns) const;
    bool FindByPrefix(const ASString& pref) const;
    bool FindByUri(const ASString& uri) const;

private:
    Scaleform::ArrayDH<SPtr<Instances::fl::Namespace> > Namespaces;
};

class XMLParser;

CheckResult ToXMLName(VM& vm, const Value& v, SPtr<Instances::fl::QName>& result);
CheckResult IsNameStartChar(int c);
CheckResult IsNameChar(int c);
CheckResult IsNCNameChar(int c);
CheckResult IsNCNameStartChar(int c);
CheckResult IsValidName(const ASString& tag);

// 13.3.5.4 [[GetNamespace]]
Pickable<Instances::fl::Namespace> LookupNamespace(const Instances::fl::Namespace& qName, const NamespaceArray* inScopeNamespaces);

#endif

//##protect##"forward_declaration"

namespace Instances { namespace fl
{
    class XML : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl::XML;
        
    public:
        typedef XML SelfType;
        typedef Classes::fl::XML ClassType;
        typedef InstanceTraits::fl::XML TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::XMLTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl::XML"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        XML(InstanceTraits::Traits& t);

//##protect##"instance$methods"
#ifdef GFX_ENABLE_XML
    public:
        friend class AS3::XMLParser;

        typedef ArrayLH<SPtr<Namespace> > NamespaceH;

        enum Kind {kNone, kElement, kText, kComment, kInstruction, kAttr};

        XML(InstanceTraits::Traits& t, const ASString& n, XML* p = NULL);
        virtual ~XML();

    public:
        XML* GetParent() const { return Parent; }
        void SetParent(XML* parent) { Parent = parent;}

        // 9.1.1.10 [[ResolveValue]]
        XML* ResolveValue() { return this; }

        const ASString& GetText() const { return Text; }
        ASString& GetText() { return Text; }

        Pickable<XML>     MakeInstance();

        Pickable<XMLList> MakeXMLListInstance();
        Pickable<XMLList> MakeXMLListInstance(const Multiname& mn);
        Pickable<XMLList> MakeXMLListInstance(Instances::fl::Object& target_obj, const ASString& target_prop, Namespace& target_ns);

        // E4X 9.1.1.9
        bool Equals(const XML& other) { return EqualsInternal(other) == true3; }
        bool Matches(const Multiname& prop_name) const;

        static void AppendIdent(StringBuffer& buf, SInt32 ident);
        // ECMA 10.2.1.1
        static void EscapeElementValue(StringBuffer& buf, const ASString& v);
        // ECMA 10.2.1.2
        static void EscapeAttributeValue(StringBuffer& buf, const ASString& v);

    public:
        // Newly introduced virtual methods.

        virtual const NamespaceH* GetInScopeNamespaces() const;
        // 9.1.1.13[[AddInScopeNamespace]] (N)
        virtual void AddInScopeNamespace(const Namespace& ns);

        virtual void AppendChild(const SPtr<XML>& child);
        // Return false in case of exception.
        virtual CheckResult AppendChild(const Value& child);
        virtual void ToString(StringBuffer& buf, SInt32 ident);
        // ancestorNamespaces is a list of already declared namespaces.
        virtual void ToXMLString(StringBuffer& buf, SInt32 ident, const NamespaceArray* ancestorNamespaces, const NamespaceArray* usedNotDeclared);
        virtual Kind GetKind() const;
        virtual Boolean3 EqualsInternal(const XML& other); 
        virtual bool HasSimpleContent() const;
        virtual bool HasOwnProperty(const ASString& p);

        virtual const ASString& GetName() const;
        virtual const Namespace& GetNamespace() const;
        // This is not ECMA 13.4.4.36
        virtual void SetNamespace(Namespace& ns);
        // Experimental method.
        virtual void SetNamespace2(Namespace& ns);

        // This function returns non-const reference to Namespace.
        // It cannot be replaced with GetNamespace().
        virtual Namespace& GetCurrNamespace() const;
        virtual Pickable<XML> DeepCopy(XML* parent);
        virtual void GetChildren(Instances::fl::XMLList& list, const Multiname& prop_name);
        virtual CheckResult GetChildIndex(UPInt& ind) const;

        // Can throw exceptions.
        virtual CheckResult InsertChildAt(UPInt pos, const Value& child);
        // Can throw exceptions.
        virtual CheckResult InsertChildAfter(const Value& child1, const Value& child2);
        // Can throw exceptions.
        virtual CheckResult InsertChildBefore(const Value& child1, const Value& child2);

        virtual void DeleteChildren(XML* child);
        // 9.1.1.4 [[DeleteByIndex]] (P)
        virtual void DeleteByIndex(UPInt ind);

        virtual void GetAttributes(Instances::fl::XMLList& list);
        // If k == kNone this method will return all children.
        virtual void GetChildren(Instances::fl::XMLList& list, Kind k, const ASString* name = NULL);
        virtual SPtr<Instances::fl::QName> GetQName();
        virtual void Normalize();
        // lp stands for "Last Parent"
        virtual Namespace* FindNamespaceByPrefix(const ASString& prefix, XML* lp) const;
        virtual Namespace* FindNamespaceByURI(const ASString& uri, XML* lp) const;
        // nr stands for "not resolved prefixes".
        virtual void ResolveNamespaces(HashSetDH<ASString>& nr, XML* lp) const;
        virtual XML* RemoveNamespace(const Value& ns);

        // Can throw exceptions.
        // 9.1.1.12 [[Replace]] (P, V)
        virtual CheckResult Replace(const Multiname& prop_name, const Value& value);
        // 13.4.4.32
        virtual CheckResult AS3Replace(const Multiname& prop_name, const Value& value);

        // Can throw exceptions.
        virtual void SetChildren(const Value& value);

    public:
        // Inherited virtual methods.

        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;

        virtual void GetDescendants(XMLList& list, const Multiname& prop_name);

        virtual CheckResult SetProperty(const Multiname& prop_name, const Value& value);
        virtual CheckResult GetProperty(const Multiname& prop_name, XMLList& list);
        virtual CheckResult GetProperty(const Multiname& prop_name, Value& value);
        virtual CheckResult DeleteProperty(const Multiname& prop_name);

        virtual void GetNextPropertyName(Value& name, GlobalSlotIndex ind) const;
        virtual GlobalSlotIndex GetNextDynPropIndex(GlobalSlotIndex ind) const;
        virtual void GetNextPropertyValue(Value& value, GlobalSlotIndex ind);
#endif
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_prototypeGet, 
            mid_prototypeSet, 
            mid_AS3toString, 
            mid_AS3hasOwnProperty, 
            mid_AS3propertyIsEnumerable, 
            mid_AS3addNamespace, 
            mid_AS3appendChild, 
            mid_AS3attribute, 
            mid_AS3attributes, 
            mid_AS3child, 
            mid_AS3childIndex, 
            mid_AS3children, 
            mid_AS3comments, 
            mid_AS3contains, 
            mid_AS3copy, 
            mid_AS3descendants, 
            mid_AS3elements, 
            mid_AS3hasComplexContent, 
            mid_AS3hasSimpleContent, 
            mid_AS3inScopeNamespaces, 
            mid_AS3insertChildAfter, 
            mid_AS3insertChildBefore, 
            mid_AS3length, 
            mid_AS3localName, 
            mid_AS3name, 
            mid_AS3namespace_, 
            mid_AS3namespaceDeclarations, 
            mid_AS3nodeKind, 
            mid_AS3normalize, 
            mid_AS3parent, 
            mid_AS3processingInstructions, 
            mid_AS3prependChild, 
            mid_AS3removeNamespace, 
            mid_AS3replace, 
            mid_AS3setChildren, 
            mid_AS3setLocalName, 
            mid_AS3setName, 
            mid_AS3setNamespace, 
            mid_AS3text, 
            mid_AS3toXMLString, 
            mid_AS3valueOf, 
        };
        void prototypeGet(SPtr<Instances::fl::XMLList>& result);
        void prototypeSet(const Value& result, const Value& p);
        void AS3toString(ASString& result);
        void AS3hasOwnProperty(bool& result, const ASString& p);
        void AS3propertyIsEnumerable(Value& result, unsigned argc, const Value* const argv);
        void AS3addNamespace(SPtr<Instances::fl::XML>& result, const Value& ns);
        void AS3appendChild(SPtr<Instances::fl::XML>& result, const Value& child);
        void AS3attribute(SPtr<Instances::fl::XMLList>& result, const Value& arg);
        void AS3attributes(SPtr<Instances::fl::XMLList>& result);
        void AS3child(SPtr<Instances::fl::XMLList>& result, const Value& propertyName);
        void AS3childIndex(SInt32& result);
        void AS3children(SPtr<Instances::fl::XMLList>& result);
        void AS3comments(SPtr<Instances::fl::XMLList>& result);
        void AS3contains(bool& result, const Value& value);
        void AS3copy(SPtr<Instances::fl::XML>& result);
        void AS3descendants(Value& result, unsigned argc, const Value* const argv);
        void AS3elements(Value& result, unsigned argc, const Value* const argv);
        void AS3hasComplexContent(bool& result);
        void AS3hasSimpleContent(bool& result);
        void AS3inScopeNamespaces(SPtr<Instances::fl::Array>& result);
        void AS3insertChildAfter(Value& result, const Value& child1, const Value& child2);
        void AS3insertChildBefore(Value& result, const Value& child1, const Value& child2);
        void AS3length(SInt32& result);
        void AS3localName(ASString& result);
        void AS3name(SPtr<Instances::fl::QName>& result);
        void AS3namespace_(Value& result, unsigned argc, const Value* const argv);
        void AS3namespaceDeclarations(SPtr<Instances::fl::Array>& result);
        void AS3nodeKind(ASString& result);
        void AS3normalize(SPtr<Instances::fl::XML>& result);
        void AS3parent(Value& result);
        void AS3processingInstructions(Value& result, unsigned argc, const Value* const argv);
        void AS3prependChild(SPtr<Instances::fl::XML>& result, const Value& value);
        void AS3removeNamespace(SPtr<Instances::fl::XML>& result, const Value& ns);
        void AS3replace(SPtr<Instances::fl::XML>& result, const Value& propertyName, const Value& value);
        void AS3setChildren(SPtr<Instances::fl::XML>& result, const Value& value);
        void AS3setLocalName(const Value& result, const Value& name);
        void AS3setName(const Value& result, const Value& name);
        void AS3setNamespace(const Value& result, const Value& ns);
        void AS3text(SPtr<Instances::fl::XMLList>& result);
        void AS3toXMLString(ASString& result);
        void AS3valueOf(SPtr<Instances::fl::XML>& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::XMLList> prototypeGet();
        void prototypeSet(const Value& p)
        {
            prototypeSet(Value::GetUndefined(), p);
        }
        ASString AS3toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3toString(result);
            return result;
        }
        bool AS3hasOwnProperty(const ASString& p)
        {
            bool result;
            AS3hasOwnProperty(result, p);
            return result;
        }
        SPtr<Instances::fl::XML> AS3addNamespace(const Value& ns)
        {
            SPtr<Instances::fl::XML> result;
            AS3addNamespace(result, ns);
            return result;
        }
        SPtr<Instances::fl::XML> AS3appendChild(const Value& child)
        {
            SPtr<Instances::fl::XML> result;
            AS3appendChild(result, child);
            return result;
        }
        SPtr<Instances::fl::XMLList> AS3attribute(const Value& arg);
        SPtr<Instances::fl::XMLList> AS3attributes();
        SPtr<Instances::fl::XMLList> AS3child(const Value& propertyName);
        SInt32 AS3childIndex()
        {
            SInt32 result;
            AS3childIndex(result);
            return result;
        }
        SPtr<Instances::fl::XMLList> AS3children();
        SPtr<Instances::fl::XMLList> AS3comments();
        bool AS3contains(const Value& value)
        {
            bool result;
            AS3contains(result, value);
            return result;
        }
        SPtr<Instances::fl::XML> AS3copy()
        {
            SPtr<Instances::fl::XML> result;
            AS3copy(result);
            return result;
        }
        bool AS3hasComplexContent()
        {
            bool result;
            AS3hasComplexContent(result);
            return result;
        }
        bool AS3hasSimpleContent()
        {
            bool result;
            AS3hasSimpleContent(result);
            return result;
        }
        SPtr<Instances::fl::Array> AS3inScopeNamespaces();
        Value AS3insertChildAfter(const Value& child1, const Value& child2)
        {
            Value result;
            AS3insertChildAfter(result, child1, child2);
            return result;
        }
        Value AS3insertChildBefore(const Value& child1, const Value& child2)
        {
            Value result;
            AS3insertChildBefore(result, child1, child2);
            return result;
        }
        SInt32 AS3length()
        {
            SInt32 result;
            AS3length(result);
            return result;
        }
        ASString AS3localName()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3localName(result);
            return result;
        }
        SPtr<Instances::fl::QName> AS3name();
        SPtr<Instances::fl::Array> AS3namespaceDeclarations();
        ASString AS3nodeKind()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3nodeKind(result);
            return result;
        }
        SPtr<Instances::fl::XML> AS3normalize()
        {
            SPtr<Instances::fl::XML> result;
            AS3normalize(result);
            return result;
        }
        Value AS3parent()
        {
            Value result;
            AS3parent(result);
            return result;
        }
        SPtr<Instances::fl::XML> AS3prependChild(const Value& value)
        {
            SPtr<Instances::fl::XML> result;
            AS3prependChild(result, value);
            return result;
        }
        SPtr<Instances::fl::XML> AS3removeNamespace(const Value& ns)
        {
            SPtr<Instances::fl::XML> result;
            AS3removeNamespace(result, ns);
            return result;
        }
        SPtr<Instances::fl::XML> AS3replace(const Value& propertyName, const Value& value)
        {
            SPtr<Instances::fl::XML> result;
            AS3replace(result, propertyName, value);
            return result;
        }
        SPtr<Instances::fl::XML> AS3setChildren(const Value& value)
        {
            SPtr<Instances::fl::XML> result;
            AS3setChildren(result, value);
            return result;
        }
        void AS3setLocalName(const Value& name)
        {
            AS3setLocalName(Value::GetUndefined(), name);
        }
        void AS3setName(const Value& name)
        {
            AS3setName(Value::GetUndefined(), name);
        }
        void AS3setNamespace(const Value& ns)
        {
            AS3setNamespace(Value::GetUndefined(), ns);
        }
        SPtr<Instances::fl::XMLList> AS3text();
        ASString AS3toXMLString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3toXMLString(result);
            return result;
        }
        SPtr<Instances::fl::XML> AS3valueOf()
        {
            SPtr<Instances::fl::XML> result;
            AS3valueOf(result);
            return result;
        }

//##protect##"instance$data"
#ifdef GFX_ENABLE_XML
    protected:
        ASString    Text;
        SPtr<XML>   Parent;
#endif
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    class XML : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::XML"; }
#endif
    public:
        typedef Instances::fl::XML InstanceType;

    public:
        XML(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl::XML> MakeInstance(XML& t)
        {
            return Pickable<Instances::fl::XML>(new(t.Alloc()) Instances::fl::XML(t));
        }
        SPtr<Instances::fl::XML> MakeInstanceS(XML& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 41 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[60];
//##protect##"instance_traits$methods"
#ifdef GFX_ENABLE_XML
        // Prototype related functions.
        static void toStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void PropertyIsEnumerableProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void HasOwnPropertyProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);

        Pickable<Instances::fl::XML> MakeInstance(Traits& t, const ASString& n, Instances::fl::XML* p = NULL);
        Pickable<Instances::fl::XMLElement> MakeInstanceElement(Traits& t, Instances::fl::Namespace& ns, const ASString& n, Instances::fl::XML* p = NULL);
        Pickable<Instances::fl::XMLText> MakeInstanceText(Traits& t, const ASString& txt, Instances::fl::XML* p = NULL);
        Pickable<Instances::fl::XMLText> MakeInstanceText(Traits& t, const StringDataPtr& txt, Instances::fl::XML* p = NULL);
        Pickable<Instances::fl::XMLAttr> MakeInstanceAttr(Traits& t, Instances::fl::Namespace& ns, const ASString& n, const ASString& v, Instances::fl::XML* p = NULL);
        Pickable<Instances::fl::XMLProcInstr> MakeInstanceProcInstr(Traits& t, const ASString& n, const ASString& v, Instances::fl::XML* p = NULL);
        Pickable<Instances::fl::XMLComment> MakeInstanceComment(Traits& t, const ASString& n, Instances::fl::XML* p = NULL);
#endif
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl
{
    class XML : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::XML"; }
#endif
    public:
        typedef Classes::fl::XML ClassType;
        typedef InstanceTraits::fl::XML InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        XML(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 5 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 6 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[8];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl
{
    class XML : public Class
    {
        friend class ClassTraits::fl::XML;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::XMLTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::XML"; }
#endif
    public:
        typedef XML SelfType;
        typedef XML ClassType;
        
    private:
        XML(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
#ifdef GFX_ENABLE_XML
        virtual void Construct(Value& _this, unsigned argc, const Value* argv, bool extCall = false);
        virtual void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);
        virtual void InitPrototype(AS3::Object& obj) const;

        void SetDefaultValues();
#endif
//##protect##"class_$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_AS3settings, 
            mid_settings, 
            mid_AS3setSettings, 
            mid_setSettings, 
            mid_AS3defaultSettings, 
            mid_defaultSettings, 
        };
        void AS3settings(SPtr<Instances::fl::Object>& result);
        void settings(SPtr<Instances::fl::Object>& result);
        void AS3setSettings(const Value& result, const Value& o = Value::GetNull());
        void setSettings(const Value& result, const Value& o = Value::GetNull());
        void AS3defaultSettings(SPtr<Instances::fl::Object>& result);
        void defaultSettings(SPtr<Instances::fl::Object>& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::Object> AS3settings()
        {
            SPtr<Instances::fl::Object> result;
            AS3settings(result);
            return result;
        }
        SPtr<Instances::fl::Object> settings()
        {
            SPtr<Instances::fl::Object> result;
            settings(result);
            return result;
        }
        void AS3setSettings(const Value& o = Value::GetNull())
        {
            AS3setSettings(Value::GetUndefined(), o);
        }
        void setSettings(const Value& o = Value::GetNull())
        {
            setSettings(Value::GetUndefined(), o);
        }
        SPtr<Instances::fl::Object> AS3defaultSettings()
        {
            SPtr<Instances::fl::Object> result;
            AS3defaultSettings(result);
            return result;
        }
        SPtr<Instances::fl::Object> defaultSettings()
        {
            SPtr<Instances::fl::Object> result;
            defaultSettings(result);
            return result;
        }

    public:
        // AS3 API members.
        bool ignoreComments;
        bool ignoreProcessingInstructions;
        bool ignoreWhitespace;
        bool prettyPrinting;
        SInt32 prettyIndent;

//##protect##"class_$data"
#ifdef GFX_ENABLE_XML
        static const TypeInfo* tit[4];
        static const ThunkInfo f[3];
#endif
//##protect##"class_$data"

    };
}}

//##protect##"methods"
#ifdef GFX_ENABLE_XML

namespace Instances { namespace fl
{
    class XMLAttr : public XML
    {
    public:
        XMLAttr(InstanceTraits::Traits& t, Namespace& ns, const ASString& n, const ASString& v, XML* p);
        virtual ~XMLAttr();

    public:
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;

    public:
        bool NameMatches(const XMLAttr& other) const;

        const ASString& GetData() const { return Data; }
        void SetData(const ASString& v) { Data = v; }

        virtual Kind GetKind() const;
        virtual Namespace& GetCurrNamespace() const;
        virtual SPtr<Instances::fl::QName> GetQName();
        virtual void ToString(StringBuffer& buf, SInt32 ident);
        virtual void ToXMLString(StringBuffer& buf, SInt32 ident, const NamespaceArray* ancestorNamespaces, const NamespaceArray* usedNotDeclared);
        virtual Boolean3 EqualsInternal(const XML& other); 
        virtual bool HasSimpleContent() const;
        virtual Pickable<XML> DeepCopy(XML* parent);
        virtual const Namespace& GetNamespace() const;
        virtual void ResolveNamespaces(HashSetDH<ASString>& nr, XML* lp) const;
        virtual void SetNamespace(Namespace& ns);
        virtual void SetNamespace2(Namespace& ns);

    private:
        SPtr<Namespace> Ns;
        ASString        Data;
    };

    class XMLElement : public XML
    {
        friend class AS3::XMLParser; // Because of Namespaces.

    public:
        XMLElement(InstanceTraits::Traits& t, Namespace& ns, const ASString& n, XML* p);
        virtual ~XMLElement();

    public:
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;

    public:
        virtual Kind GetKind() const;
        virtual Namespace& GetCurrNamespace() const;

        void AddAttr(Namespace& ns, const ASString& n, const ASString& v);

        bool FindAttr(const Multiname& prop_name, UPInt& i) const;
        bool FindChild(const Multiname& prop_name, UPInt& i) const;
        // Return NULL if value cannot be converted to XML.
        XML* ToXML(const Value& value) const;
        // 9.1.1.6 [[HasProperty]] (P)
        virtual bool HasProperty(const Multiname& prop_name, bool check_prototype);

        void MakeNsPrefix(StringBuffer& buf, const Namespace& ns, bool checkDefaultNs) const;
        void MakeNsSuffix(StringBuffer& buf, const Namespace& ns, bool checkDefaultNs) const;

        class CallBack
        {
        public:
            virtual ~CallBack() { }
            CallBack(XMLElement& el) : Element(el) {}

            // Return false to stop iteration.
            virtual bool Call(UPInt ind) = 0;

        private:
            CallBack& operator=(const CallBack&);

        protected:
            XMLElement& Element;
        };
        UInt32  ForEachAttr(const Multiname& prop_name, CallBack& cb);
        UInt32  ForEachChild(const Multiname& prop_name, CallBack& cb);

        ArrayLH<SPtr<XMLAttr> >&    GetElementAttrs() { return Attrs; }
        ArrayLH<SPtr<XML> >&        GetElementChildren() { return Children; }

        XMLAttr*    GetAttr(UPInt i) const { return Attrs[i]; }
        XML*        GetChild(UPInt i) const { return Children[i]; }

    public:
        virtual const NamespaceH* GetInScopeNamespaces() const;
        virtual void AddInScopeNamespace(const Namespace& ns);

        virtual void AppendChild(const SPtr<XML>& child);
        virtual CheckResult AppendChild(const Value& child);
        virtual void GetAttributes(Instances::fl::XMLList& list);
        virtual void GetChildren(Instances::fl::XMLList& list, Kind k, const ASString* name = NULL);
        virtual SPtr<Instances::fl::QName> GetQName();
        virtual void ToString(StringBuffer& buf, SInt32 ident);
        virtual void ToXMLString(StringBuffer& buf, SInt32 ident, const NamespaceArray* ancestorNamespaces, const NamespaceArray* usedNotDeclared);
        virtual Boolean3 EqualsInternal(const XML& other); 
        virtual bool HasSimpleContent() const;
        virtual bool HasOwnProperty(const ASString& p);
        virtual Pickable<XML> DeepCopy(XML* parent);
        virtual void GetChildren(Instances::fl::XMLList& list, const Multiname& prop_name);
        virtual CheckResult GetChildIndex(UPInt& ind) const;
        virtual const Namespace& GetNamespace() const;
        virtual void SetNamespace(Namespace& ns);
        virtual void SetNamespace2(Namespace& ns);

        // Can throw exceptions.
        virtual CheckResult InsertChildAt(UPInt pos, const Value& child);
        // Can throw exceptions.
        virtual CheckResult InsertChildAfter(const Value& child1, const Value& child2);
        // Can throw exceptions.
        virtual CheckResult InsertChildBefore(const Value& child1, const Value& child2);

        // If child is null then delete all children, otherwise delete only specified one.
        virtual void DeleteChildren(XML* child);
        // 9.1.1.4 [[DeleteByIndex]] (P)
        virtual void DeleteByIndex(UPInt ind);

        virtual void Normalize();
        virtual Namespace* FindNamespaceByPrefix(const ASString& prefix, XML* lp) const;
        virtual Namespace* FindNamespaceByURI(const ASString& uri, XML* lp) const;
        virtual void ResolveNamespaces(HashSetDH<ASString>& nr, XML* lp) const;
        virtual XML* RemoveNamespace(const Value& ns);

        // Can throw exceptions.
        // 9.1.1.12 [[Replace]] (P, V)
        virtual CheckResult Replace(const Multiname& prop_name, const Value& value);
        // Can throw exceptions.
        CheckResult Replace(UPInt ind, const Value& value);
        // 13.4.4.32
        virtual CheckResult AS3Replace(const Multiname& prop_name, const Value& value);

        // Can throw exceptions.
        virtual void SetChildren(const Value& value);

    protected:
        // Inherited virtual methods.

        virtual void GetDescendants(XMLList& list, const Multiname& prop_name);

        // 9.1.1.2 [[Put]] (P, V)
        virtual CheckResult SetProperty(const Multiname& prop_name, const Value& value);
        // This method will return false if property wasn't found or we got an exception.
        // 9.1.1.1 [[Get]] (P)
        virtual CheckResult GetProperty(const Multiname& prop_name, Value& value);
        virtual CheckResult GetProperty(const Multiname& prop_name, XMLList& list);
        virtual CheckResult DeleteProperty(const Multiname& prop_name);
        virtual PropRef FindDynamicSlot(const Multiname& mn);

    private:
        SPtr<Namespace>         Ns;
        // [[InScopeNamespaces]]
        NamespaceH              Namespaces;
        ArrayLH<SPtr<XMLAttr> > Attrs;
        ArrayLH<SPtr<XML> >     Children;
    };

    class XMLText : public XML
    {
    public:
        XMLText(InstanceTraits::Traits& t, const ASString& txt, XML* p);

    public:
        virtual Kind GetKind() const;
        virtual bool HasSimpleContent() const;
        virtual void ToXMLString(StringBuffer& buf, SInt32 ident, const NamespaceArray* ancestorNamespaces, const NamespaceArray* usedNotDeclared);
        virtual Pickable<XML> DeepCopy(XML* parent);
    };

    class XMLProcInstr : public XML
    {
    public:
        XMLProcInstr(InstanceTraits::Traits& t, const ASString& n, const ASString& v, XML* p);

    public:
        virtual Kind GetKind() const;
        virtual Pickable<XML> DeepCopy(XML* parent);
        virtual SPtr<Instances::fl::QName> GetQName();
        virtual void ToXMLString(StringBuffer& buf, SInt32 ident, const NamespaceArray* ancestorNamespaces, const NamespaceArray* usedNotDeclared);
        virtual Boolean3 EqualsInternal(const XML& other); 

    private:
        ASString Data;
    };

    class XMLComment : public XML
    {
    public:
        XMLComment(InstanceTraits::Traits& t, const ASString& n, XML* p);

    public:
        virtual Kind GetKind() const;
        virtual Pickable<XML> DeepCopy(XML* parent);
        virtual void ToString(StringBuffer& buf, SInt32 ident);
        virtual void ToXMLString(StringBuffer& buf, SInt32 ident, const NamespaceArray* ancestorNamespaces, const NamespaceArray* usedNotDeclared);
        virtual Boolean3 EqualsInternal(const XML& other); 
    };
}}

class XMLParser
{
public:
    XMLParser(InstanceTraits::fl::XML& itr);
    ~XMLParser();

public:
    // Start parsing from position "pos".
    CheckResult Parse(const StringDataPtr& str, UPInt& pos, bool isList = false);
    Instances::fl::XML* GetCurrNode() const
    {
        return pCurrElem;
    }
    void GetRootNodes(ArrayLH<SPtr<Instances::fl::XML> >& nodes) const;

private:
    enum Kind {kNone, kElement, kText, kComment, kDecl, kCData, kDocType, kProcInstr};

    XMLParser& operator =(const XMLParser& other);

    static void StartElementExpatCallback(
        void*           userData,
        const XML_Char* name,
        const XML_Char**atts);
    static void EndElementExpatCallback(
        void*           userData,
        const XML_Char* name);
    static void CharacterDataExpatCallback(
        void*           userData,
        const XML_Char* s,
        int             len);
    static void CommentExpatCallback(
        void*           userData,
        const XML_Char* data);
    static void DeclExpatCallback(
        void*           userData,
        const XML_Char* version,
        const XML_Char* encoding,
        int             standalone);
    static void StartCDataSectionExpatCallback(
        void*           userData);
    static void EndCDataSectionExpatCallback(
        void*           userData);
    static void StartDoctypeDeclExpatCallback(
        void           *userData,
        const XML_Char *doctypeName,
        const XML_Char *sysid,
        const XML_Char *pubid,
        int            has_internal_subset);    
    static void EndDoctypeDeclExpatCallback(
        void           *userData);
    static void ProcessingInstructionExpatCallback(
        void*           userData,
        const XML_Char* target,
        const XML_Char* data);
    static void DefaultExpatCallback(
        void*           userData,
        const XML_Char* s,
        int             len);
    void SetNodeKind(Kind k);
    void PushNodeKind(Kind k) { KindStack.PushBack(k); }
    void PopNodeKind() { KindStack.Pop(); }

private:
    const char              NsSep;
    Kind                    NodeKind;
    InstanceTraits::fl::XML&    ITr;
    XML_Parser              Parser;
    SPtr<Instances::fl::XML>    pCurrElem;
    ArrayDH<SPtr<Instances::fl::XML> > RootElements;
    ASString                Text;
    ArrayDH_POD<Kind>       KindStack;
};
#endif

//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

