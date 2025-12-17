//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Date.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Date.h"
#include "../AS3_VM.h"
#include "../AS3_Marshalling.h"
//##protect##"includes"
#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360)
#include <sys/timeb.h>
#if defined(SF_OS_WIN32)
#include <windows.h>
#endif
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

#ifdef _DURANGO
#include <time.h>
#endif

#include "Kernel/SF_MsgFormat.h"
#include "GFx/GFx_ASUtils.h"
#include "AS3_Obj_Namespace.h" // For namespace creation
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3valueOf, Value::Number> TFunc_Instances_Date_AS3valueOf;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_AS3setTime, Value::Number, Value::Number> TFunc_Instances_Date_AS3setTime;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3toString, ASString> TFunc_Instances_Date_AS3toString;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3toDateString, ASString> TFunc_Instances_Date_AS3toDateString;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3toTimeString, ASString> TFunc_Instances_Date_AS3toTimeString;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3toLocaleString, ASString> TFunc_Instances_Date_AS3toLocaleString;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3toLocaleDateString, ASString> TFunc_Instances_Date_AS3toLocaleDateString;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3toLocaleTimeString, ASString> TFunc_Instances_Date_AS3toLocaleTimeString;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3toUTCString, ASString> TFunc_Instances_Date_AS3toUTCString;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getUTCFullYear, Value::Number> TFunc_Instances_Date_AS3getUTCFullYear;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getUTCMonth, Value::Number> TFunc_Instances_Date_AS3getUTCMonth;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getUTCDate, Value::Number> TFunc_Instances_Date_AS3getUTCDate;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getUTCDay, Value::Number> TFunc_Instances_Date_AS3getUTCDay;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getUTCHours, Value::Number> TFunc_Instances_Date_AS3getUTCHours;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getUTCMinutes, Value::Number> TFunc_Instances_Date_AS3getUTCMinutes;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getUTCSeconds, Value::Number> TFunc_Instances_Date_AS3getUTCSeconds;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getUTCMilliseconds, Value::Number> TFunc_Instances_Date_AS3getUTCMilliseconds;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getFullYear, Value::Number> TFunc_Instances_Date_AS3getFullYear;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getMonth, Value::Number> TFunc_Instances_Date_AS3getMonth;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getDate, Value::Number> TFunc_Instances_Date_AS3getDate;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getDay, Value::Number> TFunc_Instances_Date_AS3getDay;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getHours, Value::Number> TFunc_Instances_Date_AS3getHours;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getMinutes, Value::Number> TFunc_Instances_Date_AS3getMinutes;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getSeconds, Value::Number> TFunc_Instances_Date_AS3getSeconds;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getMilliseconds, Value::Number> TFunc_Instances_Date_AS3getMilliseconds;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getTimezoneOffset, Value::Number> TFunc_Instances_Date_AS3getTimezoneOffset;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_AS3getTime, Value::Number> TFunc_Instances_Date_AS3getTime;
typedef ThunkFunc2<Instances::fl::Date, Instances::fl::Date::mid_AS3setFullYear, Value, unsigned, const Value*> TFunc_Instances_Date_AS3setFullYear;
typedef ThunkFunc2<Instances::fl::Date, Instances::fl::Date::mid_AS3setMonth, Value, unsigned, const Value*> TFunc_Instances_Date_AS3setMonth;
typedef ThunkFunc2<Instances::fl::Date, Instances::fl::Date::mid_AS3setDate, Value, unsigned, const Value*> TFunc_Instances_Date_AS3setDate;
typedef ThunkFunc2<Instances::fl::Date, Instances::fl::Date::mid_AS3setHours, Value, unsigned, const Value*> TFunc_Instances_Date_AS3setHours;
typedef ThunkFunc2<Instances::fl::Date, Instances::fl::Date::mid_AS3setMinutes, Value, unsigned, const Value*> TFunc_Instances_Date_AS3setMinutes;
typedef ThunkFunc2<Instances::fl::Date, Instances::fl::Date::mid_AS3setSeconds, Value, unsigned, const Value*> TFunc_Instances_Date_AS3setSeconds;
typedef ThunkFunc2<Instances::fl::Date, Instances::fl::Date::mid_AS3setMilliseconds, Value, unsigned, const Value*> TFunc_Instances_Date_AS3setMilliseconds;
typedef ThunkFunc2<Instances::fl::Date, Instances::fl::Date::mid_AS3setUTCFullYear, Value, unsigned, const Value*> TFunc_Instances_Date_AS3setUTCFullYear;
typedef ThunkFunc2<Instances::fl::Date, Instances::fl::Date::mid_AS3setUTCMonth, Value, unsigned, const Value*> TFunc_Instances_Date_AS3setUTCMonth;
typedef ThunkFunc2<Instances::fl::Date, Instances::fl::Date::mid_AS3setUTCDate, Value, unsigned, const Value*> TFunc_Instances_Date_AS3setUTCDate;
typedef ThunkFunc2<Instances::fl::Date, Instances::fl::Date::mid_AS3setUTCHours, Value, unsigned, const Value*> TFunc_Instances_Date_AS3setUTCHours;
typedef ThunkFunc2<Instances::fl::Date, Instances::fl::Date::mid_AS3setUTCMinutes, Value, unsigned, const Value*> TFunc_Instances_Date_AS3setUTCMinutes;
typedef ThunkFunc2<Instances::fl::Date, Instances::fl::Date::mid_AS3setUTCSeconds, Value, unsigned, const Value*> TFunc_Instances_Date_AS3setUTCSeconds;
typedef ThunkFunc2<Instances::fl::Date, Instances::fl::Date::mid_AS3setUTCMilliseconds, Value, unsigned, const Value*> TFunc_Instances_Date_AS3setUTCMilliseconds;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_fullYearGet, Value::Number> TFunc_Instances_Date_fullYearGet;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_fullYearSet, const Value, Value::Number> TFunc_Instances_Date_fullYearSet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_monthGet, Value::Number> TFunc_Instances_Date_monthGet;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_monthSet, const Value, Value::Number> TFunc_Instances_Date_monthSet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_dateGet, Value::Number> TFunc_Instances_Date_dateGet;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_dateSet, const Value, Value::Number> TFunc_Instances_Date_dateSet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_hoursGet, Value::Number> TFunc_Instances_Date_hoursGet;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_hoursSet, const Value, Value::Number> TFunc_Instances_Date_hoursSet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_minutesGet, Value::Number> TFunc_Instances_Date_minutesGet;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_minutesSet, const Value, Value::Number> TFunc_Instances_Date_minutesSet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_secondsGet, Value::Number> TFunc_Instances_Date_secondsGet;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_secondsSet, const Value, Value::Number> TFunc_Instances_Date_secondsSet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_millisecondsGet, Value::Number> TFunc_Instances_Date_millisecondsGet;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_millisecondsSet, const Value, Value::Number> TFunc_Instances_Date_millisecondsSet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_fullYearUTCGet, Value::Number> TFunc_Instances_Date_fullYearUTCGet;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_fullYearUTCSet, const Value, Value::Number> TFunc_Instances_Date_fullYearUTCSet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_monthUTCGet, Value::Number> TFunc_Instances_Date_monthUTCGet;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_monthUTCSet, const Value, Value::Number> TFunc_Instances_Date_monthUTCSet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_dateUTCGet, Value::Number> TFunc_Instances_Date_dateUTCGet;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_dateUTCSet, const Value, Value::Number> TFunc_Instances_Date_dateUTCSet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_hoursUTCGet, Value::Number> TFunc_Instances_Date_hoursUTCGet;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_hoursUTCSet, const Value, Value::Number> TFunc_Instances_Date_hoursUTCSet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_minutesUTCGet, Value::Number> TFunc_Instances_Date_minutesUTCGet;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_minutesUTCSet, const Value, Value::Number> TFunc_Instances_Date_minutesUTCSet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_secondsUTCGet, Value::Number> TFunc_Instances_Date_secondsUTCGet;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_secondsUTCSet, const Value, Value::Number> TFunc_Instances_Date_secondsUTCSet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_millisecondsUTCGet, Value::Number> TFunc_Instances_Date_millisecondsUTCGet;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_millisecondsUTCSet, const Value, Value::Number> TFunc_Instances_Date_millisecondsUTCSet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_timeGet, Value::Number> TFunc_Instances_Date_timeGet;
typedef ThunkFunc1<Instances::fl::Date, Instances::fl::Date::mid_timeSet, const Value, Value::Number> TFunc_Instances_Date_timeSet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_timezoneOffsetGet, Value::Number> TFunc_Instances_Date_timezoneOffsetGet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_dayGet, Value::Number> TFunc_Instances_Date_dayGet;
typedef ThunkFunc0<Instances::fl::Date, Instances::fl::Date::mid_dayUTCGet, Value::Number> TFunc_Instances_Date_dayUTCGet;

