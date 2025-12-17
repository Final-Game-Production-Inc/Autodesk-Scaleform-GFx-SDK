/**************************************************************************

Filename    :   AllyInfantry.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.entities
{
    import flash.display.MovieClip;
    import flash.geom.Point;
    import flash.events.Event;
    
    import fl.motion.easing.*;
    
    import com.scaleform.std.levels.Level;
    import com.scaleform.std.utils.Utils;
    import com.scaleform.std.events.EnemyEvent;
    import com.scaleform.std.events.AttackEvent;
    
    import scaleform.clik.motion.Tween;
    
    public class AllyInfantry extends Infantry
    {
        
        public function AllyInfantry( level:Level ) {
            super( level );
        }
        
        public function upgrade():void {
            grade++;
            configStats();
            updateGraphics();
        }
        
        public function setGrade( g:uint ):void {
            grade = g;
            configStats();
            updateGraphics();
        }
        
        override public function toString():String {
            return "[SB AllyInfantry " + name + "]";
        }
        
    }

}