/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_StringHash.h
Content     :   String hash table used when optional case-insensitive
                lookup is required.
Created     :
Authors     :

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_StringHash_H
#define INC_SF_GFX_StringHash_H

#include "Kernel/SF_StringHash.h"

namespace Scaleform { namespace GFx {

//@TODO: REMOVE COMPLETELY

/*
// StringHash... classes have been renamed to StringHash... classes and moved to GKernel
// These classes are here only for backward compatibility


template<class U, class Allocator = AllocatorGH<U> >
class StringHash : public StringHash<U, Allocator>
{
    typedef StringHash<U, Allocator>                                 SelfType;
    typedef Hash<String, U, String::NoCaseHashFunctor, Allocator>    BaseType;
public:    

    void    operator = (const SelfType& src) { BaseType::operator = (src); }
};

template<class U>
class GFxStringHash_sid : public SF::StringHash_sid<U>
{
    typedef GFxStringHash_sid<U>      SelfType;
    typedef StringHash<U>            BaseType;
public:    

    void    operator = (const SelfType& src) { BaseType::operator = (src); }
};


template<class U, int SID = Stat_Default_Mem,
         class HashF = String::NoCaseHashFunctor,         
         class HashNode = SF::StringLH_HashNode<U,HashF>,
         class Entry = GHashsetCachedNodeEntry<HashNode, typename HashNode::NodeHashF> >
class StringHashLH : public SF::StringHashLH<U, SID, HashF, HashNode, Entry>
{
public:
    typedef StringHashLH<U, SID, HashF, HashNode, Entry>   SelfType;

public:    

StringHashLH() : SF::StringHashLH<U,SID,HashF,HashNode,Entry>()  {  }
    StringHashLH(int sizeHint) : SF::StringHashLH<U,SID,HashF,HashNode,Entry>(sizeHint)  { }
    StringHashLH(const SelfType& src) : SF::StringHashLH<U,SID,HashF,HashNode,Entry>(src) { }
    ~StringHashLH()    { }

    void    operator = (const SelfType& src) { SF::StringHashLH<U,SID,HashF,HashNode,Entry>::operator = (src); }
};
*/
}} // Scaleform::GFx

#endif
