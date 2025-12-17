//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_XMLList.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy, Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_XMLList_H
#define INC_AS3_Obj_XMLList_H

#include "AS3_Obj_Object.h"
//##protect##"includes"
// This header is required by the 3DS compiler.
#include "AS3_Obj_XML.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl
{
    extern const TypeInfo XMLListTI;
    extern const ClassInfo XMLListCI;
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo anyTI;
    extern const ClassInfo anyCI;
    extern const TypeInfo QNameTI;
    extern const ClassInfo QNameCI;
    extern const TypeInfo XMLTI;
    extern const ClassInfo XMLCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
} // namespace fl

namespace ClassTraits { namespace fl
{
    class XMLList;
}}

namespace InstanceTraits { namespace fl
{
    class XMLList;
}}

namespace Classes { namespace fl
{
    class XMLList;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class XML;
    class QName;
}
//##protect##"forward_declaration"

namespace Instances { namespace fl
{
    class XMLList : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl::XMLList;
        
    public:
        typedef XMLList SelfType;
        typedef Classes::fl::XMLList ClassType;
        typedef InstanceTraits::fl::XMLList TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::XMLListTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl::XMLList"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        XMLList(InstanceTraits::Traits& t);

//##protect##"instance$methods"
#ifdef GFX_ENABLE_XML

    public:
        ~XMLList();

    public:
        // E4X 9.2.1.9
        CheckResult Equals(bool& result, const Value& v); 

        void Apppend(XML& v) { List.PushBack(&v); }
        template <typename T>
        void Apppend(Pickable<T> v) { List.PushBack(SPtr<XML>(v)); }
        void Apppend(XMLList& v);
        CheckResult HasOneItem(SF_DEBUG_CODE(const char* fn)) const;

        Pickable<XMLList> DeepCopy(XML* parent);
        Pickable<XMLList> ShallowCopy();
        
        Pickable<Instances::fl::XMLList> MakeInstance();
        Pickable<Instances::fl::XMLList> MakeInstance(const Multiname& mn);
        Pickable<Instances::fl::XMLList> MakeInstance(Instances::fl::Object& tgt_obj, const ASString& tgt_prop, Namespace& tgt_ns);

        UPInt GetSize() const { return List.GetSize(); }

        class CallBack
        {
        public:
            virtual ~CallBack() { }
            CallBack(XMLList& l) : List(l) {}

            // li - index in the list.
            // ii - index inside of element with index li.
            // ii can be index of an attribute or a child.
            virtual void Call(UPInt li, UPInt ii) = 0;

        private:
            CallBack& operator=(const CallBack&);

        protected:
            XMLList& List;
        };
        // Return number of matched properties.
        UInt32 ForEachChild(const Multiname& prop_name, CallBack& cb);

        Object* GetTargetObject() const { return TargetObject.GetPtr(); }
        void SetTargetObject(Object* v) { TargetObject = v; };

        ASStringNode* GetTargetProperty() const { return TargetProperty; }
        void SetTargetProperty(ASStringNode* v);

        Namespace& GetTargetNamespace() const { return *TargetNamespace; }
        void SetTargetNamespace(Namespace& v)
        {
            TargetNamespace = &v;
        }

        // 9.2.1.10 [[ResolveValue]] ( )
        CheckResult ResolveValue(XML*& result);
        // 9.2.1.5 [[HasProperty]] (P)
        virtual bool HasProperty(const Multiname& prop_name, bool check_prototype);
        // This method won't handle numeric properties.
        CheckResult GetProperty(const Multiname& prop_name, XMLList& list);

        Namespace* FindNamespaceByURI(const ASString& uri) const;

    public:
        // Inherited virtual methods.

        virtual void AS3Constructor(unsigned argc, const Value* argv);

        // 9.1.1.8 [[Descendants]] (P)
        virtual void GetDescendants(XMLList& list, const Multiname& prop_name);

        // 11.6.2 XMLList Assignment Operator
        // 9.2.1.2 [[Put]] (P, V)
        virtual CheckResult SetProperty(const Multiname& prop_name, const Value& value);
        // 9.2.1.1 [[Get]] (P)
        virtual CheckResult GetProperty(const Multiname& prop_name, Value& value);
        virtual void GetDynamicProperty(AbsoluteIndex ind, Value& value);
        virtual CheckResult DeleteProperty(const Multiname& prop_name);

