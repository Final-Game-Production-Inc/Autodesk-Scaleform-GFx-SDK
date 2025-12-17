/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_Stats.h
Content     :   Statistics tracking and reporting APIs
Created     :   May 20, 2008
Authors     :   Michael Antonov, Boris Rayskiy, Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_Stats_H
#define INC_SF_Kernel_Stats_H

#include "SF_Types.h"
#include "SF_Timer.h"

namespace Scaleform {


// ***** Stat Group Identifiers
//
// Stat identifiers are organized into groups corresponding to different
// sub-systems such as the renderer, MovieDef, etc. Every group receives
// its own Id range, allowing identifiers within that system to be added
// without having to modify StatId (those identifiers are normally defined
// as a part of a separate enumeration in their own header file).

// All identifier constants must follow this naming convention:
// - Start with a system group prefix such as StatGroup_ (for group ids),
//   StatHeap_, StatRenderer_, GFxStatMV_, etc.
// - User one of the following endings to identify stat types:
//     - _Mem for MemoryStat.
//     - _Tks for TimerStat.
//     - _Cnt for CounterStat.
//---------------------------------------------------------------------------
enum StatBasicValues
{
    // Starting values
    StatGroup_Default              = 0,
    StatGroup_Core                 = 16,    

    Stat_Mem                       = StatGroup_Default + 1,
    Stat_Default_Mem               = Stat_Mem + 1,

    // Core entries.
    StatHeap_Start                 =  StatGroup_Core,
    // 16 slots for HeapSummary

    // How many entries we support by default
    Stat_MaxId         = 64 << 6, // 64 * 64 = 4096
    Stat_EntryCount    = 512,

    // Other values
    HeapId_Default     = 1
};

#ifndef SF_STATS_H
#define SF_STATS_H "GFx/GFx_Stats.h"
#endif

#include SF_STATS_H

// ***** Stat

// Base class for all of the statistics. This class does not actually store
// any data, expecting derived classes such as MemoryStat to do so.

class Stat
{
public:

    // Different statistics types that we collect.
    enum StatType
    {
        Stat_LogicalGroup,
        Stat_Memory,
        Stat_Timer,
        Stat_Counter,
        // The number of supported types.
        Stat_TypeCount
    };

    // A structure that represents a single statistics value.
    struct StatValue
    {
        enum ValueType
        {
            VT_None,
            VT_Int,
            VT_Int64,
            VT_Float
        };

        ValueType       Type;
        const char*     pName;
        union
        {
            UPInt       IValue;
            UInt64      I64Value;
            float       FValue;
        };

        StatValue() : Type(VT_None), pName(""), IValue(0)
        { }
        StatValue(const StatValue& src)
            : Type(src.Type), pName(src.pName), I64Value(src.I64Value)
        { }

        StatValue& operator = (const StatValue& src)
        {
            Type = src.Type; pName = src.pName; I64Value = src.I64Value;
            return *this;
        }

        // Assignment of particular types
        void SetInt(const char* pname, UPInt val)
        {
            Type   = VT_Int;
            pName  = pname;
            IValue = val;
        }

        void SetInt64(const char* pname, UInt64 val)
        {
            Type   = VT_Int64;
            pName  = pname;
            I64Value = val;
        }

        void SetFloat(const char* pname, float val)
        {
            Type   = VT_Float;
            pName  = pname;
            FValue = val;
        }
    };

};


// ***** MemoryStat

// Records memory allocation statics for a particular allocation Id type.
// Keeps track of the number of bytes allocated and used, including use peak.
// Unlike allocation size, used includes the number of bytes occupied
// in the allocator for maintaining the block, including the different
// header and alignment space.

class MemoryStat : public Stat
{
#ifndef SF_ENABLE_STATS

    // *** Null Stat implementation
public:

    MemoryStat() { }
    
    void        Init()  { }
    void        Increment(UPInt, UPInt ) { }
    MemoryStat& operator += (MemoryStat& )   { return *this; }
    MemoryStat& operator -= (MemoryStat& )   { return *this; }

    StatType    GetStatType() const     { return Stat_Memory; }
    unsigned    GetStatCount() const    { return 3; }

