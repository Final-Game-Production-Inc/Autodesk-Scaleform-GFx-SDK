/**************************************************************************

PublicHeader:   GFx
Filename    :   AS2_Object.h
Content     :   ActionScript Object implementation classes
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_SF_GFX_OBJECT_H
#define INC_SF_GFX_OBJECT_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "GFx/GFx_ASString.h"
#include "GFx/GFx_Player.h"
#include "GFx/AS2/AS2_ActionTypes.h"
#include "GFx/AS2/AS2_StringManager.h"
#include "AS2_FunctionRef.h"
#include "AS2_Value.h"
#include "Kernel/SF_AmpInterface.h"

#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace GFx { 

class Sprite;

namespace AS2 {

// ***** Declared Classes
class PropFlags;
class Member;
class Object;
class ObjectInterface;
class ObjectProto;
class ObjectCtorFunction;
class FunctionObject;
class FunctionProto;
class FunctionCtorFunction;
class CustomFunctionObject;
class SuperObject;
class AsFunctionObject;

// Parameterized so Environment can be used before definition
template <typename BaseClass, typename Environment = Environment> 
class Prototype;

// ***** External Classes
class Environment;

class AvmSprite;

// ***** PropFlags

// Flags defining the level of protection of an ActionScript member (Member).
// Members can be marked as readOnly, non-enumeratebale, etc.

class PropFlags
{
public:
    // Numeric flags
    UByte        Flags;
    // Flag bit values
    enum PropFlagConstants
    {
        PropFlag_ReadOnly   =   0x04,
        PropFlag_DontDelete =   0x02,
        PropFlag_DontEnum   =   0x01,
        PropFlag_Mask       =   0x07
    };
    
    // Constructors
    PropFlags()
    {
        Flags = 0;        
    }
    
    // Constructor, from numerical value
    PropFlags(UByte flags)    
    {
        Flags = flags;
    }
            
    PropFlags(bool readOnly, bool dontDelete, bool dontEnum)         
    {
        Flags = (UByte) ( ((readOnly) ?   PropFlag_ReadOnly : 0) |
                          ((dontDelete) ? PropFlag_DontDelete : 0) |
                          ((dontEnum) ?   PropFlag_DontEnum : 0) );
    }
        
    // Accessors
    bool    GetReadOnly() const     { return ((Flags & PropFlag_ReadOnly)   ? true : false); }
    bool    GetDontDelete() const   { return ((Flags & PropFlag_DontDelete) ? true : false); }
    bool    GetDontEnum() const     { return ((Flags & PropFlag_DontEnum)   ? true : false); }
    UByte   GetFlags() const        { return Flags; }       
    
    // set the numerical flags value (return the new value )
    // If unlocked is false, you cannot un-protect from over-write,
    // you cannot un-protect from deletion and you cannot
    // un-hide from the for..in loop construct
    UByte SetFlags(UByte setTrue, UByte setFalse)
    {
        Flags = Flags & (~setFalse);
        Flags |= setTrue;
        return GetFlags();
    }
    UByte SetFlags(UByte flags)
    {        
        Flags = flags;
        return GetFlags();
    }
};
 

// ActionScript member: combines value and its properties. Used in Object,
// and potentially other types.
class Member
{    
public:
    Value        mValue;
    
    // Constructors
    Member()
    { mValue.SetPropFlags(0); }
    
    Member(const Value &value,const PropFlags flags = PropFlags())
        : mValue(value)
    { mValue.SetPropFlags(flags.GetFlags()); }

    Member(const Member &src)
        : mValue(src.mValue)
    {
        // Copy PropFlags explicitly since they are not copied as part of value.
        mValue.SetPropFlags(src.mValue.GetPropFlags());
    }

    void operator = (const Member &src)     
    {
        // Copy PropFlags explicitly since they are not copied as part of value.
        mValue = src.mValue;
        mValue.SetPropFlags(src.mValue.GetPropFlags());
    }
    
    // Accessors
    const Value&    GetMemberValue() const                      { return mValue; }
    PropFlags       GetMemberFlags() const                      { return PropFlags(mValue.GetPropFlags()); }
    void            SetMemberValue(const Value &value)       { mValue = value; }
    void            SetMemberFlags(const PropFlags &flags)   { mValue.SetPropFlags(flags.GetFlags()); }

#ifdef GFX_AS_ENABLE_GC
    void Finalize_GC() { mValue.Finalize_GC(); }
#endif
};

// This is the base class for all ActionScript-able objects ("GAS_" stands for ActionScript).   
class ObjectInterface
{
public:
    ObjectInterface();
    virtual ~ObjectInterface();

    // Specific class types.
    enum ObjectType
    {
        Object_Unknown,

        // This type is for non-scriptable characters; it is not ObjectInterface.
        Object_BaseCharacter,
        
        // These need to be grouped in range for IsASCharacter().
        Object_Sprite,
        Object_ASCharacter_Begin    = Object_Sprite,
        Object_Button,
        Object_TextField,
        Object_Video,
        Object_ASCharacter_End      = Object_Video,

        Object_ASObject,
        Object_ASObject_Begin       = Object_ASObject,
        Object_Array,
        Object_String,
        Object_Number,
        Object_Boolean,
        Object_MovieClipObject,
        Object_ButtonASObject,
        Object_TextFieldASObject,
        Object_VideoASObject,
        Object_Matrix,
        Object_Point,
        Object_Rectangle,
        Object_ColorTransform,
        Object_Capabilities,
        Object_Transform,
        Object_Color,
        Object_Key,
        Object_Function,
        Object_Stage,
        Object_MovieClipLoader,
        Object_BitmapData,
        Object_LoadVars,
        Object_XML,
        Object_XMLNode,
        Object_TextFormat,
        Object_StyleSheet,
        Object_Sound,
        Object_NetConnection,
        Object_NetStream,
        Object_Date,
        Object_AsBroadcaster,
        Object_BitmapFilter,
        Object_DropShadowFilter,
        Object_GlowFilter,
        Object_BlurFilter,
        Object_BevelFilter,
        Object_ColorMatrixFilter,
        Object_TextSnapshot,
        Object_SharedObject,
        Object_ASObject_End        = Object_SharedObject
    };

    // Returns text representation of the object.
    // - penv parameter is optional and may be not required for most of the types.
    virtual const char*         GetTextValue(Environment* penv = 0) const    { SF_UNUSED(penv); return 0; }

    virtual ObjectType          GetObjectType() const                           { return Object_Unknown; }

    struct MemberVisitor {
        virtual ~MemberVisitor () { }
        virtual void    Visit(const ASString& name, const Value& val, UByte flags) = 0;
    };

    enum VisitMemberFlags
    {
        VisitMember_Prototype   = 0x01, // Visit prototypes.
        VisitMember_ChildClips  = 0x02, // Visit child clips in sprites as members.
        VisitMember_DontEnum    = 0x04, // Visit members marked as not enumerable.
        VisitMember_NamesOnly   = 0x08  // Get names only, value will be reported empty
    };
    
    virtual bool            SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags = PropFlags()) = 0;
    virtual bool            GetMember(Environment* penv, const ASString& name, Value* val)                       = 0;
    virtual bool            FindMember(ASStringContext *psc, const ASString& name, Member* pmember)                = 0;
    virtual bool            DeleteMember(ASStringContext *psc, const ASString& name)                                  = 0;
    virtual bool            SetMemberFlags(ASStringContext *psc, const ASString& name, const UByte flags)              = 0;
    virtual void            VisitMembers(ASStringContext *psc, MemberVisitor *pvisitor, 
        unsigned visitFlags = 0, const ObjectInterface* instance = 0) const     = 0;
    virtual bool            HasMember(ASStringContext *psc, const ASString& name, bool inclPrototypes)                = 0;

    virtual bool            SetMemberRaw(ASStringContext *psc, const ASString& name, const Value& val,
                                         const PropFlags& flags = PropFlags())                                    = 0;
    virtual bool            GetMemberRaw(ASStringContext *psc, const ASString& name, Value* val)                   = 0;

    // Helper for string names: registers string automatically.
    // Should not be used for built-ins.
    bool    SetMemberRaw(ASStringContext *psc, const char *pname, const Value& val)
    {
        return SetMemberRaw(psc, ASString(psc->CreateString(pname)), val);        
    }
    bool    SetConstMemberRaw(ASStringContext *psc, const char *pname, const Value& val)
    {
        return SetMemberRaw(psc, ASString(psc->CreateConstString(pname)), val);
    }
    bool    SetConstMemberRaw(ASStringContext *psc, const char *pname, const Value& val, const PropFlags& flags)
    {
        return SetMemberRaw(psc, ASString(psc->CreateConstString(pname)), val, flags);
    }
    bool    GetConstMemberRaw(ASStringContext *psc, const char *pname, Value* val)
    {
        return GetMemberRaw(psc, ASString(psc->CreateConstString(pname)), val);
    }

    // Convenience commonly used RTTI inlines.
    SF_INLINE static bool     IsTypeASCharacter(ObjectType t) { return (t>=Object_ASCharacter_Begin) && (t<=Object_ASCharacter_End);  }
    SF_INLINE static bool     IsTypeASObject(ObjectType t) { return (t>=Object_ASObject_Begin) && (t<=Object_ASObject_End);  }

    SF_INLINE bool            IsASCharacter() const   { return IsTypeASCharacter(GetObjectType());  }
    SF_INLINE bool            IsASObject() const      { return IsTypeASObject(GetObjectType());  }
    SF_INLINE bool            IsSprite() const        { return GetObjectType() == Object_Sprite; }
    SF_INLINE bool            IsFunction() const      { return GetObjectType() == Object_Function; }

    // Convenience casts; in cpp file because these require GFxSprite/InteractiveObject definitions.
    InteractiveObject*      ToCharacter();
    AvmCharacter*           ToAvmCharacter();
    Object*                 ToASObject();
    Sprite*                 ToSprite();
    AvmSprite*              ToAvmSprite();
    virtual FunctionRef     ToFunction();

    const InteractiveObject* ToCharacter() const;
    const AvmCharacter*     ToAvmCharacter() const;
    const Object*           ToASObject() const;
    const Sprite*           ToSprite() const;
    const AvmSprite*        ToAvmSprite() const;

    // sets __proto__ property 
    virtual void            Set__proto__(ASStringContext *psc, Object* protoObj) = 0;

    // gets __proto__ property
    inline Object*       Get__proto__() const { return pProto; }


    // sets __constructor__ property
    void                    Set__constructor__(ASStringContext *psc, const FunctionRef& ctorFunc) 
    {   
        SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin___constructor__), ctorFunc,
                     PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
    }

    // gets __constructor__ property
    // virtual because it's overridden in SuperObject
    virtual FunctionRef  Get__constructor__(ASStringContext *psc)
    { 
        Value val;
        if (GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin___constructor__), &val))
            return val.ToFunction(NULL);
        return 0; 
    }

    // sets "constructor" property
    void                    Set_constructor(ASStringContext *psc, const FunctionRef& ctorFunc) 
    {
        SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_constructor), ctorFunc,
                     PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
    }
    // gets "constructor" property
    FunctionRef          Get_constructor(ASStringContext *psc)
    { 
        Value val;
        if (GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_constructor), &val))
            return val.ToFunction(NULL);
        return 0; 
    }
    
    // is it super-class object
    virtual bool            IsSuper() const { return false; }
    // is it prototype of built-in class?
    virtual bool            IsBuiltinPrototype() const { return false; }

    // adds interface to "implements-of" list. If ctor is null, then index
    // specifies the total number of interfaces to pre-allocate the array.
    virtual void AddInterface(ASStringContext *psc, int index, FunctionObject* ctor) { SF_UNUSED3(psc, index, ctor); } // implemented for prototypes only

    virtual bool InstanceOf(Environment*, const Object* prototype, bool inclInterfaces = true) const { SF_UNUSED2(prototype, inclInterfaces); return false; }

    virtual bool Watch(ASStringContext *psc, const ASString& prop, const FunctionRef& callback, const Value& userData) { SF_UNUSED4(psc, prop,callback,userData); return false; }
    virtual bool Unwatch(ASStringContext *psc, const ASString& prop)  { SF_UNUSED2(psc, prop); return false; }

    Object*   FindOwner(ASStringContext *psc, const ASString& name);

    AmpStats* GetAdvanceStats() const { return NULL; }

#ifdef GFX_AS_ENABLE_USERDATA
protected:
    struct UserDataHolder : public NewOverrideBase<StatMV_ActionScript_Mem>
    {
        Movie*          pMovieView;
        ASUserData*     pUserData;
        UserDataHolder(Movie* pmovieView, ASUserData* puserData) :
        pMovieView(pmovieView), pUserData(puserData) {}
        ~UserDataHolder() 
        { 
            if (pUserData)
                pUserData->SetLastObjectValue(NULL, NULL, false);
        }
        
        void    NotifyDestroy(ObjectInterface* pthis)
        {
            if (pUserData)  
            {
                // Remove user data weak ref
                pUserData->SetLastObjectValue(NULL, NULL, false);
                pUserData->OnDestroy(pMovieView, pthis);
        
                // Clear user data pointer so destructor doesn't access a bad pointer
                pUserData = NULL;
            }
        }
    } *pUserDataHolder;

public:
    ASUserData*         GetUserData() const     { return pUserDataHolder ? pUserDataHolder->pUserData : NULL; }
    void                SetUserData(Movie* pmovieView, ASUserData* puserData, bool isdobj);

#endif  // GFX_AS_ENABLE_USERDATA

protected:
    Ptr<Object>     pProto;          // __proto__
};


// ***** Object

// A generic bag of attributes.  Base-class for ActionScript
// script-defined objects.

//class Object : public RefCountBase<Object, StatMV_ActionScript_Mem>, public ObjectInterface
class Object : public ASRefCountBase<Object>, public ObjectInterface
{
private:
    Object(const Object&):ASRefCountBase<Object>(NULL) {}
public:

    struct Watchpoint
    {
        FunctionRef  Callback;
        Value        UserData;

#ifdef GFX_AS_ENABLE_GC
        template <class Functor>
        void ForEachChild_GC(Collector* prcc) const
        {
            Callback.template ForEachChild_GC<Functor>(prcc);
            UserData.template ForEachChild_GC<Functor>(prcc);
        }
        void Finalize_GC() 
        { 
            Callback.Finalize_GC(); 
            UserData.Finalize_GC(); 
        }
#endif //SF_NO_GC
    };

#ifdef GFX_AS_ENABLE_GC
    // Hash table used for ActionScript members, garbage collection version
    typedef ASStringHash_GC<Member>     MemberHash;
    typedef ASStringHash_GC<Watchpoint> WatchpointHash;
#else
    // Hash table used for ActionScript members.
    typedef ASStringHash<Member>        MemberHash;
    typedef ASStringHash<Watchpoint>    WatchpointHash;
#endif

    MemberHash          Members;
    FunctionRef         ResolveHandler;  // __resolve
    
    WatchpointHash*     pWatchpoints;

    bool                ArePropertiesSet; // indicates if any property (Object.addProperty) was set
    bool                IsListenerSet; // See comments in Value.cpp, CheckForListenersMemLeak. 

    Object(ASStringContext *psc);
    Object(ASStringContext *psc, Object* proto);
    Object(Environment* pEnv);

    virtual ~Object();
    
    virtual const char* GetTextValue(Environment* penv = 0) const { SF_UNUSED(penv); return NULL; }

    virtual bool    SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags = PropFlags());
    virtual bool    GetMember(Environment* penv, const ASString& name, Value* val);
    virtual bool    FindMember(ASStringContext *psc, const ASString& name, Member* pmember);
    virtual bool    DeleteMember(ASStringContext *psc, const ASString& name);
    virtual bool    SetMemberFlags(ASStringContext *psc, const ASString& name, const UByte flags);
    virtual void    VisitMembers(ASStringContext *psc, MemberVisitor *pvisitor, unsigned visitFlags = 0, const ObjectInterface* instance = 0) const;
    virtual bool    HasMember(ASStringContext *psc, const ASString& name, bool inclPrototypes);

    virtual bool    SetMemberRaw(ASStringContext *psc, const ASString& name, const Value& val, const PropFlags& flags = PropFlags());
    virtual bool    GetMemberRaw(ASStringContext *psc, const ASString& name, Value* val);

    virtual ObjectType  GetObjectType() const
    {
        return Object_ASObject;
    }

    void    Clear()
    {
        Members.Clear();
        pProto = 0;             
    }

    // sets __proto__ property 
    virtual void            Set__proto__(ASStringContext *psc, Object* protoObj) 
    {   
        if (!pProto)
            SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin___proto__), Value(Value::UNSET),
                         PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
        pProto = protoObj;
    }
    // gets __proto__ property
    /*virtual Object*      Get__proto__()
    { 
        return pProto; 
    }*/

    virtual void        SetValue(Environment* penv, const Value&);
    virtual Value       GetValue() const;

    virtual bool        InstanceOf(Environment* penv, const Object* prototype, bool inclInterfaces = true) const;
    virtual bool        DoesImplement(Environment*, const Object* prototype) const { return this == prototype; }

    virtual bool        Watch(ASStringContext *psc, const ASString& prop, const FunctionRef& callback, const Value& userData);
    virtual bool        Unwatch(ASStringContext *psc, const ASString& prop);
    bool                InvokeWatchpoint(Environment* penv, const ASString& prop, const Value& newVal, Value* resultVal);
    bool                HasWatchpoint() const { return pWatchpoints != NULL; }

    // these method will return an original InteractiveObject for GASObjects from GFxSprite, GFxButton,
    // GFxText.
    virtual InteractiveObject*         GetASCharacter() { return 0; }
    virtual const InteractiveObject*   GetASCharacter() const { return 0; }

    static Object*   GetPrototype(GlobalContext* pgc, ASBuiltinType classNameId);

    // checks and reset the "constructor" property (in the case if param is equal to ctor). 
    // Works only for prototypes, otherwise does nothing.
    virtual void CheckAndResetCtorRef(FunctionObject*) { }
