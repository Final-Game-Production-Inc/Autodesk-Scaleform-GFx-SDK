/**************************************************************************

Filename    :   HUDMgr.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package {
    
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.FocusEvent;
    import flash.events.MouseEvent;
    import flash.events.TextEvent;
    import flash.external.ExternalInterface;
    import flash.ui.Keyboard;
    import flash.system.fscommand;
    
    import scaleform.clik.constants.InputValue;
    import scaleform.clik.controls.TextInput;
    import scaleform.clik.controls.Button;
    import scaleform.clik.controls.DropdownMenu;
    import scaleform.clik.controls.Label;
    import scaleform.clik.controls.Slider;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.events.ComponentEvent;
    import scaleform.clik.events.InputEvent;
    import scaleform.clik.events.ListEvent;
    import scaleform.clik.events.SliderEvent;
    import scaleform.clik.interfaces.IDataProvider;
    import scaleform.clik.ui.InputDetails;
    
    import scaleform.gfx.Extensions;
    
    public class HUDMgr extends UIComponent 
    {
    // Public Properties:
        public var m_oldUIToggle:Boolean = true;
        
    // UI Elements:
        public var hud:d3d9HUD;
        //public var settings:d3d9Settings;
        
        public function HUDMgr() {
            // Click callbacks for buttons
            hud.button_Fullscreen.addEventListener( MouseEvent.CLICK, toggleFullscreen, false, 0, true );
            // hud.button_ChangeDevice.addEventListener( MouseEvent.CLICK, toggleSettings, false, 0, true  );
            hud.button_ToggleUI.addEventListener( MouseEvent.CLICK, toggleUI, false, 0, true  );
            
            // Populate Dropdown Menus
            //settings.visible = false;
            
            // Listen for keyboard input.
            addEventListener( InputEvent.INPUT, handleInput, false, 0, true  );
            
            // Give hudMgr focus
            focused = 1;
        }
        
        override public function handleInput( ie:InputEvent ):void {
            super.handleInput( ie );
            
            if ( ie.details.value == "keyDown" ) {
                switch( ie.details.code ) {
                    // case Keyboard.TAB:  
                        // toggleSettings(null);
                        // break;
                    case Keyboard.SPACE:
                        toggleUI(null);
                        break;
                    default:
                        break;
                }
            }
        }
        
        //--------------------------------------------------
        // Button Callback Functions
        public function toggleFullscreen(e:MouseEvent) {
            fscommand("ToggleFullscreen", String(hud.visible));
        }
        
        // public function toggleSettings(e:MouseEvent) {
            // if (settings) {
                // // Toggle visibility of settings
                // settings.visible = !settings.visible;
                
                // if (settings.visible) {
                    // // Save current state of UI visibility and hide hud if settings are visible
                    // m_oldUIToggle = hud.visible;
                    // hud.visible = !settings.visible;
                // } else {
                    // // Restore visibility of UI if hiding settings
                    // hud.visible = m_oldUIToggle;
                // }
            // }
        // }
        
        public function toggleUI(ev:MouseEvent) {
            hud.visible = !hud.visible;
        }
    }
}