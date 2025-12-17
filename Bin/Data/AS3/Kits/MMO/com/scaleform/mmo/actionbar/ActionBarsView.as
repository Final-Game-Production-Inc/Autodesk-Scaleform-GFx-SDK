/**************************************************************************

Filename    :   ActionBarsView.as
Content    :   
                


Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo.actionbar {
    
    import flash.display.MovieClip;
    import flash.events.Event;
    
    import scaleform.clik.data.DataProvider;
       
    public class ActionBarsView extends MovieClip {
    
    // Public Properties:
        
    // Protected Properties:
        
    // UI Elements:
        public var actionBar0:PrimaryActionBar;
        public var actionBar1:ActionBar;
        public var actionBar2:ActionBar;
        
        public function ActionBarsView() {
            super();
            addEventListener(Event.ENTER_FRAME, configUI, false, 0, true);
        }
        
        protected function configUI(e:Event):void {
            removeEventListener(Event.ENTER_FRAME, configUI, false);
                        
            // ACTION BAR 1
            actionBar0 = new PrimaryActionBar();
            actionBar0.name = "actionbar0";
            actionBar0.x = (stage.stageWidth / 2) - (actionBar0.width / 2);
            actionBar0.y = (stage.stageHeight - actionBar0.height + 30); // 10 == extra buffer.
            addChild(actionBar0);
            
            // ACTION BAR 2
            actionBar1 = new ActionBar();
            actionBar1.name = "actionbar1";
            actionBar1.stageRef = stage;
            actionBar1.dataProvider = createActionBarBindings(1, 12);
            actionBar1.totalColumnsOverride = 12;
            actionBar1.totalRowsOverride = 1;
            actionBar1.validateNow();
            actionBar1.x = actionBar0.x + 20; // 20 is to center it above 0.
            actionBar1.y = (actionBar0.y - actionBar1.height - 5 - 60); // -30 is actionBar0's screwed up postion. == extra buffer.          
            addChild(actionBar1);
            
            
            // ACTION BAR 3
            actionBar2 = new ActionBar();
            actionBar2.name = "actionbar2";
            actionBar2.stageRef = stage;
            actionBar2.dataProvider = createActionBarBindings(2, 8);
            actionBar2.totalColumnsOverride = 1;
            actionBar2.totalRowsOverride = 8;
            actionBar2.validateNow();
            actionBar2.x = (stage.stageWidth) - (actionBar2.getWidth()) - 10;
            actionBar2.y = (stage.stageHeight / 2) - (actionBar2.getHeight() / 2);
            addChild(actionBar2);
        }
		
		protected function createActionBarBindings(barIdx:uint, numSlots:uint):DataProvider {
			var bindings:Array = [];
			for (var i:uint = 0; i < numSlots; i++) {
				bindings.push("actionbar" + barIdx + ".slot" + i);
			}
			return new DataProvider(bindings);
		}
    }
}
