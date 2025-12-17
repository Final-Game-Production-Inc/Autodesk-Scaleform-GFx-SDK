/**************************************************************************

Filename    :   d3d9HUD.as

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
    import scaleform.clik.controls.TextInput;
    import scaleform.clik.core.UIComponent;
    
    
    import scaleform.clik.constants.InputValue;
    import scaleform.clik.controls.Button;
    import scaleform.clik.controls.DropdownMenu;
    import scaleform.clik.controls.Label;
    import scaleform.clik.controls.Slider;
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.events.ComponentEvent;
    import scaleform.clik.events.InputEvent;
    import scaleform.clik.events.ListEvent;
    import scaleform.clik.events.SliderEvent;
    import scaleform.clik.interfaces.IDataProvider;
    import scaleform.clik.ui.InputDetails;
    import scaleform.clik.events.FocusHandlerEvent;
    
    import scaleform.gfx.Extensions;

    public class d3d9HUD extends UIComponent 
    {
    // Static Properties:
        // Hard coded luminance values
        public static var s_LuminanceMin:Number = 0.1;
        public static var s_LuminanceMax:Number = 40.0;
        
    // Public Properties:
        public var BackgroundData:Array;
        public var LightsData:Array;
        public var SceneData:Array;
        
    // UI Elements:
        public var button_Fullscreen:Button;
        public var button_ChangeDevice:Button;
        public var button_ToggleUI:Button;
        public var button_ChangeMesh:Button;
        
        public var slider_Luminance:Slider;
        public var label_Luminance:Label;
        
        public var combo_Background:MovieClip;
        public var combo_Lights:MovieClip;
        public var combo_Scene:MovieClip;
        
        public var meshPath:MovieClip;
        public var text_MeshPath:TextInput;

        public function d3d9HUD() {
            // Setup change mesh input handling for button & text box
            button_ChangeMesh.addEventListener( MouseEvent.CLICK, changeMesh, false, 0, true );
            
            text_MeshPath = meshPath.text_MeshPath;
            text_MeshPath.addEventListener( FocusEvent.KEY_FOCUS_CHANGE, onSubmit, false, 0, true );
            text_MeshPath.addEventListener( FocusHandlerEvent.FOCUS_IN, onFocusIn, false, 0, true );
            text_MeshPath.addEventListener( FocusHandlerEvent.FOCUS_OUT, onFocusOut, false, 0, true );
            
            // Setup luminance slider
            slider_Luminance.addEventListener( SliderEvent.VALUE_CHANGE, changeLuminance, false, 0, true  );
            slider_Luminance.minimum = s_LuminanceMin;
            slider_Luminance.maximum = s_LuminanceMax;
            slider_Luminance.value = (s_LuminanceMax - s_LuminanceMin) / 2;
            setLuminance(slider_Luminance.value);
            
            // Setup dropdowns once menu is initialized
            addEventListener( Event.ENTER_FRAME, updateDropdowns, false, 0, true );
                
            // Setup combo box interactions
            combo_Background.dropdown.addEventListener(ListEvent.INDEX_CHANGE, changeBackground);
            combo_Lights.dropdown.addEventListener(ListEvent.INDEX_CHANGE, changeLights);
            combo_Scene.dropdown.addEventListener(ListEvent.INDEX_CHANGE, changeScene);
        }
        
        public function updateDropdowns(ev:Event) {
            var updateNeeded:Boolean = false;
            var parentMC:MovieClip = ((parent as MovieClip).parent as MovieClip);
            if (parentMC != null) {
                // Check to see if root data has been changed by game and needs updating
                if (BackgroundData == null || BackgroundData != parentMC.BackgroundData) {
                    BackgroundData = parentMC.BackgroundData;
                    updateNeeded = true;
                } if (LightsData == null || LightsData != parentMC.LightsData) {
                    LightsData = parentMC.LightsData;
                    updateNeeded = true;
                } if (SceneData == null || SceneData != parentMC.SceneData) {
                    SceneData = parentMC.SceneData;
                    updateNeeded = true;
                }
                
                if (updateNeeded) {
                    // Populate Background Dropdown Menu
                    var myArray:Array = (parentMC && parentMC.BackgroundData != null) ? 
                                            parentMC.BackgroundData : null;
                    PopulateComboBox(combo_Background, "", myArray);
                    
                    // Populate Lights Dropdown Menu
                    myArray = (parentMC && parentMC.LightsData != null) ? parentMC.LightsData : null;
                    PopulateComboBox(combo_Lights, "", myArray);
                    
                    // Populate Scene Dropdown Menu
                    myArray = (parentMC && parentMC.SceneData != null) ? parentMC.SceneData : null;
                    PopulateComboBox(combo_Scene, "", myArray);
                }
            }
        }
        
        function PopulateComboBox(comboMC:MovieClip, label:String, dataArray:Array) {
            // Set Label and get Dropdown Menu
			
            comboMC.label.text = label;
            var dropdown:DropdownMenu = comboMC.dropdown;
            
            if ( dataArray.length == 0 ) {
                dataArray = [ { label:"No Data" } ];
            }
            
            // Set dropdowns data provider and initialize to first index
            dropdown.dataProvider = new DataProvider( dataArray );
            dropdown.selectedIndex = 0;
        }
        
        // Button Callback Functions
        function changeMesh(ev:MouseEvent) {
            // Toggle mesh edit box
            if (text_MeshPath.visible == false) {
                OpenMeshPath();
                text_MeshPath.textField.text = "Type mesh name and press Enter";
                text_MeshPath.focused = 1;
            }
            else {
                CloseMeshPath();
            }
        }
        
        public function OpenMeshPath() {
            // Show edit box for mesh path
            meshPath.gotoAndPlay("on");
            button_ChangeMesh.label = "Cancel";	
        }

        public function CloseMeshPath() {
            // Hid edit box for mesh path
            meshPath.gotoAndPlay("off");
            button_ChangeMesh.focused = 1;
            button_ChangeMesh.label = "Change Mesh";
        }
        
        function onSubmit(path:String) {
            ExternalInterface.call("MeshPath", path);
            CloseMeshPath();
        }
        
        function onFocusIn(ev:FocusHandlerEvent) {
            ExternalInterface.call("MeshPathFocus", true);
        }

        function onFocusOut(ev:FocusHandlerEvent) {
            ExternalInterface.call("MeshPathFocus", false);
        }
        
        function changeLuminance(ev:Event) {
            setLuminance(slider_Luminance.value);
        }
        
        function setLuminance(lum:Number) {
            // get luminance as a percent of the luminance range and send to code
            lum = Math.round(lum); 
            lum = (lum - s_LuminanceMin) / (s_LuminanceMax - s_LuminanceMin); 
            ExternalInterface.call("Luminance", lum);
        }

        function changeBackground(v:Event) {
            ExternalInterface.call("NewBackground", (v.target.selectedIndex as Number));
        }

        function changeLights(v:Event) {	
            ExternalInterface.call("NewLight", v.target.selectedIndex);	
        }

        function changeScene(v:Event) {
            ExternalInterface.call("NewScene", v.target.selectedIndex);	
        }
        //---------------------------------------------------
    }
}