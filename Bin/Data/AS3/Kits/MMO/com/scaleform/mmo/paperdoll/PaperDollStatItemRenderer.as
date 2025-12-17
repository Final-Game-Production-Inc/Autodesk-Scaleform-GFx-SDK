/**************************************************************************

Filename    :   PaperDollStatItemRenderer.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo.paperdoll {
    
    import flash.display.Sprite;
    import flash.events.FocusEvent;
    import flash.text.TextField;
    
    
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.data.ListData;
    import scaleform.clik.controls.ListItemRenderer;
    import scaleform.clik.core.InvalidationType;
    
    public class PaperDollStatItemRenderer extends ListItemRenderer {
        
    // Constants:
        
    // Public Properties:
        
    // Protected Properties:
        protected var _statLines:Array;
        
    // UI Elements:
        protected var _canvas:Sprite;
        
    // Initialization:
        public function PaperDollStatItemRenderer() {
            super();
            
            _statLines = [];
            
            _canvas = new Sprite();
            addChild(_canvas);
        }
        
    // Public getter / setters:
        
    // Public Methods:
        override protected function draw():void {
            if (isInvalid(InvalidationType.DATA)) {
                /*
                var ypos:int = 0;
                var statData:Array = [ { label:"Strength",  val:100 },
                                       { label:"Wisdom",    val:100 },
                                       { label:"Luck",      val:100 },
                                       { label:"Willpower", val:100 } ]
                
                for (var i:uint = 0; i < statData.length; i++) {
                    var statLine:PaperDollStatLine = new PaperDollStatLine();
                    
                    statLine.statName = statData[i].label;
                    statLine.statVal = statData[i].val;
                    statLine.y = ypos;
                    ypos += 16;
                    
                    _canvas.addChild(statLine);
                    _statLines.push(statLine);
                }
                */
            }
        }
        
        override public function toString():String {
            return "[CLIK PaperDollStatItemRenderer " + index + ", " + name + "]";
        }
        
    // Protected Methods:
        
    }
}