template <> const TFunc_Instances_Date_AS3valueOf::TMethod TFunc_Instances_Date_AS3valueOf::Method = &Instances::fl::Date::AS3valueOf;
template <> const TFunc_Instances_Date_AS3setTime::TMethod TFunc_Instances_Date_AS3setTime::Method = &Instances::fl::Date::AS3setTime;
template <> const TFunc_Instances_Date_AS3toString::TMethod TFunc_Instances_Date_AS3toString::Method = &Instances::fl::Date::AS3toString;
template <> const TFunc_Instances_Date_AS3toDateString::TMethod TFunc_Instances_Date_AS3toDateString::Method = &Instances::fl::Date::AS3toDateString;
template <> const TFunc_Instances_Date_AS3toTimeString::TMethod TFunc_Instances_Date_AS3toTimeString::Method = &Instances::fl::Date::AS3toTimeString;
template <> const TFunc_Instances_Date_AS3toLocaleString::TMethod TFunc_Instances_Date_AS3toLocaleString::Method = &Instances::fl::Date::AS3toLocaleString;
template <> const TFunc_Instances_Date_AS3toLocaleDateString::TMethod TFunc_Instances_Date_AS3toLocaleDateString::Method = &Instances::fl::Date::AS3toLocaleDateString;
template <> const TFunc_Instances_Date_AS3toLocaleTimeString::TMethod TFunc_Instances_Date_AS3toLocaleTimeString::Method = &Instances::fl::Date::AS3toLocaleTimeString;
template <> const TFunc_Instances_Date_AS3toUTCString::TMethod TFunc_Instances_Date_AS3toUTCString::Method = &Instances::fl::Date::AS3toUTCString;
template <> const TFunc_Instances_Date_AS3getUTCFullYear::TMethod TFunc_Instances_Date_AS3getUTCFullYear::Method = &Instances::fl::Date::AS3getUTCFullYear;
template <> const TFunc_Instances_Date_AS3getUTCMonth::TMethod TFunc_Instances_Date_AS3getUTCMonth::Method = &Instances::fl::Date::AS3getUTCMonth;
template <> const TFunc_Instances_Date_AS3getUTCDate::TMethod TFunc_Instances_Date_AS3getUTCDate::Method = &Instances::fl::Date::AS3getUTCDate;
template <> const TFunc_Instances_Date_AS3getUTCDay::TMethod TFunc_Instances_Date_AS3getUTCDay::Method = &Instances::fl::Date::AS3getUTCDay;
template <> const TFunc_Instances_Date_AS3getUTCHours::TMethod TFunc_Instances_Date_AS3getUTCHours::Method = &Instances::fl::Date::AS3getUTCHours;
template <> const TFunc_Instances_Date_AS3getUTCMinutes::TMethod TFunc_Instances_Date_AS3getUTCMinutes::Method = &Instances::fl::Date::AS3getUTCMinutes;
template <> const TFunc_Instances_Date_AS3getUTCSeconds::TMethod TFunc_Instances_Date_AS3getUTCSeconds::Method = &Instances::fl::Date::AS3getUTCSeconds;
template <> const TFunc_Instances_Date_AS3getUTCMilliseconds::TMethod TFunc_Instances_Date_AS3getUTCMilliseconds::Method = &Instances::fl::Date::AS3getUTCMilliseconds;
template <> const TFunc_Instances_Date_AS3getFullYear::TMethod TFunc_Instances_Date_AS3getFullYear::Method = &Instances::fl::Date::AS3getFullYear;
template <> const TFunc_Instances_Date_AS3getMonth::TMethod TFunc_Instances_Date_AS3getMonth::Method = &Instances::fl::Date::AS3getMonth;
template <> const TFunc_Instances_Date_AS3getDate::TMethod TFunc_Instances_Date_AS3getDate::Method = &Instances::fl::Date::AS3getDate;
template <> const TFunc_Instances_Date_AS3getDay::TMethod TFunc_Instances_Date_AS3getDay::Method = &Instances::fl::Date::AS3getDay;
template <> const TFunc_Instances_Date_AS3getHours::TMethod TFunc_Instances_Date_AS3getHours::Method = &Instances::fl::Date::AS3getHours;
template <> const TFunc_Instances_Date_AS3getMinutes::TMethod TFunc_Instances_Date_AS3getMinutes::Method = &Instances::fl::Date::AS3getMinutes;
template <> const TFunc_Instances_Date_AS3getSeconds::TMethod TFunc_Instances_Date_AS3getSeconds::Method = &Instances::fl::Date::AS3getSeconds;
template <> const TFunc_Instances_Date_AS3getMilliseconds::TMethod TFunc_Instances_Date_AS3getMilliseconds::Method = &Instances::fl::Date::AS3getMilliseconds;
template <> const TFunc_Instances_Date_AS3getTimezoneOffset::TMethod TFunc_Instances_Date_AS3getTimezoneOffset::Method = &Instances::fl::Date::AS3getTimezoneOffset;
template <> const TFunc_Instances_Date_AS3getTime::TMethod TFunc_Instances_Date_AS3getTime::Method = &Instances::fl::Date::AS3getTime;
template <> const TFunc_Instances_Date_AS3setFullYear::TMethod TFunc_Instances_Date_AS3setFullYear::Method = &Instances::fl::Date::AS3setFullYear;
template <> const TFunc_Instances_Date_AS3setMonth::TMethod TFunc_Instances_Date_AS3setMonth::Method = &Instances::fl::Date::AS3setMonth;
template <> const TFunc_Instances_Date_AS3setDate::TMethod TFunc_Instances_Date_AS3setDate::Method = &Instances::fl::Date::AS3setDate;
template <> const TFunc_Instances_Date_AS3setHours::TMethod TFunc_Instances_Date_AS3setHours::Method = &Instances::fl::Date::AS3setHours;
template <> const TFunc_Instances_Date_AS3setMinutes::TMethod TFunc_Instances_Date_AS3setMinutes::Method = &Instances::fl::Date::AS3setMinutes;
template <> const TFunc_Instances_Date_AS3setSeconds::TMethod TFunc_Instances_Date_AS3setSeconds::Method = &Instances::fl::Date::AS3setSeconds;
template <> const TFunc_Instances_Date_AS3setMilliseconds::TMethod TFunc_Instances_Date_AS3setMilliseconds::Method = &Instances::fl::Date::AS3setMilliseconds;
template <> const TFunc_Instances_Date_AS3setUTCFullYear::TMethod TFunc_Instances_Date_AS3setUTCFullYear::Method = &Instances::fl::Date::AS3setUTCFullYear;
template <> const TFunc_Instances_Date_AS3setUTCMonth::TMethod TFunc_Instances_Date_AS3setUTCMonth::Method = &Instances::fl::Date::AS3setUTCMonth;
template <> const TFunc_Instances_Date_AS3setUTCDate::TMethod TFunc_Instances_Date_AS3setUTCDate::Method = &Instances::fl::Date::AS3setUTCDate;
template <> const TFunc_Instances_Date_AS3setUTCHours::TMethod TFunc_Instances_Date_AS3setUTCHours::Method = &Instances::fl::Date::AS3setUTCHours;
template <> const TFunc_Instances_Date_AS3setUTCMinutes::TMethod TFunc_Instances_Date_AS3setUTCMinutes::Method = &Instances::fl::Date::AS3setUTCMinutes;
template <> const TFunc_Instances_Date_AS3setUTCSeconds::TMethod TFunc_Instances_Date_AS3setUTCSeconds::Method = &Instances::fl::Date::AS3setUTCSeconds;
template <> const TFunc_Instances_Date_AS3setUTCMilliseconds::TMethod TFunc_Instances_Date_AS3setUTCMilliseconds::Method = &Instances::fl::Date::AS3setUTCMilliseconds;
template <> const TFunc_Instances_Date_fullYearGet::TMethod TFunc_Instances_Date_fullYearGet::Method = &Instances::fl::Date::fullYearGet;
template <> const TFunc_Instances_Date_fullYearSet::TMethod TFunc_Instances_Date_fullYearSet::Method = &Instances::fl::Date::fullYearSet;
template <> const TFunc_Instances_Date_monthGet::TMethod TFunc_Instances_Date_monthGet::Method = &Instances::fl::Date::monthGet;
template <> const TFunc_Instances_Date_monthSet::TMethod TFunc_Instances_Date_monthSet::Method = &Instances::fl::Date::monthSet;
template <> const TFunc_Instances_Date_dateGet::TMethod TFunc_Instances_Date_dateGet::Method = &Instances::fl::Date::dateGet;
template <> const TFunc_Instances_Date_dateSet::TMethod TFunc_Instances_Date_dateSet::Method = &Instances::fl::Date::dateSet;
template <> const TFunc_Instances_Date_hoursGet::TMethod TFunc_Instances_Date_hoursGet::Method = &Instances::fl::Date::hoursGet;
template <> const TFunc_Instances_Date_hoursSet::TMethod TFunc_Instances_Date_hoursSet::Method = &Instances::fl::Date::hoursSet;
template <> const TFunc_Instances_Date_minutesGet::TMethod TFunc_Instances_Date_minutesGet::Method = &Instances::fl::Date::minutesGet;
template <> const TFunc_Instances_Date_minutesSet::TMethod TFunc_Instances_Date_minutesSet::Method = &Instances::fl::Date::minutesSet;
template <> const TFunc_Instances_Date_secondsGet::TMethod TFunc_Instances_Date_secondsGet::Method = &Instances::fl::Date::secondsGet;
template <> const TFunc_Instances_Date_secondsSet::TMethod TFunc_Instances_Date_secondsSet::Method = &Instances::fl::Date::secondsSet;
template <> const TFunc_Instances_Date_millisecondsGet::TMethod TFunc_Instances_Date_millisecondsGet::Method = &Instances::fl::Date::millisecondsGet;
template <> const TFunc_Instances_Date_millisecondsSet::TMethod TFunc_Instances_Date_millisecondsSet::Method = &Instances::fl::Date::millisecondsSet;
template <> const TFunc_Instances_Date_fullYearUTCGet::TMethod TFunc_Instances_Date_fullYearUTCGet::Method = &Instances::fl::Date::fullYearUTCGet;
template <> const TFunc_Instances_Date_fullYearUTCSet::TMethod TFunc_Instances_Date_fullYearUTCSet::Method = &Instances::fl::Date::fullYearUTCSet;
template <> const TFunc_Instances_Date_monthUTCGet::TMethod TFunc_Instances_Date_monthUTCGet::Method = &Instances::fl::Date::monthUTCGet;
template <> const TFunc_Instances_Date_monthUTCSet::TMethod TFunc_Instances_Date_monthUTCSet::Method = &Instances::fl::Date::monthUTCSet;
template <> const TFunc_Instances_Date_dateUTCGet::TMethod TFunc_Instances_Date_dateUTCGet::Method = &Instances::fl::Date::dateUTCGet;
template <> const TFunc_Instances_Date_dateUTCSet::TMethod TFunc_Instances_Date_dateUTCSet::Method = &Instances::fl::Date::dateUTCSet;
template <> const TFunc_Instances_Date_hoursUTCGet::TMethod TFunc_Instances_Date_hoursUTCGet::Method = &Instances::fl::Date::hoursUTCGet;
template <> const TFunc_Instances_Date_hoursUTCSet::TMethod TFunc_Instances_Date_hoursUTCSet::Method = &Instances::fl::Date::hoursUTCSet;
template <> const TFunc_Instances_Date_minutesUTCGet::TMethod TFunc_Instances_Date_minutesUTCGet::Method = &Instances::fl::Date::minutesUTCGet;
template <> const TFunc_Instances_Date_minutesUTCSet::TMethod TFunc_Instances_Date_minutesUTCSet::Method = &Instances::fl::Date::minutesUTCSet;
template <> const TFunc_Instances_Date_secondsUTCGet::TMethod TFunc_Instances_Date_secondsUTCGet::Method = &Instances::fl::Date::secondsUTCGet;
template <> const TFunc_Instances_Date_secondsUTCSet::TMethod TFunc_Instances_Date_secondsUTCSet::Method = &Instances::fl::Date::secondsUTCSet;
template <> const TFunc_Instances_Date_millisecondsUTCGet::TMethod TFunc_Instances_Date_millisecondsUTCGet::Method = &Instances::fl::Date::millisecondsUTCGet;
template <> const TFunc_Instances_Date_millisecondsUTCSet::TMethod TFunc_Instances_Date_millisecondsUTCSet::Method = &Instances::fl::Date::millisecondsUTCSet;
template <> const TFunc_Instances_Date_timeGet::TMethod TFunc_Instances_Date_timeGet::Method = &Instances::fl::Date::timeGet;
template <> const TFunc_Instances_Date_timeSet::TMethod TFunc_Instances_Date_timeSet::Method = &Instances::fl::Date::timeSet;
template <> const TFunc_Instances_Date_timezoneOffsetGet::TMethod TFunc_Instances_Date_timezoneOffsetGet::Method = &Instances::fl::Date::timezoneOffsetGet;
template <> const TFunc_Instances_Date_dayGet::TMethod TFunc_Instances_Date_dayGet::Method = &Instances::fl::Date::dayGet;
template <> const TFunc_Instances_Date_dayUTCGet::TMethod TFunc_Instances_Date_dayUTCGet::Method = &Instances::fl::Date::dayUTCGet;

