/**************************************************************************

Filename    :   AS2_Date.h
Content     :   SWF (Shockwave Flash) player library
Created     :   October 24, 2006
Authors     :   Andrew Reisse

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_DATE_H
#define INC_SF_GFX_DATE_H

#include "GFx/AS2/AS2_Action.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// SF_NO_FXPLAYER_AS_DATE disables Date class.
#ifdef GFX_AS2_ENABLE_DATE

// ***** Declared Classes
class DateObject;
class DateProto;
class DateCtorFunction;

// ***** External Classes
class ArrayObject;
class Environment;



class DateObject : public Object
{
protected:
    // GMT
    SInt64  Date; // ms +/- 1 Jan 1970
    int     Time; // ms within day
    int     Year;
    int     JDate; // days within year

    // local time
    SInt64  LDate; // ms +/- 1 Jan 1970
    int     LTime; // ms within day
    int     LYear;
    int     LJDate; // days within year

    // time zone
    int     LocalOffset;

    void commonInit (Environment* penv);

    void UpdateLocal();
    void UpdateGMT();

    friend class DateProto;
    friend class DateCtorFunction;

    DateObject (ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype);
    }    
public:
    
    //DateObject () : Object() { SF_UNUSED(psc); }
    DateObject (Environment* penv);
    DateObject (Environment* penv, SInt64 val);

    const char*         GetTextValue(Environment* =0) const;
    ObjectType          GetObjectType() const   { return Object_Date; }

    void                SetDate(SInt64 utc);
};

class DateProto : public Prototype<DateObject>
{
public:
    DateProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);

    static void DateGetDate(const FnCall& fn);
    static void DateGetDay(const FnCall& fn);
    static void DateGetFullYear(const FnCall& fn);
    static void DateGetHours(const FnCall& fn);
    static void DateGetMilliseconds(const FnCall& fn);
    static void DateGetMinutes(const FnCall& fn);
    static void DateGetMonth(const FnCall& fn);
    static void DateGetSeconds(const FnCall& fn);
    static void DateGetTime(const FnCall& fn);
    static void DateGetTimezoneOffset(const FnCall& fn);
    static void DateGetYear(const FnCall& fn);

    static void DateSetDate(const FnCall& fn);
    static void DateSetFullYear(const FnCall& fn);
    static void DateSetHours(const FnCall& fn);
    static void DateSetMilliseconds(const FnCall& fn);
    static void DateSetMinutes(const FnCall& fn);
    static void DateSetMonth(const FnCall& fn);
    static void DateSetSeconds(const FnCall& fn);
    static void DateSetTime(const FnCall& fn);
    static void DateSetYear(const FnCall& fn);

    static void DateGetUTCDate(const FnCall& fn);
    static void DateGetUTCDay(const FnCall& fn);
    static void DateGetUTCFullYear(const FnCall& fn);
    static void DateGetUTCHours(const FnCall& fn);
    static void DateGetUTCMilliseconds(const FnCall& fn);
    static void DateGetUTCMinutes(const FnCall& fn);
    static void DateGetUTCMonth(const FnCall& fn);
    static void DateGetUTCSeconds(const FnCall& fn);
    static void DateGetUTCYear(const FnCall& fn);

    static void DateSetUTCDate(const FnCall& fn);
    static void DateSetUTCFullYear(const FnCall& fn);
    static void DateSetUTCHours(const FnCall& fn);
    static void DateSetUTCMilliseconds(const FnCall& fn);
    static void DateSetUTCMinutes(const FnCall& fn);
    static void DateSetUTCMonth(const FnCall& fn);
    static void DateSetUTCSeconds(const FnCall& fn);
    static void DateSetUTCYear(const FnCall& fn);

    static void DateToString(const FnCall& fn);
    static void DateValueOf(const FnCall& fn);
};

class DateCtorFunction : public CFunctionObject
{
    static const NameFunction StaticFunctionTable[];

    static void DateUTC(const FnCall& fn);

public:
    DateCtorFunction(ASStringContext *psc);

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment* penv) const;        

    static FunctionRef Register(GlobalContext* pgc);
};


#endif // SF_NO_FXPLAYER_AS_DATE
}}} //SF::GFx::AS2

#endif // INC_SF_GFX_STRINGOBJECT_H
