/**************************************************************************

Filename    :   TargetUnitFrameWidget.as

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
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.managers.DragManager;
    
    import com.scaleform.mmo.GameWidgetDefns;
    import com.scaleform.mmo.MMOKit;
    import com.scaleform.mmo.GameDataModel;
    
    public class TargetUnitFrameWidget extends MovieClip {
    
    // Public Properties:
        
    // Protected Properties:
        
    // UI Elements:
        public var frame:TargetUnitFrame;
        
        public function TargetUnitFrameWidget() {
            super();
        }        
    }
}