namespace Instances { namespace fl
{
    Date::Date(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::Date::Date()$data"
    , LocalTZA(0)
    , UseDST(false)
    , TimeValue(0)
//##protect##"instance::Date::Date()$data"
    {
//##protect##"instance::Date::Date()$code"
//##protect##"instance::Date::Date()$code"
    }

    void Date::AS3valueOf(Value::Number& result)
    {
//##protect##"instance::Date::AS3valueOf()"
        result = TimeValue;
//##protect##"instance::Date::AS3valueOf()"
    }
    void Date::AS3setTime(Value::Number& result, Value::Number millisecond)
    {
//##protect##"instance::Date::AS3setTime()"
        // ECMA TBD: If 'this' value is not Date object, throw type exception.
        TimeValue = TimeClip(millisecond);
        UseDST    = false;
        result    = TimeValue;
//##protect##"instance::Date::AS3setTime()"
    }
    void Date::AS3toString(ASString& result)
    {
//##protect##"instance::Date::AS3toString()"
        StringManager& sm = GetStringManager();

        // In case this is a prototype object.
        if (this == &GetTraits().GetClass().GetPrototype())
            result = sm.CreateConstString("Invalid Date");
        else
        {
            char  out[128];
            UPInt size = formatDateTimeString(out, 128, TimeValue, GetLocalTZA(), true, true);
            result = sm.CreateString(out, size);
        }
//##protect##"instance::Date::AS3toString()"
    }
    void Date::AS3toDateString(ASString& result)
    {
//##protect##"instance::Date::AS3toDateString()"
        char  out[128];
        UPInt size = formatDateTimeString(out, 128, TimeValue, GetLocalTZA(), true, false);
        result = GetStringManager().CreateString(out, size);
//##protect##"instance::Date::AS3toDateString()"
    }
    void Date::AS3toTimeString(ASString& result)
    {
//##protect##"instance::Date::AS3toTimeString()"
        char  out[128];
        UPInt size = formatDateTimeString(out, 128, TimeValue, GetLocalTZA(), false, true);
        result = GetStringManager().CreateString(out, size);
//##protect##"instance::Date::AS3toTimeString()"
    }
    void Date::AS3toLocaleString(ASString& result)
    {
//##protect##"instance::Date::AS3toLocaleString()"
        AS3toString(result);
//##protect##"instance::Date::AS3toLocaleString()"
    }
    void Date::AS3toLocaleDateString(ASString& result)
    {
//##protect##"instance::Date::AS3toLocaleDateString()"
        AS3toDateString(result);
//##protect##"instance::Date::AS3toLocaleDateString()"
    }
    void Date::AS3toLocaleTimeString(ASString& result)
    {
//##protect##"instance::Date::AS3toLocaleTimeString()"
        AS3toTimeString(result);
//##protect##"instance::Date::AS3toLocaleTimeString()"
    }
    void Date::AS3toUTCString(ASString& result)
    {
//##protect##"instance::Date::AS3toUTCString()"
        char  out[128];
        UPInt size = formatDateTimeString(out, 128, TimeValue, 0, true, true, true); // UTC
        result = GetStringManager().CreateString(out, size);
//##protect##"instance::Date::AS3toUTCString()"
    }
    void Date::AS3getUTCFullYear(Value::Number& result)
    {
//##protect##"instance::Date::AS3getUTCFullYear()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : YearFromTime(TimeValue);
//##protect##"instance::Date::AS3getUTCFullYear()"
    }
    void Date::AS3getUTCMonth(Value::Number& result)
    {
//##protect##"instance::Date::AS3getUTCMonth()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : Value::Number(MonthFromTime(TimeValue));
//##protect##"instance::Date::AS3getUTCMonth()"
    }
    void Date::AS3getUTCDate(Value::Number& result)
    {
//##protect##"instance::Date::AS3getUTCDate()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : Value::Number(DateFromTime(TimeValue));
//##protect##"instance::Date::AS3getUTCDate()"
    }
    void Date::AS3getUTCDay(Value::Number& result)
    {
//##protect##"instance::Date::AS3getUTCDay()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : Value::Number(WeekDay(TimeValue));
//##protect##"instance::Date::AS3getUTCDay()"
    }
    void Date::AS3getUTCHours(Value::Number& result)
    {
//##protect##"instance::Date::AS3getUTCHours()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : HourFromTime(TimeValue);
//##protect##"instance::Date::AS3getUTCHours()"
    }
    void Date::AS3getUTCMinutes(Value::Number& result)
    {
//##protect##"instance::Date::AS3getUTCMinutes()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : MinFromTime(TimeValue);
//##protect##"instance::Date::AS3getUTCMinutes()"
    }
    void Date::AS3getUTCSeconds(Value::Number& result)
    {
//##protect##"instance::Date::AS3getUTCSeconds()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : SecFromTime(TimeValue);
//##protect##"instance::Date::AS3getUTCSeconds()"
    }
    void Date::AS3getUTCMilliseconds(Value::Number& result)
    {
//##protect##"instance::Date::AS3getUTCMilliseconds()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : MsFromTime(TimeValue);
//##protect##"instance::Date::AS3getUTCMilliseconds()"
    }
    void Date::AS3getFullYear(Value::Number& result)
    {
//##protect##"instance::Date::AS3getFullYear()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : YearFromTime(LocalTime());
//##protect##"instance::Date::AS3getFullYear()"
    }
    void Date::AS3getMonth(Value::Number& result)
    {
//##protect##"instance::Date::AS3getMonth()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : Value::Number(MonthFromTime(LocalTime()));
//##protect##"instance::Date::AS3getMonth()"
    }
    void Date::AS3getDate(Value::Number& result)
    {
//##protect##"instance::Date::AS3getDate()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : Value::Number(DateFromTime(LocalTime()));
//##protect##"instance::Date::AS3getDate()"
    }
    void Date::AS3getDay(Value::Number& result)
    {
//##protect##"instance::Date::AS3getDay()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : (Double)WeekDay(LocalTime());
//##protect##"instance::Date::AS3getDay()"
    }
    void Date::AS3getHours(Value::Number& result)
    {
//##protect##"instance::Date::AS3getHours()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : HourFromTime(LocalTime());
//##protect##"instance::Date::AS3getHours()"
    }
    void Date::AS3getMinutes(Value::Number& result)
    {
//##protect##"instance::Date::AS3getMinutes()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : MinFromTime(LocalTime());
//##protect##"instance::Date::AS3getMinutes()"
    }
    void Date::AS3getSeconds(Value::Number& result)
    {
//##protect##"instance::Date::AS3getSeconds()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : SecFromTime(LocalTime());
//##protect##"instance::Date::AS3getSeconds()"
    }
    void Date::AS3getMilliseconds(Value::Number& result)
    {
//##protect##"instance::Date::AS3getMilliseconds()"
        result = NumberUtil::IsNaN(TimeValue) ? TimeValue : MsFromTime(LocalTime());
//##protect##"instance::Date::AS3getMilliseconds()"
    }
    void Date::AS3getTimezoneOffset(Value::Number& result)
    {
//##protect##"instance::Date::AS3getTimezoneOffset()"
        if (NumberUtil::IsNaN(TimeValue))        
            result = TimeValue;        
        else
            result = (TimeValue - LocalTime()) / MsPerMinute;             
//##protect##"instance::Date::AS3getTimezoneOffset()"
    }
    void Date::AS3getTime(Value::Number& result)
    {
//##protect##"instance::Date::AS3getTime()"
        // ECMA TBD: If the 'this' value is not an object whose [[Class]] property
        // is "Date", throw TypeError.
        result = TimeValue;
//##protect##"instance::Date::AS3getTime()"
    }
    void Date::AS3setFullYear(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Date::AS3setFullYear()"
        applyDateArgs(result, argc, argv, Date_Year, static_cast<Double>(GetLocalTZA()));
//##protect##"instance::Date::AS3setFullYear()"
    }
    void Date::AS3setMonth(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Date::AS3setMonth()"
        applyDateArgs(result, argc, argv, Date_Month, static_cast<Double>(GetLocalTZA()));
//##protect##"instance::Date::AS3setMonth()"
    }
    void Date::AS3setDate(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Date::AS3setDate()"
        applyDateArgs(result, argc, argv, Date_Day, static_cast<Double>(GetLocalTZA()));
//##protect##"instance::Date::AS3setDate()"
    }
    void Date::AS3setHours(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Date::AS3setHours()"
        applyTimeArgs(result, argc, argv, Time_Hour, static_cast<Double>(GetLocalTZA()));
//##protect##"instance::Date::AS3setHours()"
    }
    void Date::AS3setMinutes(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Date::AS3setMinutes()"
        applyTimeArgs(result, argc, argv, Time_Minute, static_cast<Double>(GetLocalTZA()));
//##protect##"instance::Date::AS3setMinutes()"
    }
    void Date::AS3setSeconds(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Date::AS3setSeconds()"
        applyTimeArgs(result, argc, argv, Time_Second, static_cast<Double>(GetLocalTZA()));
//##protect##"instance::Date::AS3setSeconds()"
    }
    void Date::AS3setMilliseconds(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Date::AS3setMilliseconds()"
        applyTimeArgs(result, argc, argv, Time_Ms, static_cast<Double>(GetLocalTZA()));
//##protect##"instance::Date::AS3setMilliseconds()"
    }
    void Date::AS3setUTCFullYear(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Date::AS3setUTCFullYear()"
        applyDateArgs(result, argc, argv, Date_Year, 0.0);        
//##protect##"instance::Date::AS3setUTCFullYear()"
    }
    void Date::AS3setUTCMonth(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Date::AS3setUTCMonth()"
        applyDateArgs(result, argc, argv, Date_Month, 0.0);        
//##protect##"instance::Date::AS3setUTCMonth()"
    }
    void Date::AS3setUTCDate(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Date::AS3setUTCDate()"
        applyDateArgs(result, argc, argv, Date_Day, 0.0);        
//##protect##"instance::Date::AS3setUTCDate()"
    }
    void Date::AS3setUTCHours(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Date::AS3setUTCHours()"
        applyTimeArgs(result, argc, argv, Time_Hour, 0.0);
//##protect##"instance::Date::AS3setUTCHours()"
    }
    void Date::AS3setUTCMinutes(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Date::AS3setUTCMinutes()"
        applyTimeArgs(result, argc, argv, Time_Minute, 0.0);
//##protect##"instance::Date::AS3setUTCMinutes()"
    }
    void Date::AS3setUTCSeconds(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Date::AS3setUTCSeconds()"
        applyTimeArgs(result, argc, argv, Time_Second, 0.0);
//##protect##"instance::Date::AS3setUTCSeconds()"
    }
    void Date::AS3setUTCMilliseconds(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Date::AS3setUTCMilliseconds()"
        applyTimeArgs(result, argc, argv, Time_Ms, 0.0);
//##protect##"instance::Date::AS3setUTCMilliseconds()"
    }
    void Date::fullYearGet(Value::Number& result)
    {
//##protect##"instance::Date::fullYearGet()"
        AS3getFullYear(result);
//##protect##"instance::Date::fullYearGet()"
    }
    void Date::fullYearSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Date::fullYearSet()"
        SF_UNUSED(result);
        applyDateArg(Date_Year, value, static_cast<Double>(GetLocalTZA()));        
//##protect##"instance::Date::fullYearSet()"
    }
    void Date::monthGet(Value::Number& result)
    {
//##protect##"instance::Date::monthGet()"
        AS3getMonth(result);
//##protect##"instance::Date::monthGet()"
    }
    void Date::monthSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Date::monthSet()"
        SF_UNUSED(result);
        applyDateArg(Date_Month, value, static_cast<Double>(GetLocalTZA()));        
//##protect##"instance::Date::monthSet()"
    }
    void Date::dateGet(Value::Number& result)
    {
//##protect##"instance::Date::dateGet()"
        AS3getDate(result);
//##protect##"instance::Date::dateGet()"
    }
    void Date::dateSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Date::dateSet()"
        SF_UNUSED(result);
        applyDateArg(Date_Day, value, static_cast<Double>(GetLocalTZA()));
//##protect##"instance::Date::dateSet()"
    }
    void Date::hoursGet(Value::Number& result)
    {
//##protect##"instance::Date::hoursGet()"
        AS3getHours(result);
//##protect##"instance::Date::hoursGet()"
    }
    void Date::hoursSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Date::hoursSet()"
        SF_UNUSED(result);
        applyTimeArg(Time_Hour, value, static_cast<Double>(GetLocalTZA()));