        virtual void GetNextPropertyName(Value& name, GlobalSlotIndex ind) const;
        virtual GlobalSlotIndex GetNextDynPropIndex(GlobalSlotIndex ind) const;
        virtual void GetNextPropertyValue(Value& value, GlobalSlotIndex ind);

        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
#endif

//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_prototypeGet, 
            mid_prototypeSet, 
            mid_AS3length, 
            mid_AS3toString, 
            mid_AS3valueOf, 
            mid_AS3hasOwnProperty, 
            mid_AS3propertyIsEnumerable, 
            mid_AS3attribute, 
            mid_AS3attributes, 
            mid_AS3child, 
            mid_AS3children, 
            mid_AS3comments, 
            mid_AS3contains, 
            mid_AS3copy, 
            mid_AS3descendants, 
            mid_AS3elements, 
            mid_AS3hasComplexContent, 
            mid_AS3hasSimpleContent, 
            mid_AS3name, 
            mid_AS3normalize, 
            mid_AS3parent, 
            mid_AS3processingInstructions, 
            mid_AS3text, 
            mid_AS3toXMLString, 
            mid_AS3addNamespace, 
            mid_AS3appendChild, 
            mid_AS3childIndex, 
            mid_AS3inScopeNamespaces, 
            mid_AS3insertChildAfter, 
            mid_AS3insertChildBefore, 
            mid_AS3nodeKind, 
            mid_AS3namespace_, 
            mid_AS3localName, 
            mid_AS3namespaceDeclarations, 
            mid_AS3prependChild, 
            mid_AS3removeNamespace, 
            mid_AS3replace, 
            mid_AS3setChildren, 
            mid_AS3setLocalName, 
            mid_AS3setName, 
            mid_AS3setNamespace, 
        };
        void prototypeGet(SPtr<Instances::fl::XMLList>& result);
        void prototypeSet(const Value& result, const Value& p);
        void AS3length(SInt32& result);
        void AS3toString(ASString& result);
        void AS3valueOf(SPtr<Instances::fl::XMLList>& result);
        static void AS3hasOwnProperty(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        void AS3propertyIsEnumerable(Value& result, unsigned argc, const Value* const argv);
        void AS3attribute(SPtr<Instances::fl::XMLList>& result, const Value& arg);
        void AS3attributes(SPtr<Instances::fl::XMLList>& result);
        void AS3child(SPtr<Instances::fl::XMLList>& result, const Value& propertyName);
        void AS3children(SPtr<Instances::fl::XMLList>& result);
        void AS3comments(SPtr<Instances::fl::XMLList>& result);
        void AS3contains(bool& result, const Value& value);
        void AS3copy(SPtr<Instances::fl::XMLList>& result);
        void AS3descendants(Value& result, unsigned argc, const Value* const argv);
        void AS3elements(Value& result, unsigned argc, const Value* const argv);
        void AS3hasComplexContent(bool& result);
        void AS3hasSimpleContent(bool& result);
        void AS3name(SPtr<Instances::fl::QName>& result);
        void AS3normalize(SPtr<Instances::fl::XMLList>& result);
        void AS3parent(Value& result);
        void AS3processingInstructions(Value& result, unsigned argc, const Value* const argv);
        void AS3text(SPtr<Instances::fl::XMLList>& result);
        void AS3toXMLString(ASString& result);
        void AS3addNamespace(SPtr<Instances::fl::XML>& result, const Value& ns);
        void AS3appendChild(SPtr<Instances::fl::XML>& result, const Value& child);
        void AS3childIndex(SInt32& result);
        void AS3inScopeNamespaces(SPtr<Instances::fl::Array>& result);
        void AS3insertChildAfter(Value& result, const Value& child1, const Value& child2);
        void AS3insertChildBefore(Value& result, const Value& child1, const Value& child2);
        void AS3nodeKind(ASString& result);
        void AS3namespace_(Value& result, unsigned argc, const Value* const argv);
        void AS3localName(ASString& result);
        void AS3namespaceDeclarations(SPtr<Instances::fl::Array>& result);
        void AS3prependChild(SPtr<Instances::fl::XML>& result, const Value& value);
        void AS3removeNamespace(SPtr<Instances::fl::XML>& result, const Value& ns);
        void AS3replace(SPtr<Instances::fl::XML>& result, const Value& propertyName, const Value& value);
        void AS3setChildren(SPtr<Instances::fl::XML>& result, const Value& value);
        void AS3setLocalName(const Value& result, const Value& name);
        void AS3setName(const Value& result, const Value& name);
        void AS3setNamespace(const Value& result, const Value& ns);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::XMLList> prototypeGet()
        {
            SPtr<Instances::fl::XMLList> result;
            prototypeGet(result);
            return result;
        }
        void prototypeSet(const Value& p)
        {
            prototypeSet(Value::GetUndefined(), p);
        }
        SInt32 AS3length()
        {
            SInt32 result;
            AS3length(result);
            return result;
        }
        ASString AS3toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3toString(result);
            return result;
        }
        SPtr<Instances::fl::XMLList> AS3valueOf()
        {
            SPtr<Instances::fl::XMLList> result;
            AS3valueOf(result);
            return result;
        }
        SPtr<Instances::fl::XMLList> AS3attribute(const Value& arg)
        {
            SPtr<Instances::fl::XMLList> result;
            AS3attribute(result, arg);
            return result;
        }
        SPtr<Instances::fl::XMLList> AS3attributes()
        {
            SPtr<Instances::fl::XMLList> result;
            AS3attributes(result);
            return result;
        }
        SPtr<Instances::fl::XMLList> AS3child(const Value& propertyName)
        {
            SPtr<Instances::fl::XMLList> result;
            AS3child(result, propertyName);
            return result;
        }
        SPtr<Instances::fl::XMLList> AS3children()
        {
            SPtr<Instances::fl::XMLList> result;
            AS3children(result);
            return result;
        }
        SPtr<Instances::fl::XMLList> AS3comments()
        {
            SPtr<Instances::fl::XMLList> result;
            AS3comments(result);
            return result;
        }
        bool AS3contains(const Value& value)
        {
            bool result;
            AS3contains(result, value);
            return result;
        }
        SPtr<Instances::fl::XMLList> AS3copy()
        {
            SPtr<Instances::fl::XMLList> result;
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
        SPtr<Instances::fl::QName> AS3name();
        SPtr<Instances::fl::XMLList> AS3normalize()
        {
            SPtr<Instances::fl::XMLList> result;
            AS3normalize(result);
            return result;
        }
        Value AS3parent()
        {
            Value result;
            AS3parent(result);
            return result;
        }
        SPtr<Instances::fl::XMLList> AS3text()
        {
            SPtr<Instances::fl::XMLList> result;
            AS3text(result);
            return result;
        }
        ASString AS3toXMLString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3toXMLString(result);
            return result;
        }
        SPtr<Instances::fl::XML> AS3addNamespace(const Value& ns);
        SPtr<Instances::fl::XML> AS3appendChild(const Value& child);
        SInt32 AS3childIndex()
        {
            SInt32 result;
            AS3childIndex(result);
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
        ASString AS3nodeKind()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3nodeKind(result);
            return result;
        }
        ASString AS3localName()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3localName(result);
            return result;
        }
        SPtr<Instances::fl::Array> AS3namespaceDeclarations();
        SPtr<Instances::fl::XML> AS3prependChild(const Value& value);
        SPtr<Instances::fl::XML> AS3removeNamespace(const Value& ns);
        SPtr<Instances::fl::XML> AS3replace(const Value& propertyName, const Value& value);
        SPtr<Instances::fl::XML> AS3setChildren(const Value& value);
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

//##protect##"instance$data"
#ifdef GFX_ENABLE_XML
    private:
        // 9.2.1 Internal Properties and Methods
        // The XML or XMLList object associated with this object that will 
        // be affected when items are inserted into this XMLList.
        SPtr<Object>        TargetObject;
        // 9.2.1 Internal Properties and Methods
        // The name of a property that may be created in the [[TargetObject]] 
        // when objects are added to an empty XMLList.
        ASStringNode*       TargetProperty;
        SPtr<Namespace>     TargetNamespace;

