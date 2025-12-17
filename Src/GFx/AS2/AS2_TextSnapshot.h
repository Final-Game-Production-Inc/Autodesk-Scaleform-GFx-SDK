/**************************************************************************

Filename    :   AS2_TextSnapshot.h
Content     :   TextSnapshot reference class for ActionScript 2.0
Created     :   12/10/2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GASTextSnapshot_H
#define INC_GASTextSnapshot_H

#include "GFxConfig.h"
#include "GFx/GFx_StaticText.h"

#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT

#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class TextSnapshotObject;
class TextSnapshotProto;
class TextSnapshotCtorFunction;

// 
// Actionscript TextSnapshot object declaration
//

class TextSnapshotObject : public Object
{
    friend class TextSnapshotProto;
    friend class TextSnapshotCtorFunction;

#ifdef GFX_AS_ENABLE_GC
protected:
    virtual void Finalize_GC()
    {
        SnapshotData.~StaticTextSnapshotData();
        Object::Finalize_GC();
    }
#endif //SF_NO_GC
protected:

    StaticTextSnapshotData       SnapshotData;

    TextSnapshotObject(ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }

    virtual         ~TextSnapshotObject();

public:

    TextSnapshotObject(Environment* penv);

    virtual ObjectType      GetObjectType() const { return Object_TextSnapshot; }

    void                    Process(Sprite* pmovieClip);
    
    SF_INLINE const StaticTextSnapshotData&    GetData()   { return SnapshotData; }
    SF_INLINE void            SetSelectColor(const Color& color) { SnapshotData.SetSelectColor(color); }
};


class TextSnapshotProto : public Prototype<TextSnapshotObject>
{
public:
    TextSnapshotProto(ASStringContext *psc, Object* prototype, 
        const FunctionRef& constructor);

    static const NameFunction FunctionTable[];

    static void             FindText(const FnCall& fn);
    static void             GetCount(const FnCall& fn);
    static void             GetSelected(const FnCall& fn);
    static void             GetSelectedText(const FnCall& fn);
    static void             GetText(const FnCall& fn);
    static void             GetTextRunInfo(const FnCall& fn);
    static void             HitTestTextNearPos(const FnCall& fn);
    static void             SetSelectColor(const FnCall& fn);
    static void             SetSelected(const FnCall& fn);
};


class TextSnapshotCtorFunction : public CFunctionObject
{
public:
    TextSnapshotCtorFunction(ASStringContext *psc);

    static FunctionRef Register(GlobalContext* pgc);
};

}}} //SF::GFx::AS2

#endif  // SF_NO_FXPLAYER_AS_TEXTSNAPSHOT

#endif  // INC_GASTextSnapshot_H
