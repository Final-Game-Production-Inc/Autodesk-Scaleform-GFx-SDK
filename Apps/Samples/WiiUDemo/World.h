/**************************************************************************

Filename    :   World.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_World_H
#define INC_World_H

#include "Kernel/SF_String.h"
#include "Kernel/SF_Hash.h"
using namespace Scaleform;


//
// Battle map types
//
enum MapType
{
    MAP_Forest,
    MAP_Town,
    MAP_Castle,
    MAP_Ruins,
    MAP_Pass,
};


//
// Definition of a location in the world map
//
struct WorldLocation 
{
    String      Name;
    String      Description;
    MapType     Type;
    bool        Locked;
    bool        Completed;

    WorldLocation() {}
    WorldLocation(const char* name, const char* desc, MapType type, bool locked, bool completed) :
        Name(name), Description(desc), Type(type), Locked(locked), Completed(completed) {}

    static const char*     ToString(MapType type)
    {
        switch (type)
        {
        case MAP_Forest:    return "forest";
        case MAP_Town:      return "town";
        case MAP_Castle:    return "castle";
        case MAP_Ruins:     return "ruins";
        case MAP_Pass:      return "pass";
        }
        return "???";
    }
};


//
// The world map
//
class World
{
public:
    WorldLocation*  GetLocation(const String& name) const;
    void            AddLocation(const WorldLocation& loc);

protected:
    mutable Hash<String, WorldLocation, String::HashFunctor>    Locations;
};


#endif
