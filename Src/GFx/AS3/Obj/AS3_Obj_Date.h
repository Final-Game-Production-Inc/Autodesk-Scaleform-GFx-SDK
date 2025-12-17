//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Date.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Date_H
#define INC_AS3_Obj_Date_H

#include "AS3_Obj_Object.h"
//##protect##"includes"
#include "Kernel/SF_Math.h" // for fmod, floor
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl
{
    extern const TypeInfo DateTI;
    extern const ClassInfo DateCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo anyTI;
    extern const ClassInfo anyCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl
{
    class Date;
}}

namespace InstanceTraits { namespace fl
{
    class Date;
}}

namespace Classes { namespace fl
{
    class Date;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl
{
    class Date : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl::Date;
        
    public:
        typedef Date SelfType;
        typedef Classes::fl::Date ClassType;
        typedef InstanceTraits::fl::Date TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::DateTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl::Date"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Date(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        friend class Classes::fl::Date;

        virtual void AS3Constructor(unsigned argc, const Value* argv);

        void UpdateLocal();
        void UpdateGMT();

    public:
        Double  LocalTime() const { return TimeValue + GetLocalTZA(); }
        void SetDate(Double val) { TimeValue = val; }

//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_AS3valueOf, 
            mid_AS3setTime, 
            mid_AS3toString, 
            mid_AS3toDateString, 
            mid_AS3toTimeString, 
            mid_AS3toLocaleString, 
            mid_AS3toLocaleDateString, 
            mid_AS3toLocaleTimeString, 
            mid_AS3toUTCString, 
            mid_AS3getUTCFullYear, 
            mid_AS3getUTCMonth, 
            mid_AS3getUTCDate, 
            mid_AS3getUTCDay, 
            mid_AS3getUTCHours, 
            mid_AS3getUTCMinutes, 
            mid_AS3getUTCSeconds, 
            mid_AS3getUTCMilliseconds, 
            mid_AS3getFullYear, 
            mid_AS3getMonth, 
            mid_AS3getDate, 
            mid_AS3getDay, 
            mid_AS3getHours, 
            mid_AS3getMinutes, 
            mid_AS3getSeconds, 
            mid_AS3getMilliseconds, 
            mid_AS3getTimezoneOffset, 
            mid_AS3getTime, 
            mid_AS3setFullYear, 
            mid_AS3setMonth, 
            mid_AS3setDate, 
            mid_AS3setHours, 
            mid_AS3setMinutes, 
            mid_AS3setSeconds, 
            mid_AS3setMilliseconds, 
            mid_AS3setUTCFullYear, 
            mid_AS3setUTCMonth, 
            mid_AS3setUTCDate, 
            mid_AS3setUTCHours, 
            mid_AS3setUTCMinutes, 
            mid_AS3setUTCSeconds, 
            mid_AS3setUTCMilliseconds, 
            mid_fullYearGet, 
            mid_fullYearSet, 
            mid_monthGet, 
            mid_monthSet, 
            mid_dateGet, 
            mid_dateSet, 
            mid_hoursGet, 
            mid_hoursSet, 
            mid_minutesGet, 
            mid_minutesSet, 
            mid_secondsGet, 
            mid_secondsSet, 
            mid_millisecondsGet, 
            mid_millisecondsSet, 
            mid_fullYearUTCGet, 
            mid_fullYearUTCSet, 
            mid_monthUTCGet, 
            mid_monthUTCSet, 
            mid_dateUTCGet, 
            mid_dateUTCSet, 
            mid_hoursUTCGet, 
            mid_hoursUTCSet, 
            mid_minutesUTCGet, 
            mid_minutesUTCSet, 
            mid_secondsUTCGet, 
            mid_secondsUTCSet, 
            mid_millisecondsUTCGet, 
            mid_millisecondsUTCSet, 
            mid_timeGet, 
            mid_timeSet, 
            mid_timezoneOffsetGet, 
            mid_dayGet, 
            mid_dayUTCGet, 
        };
        void AS3valueOf(Value::Number& result);
        void AS3setTime(Value::Number& result, Value::Number millisecond = NumberUtil::NaN());
        void AS3toString(ASString& result);
        void AS3toDateString(ASString& result);
        void AS3toTimeString(ASString& result);
        void AS3toLocaleString(ASString& result);
        void AS3toLocaleDateString(ASString& result);
        void AS3toLocaleTimeString(ASString& result);
        void AS3toUTCString(ASString& result);
        void AS3getUTCFullYear(Value::Number& result);
        void AS3getUTCMonth(Value::Number& result);
        void AS3getUTCDate(Value::Number& result);
        void AS3getUTCDay(Value::Number& result);
        void AS3getUTCHours(Value::Number& result);
        void AS3getUTCMinutes(Value::Number& result);
        void AS3getUTCSeconds(Value::Number& result);
        void AS3getUTCMilliseconds(Value::Number& result);
        void AS3getFullYear(Value::Number& result);
        void AS3getMonth(Value::Number& result);
        void AS3getDate(Value::Number& result);
        void AS3getDay(Value::Number& result);
        void AS3getHours(Value::Number& result);
        void AS3getMinutes(Value::Number& result);
        void AS3getSeconds(Value::Number& result);
        void AS3getMilliseconds(Value::Number& result);
        void AS3getTimezoneOffset(Value::Number& result);
        void AS3getTime(Value::Number& result);
        void AS3setFullYear(Value& result, unsigned argc, const Value* const argv);
        void AS3setMonth(Value& result, unsigned argc, const Value* const argv);
        void AS3setDate(Value& result, unsigned argc, const Value* const argv);
        void AS3setHours(Value& result, unsigned argc, const Value* const argv);
        void AS3setMinutes(Value& result, unsigned argc, const Value* const argv);
        void AS3setSeconds(Value& result, unsigned argc, const Value* const argv);
        void AS3setMilliseconds(Value& result, unsigned argc, const Value* const argv);
        void AS3setUTCFullYear(Value& result, unsigned argc, const Value* const argv);
        void AS3setUTCMonth(Value& result, unsigned argc, const Value* const argv);
        void AS3setUTCDate(Value& result, unsigned argc, const Value* const argv);
        void AS3setUTCHours(Value& result, unsigned argc, const Value* const argv);
        void AS3setUTCMinutes(Value& result, unsigned argc, const Value* const argv);
        void AS3setUTCSeconds(Value& result, unsigned argc, const Value* const argv);
        void AS3setUTCMilliseconds(Value& result, unsigned argc, const Value* const argv);
        void fullYearGet(Value::Number& result);
        void fullYearSet(const Value& result, Value::Number value);
        void monthGet(Value::Number& result);
        void monthSet(const Value& result, Value::Number value);
        void dateGet(Value::Number& result);
        void dateSet(const Value& result, Value::Number value);
        void hoursGet(Value::Number& result);
        void hoursSet(const Value& result, Value::Number value);
        void minutesGet(Value::Number& result);
        void minutesSet(const Value& result, Value::Number value);
        void secondsGet(Value::Number& result);
        void secondsSet(const Value& result, Value::Number value);
        void millisecondsGet(Value::Number& result);
        void millisecondsSet(const Value& result, Value::Number value);
        void fullYearUTCGet(Value::Number& result);
        void fullYearUTCSet(const Value& result, Value::Number value);
        void monthUTCGet(Value::Number& result);
        void monthUTCSet(const Value& result, Value::Number value);
        void dateUTCGet(Value::Number& result);
        void dateUTCSet(const Value& result, Value::Number value);
        void hoursUTCGet(Value::Number& result);
        void hoursUTCSet(const Value& result, Value::Number value);
        void minutesUTCGet(Value::Number& result);
        void minutesUTCSet(const Value& result, Value::Number value);
        void secondsUTCGet(Value::Number& result);
        void secondsUTCSet(const Value& result, Value::Number value);
        void millisecondsUTCGet(Value::Number& result);
        void millisecondsUTCSet(const Value& result, Value::Number value);
        void timeGet(Value::Number& result);
        void timeSet(const Value& result, Value::Number value);
        void timezoneOffsetGet(Value::Number& result);
        void dayGet(Value::Number& result);
        void dayUTCGet(Value::Number& result);