//##protect##"instance::Date::hoursSet()"
    }
    void Date::minutesGet(Value::Number& result)
    {
//##protect##"instance::Date::minutesGet()"
        AS3getMinutes(result);
//##protect##"instance::Date::minutesGet()"
    }
    void Date::minutesSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Date::minutesSet()"
        SF_UNUSED(result);
        applyTimeArg(Time_Minute, value, static_cast<Double>(GetLocalTZA()));
//##protect##"instance::Date::minutesSet()"
    }
    void Date::secondsGet(Value::Number& result)
    {
//##protect##"instance::Date::secondsGet()"
        AS3getSeconds(result);
//##protect##"instance::Date::secondsGet()"
    }
    void Date::secondsSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Date::secondsSet()"
        SF_UNUSED(result);
        applyTimeArg(Time_Second, value, static_cast<Double>(GetLocalTZA()));
//##protect##"instance::Date::secondsSet()"
    }
    void Date::millisecondsGet(Value::Number& result)
    {
//##protect##"instance::Date::millisecondsGet()"
        AS3getMilliseconds(result);
//##protect##"instance::Date::millisecondsGet()"
    }
    void Date::millisecondsSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Date::millisecondsSet()"
        SF_UNUSED(result);
        applyTimeArg(Time_Ms, value, static_cast<Double>(GetLocalTZA()));
//##protect##"instance::Date::millisecondsSet()"
    }
    void Date::fullYearUTCGet(Value::Number& result)
    {
//##protect##"instance::Date::fullYearUTCGet()"
        AS3getUTCFullYear(result);
//##protect##"instance::Date::fullYearUTCGet()"
    }
    void Date::fullYearUTCSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Date::fullYearUTCSet()"
        SF_UNUSED(result);
        applyDateArg(Date_Year, value, 0.0);
//##protect##"instance::Date::fullYearUTCSet()"
    }
    void Date::monthUTCGet(Value::Number& result)
    {
//##protect##"instance::Date::monthUTCGet()"
        AS3getUTCMonth(result);
//##protect##"instance::Date::monthUTCGet()"
    }
    void Date::monthUTCSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Date::monthUTCSet()"
        SF_UNUSED(result);
        applyDateArg(Date_Month, value, 0.0);
//##protect##"instance::Date::monthUTCSet()"
    }
    void Date::dateUTCGet(Value::Number& result)
    {
//##protect##"instance::Date::dateUTCGet()"
        AS3getUTCDate(result);
//##protect##"instance::Date::dateUTCGet()"
    }
    void Date::dateUTCSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Date::dateUTCSet()"
        SF_UNUSED(result);
        applyDateArg(Date_Day, value, 0.0);
//##protect##"instance::Date::dateUTCSet()"
    }
    void Date::hoursUTCGet(Value::Number& result)
    {
//##protect##"instance::Date::hoursUTCGet()"
        AS3getUTCHours(result);
//##protect##"instance::Date::hoursUTCGet()"
    }
    void Date::hoursUTCSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Date::hoursUTCSet()"
        SF_UNUSED(result);
        applyTimeArg(Time_Hour, value, 0.0);
//##protect##"instance::Date::hoursUTCSet()"
    }
    void Date::minutesUTCGet(Value::Number& result)
    {
//##protect##"instance::Date::minutesUTCGet()"
        AS3getUTCMinutes(result);
//##protect##"instance::Date::minutesUTCGet()"
    }
    void Date::minutesUTCSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Date::minutesUTCSet()"
        SF_UNUSED(result);
        applyTimeArg(Time_Minute, value, 0.0);
