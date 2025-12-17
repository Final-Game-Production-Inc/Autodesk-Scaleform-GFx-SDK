/**************************************************************************

Filename    :   d3d9Settings.as

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
    import flash.system.fscommand;
    import flash.text.TextField;
    import flash.ui.Keyboard;
    
    import scaleform.clik.constants.InputValue;
    import scaleform.clik.controls.CheckBox;
    import scaleform.clik.controls.RadioButton;
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

    public class d3d9Settings extends UIComponent 
    {
        // Public Properties
        public var APIVersionData:Array;
        public var DisplayAdapterData:Array;
        public var RenderDeviceData:Array;
        public var AdapterFormatData:Array;
        public var ResolutionData:Array;
        public var RefreshRateData:Array;
        public var BackBufferFormatData:Array;
        public var DepthStencilFormatData:Array;
        public var MultisampleTypeData:Array;
        public var MultisampleQualityData:Array;
        public var VertexProcessingData:Array;
        public var VerticalSyncData:Array;
        
        // UI Elements
        public var title:TextField;
        
        public var combo_APIVersion:MovieClip;
        public var combo_DisplayAdapter:MovieClip;
        public var combo_RenderDevice:MovieClip;
        public var combo_AdapterFormat:MovieClip;
        public var combo_Resolution:MovieClip;
        public var combo_RefreshRate:MovieClip;
        public var combo_BackBufferFormat:MovieClip;
        public var combo_DepthStencilFormat:MovieClip;
        public var combo_MultisampleType:MovieClip;
        public var combo_MultisampleQuality:MovieClip;
        public var combo_VertexProcessing:MovieClip;
        public var combo_VerticalSync:MovieClip;
        
        public var button_OK:Button;
        public var button_Cancel:Button;
        
        public var radio_Windowed:RadioButton;
        public var radio_FullScreen:RadioButton;
        
        public var checkbox_ClipToDevice:CheckBox;
        public var bg:MovieClip;
        
        public function d3d9Settings() {
            button_OK.addEventListener( MouseEvent.CLICK, handleOkClick, false, 0, true );
            button_Cancel.addEventListener( MouseEvent.CLICK, handleCancelClick, false, 0, true );
            
            addEventListener( Event.ENTER_FRAME, updateDropdowns, false, 0, true );
        }
        
        override protected function configUI():void {
            // Prevent the background from receiving mouse input.
            bg.mouseEnabled = bg.mouseChildren = bg.tabEnabled = false;
            
            // Set labels for buttons, radio buttons & checkbox
            button_OK.label = "OK";
            button_Cancel.label = "Cancel";
            
            radio_Windowed.label = "Windowed";
            radio_FullScreen.label = "Full Screen";

            checkbox_ClipToDevice.label = "Clip to device when window spans across multiple monitors";
        }
        
        public function handleOkClick( e:Event ) {
            ExternalInterface.call("ApplyOptions");
            (parent as MovieClip).toggleSettings( e );
        }
        
        public function handleCancelClick( e:Event ) {
            (parent as MovieClip).toggleSettings( e );
        }
        
        public function updateDropdowns(ev:Event) {
            // Set all combobox data to a single item of 'No Data' since we aren't grabbing data from code.
            APIVersionData = DisplayAdapterData = RenderDeviceData =
            AdapterFormatData = ResolutionData = RefreshRateData =
            BackBufferFormatData = DepthStencilFormatData = MultisampleTypeData = 
            MultisampleQualityData = VertexProcessingData = VerticalSyncData = [ { label:"No Data" } ];
            
            // Setup each combobox on settings screen.
            var parentMC:MovieClip = ((parent as MovieClip).parent as MovieClip);
            updateComboBox(APIVersionData, parentMC.APIVersionData, combo_APIVersion, "API Version");
            updateComboBox(DisplayAdapterData, parentMC.DisplayAdapterData, combo_DisplayAdapter, "Display Adapter");
            updateComboBox(RenderDeviceData, parentMC.RenderDeviceData, combo_RenderDevice, "Render Device");
            updateComboBox(AdapterFormatData, parentMC.AdapterFormatData, combo_AdapterFormat, "Adapter Format");
            updateComboBox(ResolutionData, parentMC.ResolutionData, combo_Resolution, "Resolution");
            updateComboBox(RefreshRateData, parentMC.RefreshRateData, combo_RefreshRate, "Refresh Rate");
            updateComboBox(BackBufferFormatData, parentMC.BackBufferFormatData, combo_BackBufferFormat, "Back Buffer Format");
            updateComboBox(DepthStencilFormatData, parentMC.DepthStencilFormatData, combo_DepthStencilFormat, "Depth/Stencil Format");
            updateComboBox(MultisampleTypeData, parentMC.MultisampleTypeData, combo_MultisampleType, "Multisample Type");
            updateComboBox(MultisampleQualityData, parentMC.MultisampleQualityData, combo_MultisampleQuality, "Multisample Quality");
            updateComboBox(VertexProcessingData, parentMC.VertexProcessingData, combo_VertexProcessing, "Vertex Processing");
            updateComboBox(VerticalSyncData, parentMC.VerticalSyncData, combo_VerticalSync, "Vertical Sync");
        }
        
        function updateComboBox(locArray:Array, parArray:Array, comboMC:MovieClip, label:String) {
            var updateNeeded:Boolean = false;
            
            // only update the first time through since we are hard coding the dropdowns in AS
            if (comboMC.label.text == null) {
                updateNeeded = true;
            }
            
            if ( updateNeeded ) {
                // Populate Background Dropdown Menu
                var myArray:Array = (locArray != null) ? locArray : null;
                
                // Populate Dropdown Menu
                comboMC.label.text = label;
                var dropdown:DropdownMenu = comboMC.dropdown;
                
                if ( myArray == null ) { 
                    myArray = [ { label:"No Data"} ]; 
                }
                
                // Set data provider and initialize to first index.
                dropdown.dataProvider = new DataProvider( myArray );
                dropdown.selectedIndex = 0;
            }
        }
    }
}