protected:
    Object(ASRefCountCollector *pcc);
private:
    void Init();
#ifdef GFX_AS_ENABLE_GC
protected:
    friend class RefCountBaseGC<StatMV_ActionScript_Mem>;
    template <class Functor> void ForEachChild_GC(Collector* prcc) const;
    virtual                  void ExecuteForEachChild_GC(Collector* prcc, OperationGC operation) const;

    virtual void Finalize_GC();
public:
    ASRefCountCollector *GetCollector() 
    { 
        return static_cast<ASRefCountCollector*>(ASRefCountBase<Object>::GetCollector());
    }
#else
public:
    ASRefCountCollector *GetCollector() { return NULL; }
#endif //SF_NO_GC

};

// Base class for AS Function object.
class FunctionObject : public Object 
{
    friend class FunctionProto;
    friend class FunctionCtorFunction;
    friend class CustomFunctionObject;

#ifndef GFX_AS_ENABLE_GC
    Object* pPrototype; // also hold firmly by members
public:
    virtual bool  SetMember(Environment *penv, 
        const ASString& name, 
        const Value& val, 
        const PropFlags& flags = PropFlags());
#endif

protected:
    FunctionObject(ASStringContext* psc);
    FunctionObject(Environment* penv);
public:
    ~FunctionObject();