    void        GetStat(StatValue* pval, unsigned index) const
    {
        switch(index)
        {
        case 0: pval->SetInt("Allocated", 0);   break;
        case 1: pval->SetInt("Used", 0);        break;   
        case 2: pval->SetInt("AllocCount", 0);  break;
        }
    }

    void        Add(MemoryStat*)       { }
    void        SetMax(MemoryStat*)    { }
    void        SetMin(MemoryStat*)    { }
    void        Reset()                 { }

    UPInt       GetAllocated() const    { return 0; }
    UPInt       GetUsed() const         { return 0; }
    UPInt       GetAllocCount() const   { return 0; }

#else
    
    // *** Default Stat implementation
    UPInt   Allocated;
    UPInt   Used;    
    UPInt   AllocCount;

public:

    MemoryStat()
    {
        Init();
    }

    void Init()
    {
        Allocated = 0;
        Used      = 0;
        AllocCount= 0;        
    }


    void    Increment(UPInt alloc, UPInt use)
    {
        Allocated+= alloc;
        Used      += use;
        AllocCount++;        
    }


    MemoryStat& operator += (MemoryStat& other)
    {
        Allocated += other.Allocated;
        Used      += other.Used;
        AllocCount+= other.AllocCount;
        return *this;
    }

    MemoryStat& operator -= (MemoryStat& other)
    {
        Allocated -= other.Allocated;
        Used      -= other.Used;
        AllocCount-= other.AllocCount;
        return *this;
    }


    // ***  General accessibility API.
    StatType    GetStatType() const     { return Stat_Memory; }
    unsigned    GetStatCount() const    { return 3; }

    void        GetStat(StatValue* pval, unsigned index) const
    {
        switch(index)
        {
        case 0: pval->SetInt("Allocated", Allocated);   break;
        case 1: pval->SetInt("Used", Used);             break;
        case 2: pval->SetInt("AllocCount", AllocCount); break;    
        }
    }
    
    // Reset, Add, SetMin, SetMax
    void            Add(MemoryStat* p)
    { *this += *p; }

    void            SetMin(MemoryStat* p)
    { 
        Allocated = (p->Allocated < Allocated) ? p->Allocated : Allocated;
        Used      = (p->Used      < Used     ) ? p->Used      : Used;
        AllocCount= (p->AllocCount<AllocCount) ? p->AllocCount: AllocCount;
    }
    void            SetMax(MemoryStat* p)
    { 
        Allocated = (p->Allocated > Allocated) ? p->Allocated : Allocated;
        Used      = (p->Used      > Used     ) ? p->Used      : Used;
        AllocCount= (p->AllocCount>AllocCount) ? p->AllocCount: AllocCount;
    }


    void  Reset()
    {      
        Allocated = 0;
        Used      = 0;
        AllocCount= 0;
    }

    UPInt GetAllocated() const      { return Allocated; }
    UPInt GetUsed() const           { return Used; }
    UPInt GetAllocCount() const     { return AllocCount; }

#endif
};



// ***** TimerStat

// TimerStat records the number of nano-second ticks used for a certain
// measured activity, such as rendering or tessellation. The time is most
// easily measured by using the TimerStat::ScopeTimer scope object around
// the code whose performance is to be measured. Every time that block of
// code is executed, the number of ticks will be incremented appropriately.

class TimerStat : public Stat
{
#ifndef SF_ENABLE_STATS

    // *** Null Stat implementation
public:
    TimerStat() { }

    void        Init() { }
    TimerStat&  operator += (TimerStat&) { return *this; }
    StatType    GetStatType() const     { return Stat_Timer; }
    unsigned    GetStatCount() const    { return 1; }

    void        GetStat(StatValue* pval, unsigned index) const
    {
        switch(index)
        {
        case 0: pval->SetInt64("Ticks", 0);     break;   
        }
    }

    void            Add(TimerStat*) { }
    void            SetMax(TimerStat*) { }
    void            SetMin(TimerStat*) { }
    void            AddTicks(UInt64) { }     
    void            Reset() { }

    class ScopeTimer
    {           
    public:
        ScopeTimer(TimerStat*) { }
    };

    UInt64 GetTicks() const         { return 0; }  

#else

    // *** Full TimerStat implementation

    // Tracks the number of nano-second ticks that passed.
    UInt64   Ticks;    

public:

    TimerStat()
    {
        Init();
    }

    void Init()
    {
        Ticks     = 0;     
    }

