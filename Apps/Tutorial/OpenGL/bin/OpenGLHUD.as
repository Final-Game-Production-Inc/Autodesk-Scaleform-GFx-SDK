/**************************************************************************

Filename    :   OpenGLHUD.as

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
	import scaleform.clik.controls.CheckBox;
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.events.ComponentEvent;
    import scaleform.clik.events.InputEvent;
    import scaleform.clik.events.ListEvent;
    import scaleform.clik.events.SliderEvent;
    import scaleform.clik.interfaces.IDataProvider;
    import scaleform.clik.ui.InputDetails;
    import scaleform.clik.events.FocusHandlerEvent;
    
    import scaleform.gfx.Extensions;

    public class OpenGLHUD extends UIComponent 
    {

    // UI Elements:
        public var button_Quit:Button;
        public var button_Pause:Button;

		
		public var cb_Wireframe:CheckBox;
		public var cb_ControlMesh:CheckBox; 
		public var cb_Teapot:CheckBox; 
		
        public function OpenGLHUD() {
            
			cb_Wireframe.addEventListener(Event.SELECT, toggleWireframe);
			cb_ControlMesh.addEventListener(Event.SELECT, toggleControlMesh);
			cb_Teapot.addEventListener(Event.SELECT, toggleTeapot);
			
			button_Pause.addEventListener( MouseEvent.CLICK, togglePause);

        }
        
        public function updateDropdowns(ev:Event) {
            var updateNeeded:Boolean = false;
            var parentMC:MovieClip = ((parent as MovieClip).parent as MovieClip);

        }
        function toggleWireframe(ev:Event){
			ExternalInterface.call("Wireframe", cb_Wireframe.selected);
		}
		
		function toggleControlMesh(ev:Event){
			ExternalInterface.call("ControlMesh", cb_ControlMesh.selected);
		}
		
		function toggleTeapot(ev:Event){
			ExternalInterface.call("Teapot", cb_Teapot.selected);
		}
		
		function togglePause(ev:Event){
			ExternalInterface.call("Pause");
		}
		

    }
}