    virtual ObjectType      GetObjectType() const   { return Object_Function; }

    virtual Environment* GetEnvironment(const FnCall& fn, Ptr<InteractiveObject>* ptargetCh) = 0;
    virtual void Invoke (const FnCall& fn, LocalFrame*, const char* pmethodName) = 0;

    virtual bool IsNull () const                           = 0;
    virtual bool IsEqual(const FunctionObject& f) const = 0;

    virtual bool IsCFunction() const   { return false; }
    virtual bool IsAsFunction() const  { return false; }

    /* This method is used for creation of object by AS "new" (Environment::OperatorNew)
       If this method returns NULL, then constructor function is responsible for
       object's creation. */
    virtual Object* CreateNewObject(Environment*) const { return 0; }

    void SetPrototype(ASStringContext* psc, Object* pprototype);
    void SetProtoAndCtor(ASStringContext* psc, Object* pprototype);

    FunctionRef  ToFunction();

    // returns number of arguments expected by this function;
    // returns -1 if number of arguments is unknown (for C functions)
    virtual int     GetNumArgs() const = 0;

    Object* GetPrototype(ASStringContext* psc);

    static AsFunctionObject* CreateAsFunction(Environment* penv);
};

// Function object for built-in functions ("C-functions")
class CFunctionObject : public FunctionObject 
{
    friend class FunctionProto;
    friend class FunctionCtorFunction;
    friend class GASCustomFunctionObject;
    friend class FunctionRefBase;
protected:
    //GASCFunctionDef     Def;
    CFunctionPtr pFunction;