    TimerStat& operator += (TimerStat& other)
    {
        Ticks += other.Ticks;
        return *this;
    }


    // ***  General accessibility API.
    StatType    GetStatType() const     { return Stat_Timer; }
    unsigned    GetStatCount() const    { return 1; }

    void        GetStat(StatValue* pval, unsigned index) const
    {
        switch(index)
        {
        case 0: pval->SetInt64("Ticks", Ticks);         break;
        }
    }

    // Reset, Clear, Add
    void            Add(TimerStat* p)
    { *this += *p; }

    void            SetMin(TimerStat* p)
    { 
        Ticks = (p->Ticks < Ticks) ? p->Ticks : Ticks;
    }
    void            SetMax(TimerStat* p)
    { 
        Ticks = (p->Ticks > Ticks) ? p->Ticks : Ticks;
    }

    void            AddTicks(UInt64 ticks)
    {
        Ticks += ticks;   
    }


    void  Reset()
    {
        Ticks = 0;
    }


    // Scope used for timer measurements.
    class ScopeTimer
    {
        TimerStat*  pTimer;
        UInt64      StartTicks;
    public:
     
        ScopeTimer(TimerStat* ptimer)
            : pTimer(ptimer)
        {
            StartTicks = Timer::GetProfileTicks();
        }
        ~ScopeTimer()
        {
            UInt64 endTicks = Timer::GetProfileTicks();
            pTimer->AddTicks(endTicks - StartTicks);
        }
    };


    UInt64 GetTicks() const         { return Ticks; }   

#endif
};




// ***** CounterStat

// A counter stat tracks the number of times a certain measured event
// took place. The counter can also be used to record the number of
// certain objects processed by the algorithm, such as the number of
// DrawPrimitives or triangles drawn by the renderer.

class CounterStat : public Stat
{
#ifndef SF_ENABLE_STATS

    // *** Null CounterStat
public:
    CounterStat() { }
    CounterStat(UPInt) { }

    void        Init() { }
    CounterStat& operator += (CounterStat&) { return *this; }
    StatType    GetStatType() const     { return Stat_Counter; }
    unsigned    GetStatCount() const    { return 1; }

    void        GetStat(StatValue* pval, unsigned index) const
    {
        switch(index)
        {
        case 0: pval->SetInt("Count", 0);         break;        
        }
    }

    // Reset, Add, SetMin, SetMax
    void        Add(CounterStat*)    { }
    void        SetMin(CounterStat*) { }
    void        SetMax(CounterStat*) { }
    void        Reset()               { }
    void        AddCount(UPInt)       { }
    
    UPInt       GetCount() const         { return 0; }    

#else
    // *** Full CounterStat implementation

    UPInt   Count;    

public:
    CounterStat()
    {
        Init();
    }
    CounterStat(UPInt count)
    {        
        Count = count;
    }

    void Init()
    {
        Count     = 0;
    }

    CounterStat& operator += (CounterStat& other)
    {
        Count += other.Count;
        return *this;
    }


    // ***  General accessibility API.
    StatType    GetStatType() const     { return Stat_Counter; }
    unsigned    GetStatCount() const    { return 1; }

    void        GetStat(StatValue* pval, unsigned index) const
    {
        switch(index)
        {
        case 0: pval->SetInt("Count", Count);         break;        
        }
    }

    // Reset, Add
    void            Add(CounterStat* p)
    { *this += *p; }

    void            SetMin(CounterStat* p)
    { 
        Count = (p->Count < Count) ? p->Count : Count;
    }
    void            SetMax(CounterStat* p)
    { 
        Count = (p->Count > Count) ? p->Count : Count;
    }

    void            AddCount(UPInt count)
    {
        Count += count;
    }

    void  Reset()
    {    
        Count = 0;
    }

    UPInt GetCount() const         { return Count; }

#endif
};



/* ***** Statistics Group Types

    Group (Logical)
     - A logical group organizes a set of other stats into one group, giving
       them a common display title.

    Sum Group
     - A sum group maintains its own counter that includes the values of child
       item counters, but can be greater. It is expected that sum group can have
       additional 'remainder' values not accounted for by its children.
       The group is updated manually by the user using scopes or calling the
       appropriate Add function for the stat is question.

    AutoSum Group
     - A sum group that is calculated automatically during the call to
       StatBag::UpdateGroups; the user does not need to update the sum since it 
       is computed by adding up all of its child values.
*/

// Stat descriptor structure used by the SF_DECLARE_??_STAT macros.
struct StatDesc
{
    enum
    {    
        SF_Group         = 0x01,
        SF_Sum           = 0x02,    // This group is a manual sum of its sub-items.
        SF_AutoSum       = 0x04,    // This an automatically computed sum of its items.

