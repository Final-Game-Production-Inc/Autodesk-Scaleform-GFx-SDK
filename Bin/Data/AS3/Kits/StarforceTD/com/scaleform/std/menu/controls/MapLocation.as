/**************************************************************************

Filename    :   MapLocation.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.menu.controls {

	import com.scaleform.std.system.SoundSystem;
    import flash.display.DisplayObject;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.events.TouchEvent;
    import flash.text.TextField;
	
	import scaleform.clik.events.ButtonEvent;
    
    import scaleform.gfx.FocusEventEx;
    import scaleform.gfx.MouseEventEx;
    
    import com.scaleform.std.utils.Constants;
    import com.scaleform.std.controls.SBButton;
    
    public class MapLocation extends SBButton {
        
    // Constants:
        public static const STARS_FRAME_GALAXY:String = "galaxy";
        public static const STARS_FRAME_NEBULA:String = "nebula";
        
        public static const STAR_ANIM_LABEL_OPEN:String = "on";
        public static const STAR_ANIM_LABEL_CLOSE:String = "off";
        
    // Public Properties:
        
    // Protected Properties:
        
    // UI Elements:
        public var LocCircle:MovieClip;
		public var LocCircleBlink:MovieClip;
    
        public var LocText:MovieClip;
        public var LT_TextField:TextField;      // LocText.textField;
        
        public var LT_Star:MovieClip;           // LocText.StarPoints
        public var LT_Star_TextField:TextField  // LocText.StarPoints.textField
        public var LT_Star_Star1:MovieClip ;    // LocText.StarPoints.Star1
        public var LT_Star_Star2:MovieClip;     // LocText.StarPoints.Star2
        public var LT_Star_Star3:MovieClip;     // LocText.StarPoints.Star3

    // Initialization:
        public function MapLocation() {
            super();
            
            LT_TextField = LocText.textField;
            LT_Star = LocText.StarPoints;
			
			pressSound = "MapButtonPress.mp3";
        }
        
    // Public Getter / Setters:
        public function setData( label:String, isGalaxy:Boolean, numStars:int, maxStars:int ):void {
            LT_Star.gotoAndPlay( isGalaxy ? STARS_FRAME_GALAXY : STARS_FRAME_NEBULA );
            updateGraphicRefs();
            
            if ( isGalaxy ) {
                LT_Star_TextField.text = numStars + "/" + maxStars;
            } 
            else {
                switch ( numStars ) {
                    case 3:
                        LT_Star_Star3.gotoAndPlay( STAR_ANIM_LABEL_OPEN );
                    case 2:
                        LT_Star_Star2.gotoAndPlay( STAR_ANIM_LABEL_OPEN );
                    case 1:
                        LT_Star_Star1.gotoAndPlay( STAR_ANIM_LABEL_OPEN );
                    default:
                        break;
                }
                
            }
            
            LT_TextField.text = label;
            LocText.rotationX = 50;
        }

    // Public Functions
        /** @exclude */
        override public function toString():String { 
            return "[SB MapLocation" + name + "]";
        }
        
    // Protected Methods:
        override protected function configUI():void {
            super.configUI();
            
        }
 
        protected function updateGraphicRefs():void {
            // Galaxy
            LT_Star_TextField = LocText.StarPoints.textField;
            
            // Nebula
            LT_Star_Star1 = LocText.StarPoints.Star1;
            LT_Star_Star2 = LocText.StarPoints.Star2;
            LT_Star_Star3 = LocText.StarPoints.Star3;
        }
       
    }
}