        // C++ friendly wrappers for AS3 methods.
        Value::Number AS3valueOf()
        {
            Value::Number result;
            AS3valueOf(result);
            return result;
        }
        Value::Number AS3setTime(Value::Number millisecond = NumberUtil::NaN())
        {
            Value::Number result;
            AS3setTime(result, millisecond);
            return result;
        }
        ASString AS3toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3toString(result);
            return result;
        }
        ASString AS3toDateString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3toDateString(result);
            return result;
        }
        ASString AS3toTimeString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3toTimeString(result);
            return result;
        }
        ASString AS3toLocaleString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3toLocaleString(result);
            return result;
        }
        ASString AS3toLocaleDateString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3toLocaleDateString(result);
            return result;
        }
        ASString AS3toLocaleTimeString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3toLocaleTimeString(result);
            return result;
        }
        ASString AS3toUTCString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3toUTCString(result);
            return result;
        }
        Value::Number AS3getUTCFullYear()
        {
            Value::Number result;
            AS3getUTCFullYear(result);
            return result;
        }
        Value::Number AS3getUTCMonth()
        {
            Value::Number result;
            AS3getUTCMonth(result);
            return result;
        }
        Value::Number AS3getUTCDate()
        {
            Value::Number result;
            AS3getUTCDate(result);
            return result;
        }
        Value::Number AS3getUTCDay()
        {
            Value::Number result;
            AS3getUTCDay(result);
            return result;
        }
        Value::Number AS3getUTCHours()
        {
            Value::Number result;
            AS3getUTCHours(result);
            return result;
        }
        Value::Number AS3getUTCMinutes()
        {
            Value::Number result;
            AS3getUTCMinutes(result);
            return result;
        }
        Value::Number AS3getUTCSeconds()
        {
            Value::Number result;
            AS3getUTCSeconds(result);
            return result;
        }
        Value::Number AS3getUTCMilliseconds()
        {
            Value::Number result;
            AS3getUTCMilliseconds(result);
            return result;
        }
        Value::Number AS3getFullYear()
        {
            Value::Number result;
            AS3getFullYear(result);
            return result;
        }
        Value::Number AS3getMonth()
        {
            Value::Number result;
            AS3getMonth(result);
            return result;
        }
        Value::Number AS3getDate()
        {
            Value::Number result;
            AS3getDate(result);
            return result;
        }
        Value::Number AS3getDay()
        {
            Value::Number result;
            AS3getDay(result);
            return result;
        }
        Value::Number AS3getHours()
        {
            Value::Number result;
            AS3getHours(result);
            return result;
        }
        Value::Number AS3getMinutes()
        {
            Value::Number result;
            AS3getMinutes(result);
            return result;
        }
        Value::Number AS3getSeconds()
        {
            Value::Number result;
            AS3getSeconds(result);
            return result;
        }
        Value::Number AS3getMilliseconds()
        {
            Value::Number result;
            AS3getMilliseconds(result);
            return result;
        }
        Value::Number AS3getTimezoneOffset()
        {
            Value::Number result;
            AS3getTimezoneOffset(result);
            return result;
        }
        Value::Number AS3getTime()
        {
            Value::Number result;
            AS3getTime(result);
            return result;
        }
        Value::Number fullYearGet()
        {
            Value::Number result;
            fullYearGet(result);
            return result;
        }
        void fullYearSet(Value::Number value)
        {
            fullYearSet(Value::GetUndefined(), value);
        }
        Value::Number monthGet()
        {
            Value::Number result;
            monthGet(result);
            return result;
        }
        void monthSet(Value::Number value)
        {
            monthSet(Value::GetUndefined(), value);
        }
        Value::Number dateGet()
        {
            Value::Number result;
            dateGet(result);
            return result;
        }
        void dateSet(Value::Number value)
        {
            dateSet(Value::GetUndefined(), value);
        }
        Value::Number hoursGet()
        {
            Value::Number result;
            hoursGet(result);
            return result;
        }
        void hoursSet(Value::Number value)
        {
            hoursSet(Value::GetUndefined(), value);
        }
        Value::Number minutesGet()
        {
            Value::Number result;
            minutesGet(result);
            return result;
        }
        void minutesSet(Value::Number value)
        {
            minutesSet(Value::GetUndefined(), value);
        }
        Value::Number secondsGet()
        {
            Value::Number result;
            secondsGet(result);
            return result;
        }
        void secondsSet(Value::Number value)
        {
            secondsSet(Value::GetUndefined(), value);
        }
        Value::Number millisecondsGet()
        {
            Value::Number result;
            millisecondsGet(result);
            return result;
        }
        void millisecondsSet(Value::Number value)
        {
            millisecondsSet(Value::GetUndefined(), value);
        }
        Value::Number fullYearUTCGet()
        {
            Value::Number result;
            fullYearUTCGet(result);
            return result;
        }
        void fullYearUTCSet(Value::Number value)
        {
            fullYearUTCSet(Value::GetUndefined(), value);
        }
        Value::Number monthUTCGet()
        {
            Value::Number result;
            monthUTCGet(result);
            return result;
        }
        void monthUTCSet(Value::Number value)
        {
            monthUTCSet(Value::GetUndefined(), value);
        }
        Value::Number dateUTCGet()
        {
            Value::Number result;
            dateUTCGet(result);
            return result;
        }
        void dateUTCSet(Value::Number value)
        {
            dateUTCSet(Value::GetUndefined(), value);
        }
        Value::Number hoursUTCGet()
        {
            Value::Number result;
            hoursUTCGet(result);
            return result;
        }
        void hoursUTCSet(Value::Number value)
        {
            hoursUTCSet(Value::GetUndefined(), value);
        }
        Value::Number minutesUTCGet()
        {
            Value::Number result;
            minutesUTCGet(result);
            return result;
        }
        void minutesUTCSet(Value::Number value)
        {
            minutesUTCSet(Value::GetUndefined(), value);
        }
        Value::Number secondsUTCGet()
        {
            Value::Number result;
            secondsUTCGet(result);
            return result;
        }
        void secondsUTCSet(Value::Number value)
        {
            secondsUTCSet(Value::GetUndefined(), value);
        }
        Value::Number millisecondsUTCGet()
        {
            Value::Number result;
            millisecondsUTCGet(result);
            return result;
        }
        void millisecondsUTCSet(Value::Number value)
        {
            millisecondsUTCSet(Value::GetUndefined(), value);
        }
        Value::Number timeGet()
        {
            Value::Number result;
            timeGet(result);
            return result;
        }
        void timeSet(Value::Number value)
        {
            timeSet(Value::GetUndefined(), value);
        }
        Value::Number timezoneOffsetGet()
        {
            Value::Number result;
            timezoneOffsetGet(result);
            return result;
        }
        Value::Number dayGet()
        {
            Value::Number result;
            dayGet(result);
            return result;
        }
        Value::Number dayUTCGet()
        {
            Value::Number result;
            dayUTCGet(result);
            return result;
        }