    CFunctionObject(ASStringContext* psc);
    CFunctionObject(Environment* penv);
public:
    //CFunctionObject(Environment* penv, GASCFunctionDef* func);
    CFunctionObject(ASStringContext* psc, Object* pprototype, CFunctionPtr func);
    CFunctionObject(ASStringContext* psc, CFunctionPtr func);
    ~CFunctionObject();

    virtual Environment* GetEnvironment(const FnCall& fn, Ptr<InteractiveObject>* ptargetCh);
    void Invoke (const FnCall& fn, LocalFrame*, const char* pmethodName);

    virtual bool IsNull () const { return pFunction == 0; }
    virtual bool IsEqual(const FunctionObject& f) const;
    bool IsEqual (CFunctionPtr f) const { return pFunction == f; }

    virtual bool IsCFunction() const { return true; }

    // returns number of arguments expected by this function;
    // returns -1 if number of arguments is unknown (for C functions)
    int     GetNumArgs() const { return -1; }
};

// A constructor function object for Object
class ObjectCtorFunction : public CFunctionObject
{
    static const NameFunction StaticFunctionTable[];

    static void RegisterClass (const FnCall& fn);

public:
    ObjectCtorFunction(ASStringContext *psc);
    ~ObjectCtorFunction() {}

    virtual Object* CreateNewObject(Environment* penv) const;
};

