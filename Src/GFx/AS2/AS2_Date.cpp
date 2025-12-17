/**************************************************************************

Filename    :   AS2_Date.cpp
Content     :   SWF (Shockwave Flash) player library
Created     :   October 24, 2006
Authors     :   Andrew Reisse

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_ASString.h"
#include "GFx/AS2/AS2_Action.h"
#include "Kernel/SF_MsgFormat.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_NumberObject.h"
#include "GFx/AS2/AS2_Date.h"

// SF_NO_FXPLAYER_AS_DATE disables Date class.
#ifdef GFX_AS2_ENABLE_DATE

#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360)
#include <sys/timeb.h>
# if defined(SF_OS_WIN32)
#include <windows.h>
# endif
#elif defined(SF_OS_PS3)
#include <cell/rtc.h>
#elif defined(SF_OS_WII)
#include <revolution/os.h>
#elif defined(SF_OS_PSVITA)
#include <rtc.h>
#elif defined(SF_OS_3DS)
#include <nn/fnd.h>
#elif defined(SF_OS_WIIU)
#else
#include <sys/time.h>
#endif

namespace Scaleform { namespace GFx { namespace AS2 {

DateObject::DateObject(Environment* penv)
: Object(penv)
{
    commonInit(penv);
}

DateObject::DateObject(Environment* penv, SInt64 val)
: Object(penv)
{
    commonInit(penv);
    SetDate(val);
}

void DateObject::commonInit (Environment* penv)
{    
    Date        = 0; // ms +/- 1 Jan 1970
    Time        = 0; // ms within day
    Year        = 0;
    JDate       = 0; // days within year

    // local time
    LDate       = 0; // ms +/- 1 Jan 1970
    LTime       = 0; // ms within day
    LYear       = 0;
    LJDate      = 0; // days within year

    // time zone
    LocalOffset = 0;

    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_Date));
}

const char* DateObject::GetTextValue(Environment*) const
{
    return "Date";
}

//////////////////////////////////////////
//

static inline bool IsLeapYear(int y)
{
    return 0 == (y % 4) && ((y % 100) || 0 == (y % 400));
}

static inline int StartOfYear (int y)
{
    y -= 1970;
    return 365 * y + ((y+1) / 4) - ((y+69) / 100) + ((y+369) / 400);
}

void DateObject::UpdateLocal()
{
    LTime = Time + LocalOffset;
    LDate = Date + LocalOffset;
    LJDate = JDate;
    LYear = Year;
    if (LTime >= 86400000 || LTime < 0)
    {
        int dd = ((LTime + 864000000) / 86400000) - 10;
        LJDate += dd;
        LTime -= dd * 86400000;
        if (LJDate >= (IsLeapYear(LYear) ? 366 : 365))
        {
            LJDate -= IsLeapYear(LYear) ? 366 : 365;
            LYear++;
        }
        else if (LJDate < 0)
        {
            LYear--;
            LJDate += IsLeapYear(LYear) ? 366 : 365;
        }
    }
}

void DateObject::UpdateGMT()
{
    Time = LTime - LocalOffset;
    Date = LDate - LocalOffset;
    JDate = LJDate;
    Year = LYear;
    if (Time >= 86400000 || Time < 0)
    {
        int dd = ((Time + 864000000) / 86400000) - 10;
        JDate += dd;
        Time -= dd * 86400000;
        if (JDate >= (IsLeapYear(Year) ? 366 : 365))
        {
            JDate -= IsLeapYear(Year) ? 366 : 365;
            Year++;
        }
        else if (JDate < 0)
        {
            Year--;
            JDate += IsLeapYear(Year);
        }
    }
}

void DateObject::SetDate(SInt64 val)
{
    SInt64 days = val / 86400000;
    Time = SInt32(val % 86400000);
    SInt64 qcents = days / (400 * 365 + 97);
    SInt64 qcdays = days % (400 * 365 + 97);
    Year = 1970 + SInt32(qcents * 400);
    if (val >= 0)
    {
        while (qcdays >= (IsLeapYear(Year) ? 366 : 365))
        {
            qcdays -= IsLeapYear(Year) ? 366 : 365;
            Year++;
        }
    }
    else
    {
        while (Alg::Abs(qcdays) >= (IsLeapYear(Year) ? 366 : 365))
        {
            Year--;
            qcdays += IsLeapYear(Year) ? 366 : 365;
        }
    }
    JDate = int(qcdays);
    Date = val;
    UpdateLocal();
}

void DateProto::DateGetTimezoneOffset(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber( Number(-pThis->LocalOffset / 60000) );
}

static const int months[2][12] = {{31,59,90,120,151,181,212,243,273,304,334,365},
                                  {31,60,91,121,152,182,213,244,274,305,335,366}};

void DateProto::DateGetMonth(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    for (int i = 0; i < 12; i++)
        if (pThis->LJDate < months[IsLeapYear(pThis->LYear)][i])
        {
            fn.Result->SetNumber( Number(i) );
            return;
        }
    fn.Result->SetNumber(-1);
}

void DateProto::DateGetDate(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (pThis->LJDate < months[IsLeapYear(pThis->LYear)][0])
    {
        fn.Result->SetNumber( Number(1 + pThis->LJDate) );
        return;
    }

    for (int i = 1; i < 12; i++)
        if (pThis->LJDate < months[IsLeapYear(pThis->LYear)][i])
        {
            fn.Result->SetNumber( Number(1 + pThis->LJDate - months[IsLeapYear(pThis->LYear)][i-1]) );
            return;
        }
    fn.Result->SetNumber(-1);
}

void DateProto::DateGetDay(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber( Number((4 + int(pThis->LDate / 86400000)) % 7) );
}

void DateProto::DateGetTime(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber((Number)pThis->Date);
}

void DateProto::DateGetHours(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber( Number(pThis->LTime / 3600000) );
}

void DateProto::DateGetMinutes(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber( Number((pThis->LTime % 3600000) / 60000) );
}

void DateProto::DateGetSeconds(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber( Number((pThis->LTime % 60000) / 1000) );
}

void DateProto::DateGetMilliseconds(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber( Number(pThis->LTime % 1000) );
}

void DateProto::DateGetYear(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber( Number(pThis->LYear - 1900) );
}

void DateProto::DateGetFullYear(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber( Number(pThis->LYear) );
}

void DateProto::DateSetYear(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    if (newVal < 100 && newVal >= 0)
        newVal += 1900;
    if (pThis->LJDate >= 60)
        pThis->LJDate += IsLeapYear(newVal) - IsLeapYear(pThis->LYear);
    pThis->LDate = SInt64(pThis->LTime) + SInt64(StartOfYear(newVal) + pThis->LJDate) * SInt64(86400000);
    pThis->LYear = newVal;
    pThis->UpdateGMT();
}

void DateProto::DateSetFullYear(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    if (pThis->LJDate >= 60)
        pThis->LJDate += IsLeapYear(newVal) - IsLeapYear(pThis->LYear);
    pThis->LDate = SInt64(pThis->LTime) + SInt64(StartOfYear(newVal) + pThis->LJDate) * SInt64(86400000);
    pThis->LYear = newVal;
    pThis->UpdateGMT();
}

void DateProto::DateSetMonth(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    DateGetDate(fn);
    int d = (int) fn.Result->ToNumber(fn.Env); //current date
    int oldLJDate =  pThis->LJDate;
    int monthStart = 0;
    if (newVal > 0)
    {
        monthStart = months[IsLeapYear(pThis->LYear)][newVal - 1];
    }
    int daysInMonth = months[IsLeapYear(pThis->LYear)][newVal] - monthStart;
    if (d > daysInMonth)
        d = daysInMonth;
    pThis->LJDate = monthStart + d - 1;
    pThis->LDate += SInt64(pThis->LJDate - oldLJDate) * SInt64(86400000);

    fn.Result->SetUndefined();
}

void DateProto::DateSetDate(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    for (int i = 0; i < 12; i++)
        if (pThis->LJDate < months[IsLeapYear(pThis->LYear)][i])
        {
            int d = newVal - (pThis->LJDate - (i ? months[IsLeapYear(pThis->LYear)][i-1] : 0)) - 1;
            pThis->LJDate += d;
            pThis->LDate += SInt64(d) * SInt64(86400000);
            pThis->UpdateGMT();
            return;
        }
}

void DateProto::DateSetTime(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    pThis->SetDate((SInt64) fn.Arg(0).ToNumber(fn.Env));
}

void DateProto::DateSetHours(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    int d = (newVal - pThis->LTime / 3600000) * 3600000;
    pThis->LDate += d;
    pThis->LTime += d;
    pThis->UpdateGMT();
}

void DateProto::DateSetMinutes(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    int d = (newVal - ((pThis->LTime % 3600000) / 60000)) * 60000;
    pThis->LDate += d;
    pThis->LTime += d;
    pThis->UpdateGMT();
}

void DateProto::DateSetSeconds(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    int d = (newVal - ((pThis->LTime % 60000) / 1000)) * 1000;
    pThis->LDate += d;
    pThis->LTime += d;
    pThis->UpdateGMT();
}

void DateProto::DateSetMilliseconds(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    int d = newVal - (pThis->LTime % 1000);
    pThis->LDate += d;
    pThis->LTime += d;
    pThis->UpdateGMT();
}


void DateProto::DateGetUTCMonth(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    for (int i = 0; i < 12; i++)
        if (pThis->JDate < months[IsLeapYear(pThis->Year)][i])
        {
            fn.Result->SetNumber(Number(i));
            return;
        }
        fn.Result->SetNumber(-1);
}

void DateProto::DateGetUTCDate(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (pThis->JDate < months[IsLeapYear(pThis->Year)][0])
    {
        fn.Result->SetNumber(Number(1 + pThis->JDate));
        return;
    }

    for (int i = 1; i < 12; i++)
        if (pThis->JDate < months[IsLeapYear(pThis->Year)][i])
        {
            fn.Result->SetNumber(Number(1 + pThis->JDate - months[IsLeapYear(pThis->Year)][i-1]));
            return;
        }
        fn.Result->SetNumber(-1);
}

void DateProto::DateGetUTCDay(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber(Number((4 + int(pThis->Date / 86400000)) % 7));
}

void DateProto::DateGetUTCHours(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber(Number(pThis->Time / 3600000));
}

void DateProto::DateGetUTCMinutes(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber(Number((pThis->Time % 3600000) / 60000));
}

void DateProto::DateGetUTCSeconds(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber(Number((pThis->Time % 60000) / 1000));
}

void DateProto::DateGetUTCMilliseconds(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber(Number(pThis->Time % 1000));
}

void DateProto::DateGetUTCYear(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber(Number(pThis->Year - 1900));
}

void DateProto::DateGetUTCFullYear(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber(Number(pThis->Year));
}

void DateProto::DateSetUTCYear(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    if (newVal < 100 && newVal >= 0)
        newVal += 1900;
    if (pThis->JDate >= 60)
        pThis->JDate += IsLeapYear(newVal) - IsLeapYear(pThis->Year);
    pThis->Date = SInt64(pThis->Time) + SInt64(StartOfYear(newVal) + pThis->JDate) * SInt64(86400000);
    pThis->Year = newVal;
    pThis->UpdateLocal();
}

void DateProto::DateSetUTCFullYear(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    if (pThis->JDate >= 60)
        pThis->JDate += IsLeapYear(newVal) - IsLeapYear(pThis->Year);
    pThis->Date = SInt64(pThis->Time) + SInt64(StartOfYear(newVal) + pThis->JDate) * SInt64(86400000);
    pThis->Year = newVal;
    pThis->UpdateLocal();
}

void DateProto::DateSetUTCMonth(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    for (int i = 0; i < 12; i++)
        if (pThis->JDate < months[IsLeapYear(pThis->Year)][i])
        {
            int d = months[IsLeapYear(pThis->Year)][newVal] - months[IsLeapYear(pThis->Year)][i];
            pThis->JDate += d;
            pThis->Date += SInt64(d) * SInt64(86400000);
            pThis->UpdateLocal();
            return;
        }
}

void DateProto::DateSetUTCDate(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    for (int i = 0; i < 12; i++)
        if (pThis->JDate < months[IsLeapYear(pThis->Year)][i])
        {
            int d = newVal - (pThis->JDate - (i ? months[IsLeapYear(pThis->Year)][i-1] : 0)) - 1;
            pThis->JDate += d;
            pThis->Date += SInt64(d) * SInt64(86400000);
            pThis->UpdateLocal();
            return;
        }
}
void DateProto::DateSetUTCHours(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    int d = (newVal - pThis->Time / 3600000) * 3600000;
    pThis->Date += d;
    pThis->Time += d;
    pThis->UpdateLocal();
}

void DateProto::DateSetUTCMinutes(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    int d = (newVal - ((pThis->Time % 3600000) / 60000)) * 60000;
    pThis->Date += d;
    pThis->Time += d;
    pThis->UpdateLocal();
}

void DateProto::DateSetUTCSeconds(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    int d = (newVal - ((pThis->Time % 60000) / 1000)) * 1000;
    pThis->Date += d;
    pThis->Time += d;
    pThis->UpdateLocal();
}

void DateProto::DateSetUTCMilliseconds(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
        return;
    int newVal = (int) fn.Arg(0).ToNumber(fn.Env);
    int d = newVal - (pThis->Time % 1000);
    pThis->Date += d;
    pThis->Time += d;
    pThis->UpdateLocal();
}

static const char *daynames[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char *monthnames[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void DateProto::DateToString(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);
    int month=0, day=0;

    for (int i = 0; i < 12; i++)
        if (pThis->LJDate < months[IsLeapYear(pThis->LYear)][i])
        {
            month = i;
            day = 1 + pThis->LJDate - (i ? months[IsLeapYear(pThis->LYear)][i-1] : 0);
            break;
        }

    char out[128];
    // 01/01/1970 is Thursday
    // We add offset of 4 since the start day is Thursday
    const char* dayname = (pThis->LDate >= 0) ? daynames[(4 + pThis->LDate / 86400000) % 7] :
                                                daynames[(7 + (3 - Alg::Abs(pThis->LDate) / 86400000) % 7) %7];
    //SFsprintf(out, sizeof(out), 
    //    "%s %s %2d %02d:%02d:%02d GMT%+03d%02d %d", dayname, monthnames[month], day, 
    //    pThis->LTime / 3600000, (pThis->LTime % 3600000) / 60000, (pThis->LTime % 60000) / 1000,
    //    pThis->LocalOffset / 3600000, (pThis->LocalOffset % 3600000) / 60000, pThis->LYear);

    Format(StringDataPtr(out, sizeof(out)), 
        "{0} {1} {2:2} {3:02}:{4:02}:{5:02} GMT{6:+03}{7:02} {8}", 
        dayname, 
        monthnames[month], 
        day, 
        pThis->LTime / 3600000, 
        (pThis->LTime % 3600000) / 60000, 
        (pThis->LTime % 60000) / 1000,
        pThis->LocalOffset / 3600000, 
        (pThis->LocalOffset % 3600000) / 60000, 
        pThis->LYear
        );

    fn.Result->SetString(fn.Env->CreateString(out));
}

void DateProto::DateValueOf(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Date);
    DateObject* pThis = (DateObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetNumber((Number)pThis->Date);
}

static const NameFunction GAS_DateFunctionTable[] = 
{
    { "getDate",            &DateProto::DateGetDate             },
    { "getDay",             &DateProto::DateGetDay              },
    { "getFullYear",        &DateProto::DateGetFullYear         },
    { "getHours",           &DateProto::DateGetHours            },
    { "getMilliseconds",    &DateProto::DateGetMilliseconds     },
    { "getMinutes",         &DateProto::DateGetMinutes          },
    { "getMonth",           &DateProto::DateGetMonth            },
    { "getSeconds",         &DateProto::DateGetSeconds          },
    { "getTime",            &DateProto::DateGetTime             },
    { "getTimezoneOffset",  &DateProto::DateGetTimezoneOffset   },
    { "getYear",            &DateProto::DateGetYear             },
    { "getUTCDate",         &DateProto::DateGetUTCDate          },
    { "getUTCDay",          &DateProto::DateGetUTCDay           },
    { "getUTCFullYear",     &DateProto::DateGetUTCFullYear      },
    { "getUTCHours",        &DateProto::DateGetUTCHours         },
    { "getUTCMilliseconds", &DateProto::DateGetUTCMilliseconds  },
    { "getUTCMinutes",      &DateProto::DateGetUTCMinutes       },
    { "getUTCMonth",        &DateProto::DateGetUTCMonth         },
    { "getUTCSeconds",      &DateProto::DateGetUTCSeconds       },
    { "getUTCYear",         &DateProto::DateGetUTCYear          },

    { "setDate",            &DateProto::DateSetDate             },
    { "setFullYear",        &DateProto::DateSetFullYear         },
    { "setHours",           &DateProto::DateSetHours            },
    { "setMilliseconds",    &DateProto::DateSetMilliseconds     },
    { "setMinutes",         &DateProto::DateSetMinutes          },
    { "setMonth",           &DateProto::DateSetMonth            },
    { "setSeconds",         &DateProto::DateSetSeconds          },
    { "setTime",            &DateProto::DateSetTime             },
    { "setYear",            &DateProto::DateSetYear             },
    { "setUTCDate",         &DateProto::DateSetUTCDate          },
    { "setUTCFullYear",     &DateProto::DateSetUTCFullYear      },
    { "setUTCHours",        &DateProto::DateSetUTCHours         },
    { "setUTCMilliseconds", &DateProto::DateSetUTCMilliseconds  },
    { "setUTCMinutes",      &DateProto::DateSetUTCMinutes       },
    { "setUTCSeconds",      &DateProto::DateSetUTCSeconds       },
    { "setUTCMonth",        &DateProto::DateSetUTCMonth         },
    { "setUTCYear",         &DateProto::DateSetUTCYear          },

    { "toString",     &DateProto::DateToString     },
    { "valueOf",      &DateProto::DateValueOf      },
    { 0, 0 }
};

DateProto::DateProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor)
    : Prototype<DateObject>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, GAS_DateFunctionTable);
}

//////////////////
const NameFunction DateCtorFunction::StaticFunctionTable[] = 
{
    { "UTC", &DateCtorFunction::DateUTC },
    { 0, 0 }
};

DateCtorFunction::DateCtorFunction(ASStringContext *psc) 
: CFunctionObject(psc, GlobalCtor)
{
    NameFunction::AddConstMembers(
        this, psc, StaticFunctionTable, 
        PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
}

void DateCtorFunction::DateUTC (const FnCall& fn)
{
    if (fn.NArgs >= 2)
    {
        int       i    = (int)fn.Arg(0).ToNumber(fn.Env);
        int       y    = i > 99 || i < 0 ? i : 1900 + i;
        Number days = Number(StartOfYear(y));
        Number ms   = 0;

        i = (int)fn.Arg(1).ToNumber(fn.Env);
        if (i)
            days += months[IsLeapYear(y)][i-1];
        if (fn.NArgs >= 3)
            days += (int)fn.Arg(2).ToNumber(fn.Env)-1;

        if (fn.NArgs >= 4)
            ms += fn.Arg(3).ToNumber(fn.Env) * 3600000;
        if (fn.NArgs >= 5)
            ms += fn.Arg(4).ToNumber(fn.Env) * 60000;
        if (fn.NArgs >= 6)
            ms += fn.Arg(5).ToNumber(fn.Env) * 1000;
        if (fn.NArgs >= 7)
            ms += fn.Arg(6).ToNumber(fn.Env);

        fn.Result->SetNumber(ms + days * 86400000);
    }
    else
        fn.Result->SetNumber(0);
}

void DateCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<DateObject> pdateObj;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_Date)
        pdateObj = static_cast<DateObject*>(fn.ThisPtr);
    else
        pdateObj = *SF_HEAP_NEW(fn.Env->GetHeap()) DateObject(fn.Env);

#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360)
    struct timeb t;
    ftime(&t);
#ifdef _DURANGO
    int daylight = 0;
    _get_daylight(&daylight);
    //long timezone = 0;
    //_get_timezone(&timezone);
    //timezone /= 60;
    long bias = t.timezone - daylight * 60;
#else
    TIME_ZONE_INFORMATION tz;
    DWORD r = GetTimeZoneInformation(&tz);
    long bias = tz.Bias;
    if( r == TIME_ZONE_ID_STANDARD )
    { 
        bias += tz.StandardBias; 
    } 
    else if( r == TIME_ZONE_ID_DAYLIGHT)
    { 
        bias += tz.DaylightBias; 
    } 
#endif
    // else leave the bias alone
    pdateObj->LocalOffset = -60000 * bias;
    pdateObj->SetDate(SInt64(t.time) * 1000 + t.millitm);
#elif defined(SF_OS_WINCE)
    SYSTEMTIME stime;
    GetSystemTime(&stime);
    FILETIME ftime;
    SystemTimeToFileTime(&stime, &ftime);

    //!AB: FILETIME is a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601.
    // Need to convert it to milliseconds since midnight (00:00:00), January 1, 1970, coordinated universal time (UTC).
    // 11644473600000 is a number of milliseconds between Jan 1, 1601 and Jan 1, 1970.
    UInt64 mstime = ((UInt64(ftime.dwHighDateTime) << 32) | ftime.dwLowDateTime) / 10000 - 11644473600000i64;

    TIME_ZONE_INFORMATION tz;
    DWORD r = GetTimeZoneInformation(&tz);
    long bias = tz.Bias;
    if (r == TIME_ZONE_ID_STANDARD)
    { 
        bias += tz.StandardBias; 
    } 
    else if (r == TIME_ZONE_ID_DAYLIGHT)
    { 
        bias += tz.DaylightBias; 
    } 
    // else leave the bias alone
    pdateObj->LocalOffset = -60000 * bias;
    pdateObj->SetDate(mstime);

#elif defined(SF_OS_PS3)
    CellRtcTick t, localt;
    cellRtcGetCurrentTick(&t);
    cellRtcConvertUtcToLocalTime(&t, &localt);
    pdateObj->LocalOffset = ((localt.tick/1000)-(t.tick/1000));
    pdateObj->SetDate(SInt64(t.tick/1000 - 62135596800000ull));

#elif defined(SF_OS_WII) || defined(SF_OS_WIIU)
    SInt64 t = OSTicksToMilliseconds(OSGetTime());
    pdateObj->LocalOffset = 0;
    pdateObj->SetDate(t + 946684800000LL);

#elif defined(SF_OS_3DS)
    SInt64 t = Timer::GetTicks();
    pdateObj->LocalOffset = 0;
    pdateObj->SetDate(t + 946684800000LL);

#elif defined(SF_OS_PSVITA)
    SceRtcTick t, localt;
    sceRtcGetCurrentTick(&t);
    sceRtcConvertUtcToLocalTime(&t, &localt);
    pdateObj->LocalOffset = ((localt.tick/1000)-(t.tick/1000));
    pdateObj->SetDate(SInt64(t.tick/1000 - 62135596800000ull));

#else
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    long bias = tz.tz_minuteswest - (tz.tz_dsttime ? 60 : 0);
    pdateObj->LocalOffset = -60000 * bias;
    pdateObj->SetDate(SInt64(tv.tv_sec) * 1000 + SInt64(tv.tv_usec/1000));
#endif

    if (fn.NArgs == 1)
        pdateObj->SetDate((SInt64)fn.Arg(0).ToNumber(fn.Env));
    else if (fn.NArgs >= 2)
    {
        int    i = (int)fn.Arg(0).ToNumber(fn.Env);
        int    y = i > 99 || i < 0 ? i : 1900 + i;
        SInt32 days = StartOfYear(y), ms = 0;
        i = (int)fn.Arg(1).ToNumber(fn.Env);
        if (i)
            days += months[IsLeapYear(y)][i-1];
        if (fn.NArgs >= 3)
            days += (int)fn.Arg(2).ToNumber(fn.Env)-1;
        
        if (fn.NArgs >= 4)
            ms += (int)fn.Arg(3).ToNumber(fn.Env) * 3600000;
        if (fn.NArgs >= 5)
            ms += (int)fn.Arg(4).ToNumber(fn.Env) * 60000;
        if (fn.NArgs >= 6)
            ms += (int)fn.Arg(5).ToNumber(fn.Env) * 1000;
        if (fn.NArgs >= 7)
            ms += (int)fn.Arg(6).ToNumber(fn.Env);

        pdateObj->LJDate = days - StartOfYear(y);
        pdateObj->LYear = y;
        pdateObj->LTime = ms;
        pdateObj->LDate = SInt64(days) * SInt64(86400000) + ms;
        pdateObj->UpdateGMT();
        pdateObj->SetDate(pdateObj->Date);
    }
    fn.Result->SetAsObject(pdateObj);
    fn.Result->SetString(fn.Result->ToString(fn.Env));
}

Object* DateCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) DateObject(penv);
}

FunctionRef DateCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) DateCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) DateProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Date, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Date), Value(ctor));
    return ctor;
}

}}} //SF::GFx::AS2

#else

void GASDate_DummyFunction() {}   // Exists to quelch compiler warning

#endif // SF_NO_FXPLAYER_AS_DATE