//##protect##"instance::Date::minutesUTCSet()"
    }
    void Date::secondsUTCGet(Value::Number& result)
    {
//##protect##"instance::Date::secondsUTCGet()"
        AS3getUTCSeconds(result);
//##protect##"instance::Date::secondsUTCGet()"
    }
    void Date::secondsUTCSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Date::secondsUTCSet()"
        SF_UNUSED(result);
        applyTimeArg(Time_Second, value, 0.0);
//##protect##"instance::Date::secondsUTCSet()"
    }
    void Date::millisecondsUTCGet(Value::Number& result)
    {
//##protect##"instance::Date::millisecondsUTCGet()"
        AS3getUTCMilliseconds(result);
//##protect##"instance::Date::millisecondsUTCGet()"
    }
    void Date::millisecondsUTCSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Date::millisecondsUTCSet()"
        SF_UNUSED(result);
        applyTimeArg(Time_Ms, value, 0.0);
//##protect##"instance::Date::millisecondsUTCSet()"
    }
    void Date::timeGet(Value::Number& result)
    {
//##protect##"instance::Date::timeGet()"
        AS3getTime(result);
//##protect##"instance::Date::timeGet()"
    }
    void Date::timeSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Date::timeSet()"
        SF_UNUSED1(result);
        Value::Number r;
        AS3setTime(r, value);
//##protect##"instance::Date::timeSet()"
    }
    void Date::timezoneOffsetGet(Value::Number& result)
    {
//##protect##"instance::Date::timezoneOffsetGet()"
        AS3getTimezoneOffset(result);
//##protect##"instance::Date::timezoneOffsetGet()"
    }
    void Date::dayGet(Value::Number& result)
    {
//##protect##"instance::Date::dayGet()"
        AS3getDay(result);
//##protect##"instance::Date::dayGet()"
    }
    void Date::dayUTCGet(Value::Number& result)
    {
//##protect##"instance::Date::dayUTCGet()"
        AS3getUTCDay(result);
//##protect##"instance::Date::dayUTCGet()"
    }