        // Combination of flags
        SF_SumGroup      = SF_Group | SF_Sum,
        SF_AutoSumGroup  = SF_Group | SF_AutoSum
    };

    // Iterator used to enumerate children.
    class Iterator
    {
        const StatDesc*    pDesc;
    public:     

        Iterator(const StatDesc* p)        { pDesc = p; }
        Iterator(const Iterator&s)          { pDesc = s.pDesc; }
        Iterator& operator = (const Iterator&s)  { pDesc = s.pDesc; return *this; }

        const StatDesc*    operator * () const  { return pDesc; }
        const StatDesc*    operator -> () const { return pDesc; }

#ifndef SF_ENABLE_STATS

        bool     operator ++ () { return false; } 
        unsigned GetId() const  { return 0; }

#else // SF_ENABLE_STATS

        bool     operator ++ () { if (pDesc) pDesc = pDesc->pNextSibling; return (pDesc != 0); } 
        unsigned GetId() const  { return pDesc->Id; }

#endif // SF_ENABLE_STATS

        bool     operator == (const Iterator&s) const { return pDesc == s.pDesc; }
        bool     operator != (const Iterator&s) const { return pDesc != s.pDesc; }
        bool     IsEnd() const                        { return pDesc == 0; }
    };

    static void     InitChildTree();

#ifndef SF_ENABLE_STATS
    
    static const StatDesc*  SF_CDECL GetDesc(unsigned)   { return 0;             }
    StatDesc(Stat::StatType, UByte, unsigned, const char*, unsigned) { }

    bool            IsAutoSumGroup() const          { return false;         }
    const char*     GetName() const                 { return "";            }
    Iterator        GetChildIterator() const        { return Iterator(0);   }
    static Iterator GetGroupIterator(unsigned)          { return Iterator(0);   }
    unsigned        GetId() const                   { return 0;             }

#else  // SF_ENABLE_STATS

    // Register descriptors so that they can be looked up by id.
    static void            RegisterDesc(StatDesc* pdesc);
    static const StatDesc* SF_CDECL GetDesc(unsigned id);

private:

    UByte           Type;
    UByte           Flags;    
    unsigned        Id;
    unsigned        GroupId;
    const char*     pName;

    // Tree data representation, established by RegisterDesc.  A parent descriptor
    // can have a list if children which are enumerated by accessing pChild and
    // then traversing the chain of pNextSibling.
    StatDesc*       pChild;
    StatDesc*       pNextSibling;   

public:

    StatDesc(Stat::StatType type, UByte flags, unsigned id,
             const char* pname, unsigned groupId)
    {
        Type    = (UByte)type;
        Flags   = flags;
        Id      = id;
        GroupId = groupId;
        pName   = pname;
        pChild  = 0;
        pNextSibling = 0;

        RegisterDesc(this);
    }
    
    UByte           GetType() const { return Type; }

    unsigned        GetId() const { return Id; }
    void            SetId(unsigned value) { Id = value; }

    unsigned        GetGroupId() const { return GroupId; }
    void            SetGroupId(unsigned value) { GroupId = value; }


    bool            IsAutoSumGroup() const
    {
        return (Flags & SF_AutoSumGroup) == SF_AutoSumGroup;
    }

    const char*     GetName() const { return pName; }

    // Obtain Iterator for the first child. Users can call IsEnd() to determine
    // if a child actually exists.
    Iterator        GetChildIterator() const        { return Iterator(pChild); }
    
