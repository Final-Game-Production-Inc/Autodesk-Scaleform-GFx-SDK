/**************************************************************************

Filename    :   FxMinimap.h
Content     :   Minimap interface specific types
Created     :   November 13, 2009
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxMinimap_H
#define INC_FxMinimap_H

#include "FxGameDelegate.h"


//
// The following interfaces are used by the minimap view classes
// and are expected to be implemented by game specific minimap
// entities and environments.
//


// Forward declaration
class FxMinimapIcon;


//
// An entity that is displayed in the minimap. Game entities can implement 
// this interface to provide exposure to the minimap view.
//
class FxMinimapEntity
{
public:  
    enum IconType
    {
        ID_Unknown,
        ID_BluePlayer,
        ID_RedPlayer,
        ID_Flag,        // Capture point
        ID_Objective,   // General objective
        ID_Waypoint,

        NUM_TYPES,
    };

    FxMinimapEntity() : pIconRef(NULL), IconData(0) {}
    virtual ~FxMinimapEntity();

    // Accessors for the icon resource that is displayed in
    // the minimap view. This icon reference provides the view
    // necessary information to detect when the entity enters
    // or leaves the minimap view. This information is useful
    // for fading in/out animations.
    FxMinimapIcon*  GetIconRef() const              { return pIconRef; }
    void            SetIconRef(FxMinimapIcon* pref) { pIconRef = pref; }

    //
    // Extra subtype/payload associated with a type definition:
    //
    // ID_BluePlayer:     N/A
    // ID_RedPlayer:        N/A
    // ID_Flag:         Neutral=0, Blue=1, Red=2
    // ID_Objective:    N/A
    // ID_Waypoint:     Waypoint number
    //
    unsigned            GetIconData() const             { return IconData; }
    void            SetIconData(unsigned data)          { IconData = data; }

    // Accessors for physical properties necessary to calculate
    // virtual position and direction for the icon resource
    // displayed in the minimap. The physical position is expected
    // to be inside the bounds returned by the 
    // FxMinimapEnvironment::GetPhysicalMapBounds() method.
    virtual const PointF&  GetPhysicalPosition() const     = 0;    
    virtual float           GetDirection() const            = 0;

protected:
    FxMinimapIcon*          pIconRef;
    unsigned                    IconData;
};


//
// The environment that is displayed in the minimap. Game application can implement this
// interface to provide exposure to the FxMinimapView.
//
class FxMinimapEnvironment
{
public:
    // Query result for GetEntitiesInPhysicalRadius method.
    struct QueryResult
    {
        FxMinimapEntity*    pEntity;
        // Distance squared is expected; avoids unnecessary sqrt()
        float               DistanceSquared;    
    };

    virtual ~FxMinimapEnvironment() {}

    // Accessors for the properties of the physical map
    virtual const RectF&       GetPhysicalMapBounds() const            = 0;

    // Accessors for the player properties (the center point of the map
    // view)
    virtual const PointF&      GetPhysicalPlayerPosition() const       = 0;
    virtual float               GetPlayerDirection() const              = 0;

    // Query method to retrieve the set of entities of a specific 
    // type in a certain circular view range - based on radial 
    // distance from the player position.
    virtual void                GetEntitiesInPhysicalRadius(Array<QueryResult>* result, 
                                                            FxMinimapEntity::IconType type,
                                                            const PointF& physCenter,  
                                                            float physicalRadius) const = 0;
};


#endif  // INC_FxMinimap_H