//##protect##"instance$methods"
    const char *Date::DayNames[7] =
    {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char *Date::MonthNames[12] =
    {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    const int   Date::Months[2][12] =
    {
        {31,59,90,120,151,181,212,243,273,304,334,365},
        {31,60,91,121,152,182,213,244,274,305,335,366}
    };

    int Date::WeekDay(Double time)
    {
        int val = (int)fmod((DayOfTime(time) + 4), 7.0);
        if (val < 0)
            val += 7;
        return val;
    }

    bool Date::IsLeapYear(int y)
    {
        return (0 == (y % 4)) && ((y % 100) || (0 == (y % 400)));
    }

    Double Date::DayFromYear(Double year)
    {
        return 365 * (year - 1970) +
               floor((year - 1969)/4) -
               floor((year - 1901)/100) +
               floor((year - 1601)/400);
    }

    Double Date::YearFromTime(Double time)
    {
        int high = 1970 + (int) floor(time / (MsPerDay * 365.0));
        int low  = 1970 + (int) floor(time / (MsPerDay * 366.0));        
        
        // Check if the date is negative (before 1970).
        if (high < low)
            Alg::Swap(high, low);
        
        // Binary search for the "largest integer y such that TimeFromYear(y) <= t".
        while (high > low)
        {
            int middle = (high + low) / 2;

            if (TimeFromYear(middle) > time)
                high = middle - 1;
            else
            {
                low = middle + 1;
                if (TimeFromYear(low) > time)
                {
                    low--;
                    break;
                }
            }
        }

        SF_ASSERT(TimeFromYear(low) <= time);
        return low;
    }

    int Date::MonthFromTime(Double time)
    {
        Double year = YearFromTime(time);
        int    dayWithinYear = (int)(DayOfTime(time) - DayFromYear(year));
        return MonthFromYearDay((int)year, dayWithinYear);
    }

     int Date::MonthFromYearDay(int year, int dayWithinYear)
     {
         int leapYear = IsLeapYear(year) ? 1 : 0;
         int month;
         for (month = 0; month < 12; month++)
             if (Months[leapYear][month] > dayWithinYear)
                 break;
         return month;
     }

    int Date::DateFromTime(Double time)
    {
        Double year = YearFromTime(time);
        int    dayWithinYear = (int)(DayOfTime(time) - DayFromYear(year));
        int    leapYear = IsLeapYear((int)year) ? 1 : 0;
        int    month = MonthFromYearDay((int)year, dayWithinYear);

        dayWithinYear++; // 1 is the first day of the month.
        if (month > 0)
            dayWithinYear -= Months[leapYear][month-1];
        return dayWithinYear;
    }

    // ECMA 15.9.1.11 requires ToInteger() calls in MakeTime and MakeDay.
    // Simple casting to int in MakeTime fails on big numbers
    // MakeDay should be OK (?) with simple casting
    static SF_INLINE double toInt(double x)
    {
        return x > 0 ? floor(x) : -floor(-x); 
    }

    Double Date::MakeTime(Double hour, Double minute, Double sec, Double ms)
    {
        if (NumberUtil::IsNaNOrInfinity(hour + minute + sec + ms))
            return NumberUtil::NaN();
        Double time = toInt(hour) * MsPerHour + toInt(minute) * MsPerMinute +
                      toInt(sec) * MsPerSecond + toInt(ms);   
        return time;
    }

    Double Date::MakeDay(Double year, Double month, Double date)
    {
        if (NumberUtil::IsNaNOrInfinity(year + month + date))
            return NumberUtil::NaN();
        year  = (Double)(int)year;
        month = (Double)(int)month;
        date  = (Double)(int)date;

        year += floor(month / 12); // ECMA has no -1 ??!
        month = fmod(month, 12);
        if (month < 0)
            month += 12;

        Double daysInYear  = floor(DayFromYear(year));
        int    leapYear    = IsLeapYear((int)year) ? 1 : 0;
        int    monthIndex = (int)month;
        SF_ASSERT(unsigned(monthIndex) < 13);
        int    daysinMonth = (monthIndex == 0) ? 0 : Months[leapYear][monthIndex-1];

        return daysInYear + daysinMonth + date - 1;
    }

    Double Date::TimeClip(Double time)
    {
        if (NumberUtil::IsNaNOrInfinity(time) || (fabs(time) > 8.64e15))
            return NumberUtil::NaN();
        return (time > 0.0) ? floor(time) : ceil(time);
    }

    Double Date::HourFromTime(Double time)
    {
        Double val = fmod(floor(time / MsPerHour), 24);
        if (val < 0)
            val += HoursPerDay;
        return val;
    }
    Double Date::MinFromTime(Double time)
    {
        Double val = fmod(floor(time / MsPerMinute), MinutesPerHour);
        if (val < 0)
            val += MinutesPerHour;
        return val;
    }
    Double Date::SecFromTime(Double time)
    {
        Double val = fmod(floor(time / MsPerSecond), SecondsPerMinute);
        if (val < 0)
            val += SecondsPerMinute;
        return val;
    }
    Double Date::MsFromTime(Double time)
    {
        Double val = fmod(time, MsPerSecond);
        if (val < 0)
            val += MsPerSecond;
        return val;
    }

    Double Date::decodeUTCArgs(unsigned argc, const Value* const argv, Double tza)
    {
        // Preserve floating-point here to preserve NaN, which creates "Invalid Date".
        Double  yearArg;
        if (!argv[0].Convert2Number(yearArg)) return 0;

        Double  year   = (int(yearArg) > 99 || (int)yearArg < 0) ? yearArg : Double(1900.0) + yearArg;
        Double  month;
        if (!argv[1].Convert2Number(month)) return 0;

        Double  date   = 1.0;
        Double  hours  = 0.0, minutes = 0.0, seconds = 0.0, ms = 0.0;

        if (argc >= 3)
            if (!argv[2].Convert2Number(date)) return 0;

        if (argc >= 4)
        {
            if (!argv[3].Convert2Number(hours)) return 0;
            hours = (int)hours;
        }
        if (argc >= 5)
        {
            if (!argv[4].Convert2Number(minutes)) return 0;
            minutes = (int)minutes;
        }
        if (argc >= 6)
        {
            if (!argv[5].Convert2Number(seconds)) return 0;
            seconds = (int)seconds;
        }
        if (argc >= 7)
        {
            if (!argv[6].Convert2Number(ms)) return 0;
            ms = (int)ms;
        }
        
        return TimeClip(MakeDate(MakeDay(year, month, date),
                                 MakeTime(hours, minutes, seconds, ms))-tza);
    }

    // *** TimeHolder/DateHolder helper classes

    Date::TimeHolder::TimeHolder(Double time, Double tza) : TZA(tza)
    {
        Double t = time + tza;
        Entries[Time_Hour]   = Date::HourFromTime(t);
        Entries[Time_Minute] = Date::MinFromTime(t);
        Entries[Time_Second] = Date::SecFromTime(t);
        Entries[Time_Ms]     = Date::SecFromTime(t);
        // Save Day so that it's reapplied in MakeDate
        Day = Date::DayOfTime(t);
    }
    Double Date::TimeHolder::MakeDate() const
    {
        Double t = Date::MakeTime(Entries[Time_Hour], Entries[Time_Minute],
                                  Entries[Time_Second], Entries[Time_Ms]);
        return TimeClip(Date::MakeDate(Day, t)-TZA);
    }

    // Similar holder for Date
    Date::DateHolder::DateHolder(Double time, Double tza) : TZA(tza)
    {
        Double t = time + tza;
        Entries[Date_Year] = Date::YearFromTime(t);
        int    dayWithinYear = (int)(Date::DayOfTime(t) - Date::DayFromYear(Entries[Date_Year]));
        Entries[Date_Month]= Date::MonthFromYearDay((int)Entries[Date_Year], dayWithinYear);
        Entries[Date_Day]  = (Double)Date::DateFromTime(t);
        // Save Day so that it's reapplied in MakeDate
        TimeInDay = Date::TimeWithinDay(t);
    }
    Double Date::DateHolder::MakeDate() const
    {
        Double t = Date::MakeDay(Entries[Date_Year], Entries[Date_Month], Entries[Date_Day]);
        return TimeClip(Date::MakeDate(t, TimeInDay)-TZA);
    }


    // Applies time from arguments in order Hour, Minute, Second, Ms (starting with firstArgIndex)
    void Date::applyTimeArgs(Value& result, unsigned argc, const Value* const argv,
                             TimeEntry firstArg, Double tza)
    {
        if (argc > 0)
        {
            TimeHolder t(TimeValue, tza);
            if (argc > (unsigned)(Time_Entry_Count-firstArg))
                argc = Time_Entry_Count-firstArg;

            if (!argv[0].Convert2Number(t.Entries[firstArg]))
                return;
            if (argc >= 2)
                if (!argv[1].Convert2Number(t.Entries[firstArg+1]))
                    return;
            if (argc >= 3)
                if (!argv[2].Convert2Number(t.Entries[firstArg+2]))
                    return;
            if (argc >= 4)
                if (!argv[3].Convert2Number(t.Entries[firstArg+3]))
                    return;
            TimeValue = t.MakeDate();
        }
        result.SetNumber(TimeValue);
    }

    void Date::applyDateArgs(Value& result, unsigned argc, const Value* const argv,
                             DateEntry firstArg, Double tza)
    {
        if (argc > 0)
        {
            DateHolder d(TimeValue, tza);
            if (argc > (unsigned)(Date_Entry_Count-firstArg))
                argc = Date_Entry_Count-firstArg;
            
            if (!argv[0].Convert2Number(d.Entries[firstArg]))
                return;
            if (argc >= 2)
                if (!argv[1].Convert2Number(d.Entries[firstArg+1]))
                    return;
            if (argc >= 3)
                if (!argv[2].Convert2Number(d.Entries[firstArg+2]))
                    return;
            TimeValue = d.MakeDate();
        }
        result.SetNumber(TimeValue);
    }

    void Date::applyTimeArg(TimeEntry arg, Value::Number value, Double tza)
    {
        TimeHolder t(TimeValue, tza);
        t.Entries[arg] = value;
        TimeValue = t.MakeDate();        
    }
    void Date::applyDateArg(DateEntry arg, Value::Number value, Double tza)
    {
        DateHolder d(TimeValue, tza);
        d.Entries[arg] = value;
        TimeValue = d.MakeDate();        
    }


    UPInt Date::formatDateTimeString(char *buffer, UPInt bufferSize, Double t, int tza,
                                     bool needDate, bool needTime, bool utc)
    {
        if (NumberUtil::IsNaN(t))
        {
            memcpy(buffer, "Invalid Date", sizeof("Invalid Date"));
            return sizeof("Invalid Date")-1;
        }

        if (!utc) t += tza;

        UPInt      size = 0;
        DateHolder dh(t, 0.0);

        // Desired formatting:
        // Tue Jun 1 00:00:00 GMT-0400 1999
        if (needDate)
        {
            int weekDay = WeekDay(t);
            int monthIndex = (int)dh.Entries[Date_Month];
            SF_ASSERT((monthIndex < 12) && (weekDay < 7));
            size += Format(StringDataPtr(buffer, bufferSize),
                           "{0} {1} {2} ",
                           DayNames[weekDay], MonthNames[monthIndex], (int)dh.Entries[Date_Day]);
        }

        if (needTime)
        {
            TimeHolder  th(t, 0.0);
            int         tzaMinutes = (tza / MsPerMinute);
            int         tzaDelta   = (tzaMinutes / 60) * 100 + tzaMinutes % 60;
            const char* gmtString  = "GMT+";
            const char* format     = "{0:02}:{1:02}:{2:02}";

            if (!utc)
            {
                format = "{0:02}:{1:02}:{2:02} {3}{4:04}";
                if (tzaDelta < 0)
                {
                    tzaDelta  = -tzaDelta;
                    gmtString = "GMT-";
                }
            }

            size += Format(StringDataPtr(buffer+size, bufferSize-size), format,
                           (int)th.Entries[Time_Hour], (int)th.Entries[Time_Minute],
                           (int)th.Entries[Time_Second], gmtString, tzaDelta);
        }

        if (needDate)
        {
            size += Format(StringDataPtr(buffer+size, bufferSize-size),
                           needTime ? " {0}" : "{0}", (int)dh.Entries[Date_Year]);
        }

        if (needTime && utc)
        {
            memcpy(buffer+size, " UTC", 5);
            size += 4;
        }

        return size;
    }
 
    int Date::GetLocalTZA() const
    {
        //int localTZA = (TimeValue - LocalTime()) / MsPerMinute;
        int localTZA = LocalTZA;

        if (UseDST)
        {
#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360)
            // Recalculate LocalTZA.
            // Daylight saving time IS reflected by LocalTZA.
#ifdef _DURANGO
            struct _timeb tm;
            _ftime_s(&tm);
            int daylight = 0;
            _get_daylight(&daylight);
            long bias = tm.timezone - daylight * 60;
#else
            TIME_ZONE_INFORMATION tz;
            DWORD r = GetTimeZoneInformation(&tz);
            long bias = tz.Bias;

            if( r == TIME_ZONE_ID_STANDARD )
                bias += tz.StandardBias; 
            else if( r == TIME_ZONE_ID_DAYLIGHT)
                bias += tz.DaylightBias; 
            // else leave the bias alone
#endif
            localTZA = -MsPerMinute * bias;
#endif
        }

        return localTZA;
    }

    // *** Date::Parser helper class

    Date::Parser::Parser(const char *str)
    : Valid(true)
    , HasYear(false), HasMonth(false), HasDay(false), HasTime(false), HasTZA(false)
    , Month(-1), Day(-1), Year(-1), DayOfWeek(-1)
    , TZA(0), Hour(0), Minute(0), Sec(0) 
    {
        // Expected order: Tue Jun 1 00:00:00 GMT-0400 1999
        // Flash, however, can accept date parsing in any order,
        // including additional form such as follows:
        //  "1/1/1999 12:30 AM"

        while (*str != 0)
        {
            str = skipWhitespace(str);            
            char ch = *str;
            int  val;

            if (ch == 0)
                break;
            if (ch == '-')
            {
                // Can be year
                // Flash has strange interpretation of years, which seems incompatible
                // It interprets negative years as counting back from 1900.
                if (HasYear)
                    goto parse_fail;

                str     = scanUnsignedInt(str, &Year);
                HasYear = true;
                Year    = -Year; // TBD: Check range interpretation
                str++;
                continue;
            }

            if (isDigit(ch))
            {
                // Year, date, or time
                str = scanUnsignedInt(str, &val);
                str = skipWhitespace(str);

                // Time has ':' after the number.
                if (*str == ':')
                {
                    if (HasTime)
                        goto parse_fail;
                    HasTime = true;
                    Hour    = val;
                    str++;

                    str = skipWhitespace(str);
                    if (isDigit(*str))
                    {
                        str = scanUnsignedInt(str, &Minute);
                        if (*str == ':')
                        {
                            str++;
                            str = skipWhitespace(str);
                            if (isDigit(*str))
                                str = scanUnsignedInt(str, &Sec);
                        }
                    }
                }
                // Date in MM/DD/YYYY format... at least for US?
                else if (*str == '/')
                {
                    if (HasYear)
                        goto parse_fail;                    
                    Month = val-1;
                    str++;

                    // In this format, all parts of date are required.
                    str = skipWhitespace(str);
                    if (!isDigit(*str))
                        goto parse_fail;
                    str = scanUnsignedInt(str, &Day);
                    str = skipWhitespace(str);
                    if (*str != '/')
                        goto parse_fail;
                    str++;
                    str = skipWhitespace(str);
                    if (!isDigit(*str))
                        goto parse_fail;
                    str = scanUnsignedInt(str, &Year);

                    HasYear = HasMonth = HasDay = true;
                }

                else
                {
                    // Day (of month) or Year was scanned.
                    // We expect month first, but also analyze the numeric value,
                    // as Flash sometimes allows year to come first
                    if (HasDay)
                    {
                        if (HasYear)
                            goto parse_fail;
                        HasYear = true;
                        Year    = val;
                    }
                    else
                    {
                        if (val > 31)
                        {
                            // Must be year
                            if (HasYear)
                                goto parse_fail;
                            HasYear = true;
                            Year    = val;
                        }
                        else
                        {
                            HasDay = true;
                            Day    = val;
                        }
                    }
                }
            }
            else
            {
                // Not a digit, so interpret as a date string
                const char* end  = scanTextString(str);
                StringType  type = interpretDateString(str, end-str, &val);
                int         sign = 1;
                str = end;

                switch(type)
                {
                case String_Month:
                    if (HasMonth)
                        goto parse_fail;
                    Month    = val;
                    HasMonth = true;
                    break;

                case String_WeekDay:
                    if (DayOfWeek != -1)
                        goto parse_fail;
                    DayOfWeek = val;
                    break;

                case String_GMT:
                    if (HasTZA || (*end != '+' && *end != '-'))
                        goto parse_fail;
                    if (*end == '-')
                        sign = -1;
                    end++;
                    if (!((*end >= '0') && (*end <= '9')))
                        goto parse_fail;
                    str = scanUnsignedInt(end, &val);
                    TZA = sign * ((val / 100) * 60 + (val % 100)) * MsPerMinute;
                    HasTZA = true;
                    break;

                case String_UTC:
                    if (HasTZA)
                        goto parse_fail;
                    HasTZA = true;
                    break;

                case String_PM:
                    // Flash: 13 hrs AM/PM is reported as invalid date
                    if (!HasTime || (Hour > 12)) 
                        goto parse_fail;
                    if (Hour != 12)
                        Hour += 12;
                    break;
                case String_AM:
                    if (!HasTime || (Hour > 12))
                        goto parse_fail;
                    if (Hour == 12) // 12 AM => 0 hours.
                        Hour = 0;
                    break;

                case String_Invalid:
                    goto parse_fail;
                }
            }

        } // while (* str != 0)


        // Year, Month and Day are all required for successful parse.
        // Time and TimeZone are optional.
        if (!HasYear || !HasMonth || !HasDay)
            Valid = false;
        return;

parse_fail:
        Valid = false;
        return;
    }

    const char* Date::Parser::skipWhitespace(const char *str)
    {
        while (*str != 0 &&
            ((*str <= ' ') || (*str == ',') ||
            (*str == '-' && ((str[1] < '0') || (str[1] > '9')))))
            str++;
        return str;
    }
    
    const char* Date::Parser::scanUnsignedInt(const char* str, int* val)
    {
        *val = 0;
        while((*str >= '0') && (*str <= '9'))
        {
            *val = (*val * 10) + (*str - '0');
            str++;
        }
        return str;
    }

    Date::Parser::StringType
    Date::Parser::interpretDateString(const char* str, SPInt length, int* index)
    {
        // Handle AM/PM
        if (length == 2)
        {
            if (str[1] == 'M')
            {
                if (str[0] == 'A')
                    return String_AM;
                if (str[0] == 'P')
                    return String_PM;
            }
        }

        if (length != 3) // All other strings are 3 chars for now
            return String_Invalid;
        if (!memcmp(str, "GMT", 3))
            return String_GMT;
        if (!memcmp(str, "UTC", 3))
            return String_UTC;

        int i;
        for (i = 0; i< 7; i++)
        {
            if (!memcmp(str, DayNames[i], 3))
            {
                *index = i;
                return String_WeekDay;
            }
        }
        for (i = 0; i< 12; i++)
        {
            if (!memcmp(str, MonthNames[i], 3))
            {
                *index = i;
                return String_Month;
            }
        }
        return String_Invalid;
    }
    
    // Makes a date from parsed value, considering localTZA if it was omitted.
    Double Date::Parser::MakeDate(int localTZA) const
    {
        if (!Valid)
            return NumberUtil::NaN();
        if (HasTZA)
            localTZA = TZA;
        return TimeClip(Date::MakeDate(Date::MakeDay(Year, Month, Day),
                                       Date::MakeTime(Hour, Minute, Sec, 0.0))-localTZA);
    }



    void Date::AS3Constructor(unsigned argc, const Value* argv)
    {
        // public function Date(yearOrTimevalue:Object, month:Number, date:Number = 1,
        //                      hour:Number = 0, minute:Number = 0, second:Number = 0,
        //                      millisecond:Number = 0) 

        GetCurrentTimeValue(&TimeValue, &LocalTZA);

        switch (argc)
        {
        case 0:
            // In case of zero arguments we should apply DST.
            UseDST = true;
            break;
        case 1:
            {
                Value arg;

                if (!argv[0].Convert2PrimitiveValueUnsafe(*GetStringManager().GetStringManager(), arg, Value::hintString))
                    // Exception.
                    return;

                if (arg.IsString())
                {
                    Parser parsedDate(arg.AsString().ToCStr());
                    TimeValue = parsedDate.MakeDate(LocalTZA);
                }
                else
                {
                    if (!arg.ToNumberValue())
                        // Exception.
                        return;

                    TimeValue = TimeClip(arg.AsNumber());
                }
            }
            break;
        default:
            TimeValue = decodeUTCArgs(argc, argv, static_cast<Double>(LocalTZA));
            break;
        }
    }

    void Date::GetCurrentTimeValue( Double *timeValue, int *localTZA )
    {

        // ECMA 15.9.1.7
        // Daylight saving time is NOT reflected by LocalTZA.

#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360)
        struct timeb t;
        ftime(&t);
        TIME_ZONE_INFORMATION tz;

//#if 0
        //DWORD r = GetTimeZoneInformation(&tz);
        //long bias = tz.Bias;

         //  if( r == TIME_ZONE_ID_STANDARD )
         //    bias += tz.StandardBias; 
         //else if( r == TIME_ZONE_ID_DAYLIGHT)
         //  bias += tz.DaylightBias; 
        // else leave the bias alone
#ifdef _DURANGO
        long bias = t.timezone;
#else
        GetTimeZoneInformation(&tz);
        long bias = tz.Bias;
#endif

        *localTZA = -MsPerMinute * bias;
        *timeValue = static_cast<double>(t.time) * MsPerSecond + t.millitm;

#elif defined(SF_OS_PS3)
        CellRtcTick t, localt;
        cellRtcGetCurrentTick(&t);
        cellRtcConvertUtcToLocalTime(&t, &localt);
        *localTZA = ((localt.tick/1000)-(t.tick/1000));
        *timeValue= Double(t.tick/1000) - 62135596800000.0;
        //SetDate(SInt64(t.tick/1000 - 62135596800000ull));

#elif defined(SF_OS_WII) || defined(SF_OS_WIIU)
        SInt64 t = OSTicksToMilliseconds(OSGetTime());
        *localTZA = 0;
        *timeValue= t + 946684800000.0;
        //SetDate(t + 946684800000LL);        

#elif defined(SF_OS_PSVITA)
        SceRtcTick t, localt;
        sceRtcGetCurrentTick(&t);
        sceRtcConvertUtcToLocalTime(&t, &localt);
        *localTZA = ((localt.tick/1000)-(t.tick/1000));
        *timeValue = Double(t.tick/1000) - 62135596800000.0;
        //SetDate(SInt64(t.tick/1000 - 62135596800000ull));

#elif defined(SF_OS_3DS)
        SInt64 t = Timer::GetTicks();
        *localTZA = 0;
        *timeValue = t + 946684800000LL;
        //SetDate(t + 946684800000LL);        

#else
        struct timeval tv;
        struct timezone tz;
        gettimeofday(&tv, &tz);
        long bias = tz.tz_minuteswest - (tz.tz_dsttime ? 60 : 0);
        *localTZA = -MsPerMinute * bias;
        *timeValue = double(tv.tv_sec) * MsPerSecond + (tv.tv_usec/1000);
        // SetDate(SInt64(tv.tv_sec) * 1000 + SInt64(tv.tv_usec/1000));

#endif
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    // const UInt16 Date::tito[Date::ThunkInfoNum] = {
    //    0, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 45, 46, 48, 49, 51, 52, 54, 55, 57, 58, 60, 61, 63, 64, 66, 67, 69, 70, 72, 73, 75, 76, 78, 79, 81, 82, 84, 85, 87, 88, 89, 
    // };
    const TypeInfo* Date::tit[90] = {
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
    };
    const ThunkInfo Date::ti[Date::ThunkInfoNum] = {
        {TFunc_Instances_Date_AS3valueOf::Func, &Date::tit[0], "valueOf", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3setTime::Func, &Date::tit[1], "setTime", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {TFunc_Instances_Date_AS3toString::Func, &Date::tit[3], "toString", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3toDateString::Func, &Date::tit[4], "toDateString", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3toTimeString::Func, &Date::tit[5], "toTimeString", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3toLocaleString::Func, &Date::tit[6], "toLocaleString", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3toLocaleDateString::Func, &Date::tit[7], "toLocaleDateString", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3toLocaleTimeString::Func, &Date::tit[8], "toLocaleTimeString", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3toUTCString::Func, &Date::tit[9], "toUTCString", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getUTCFullYear::Func, &Date::tit[10], "getUTCFullYear", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getUTCMonth::Func, &Date::tit[11], "getUTCMonth", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getUTCDate::Func, &Date::tit[12], "getUTCDate", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getUTCDay::Func, &Date::tit[13], "getUTCDay", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getUTCHours::Func, &Date::tit[14], "getUTCHours", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getUTCMinutes::Func, &Date::tit[15], "getUTCMinutes", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getUTCSeconds::Func, &Date::tit[16], "getUTCSeconds", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getUTCMilliseconds::Func, &Date::tit[17], "getUTCMilliseconds", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getFullYear::Func, &Date::tit[18], "getFullYear", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getMonth::Func, &Date::tit[19], "getMonth", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getDate::Func, &Date::tit[20], "getDate", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getDay::Func, &Date::tit[21], "getDay", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getHours::Func, &Date::tit[22], "getHours", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getMinutes::Func, &Date::tit[23], "getMinutes", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getSeconds::Func, &Date::tit[24], "getSeconds", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getMilliseconds::Func, &Date::tit[25], "getMilliseconds", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getTimezoneOffset::Func, &Date::tit[26], "getTimezoneOffset", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3getTime::Func, &Date::tit[27], "getTime", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_AS3setFullYear::Func, &Date::tit[28], "setFullYear", NS_AS3, Abc::NS_Public, CT_Method, 0, 3, 1, 0, NULL},
        {TFunc_Instances_Date_AS3setMonth::Func, &Date::tit[29], "setMonth", NS_AS3, Abc::NS_Public, CT_Method, 0, 2, 1, 0, NULL},
        {TFunc_Instances_Date_AS3setDate::Func, &Date::tit[30], "setDate", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 1, 0, NULL},
        {TFunc_Instances_Date_AS3setHours::Func, &Date::tit[31], "setHours", NS_AS3, Abc::NS_Public, CT_Method, 0, 4, 1, 0, NULL},
        {TFunc_Instances_Date_AS3setMinutes::Func, &Date::tit[32], "setMinutes", NS_AS3, Abc::NS_Public, CT_Method, 0, 3, 1, 0, NULL},
        {TFunc_Instances_Date_AS3setSeconds::Func, &Date::tit[33], "setSeconds", NS_AS3, Abc::NS_Public, CT_Method, 0, 2, 1, 0, NULL},
        {TFunc_Instances_Date_AS3setMilliseconds::Func, &Date::tit[34], "setMilliseconds", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 1, 0, NULL},
        {TFunc_Instances_Date_AS3setUTCFullYear::Func, &Date::tit[35], "setUTCFullYear", NS_AS3, Abc::NS_Public, CT_Method, 0, 3, 1, 0, NULL},
        {TFunc_Instances_Date_AS3setUTCMonth::Func, &Date::tit[36], "setUTCMonth", NS_AS3, Abc::NS_Public, CT_Method, 0, 2, 1, 0, NULL},
        {TFunc_Instances_Date_AS3setUTCDate::Func, &Date::tit[37], "setUTCDate", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 1, 0, NULL},
        {TFunc_Instances_Date_AS3setUTCHours::Func, &Date::tit[38], "setUTCHours", NS_AS3, Abc::NS_Public, CT_Method, 0, 4, 1, 0, NULL},
        {TFunc_Instances_Date_AS3setUTCMinutes::Func, &Date::tit[39], "setUTCMinutes", NS_AS3, Abc::NS_Public, CT_Method, 0, 3, 1, 0, NULL},
        {TFunc_Instances_Date_AS3setUTCSeconds::Func, &Date::tit[40], "setUTCSeconds", NS_AS3, Abc::NS_Public, CT_Method, 0, 2, 1, 0, NULL},
        {TFunc_Instances_Date_AS3setUTCMilliseconds::Func, &Date::tit[41], "setUTCMilliseconds", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 1, 0, NULL},
        {TFunc_Instances_Date_fullYearGet::Func, &Date::tit[42], "fullYear", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_fullYearSet::Func, &Date::tit[43], "fullYear", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Date_monthGet::Func, &Date::tit[45], "month", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_monthSet::Func, &Date::tit[46], "month", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Date_dateGet::Func, &Date::tit[48], "date", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_dateSet::Func, &Date::tit[49], "date", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Date_hoursGet::Func, &Date::tit[51], "hours", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_hoursSet::Func, &Date::tit[52], "hours", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Date_minutesGet::Func, &Date::tit[54], "minutes", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_minutesSet::Func, &Date::tit[55], "minutes", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Date_secondsGet::Func, &Date::tit[57], "seconds", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_secondsSet::Func, &Date::tit[58], "seconds", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Date_millisecondsGet::Func, &Date::tit[60], "milliseconds", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_millisecondsSet::Func, &Date::tit[61], "milliseconds", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Date_fullYearUTCGet::Func, &Date::tit[63], "fullYearUTC", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_fullYearUTCSet::Func, &Date::tit[64], "fullYearUTC", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Date_monthUTCGet::Func, &Date::tit[66], "monthUTC", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_monthUTCSet::Func, &Date::tit[67], "monthUTC", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Date_dateUTCGet::Func, &Date::tit[69], "dateUTC", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_dateUTCSet::Func, &Date::tit[70], "dateUTC", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Date_hoursUTCGet::Func, &Date::tit[72], "hoursUTC", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_hoursUTCSet::Func, &Date::tit[73], "hoursUTC", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Date_minutesUTCGet::Func, &Date::tit[75], "minutesUTC", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_minutesUTCSet::Func, &Date::tit[76], "minutesUTC", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Date_secondsUTCGet::Func, &Date::tit[78], "secondsUTC", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_secondsUTCSet::Func, &Date::tit[79], "secondsUTC", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Date_millisecondsUTCGet::Func, &Date::tit[81], "millisecondsUTC", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_millisecondsUTCSet::Func, &Date::tit[82], "millisecondsUTC", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Date_timeGet::Func, &Date::tit[84], "time", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_timeSet::Func, &Date::tit[85], "time", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Date_timezoneOffsetGet::Func, &Date::tit[87], "timezoneOffset", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_dayGet::Func, &Date::tit[88], "day", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Date_dayUTCGet::Func, &Date::tit[89], "dayUTC", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

    Date::Date(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Date::Date()"
        SetTraitsType(Traits_Date);
//##protect##"InstanceTraits::Date::Date()"

    }

    void Date::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Date&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl
{
    Date::Date(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Date::Date()"
//##protect##"class_::Date::Date()"
    }
    void Date::parse(Value::Number& result, const Value& s)
    {
//##protect##"class_::Date::parse()"
        ASString str = GetStringManager().CreateEmptyString();
        if (!s.Convert2String(str))
            return;
        Instances::fl::Date::Parser parsedDate(str.ToCStr());
        result = parsedDate.MakeDate(0); // TBD: Need to pass LocalTZA from system
//##protect##"class_::Date::parse()"
    }
    void Date::UTC(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"class_::Date::UTC()"
        Double time = Instances::fl::Date::decodeUTCArgs(argc, argv, 0.0);
        result.SetNumber(time);
//##protect##"class_::Date::UTC()"
    }
//##protect##"class_$methods"
    void Date::Call(const Value& /*_this*/, Value& result, unsigned argc, const Value* const argv)
    {
        SF_UNUSED2(argc, argv);
        Double  timeValue;        
        int     localTZA;

        char  out[128];
        Instances::fl::Date::GetCurrentTimeValue(&timeValue, &localTZA);

        UPInt size = Instances::fl::Date::formatDateTimeString(out, 128, timeValue, localTZA, true, true);
        result = GetStringManager().CreateString(out, size);
    }

    Pickable<AS3::Object> Date::MakePrototype() const
    {
        //return GetInstanceTraits().MakeInstance();
        
#if 1
        SPtr<InstanceTraits::Prototype> prototype = MakePickable(SF_HEAP_NEW_ID(GetVM().GetMemoryHeap(), StatMV_VM_ITraits_Mem) InstanceTraits::Prototype(GetVM(), AS3::fl::DateCI, const_cast<Date&>(*this)));
#else
        // !!! We HAVE TO pass Date as a constructor otherwise several tests will fail.
        SPtr<InstanceTraits::Prototype> prototype = MakePickable(SF_HEAP_NEW_ID(GetVM().GetMemoryHeap(), StatMV_VM_ITraits_Mem) InstanceTraits::Prototype(GetVM(), GetVM().GetStringManager().CreateConstString("Date"), const_cast<Date&>(*this)));
#endif
        InstanceTraits::Traits& itr = GetClassTraits().GetInstanceTraits();
        return Pickable<AS3::Object>(new(itr.Alloc()) Instances::fl::Date(*prototype));
    }
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc1<Classes::fl::Date, Classes::fl::Date::mid_parse, Value::Number, const Value&> TFunc_Classes_Date_parse;
typedef ThunkFunc2<Classes::fl::Date, Classes::fl::Date::mid_UTC, Value, unsigned, const Value*> TFunc_Classes_Date_UTC;

template <> const TFunc_Classes_Date_parse::TMethod TFunc_Classes_Date_parse::Method = &Classes::fl::Date::parse;
template <> const TFunc_Classes_Date_UTC::TMethod TFunc_Classes_Date_UTC::Method = &Classes::fl::Date::UTC;

namespace ClassTraits { namespace fl
{
    // const UInt16 Date::tito[Date::ThunkInfoNum] = {
    //    0, 2, 
    // };
    const TypeInfo* Date::tit[10] = {
        &AS3::fl::NumberTI, NULL, 
        &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
    };
    const Abc::ConstValue Date::dva[6] = {
        {Abc::CONSTANT_Double, 3}, {Abc::CONSTANT_Double, 0}, {Abc::CONSTANT_Double, 0}, {Abc::CONSTANT_Double, 0}, {Abc::CONSTANT_Double, 0}, {}, 
    };
    const ThunkInfo Date::ti[Date::ThunkInfoNum] = {
        {TFunc_Classes_Date_parse::Func, &Date::tit[0], "parse", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_Date_UTC::Func, &Date::tit[2], "UTC", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 6, &Date::dva[0]},
    };

    Date::Date(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Date::Date()"
        SetTraitsType(Traits_Date);
//##protect##"ClassTraits::Date::Date()"

    }

    Pickable<Traits> Date::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Date(vm, AS3::fl::DateCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl::DateCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl
{
    const TypeInfo DateTI = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject | TypeInfo::Final,
        sizeof(ClassTraits::fl::Date::InstanceType),
        ClassTraits::fl::Date::ThunkInfoNum,
        0,
        InstanceTraits::fl::Date::ThunkInfoNum,
        0,
        "Date", "", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo DateCI = {
        &DateTI,
        ClassTraits::fl::Date::MakeClassTraits,
        ClassTraits::fl::Date::ti,
        NULL,
        InstanceTraits::fl::Date::ti,
        NULL,
    };
}; // namespace fl


}}} // namespace Scaleform { namespace GFx { namespace AS3