    // Get an item Iterator based on it. 
    static Iterator GetGroupIterator(unsigned groupId)  { return Iterator(GetDesc(groupId)); }

#endif  // SF_ENABLE_STATS

};


// We need to create this table during initialization.
// statId -> StatDesc
// statId -> StatInterface [identified by type]

#define SF_DECLARE_STAT_GROUP(id, name, group)       StatDesc SF_STAT_##id(Stat::Stat_LogicalGroup, StatDesc::SF_Group, id, name, group);

// Manual sum groups.
#define SF_DECLARE_MEMORY_STAT_SUM_GROUP(id, name, group)       StatDesc SF_STAT_##id(Stat::Stat_Memory, StatDesc::SF_SumGroup, id, name, group);
#define SF_DECLARE_TIMER_STAT_SUM_GROUP(id, name, group)        StatDesc SF_STAT_##id(Stat::Stat_Timer, StatDesc::SF_SumGroup, id, name, group);
#define SF_DECLARE_COUNTER_STAT_SUM_GROUP(id, name, group)      StatDesc SF_STAT_##id(Stat::Stat_Counter, StatDesc::SF_SumGroup, id, name, group);
// Automatically added groups.
#define SF_DECLARE_MEMORY_STAT_AUTOSUM_GROUP(id, name, group)   StatDesc SF_STAT_##id(Stat::Stat_Memory, StatDesc::SF_AutoSumGroup, id, name, group);
#define SF_DECLARE_TIMER_STAT_AUTOSUM_GROUP(id, name, group)    StatDesc SF_STAT_##id(Stat::Stat_Timer, StatDesc::SF_AutoSumGroup, id, name, group);
#define SF_DECLARE_COUNTER_STAT_AUTOSUM_GROUP(id, name, group)  StatDesc SF_STAT_##id(Stat::Stat_Counter, StatDesc::SF_AutoSumGroup, id, name, group);
// Stats.
#define SF_DECLARE_MEMORY_STAT(id, name, group)   StatDesc SF_STAT_##id(Stat::Stat_Memory, 0, id, name, group);
#define SF_DECLARE_TIMER_STAT(id, name, group)    StatDesc SF_STAT_##id(Stat::Stat_Timer, 0, id, name, group);
#define SF_DECLARE_COUNTER_STAT(id, name, group)  StatDesc SF_STAT_##id(Stat::Stat_Counter, 0, id, name, group);




// {stat_data, stat_id} -> Stat interface.

class StatInfo : public Stat
{
public:
    // Abstract interface use to obtain Stat
    class StatInterface
    {
    public:
        virtual ~StatInterface() { }

        virtual StatType        GetType(Stat* p)                               = 0;
        virtual unsigned        GetStatCount(Stat* p)                          = 0;
        virtual void            GetStat(Stat* p, StatValue* pval, unsigned index)  = 0;
        virtual UPInt           GetStatDataSize() const                        = 0;
        // Reset, Clear, Add
        virtual void            Init(Stat* p)                                  = 0;
        virtual void            Add(Stat* p1, Stat* p2)                        = 0;
        virtual void            SetMin(Stat* p1, Stat* p2)                     = 0;
        virtual void            SetMax(Stat* p1, Stat* p2)                     = 0;
        virtual void            Reset(Stat* p)                                 = 0;
    };

private:
    unsigned        StatId;
    StatInterface*  pInterface;
    Stat*           pData;

public:

    StatInfo()
        : StatId(0), pInterface(0), pData(0)
    {}

    StatInfo(unsigned statId, StatInterface* psi, Stat* pdata)
        : StatId(statId), pInterface(psi), pData(pdata)
    { }

    StatInfo(const StatInfo& src)
    {
        StatId      = src.StatId;
        pInterface  = src.pInterface;
        pData       = src.pData;
    }

    bool        IsNull() const                          { return pInterface == 0; }

    // Access individual stat components.
    StatType    GetType() const                         { return pInterface->GetType(pData); }
    unsigned    GetStatCount() const                    { return pInterface->GetStatCount(pData); }
    void        GetStat(StatValue* pval, unsigned index)    { pInterface->GetStat(pData, pval, index); }
    
    void        Add(StatInfo& si2)
    {
        SF_ASSERT(si2.GetType() == GetType());
        pInterface->Add(pData, si2.pData);
    }
    void        SetMin(StatInfo& si2)
    {
        SF_ASSERT(si2.GetType() == GetType());
        pInterface->SetMin(pData, si2.pData);
    }
    void        SetMax(StatInfo& si2)
    {
        SF_ASSERT(si2.GetType() == GetType());
        pInterface->SetMax(pData, si2.pData);
    }
    void        Reset()
    {        
        pInterface->Reset(pData);
    }

