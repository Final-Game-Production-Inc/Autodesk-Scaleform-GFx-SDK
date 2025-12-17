/**************************************************************************

Filename    :   PlanetDataDisplay.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.menu
{
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.events.TouchEvent;
    import flash.text.TextField;
    
    import com.scaleform.std.controls.SBButton;

    public class PlanetDataDisplay extends MovieClip 
    {
        // PlanetIcon and PlanetPath Labels
        public static const LABEL_PLANET_LEVEL1:String = "level1";
        public static const LABEL_PLANET_LEVEL2:String = "level2";
        public static const LABEL_PLANET_LEVEL3:String = "level3";
        public static const LABEL_PLANET_LEVEL4:String = "level4";
        public static const LABEL_PLANET_LEVEL5:String = "level5";
        
        // UI Scene
        public var PopupTitleText:MovieClip; // The textfield on the top right that displays the planet's name.
        public var PopupInfoText:MovieClip; // The textfield on the bottom right that displays a planet's description.
        
        public var PopupStatText:MovieClip; // The textfield on the bottom left that displays the labels and text for the planet's stats.
        
        public var PlanetIcon:MovieClip; // The picture of the planet on the left. Change via gotoAndPlay().
        public var PlanetPath:MovieClip; // The path of the planet on the left. Change via gotoAndPlay().
        
        public function PlanetDataDisplay() {
            super();
        }
        
        public function setTitle( val:String ):void {
            PopupTitleText.titleText.text = val;
        }
        public function setInfo( val:String ):void {
            PopupInfoText.infoText.text = val;
        }
        public function setLabels( val:String ):void {
            PopupStatText.statLabel.text = val;
        }
        public function setStats( val:String ):void {
            PopupStatText.statText.text = val;
        }
        
    }

}