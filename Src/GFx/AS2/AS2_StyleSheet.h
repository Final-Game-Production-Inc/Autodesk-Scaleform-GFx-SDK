/**************************************************************************

Filename    :   AS2_StyleSheet.h
Content     :   StyleSheet (Textfield.StyleSheet) object functionality
Created     :   May 20, 2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GASStyleSheet_H
#define INC_GASStyleSheet_H

#include "GFx/AS2/AS2_Action.h"
#include "Render/Text/Text_Core.h"
#include "GFx/Text/Text_StyleSheet.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

#ifdef GFX_ENABLE_CSS

// ***** Declared Classes
class StyleSheetObject;
class StyleSheetProto;
class StyleSheetCtorFunction;


//
// ActionScript CSS file loader interface. 
//
// This interface is used by the GFx core to load a CSS file to an 
// ActionScript StyleSheet object. An instance of this object is 
// provided by the CSS library and passed through the movie load 
// queues. The two methods are called at the appropriate moments from 
// within GFx. Its implementation is defined in the CSS library. This 
// interface only serves as a temporary wrapper for the duration of 
// passing through the GFx core.
//
class ASCSSFileLoader : public RefCountBase<ASCSSFileLoader, Stat_Default_Mem>
{
public:
    ASCSSFileLoader() { }
    virtual         ~ASCSSFileLoader() {}

    //
    // Load a CSS file using the file opener
    //
    virtual void    Load(const String& filename, FileOpener* pfo) = 0;

    //
    // Initialize the ActionScript StyleSheet object with the loaded data
    //
    virtual void    Init(Environment* penv, Object* pTarget) = 0;
};



class StyleSheetObject : public Object
{
protected:
    StyleSheetObject(ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }

#ifdef GFX_AS_ENABLE_GC
protected:
    virtual void Finalize_GC()
    {
        CSS.~StyleManager();
        Object::Finalize_GC();
    }
#endif //SF_NO_GC
public:
    Text::StyleManager   CSS;

    StyleSheetObject(Environment* penv);

    ObjectType      GetObjectType() const   { return Object_StyleSheet; }

    void            NotifyOnLoad(Environment* penv, bool success);

    Text::StyleManager* GetTextStyleManager() { return &CSS; }
    const Text::StyleManager* GetTextStyleManager() const { return &CSS; }
};

class StyleSheetProto : public Prototype<StyleSheetObject>
{
public:
    StyleSheetProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);

    static void Clear(const FnCall& fn);
    static void GetStyle(const FnCall& fn);
    static void GetStyleNames(const FnCall& fn);
    static void Load(const FnCall& fn);
    static void ParseCSS(const FnCall& fn);
    static void SetStyle(const FnCall& fn);
    static void Transform(const FnCall& fn);
};

class StyleSheetCtorFunction : public CFunctionObject
{
public:
    StyleSheetCtorFunction(ASStringContext *psc);

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment* penv) const;

    static FunctionRef Register(GlobalContext* pgc);
};
#endif // GFX_ENABLE_CSS

}}} // SF::GFx::AS2

#endif // INC_GASStyleSheet_H