    const char* GetName() const { return StatDesc::GetDesc(StatId)->GetName(); }

    
    // Manual interpretation.
    bool        IsMemory() const        { return GetType() == Stat::Stat_Memory; }
    bool        IsTimer() const         { return GetType() == Stat::Stat_Timer; }
    bool        IsCounter() const       { return GetType() == Stat::Stat_Counter; }

    MemoryStat*     ToMemoryStat()  const { SF_ASSERT(IsMemory());  return (MemoryStat*)pData; }
    TimerStat*      ToTimerStat()   const { SF_ASSERT(IsTimer());   return (TimerStat*)pData; }
    CounterStat*    ToCounterStat() const { SF_ASSERT(IsCounter()); return (CounterStat*)pData; }

};


// Template class implementing StatInfo::StatInterface
template<class C>
class StatInfo_InterfaceImpl : public StatInfo::StatInterface
{
    typedef Stat::StatType  StatType;
    typedef Stat::StatValue StatValue;

    StatType        GetType(Stat* p)           { return ((C*)p)->GetStatType(); }
    unsigned        GetStatCount(Stat* p)      { return ((C*)p)->GetStatCount(); }
    void            GetStat(Stat* p, StatValue* pval, unsigned index) { ((C*)p)->GetStat(pval, index); }
    UPInt           GetStatDataSize() const    { return sizeof(C); }
    void            Init(Stat* p)              { ((C*)p)->Init(); }
    void            Add(Stat* p, Stat* p2)     { ((C*)p)->Add((C*)p2); }
    void            SetMin(Stat* p, Stat* p2)  { ((C*)p)->SetMin((C*)p2); }
    void            SetMax(Stat* p, Stat* p2)  { ((C*)p)->SetMax((C*)p2); }
    void            Reset(Stat* p)             { ((C*)p)->Reset(); }
};


// ***** StatBag

// Represents a bag, or collection of statistics data. Any type of Stat can
// be added into the bag; after statistics data is added it can be iterated
// through with Iterator class, obtained from GetIterator. 

class MemoryHeap; 
class StatBag
{

public:

    typedef StatInfo::StatInterface StatInterface;


#ifndef SF_ENABLE_STATS

    // *** Null StatBag

    StatBag(MemoryHeap* = 0, unsigned = 0) { }
    StatBag(const StatBag&) { }
    ~StatBag() { }
    
    void    Clear() { }
    void    Reset() { }
    bool    Add(unsigned, Stat* )                   { return false;                     }
    StatBag& operator += (const StatBag&)       { return *this;                     }
    StatBag& operator  = (const StatBag&)       { return *this;                     }
    void    UpdateGroups() { }

    bool    GetStat(StatInfo *, unsigned) const     { return false;                     }

    inline bool AddStat(unsigned, const Stat&)      { return false;                     }

    // Add/SetMin/SetMax statistic value of a certain id.
    bool    SetMin(unsigned statId, Stat* pstat)    { SF_UNUSED(statId); SF_UNUSED(pstat); return false; }
    bool    SetMax(unsigned statId, Stat* pstat)    { SF_UNUSED(statId); SF_UNUSED(pstat); return false; }

    void    SetMin(const StatBag& other)        { SF_UNUSED(other);                 }
    void    SetMax(const StatBag& other)        { SF_UNUSED(other);                 }

    struct Iterator
    {
        StatInfo   Result;    

        // Default Iterator start points to no data.
        Iterator(StatBag*, unsigned, unsigned) { }
        Iterator(const Iterator&) { }

        const StatInfo&    operator * () const { return Result; }
        const StatInfo*    operator -> () const { return &Result; }
        bool     operator ++ () { return false; }
        unsigned GetId() const  { return StatGroup_Default; }
        bool     operator == (const Iterator&) const { return false; }
        bool     operator != (const Iterator&) const { return true; }
        bool     IsEnd() const { return true; }
    };
    Iterator    GetIterator(unsigned groupId = Stat_MaxId) { return Iterator (this, 0, groupId);}

#else // SF_ENABLE_STATS


    // *** Full StatBag Implementation