//##protect##"instance$data"
      
        // Combines local TimeZone and daylight adjustment.
        // Note that this may disagree with 15.9.1.8, which dictates that
        // Daylight saving time is not reflected in LocaTZA. Addressing this is TBD.
        int     LocalTZA;
        bool    UseDST; // Use Daylight Saving Time.
        // Time in MS since 1 Jan 1970, UTC.
        Double  TimeValue;        

    public:
        static const char *DayNames[7];
        static const char *MonthNames[12];
        static const int   Months[2][12];
        
        // ECMA-262, 15.9.1.2
        enum {
            MsPerDay        = 86400000,
            HoursPerDay     = 24,
            MinutesPerHour  = 60,
            SecondsPerMinute= 60,
            MsPerSecond     = 1000,
            MsPerMinute     = MsPerSecond * SecondsPerMinute,
            MsPerHour       = MsPerMinute * MinutesPerHour
        };
        static Double   DayOfTime(Double time) { return floor(time / Double(MsPerDay)); }
        static Double   TimeWithinDay(Double time) 
        { 
            Double result = fmod(time, double(MsPerDay));
            return result < 0 ? result + Double(MsPerDay) : result;
        }
        static int      WeekDay(Double time);

        // 15.9.1.3
        static bool     IsLeapYear(int year);
        static int      DaysInYear(int year)       { return IsLeapYear(year) ? 366 : 365; }
        static Double   DayFromYear(Double year);
        static Double   TimeFromYear(Double year)  { return DayFromYear(year) * MsPerDay; }
        static Double   YearFromTime(Double time);
        //static bool     IsLeapYear

        static Double   DayWithinYear(Double time)
        {
            return DayOfTime(time) - DayFromYear(YearFromTime(time));
        }

        static int      MonthFromTime(Double time);
        static int      MonthFromYearDay(int year, int dayWithinYear);
        static int      DateFromTime(Double time);

        static Double   MakeTime(Double hour, Double minute, Double sec, Double ms);
        static Double   MakeDay(Double year, Double month, Double date);
        static Double   MakeDate(Double day, Double time) { return day * MsPerDay + time; }
        static Double   TimeClip(Double time);

        static Double   HourFromTime(Double time);
        static Double   MinFromTime(Double time);
        static Double   SecFromTime(Double time);
        static Double   MsFromTime(Double time);

        // Helper used to implement Date constructor and UTC static function
        static Double   decodeUTCArgs(unsigned argc, const Value* const argv, Double tza);

        //Helper used in Date constructor and in Classes::Date::Call
        static void     GetCurrentTimeValue(Double *timeValue, int *localTZA);

        // Time constant used to index TimeHolder::Entries
        enum TimeEntry
        {
            Time_Hour,
            Time_Minute,
            Time_Second,
            Time_Ms,
            Time_Entry_Count
        };
        // Date constant used to index DateHolder::Entries
        enum DateEntry
        {
            Date_Year,
            Date_Month,
            Date_Day,
            Date_Entry_Count
        };
        
        // TimeHolder is as helper used by setSeconds/Milliseconds/Hours, etc. functions.
        // Constructor initializes the public variables, which can then be further modified.
        // MakeDate() can is called to obtain the resulting time value after modification.
        struct TimeHolder
        {
            Double Entries[Time_Entry_Count];

            TimeHolder(Double time, Double tza);
            Double MakeDate() const;
        private:
            Double Day, TZA;
        };

        struct DateHolder
        {
            Double Entries[Date_Entry_Count];

            DateHolder(Double time, Double tza);
            Double MakeDate() const;
        private:
            Double TimeInDay, TZA;
        };

                
        // Applies time from arguments in order Hour, Minute, Second, Ms (starting with firstArgIndex)
        // Sets result to TimeValue
        void            applyTimeArgs(Value& result, unsigned argc, const Value* const argv,
                                      TimeEntry firstArg, Double tza);
        // Applies date arguments in order: Year, Month, Day
        void            applyDateArgs(Value& result, unsigned argc, const Value* const argv,
                                      DateEntry firstArg, Double tza);

        // Applies a single date/time component
        void            applyTimeArg(TimeEntry arg, Value::Number value, Double tza);
        void            applyDateArg(DateEntry arg, Value::Number value, Double tza);
        

        // Formats a date/size string and returns generated size.
        static UPInt    formatDateTimeString(char *buffer, UPInt bufferSize,
                                              Double t, int tza, 
                                              bool needDate, bool needTime, bool utc = false);

        int             GetLocalTZA() const;

        // Parser helper class parses a date string,
        // potentially interpreting different date formats.
        struct Parser
        {
            bool    Valid;
            bool    HasYear, HasMonth, HasDay, HasTime, HasTZA;
            int     Month, Day, Year;
            int     DayOfWeek;  // Technically not needed now, but record for consistency.
            int     TZA;
            int     Hour, Minute, Sec;

            Parser(const char *str);

            // Makes a date from parsed value, considering localTZA if it was omitted.
            Double  MakeDate(int localTZA) const;

        private:
            // Date text parsing functions. All of these return
            // the next character point past scanned/processed text.
            static const char* skipWhitespace(const char *str);
            // Decodes an unsigned integer
            static const char* scanUnsignedInt(const char* str, int* val);

            static bool isDigit(char ch)
            {
                return (ch >= '0') && (ch <= '9');
            }

            // Scans a text string until end, returns first character past end.
            static const char* scanTextString(const char *str)
            {
                while ((*str > ' ') && (*str != ',') && (*str != '-'))
                    str++;
                return str;
            }

            enum StringType
            {
                String_Invalid,
                String_Month,
                String_WeekDay,
                String_GMT,
                String_UTC,
                String_AM,
                String_PM
            };

            static StringType interpretDateString(const char* pstr, SPInt length, int* index);            
        };
        
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    class Date : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Date"; }
#endif
    public:
        typedef Instances::fl::Date InstanceType;

    public:
        Date(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl::Date> MakeInstance(Date& t)
        {
            return Pickable<Instances::fl::Date>(new(t.Alloc()) Instances::fl::Date(t));
        }
        SPtr<Instances::fl::Date> MakeInstanceS(Date& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 74 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[90];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl
{
    class Date : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Date"; }
#endif
    public:
        typedef Classes::fl::Date ClassType;
        typedef InstanceTraits::fl::Date InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Date(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 2 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[10];
        static const Abc::ConstValue dva[6];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl
{
    class Date : public Class
    {
        friend class ClassTraits::fl::Date;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::DateTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Date"; }
#endif
    public:
        typedef Date SelfType;
        typedef Date ClassType;
        
    private:
        Date(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
        virtual void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);
        virtual Pickable<AS3::Object> MakePrototype() const;
//##protect##"class_$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_parse, 
            mid_UTC, 
        };
        void parse(Value::Number& result, const Value& s);
        void UTC(Value& result, unsigned argc, const Value* const argv);

        // C++ friendly wrappers for AS3 methods.
        Value::Number parse(const Value& s)
        {
            Value::Number result;
            parse(result, s);
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

