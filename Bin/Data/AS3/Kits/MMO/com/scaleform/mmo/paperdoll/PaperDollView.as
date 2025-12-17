/**************************************************************************

Filename    :   PaperDollView.as
Content    :   
                

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo.paperdoll {
    
    import flash.display.Sprite;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.external.ExternalInterface;
    
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.managers.DragManager;
    
    import com.scaleform.mmo.GameDataModel;
    
    public class PaperDollView extends MovieClip {
    
    // Public Properties:
        
    // Protected Properties:
        
    // UI Elements:
        public var paperDoll:PaperDoll;
        
        public function PaperDollView() {
            super();
            addEventListener(Event.ENTER_FRAME, configUI, false, 0, true);
        }
        
        protected function configUI(e:Event):void {
            removeEventListener(Event.ENTER_FRAME, configUI, false);
        }
    }
}
