package com.scaleform.inventory 
{
	// Imports
	import adobe.utils.CustomActions;
	import flash.display.MovieClip;
	import scaleform.clik.controls.Button
	
	public class InventoryButton extends Button 
	{
		// Stage Objects
		public var icon:MovieClip;
		
		// Member Variables
		public var tfContainer:MovieClip;
		
		
        /** Override for setActualSize that doesn't change the size. Allows for Button's to be tweened on the timeline. */
		override public function setActualSize(newWidth:Number, newHeight:Number):void { 
        	//
		}
		
		override protected function setState(state:String):void {
			super.setState(state);
			
			if (icon != null) {
				if(state == "up" || state == "disabled" || state == "out" || state == "kb_relese")
					icon.gotoAndPlay("up");
				else if (state == "over" || state == "down" || state == "release" || state == "kb_down")
					icon.gotoAndPlay("over");
			}
			else {
				trace("Error: InventoryButton::setState, icon does not exist!");
			
			}
		}
	}
	
}