    public:
        ArrayLH<SPtr<XML> > List;
#endif
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    class XMLList : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::XMLList"; }
#endif
    public:
        typedef Instances::fl::XMLList InstanceType;

    public:
        XMLList(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl::XMLList> MakeInstance(XMLList& t)
        {
            return Pickable<Instances::fl::XMLList>(new(t.Alloc()) Instances::fl::XMLList(t));
        }
        SPtr<Instances::fl::XMLList> MakeInstanceS(XMLList& t)
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
        Pickable<Instances::fl::XMLList> MakeInstance(Traits& t, Instances::fl::Object& target_obj, const ASString& target_prop, Instances::fl::Namespace& target_ns);

        // Prototype related functions.
        static void toStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
#endif
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl
{
    class XMLList : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::XMLList"; }
#endif
    public:
        typedef Classes::fl::XMLList ClassType;
        typedef InstanceTraits::fl::XMLList InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        XMLList(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl
{
    class XMLList : public Class
    {
        friend class ClassTraits::fl::XMLList;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::XMLListTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::XMLList"; }
#endif
    public:
        typedef XMLList SelfType;
        typedef XMLList ClassType;
        
    private:
        XMLList(ClassTraits::Traits& t);
       
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
#endif
//##protect##"class_$methods"

//##protect##"class_$data"
#ifdef GFX_ENABLE_XML
        static const TypeInfo* tit[2];
        static const ThunkInfo f[2];
#endif
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

