/**************************************************************************

Filename    :   PlayerUnitFrame.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo.unitframes {
    
    import com.scaleform.mmo.events.WidgetEvent;
    import flash.display.Sprite;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.external.ExternalInterface;
    import flash.text.TextField;
    
    import scaleform.clik.controls.Button;
    import scaleform.clik.events.ButtonEvent;
    import scaleform.clik.data.DataBinding;
    
    import com.scaleform.mmo.GameWidgetDefns;
    import com.scaleform.mmo.MMOKit;
    
    public class PlayerUnitFrame extends MovieClip {
    
    // Public Properties:
        public var tf_Name:TextField;
        public var tf_Level:TextField;
        public var unitType:MovieClip;
        public var unitHealthBar:MovieClip;
        public var unitManaBar:MovieClip;
    
    // Protected Properties:
        
    // UI Elements:
        public function PlayerUnitFrame() {
            super();
            configUI();
        }
        
        protected function configUI():void {           
            DataBinding.createBinding("player.name", setName, this);
            DataBinding.createBinding("player.level", setLevel, this);
        }
        
        protected function setName(value:String):void {
            tf_Name.text = value;
        }
        protected function setLevel(value:String):void {
            tf_Level.text = value;
        }
    }
}