    enum
    {
        StatBag_MemGranularity = 8,
        StatBag_PageShift      = 4,
        StatBag_PageSize       = 1 << StatBag_PageShift,
        StatBag_PageTableSize  = Stat_MaxId / StatBag_PageSize,
        StatBag_EndId          = Stat_MaxId,
        
        // Page table entries are set to this value if no memory
        // or slot is allocated to them.
        StatBag_IdUnused        = 0xFFFF
    };

    static StatInterface* GetInterface(unsigned id);
  
private:

    // Allocated memory and pointer of how much of it is left.
    UByte*   pMem;
    unsigned MemSize;
    UPInt    MemAllocOffset;

    // Id page table. 
    UInt16   IdPageTable[StatBag_PageTableSize];

    Stat*    GetStatRef(unsigned statId) const;
    UByte*   AllocStatData(unsigned statId, UPInt size);

    void     RecursiveGroupUpdate(StatDesc::Iterator it);

    // Combines statistics from two bags based on a function.
    void     CombineStatBags(const StatBag& other, bool (StatBag::*func)(unsigned id, Stat*));
    
public:
    // Create a stat bag with specified number of entries.
    StatBag(MemoryHeap* pheap = 0, unsigned memReserve = Stat_EntryCount * 16);
    StatBag(const StatBag& source);

    ~StatBag();

public:

    // Clear out the bag, removing all states.
    void    Clear();
    // Reset stat values.
    void    Reset();


    // Add/SetMin/SetMax statistic value of a certain id.
    bool    Add(unsigned statId, Stat* pstat);
    bool    SetMin(unsigned statId, Stat* pstat);
    bool    SetMax(unsigned statId, Stat* pstat);

    inline bool AddStat(unsigned statId, const Stat& stat)
    {
        return Add(statId, const_cast<Stat*>(&stat));
    }

    // Some optimization for MemoryHeapStat
    bool    AddMemoryStat(unsigned statId, const MemoryStat& stat);    
    bool    IncrementMemoryStat(unsigned statId, UPInt alloc, UPInt use);

    // Compute minimums and maximums of two bags.
    void    Add(const StatBag& other)      { CombineStatBags(other, &StatBag::Add); }
    void    SetMin(const StatBag& other)   { CombineStatBags(other, &StatBag::SetMin); }
    void    SetMax(const StatBag& other)   { CombineStatBags(other, &StatBag::SetMax); }

    // Add values of a different stat bag to ours.
    StatBag& operator += (const StatBag& other) { Add(other); return *this; }
    StatBag& operator = (const StatBag& other)
    {
        if (this == &other)
            return *this;
        Clear();
        Add(other);
        return *this;
    }

    // Update all automatically computed groups in the list.
    void    UpdateGroups();
    
    // Does this accumulate data or not?
    bool    GetStat(StatInfo *pstat, unsigned statId) const;

    
    // *** Iterating Stat Data

    struct Iterator
    {
    protected:        
        unsigned    Id;                 // Current id we are traversing.
        unsigned    GroupId;            // Group id mask.
        StatBag*    pBag;
        StatInfo    Result;             // Cached result for entry

        // Advance to this/next valid item or fail.
        bool        AdvanceTillValid();

    public:
                
        // Default Iterator start points to no data.
        Iterator(StatBag* pbag = 0, unsigned id = Stat_MaxId, unsigned groupId = Stat_MaxId);
        
        Iterator(const Iterator& src)
            :  Id(src.Id), GroupId(src.GroupId), Result(src.Result)
        { }
           

        const StatInfo&    operator * () const { return Result; }
        const StatInfo*    operator -> () const { return &Result; }

        bool    operator ++ ()
        {
            SF_ASSERT(pBag != 0);
            Id++;
            return AdvanceTillValid();
        }
      
        unsigned GetId() const { return Id; }

        bool    operator == (const Iterator& it) const
        {
            return Id == it.Id;           
        }
        bool    operator != (const Iterator& it) const
        {
            return ! (*this == it);
        }

        bool    IsEnd() const
        {
            return (Id >= StatBag_EndId) ;
        }
    };

    // Obtains an Iterator for the specified stat group. Default implementation
    // will return all of the stats in the bag.
    Iterator    GetIterator(unsigned groupId = Stat_MaxId);
   

    // Wait for stats to be ready; useful if stat update
    // request is queued up for update in a separate thread.
    //void    WaitForData();

#endif
};

} // Scaleform

#endif