// A constructor function object for class "Function"
class FunctionCtorFunction : public CFunctionObject
{
public:
    FunctionCtorFunction(ASStringContext *psc);

    virtual Object* CreateNewObject(Environment* penv) const;
};

class LocalFrame : public ASRefCountBase<LocalFrame>
{
#ifdef GFX_AS_ENABLE_GC
    // Garbage collection version
    friend class RefCountBaseGC<StatMV_ActionScript_Mem>;
    template <class Functor> void ForEachChild_GC(Collector* prcc) const;
    virtual void                  ExecuteForEachChild_GC(Collector* prcc, OperationGC operation) const;
    virtual void                  Finalize_GC();
public:
    ASStringHash_GC<Value>  Variables;
#else
public:
    // registry for locally declared funcs.
    HashLH<FunctionObject*,int, FixedSizeHash<FunctionObject*> > LocallyDeclaredFuncs; 

    ASStringHash<Value>     Variables;
#endif //SF_NO_GC
public:
    Ptr<LocalFrame>         PrevFrame;
    ObjectInterface*        SuperThis; //required for "super" in SWF6

    // these members are necessary for direct access to function call parameters
    Environment*            Env;
    int                     NArgs;
    int                     FirstArgBottomIndex;

    // callee and caller
    Value                   Callee, Caller;


    LocalFrame(ASRefCountCollector* pcc) 
        : ASRefCountBase<LocalFrame>(pcc), SuperThis(0), Env(0), NArgs(0), FirstArgBottomIndex(0) {}
    ~LocalFrame() 
    { }

    void ReleaseFramesForLocalFuncs ();
    SF_INLINE Value& Arg(int n) const;
};
}}} // SF::GFx::AS2

#include "AS2_FunctionRefImpl.h"

#endif //INC_SF_GFX_OBJECT_H
