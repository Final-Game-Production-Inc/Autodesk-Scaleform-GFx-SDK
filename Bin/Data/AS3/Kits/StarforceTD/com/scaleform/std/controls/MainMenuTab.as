/**************************************************************************

Filename    :   MainMenuTab.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.controls {
    
    import flash.display.DisplayObject;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.events.TouchEvent;
    import flash.text.TextField;
    
    import scaleform.gfx.FocusEventEx;
    import scaleform.gfx.MouseEventEx;
    
    import com.scaleform.std.utils.Constants;
    
    public class MainMenuTab extends SBButton {
        
    // Constants:
        public static const LABEL_AVAILABLESP_ON:String = "on";
        public static const LABEL_AVAILABLESP_OFF:String = "off";
        
    // Public Properties:
        
    // Protected Properties:
        
    // UI Elements:
        public var AvailableSP:MovieClip;
        public var AvailableSPTextField:TextField;

    // Initialization:
        public function MainMenuTab() {
            super();
            AvailableSPTextField = AvailableSP.SPLarger.Star.textField;
        }
        
    // Public Getter / Setters:
        public function setAvailableSP( val:Number ):void {
            AvailableSPTextField.text = String( val );
            AvailableSP.visible = (val > 0);
        }
    
    // Public Functions
        /** @exclude */
        override public function toString():String { 
            return "[SB MainMenuTab " + name + "]";
        }
        
    // Protected Methods:
       
    }
}