/**************************************************************************

Filename    :   TowerBuildSite.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.entities
{
    import flash.display.DisplayObject;
    import flash.display.MovieClip;
    import flash.display.Sprite;
    import flash.events.Event;
    import flash.geom.Point;
    import flash.display.CapsStyle;
    
    import com.scaleform.std.utils.Utils;
    import com.scaleform.std.levels.Level;
    
    import scaleform.clik.motion.Tween;
    
    public class TowerBuildSite extends Entity
    {
        public function TowerBuildSite( level:Level = null ) {
            super();
            _level = level;
        }
        
        override protected function configUI():void {
            super.configUI();
        }
        
        override public function toString():String {
            return "[TowerBuildSite " + this.name + "]";
        }
    }

}