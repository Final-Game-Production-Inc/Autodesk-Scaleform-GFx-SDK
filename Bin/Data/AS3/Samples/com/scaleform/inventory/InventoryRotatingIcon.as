package com.scaleform.inventory {
	
	// Imports
	import flash.display.MovieClip;
	import flash.events.Event;
	
	public class InventoryRotatingIcon extends MovieClip {
		
		// Stage Instances
		public var titlebar_icon_g:MovieClip;
		
		//Member Variable
		private var yRot:Number = 0;

		public function InventoryRotatingIcon() {
			addEventListener(Event.ENTER_FRAME, handleEnterFrame, false, 0, true);
		}
		
		public function handleEnterFrame(event:Event):void {
			if(titlebar_icon_g != null)
					titlebar_icon_g.rotationY = yRot++;
				else
					trace("Error: InventoryRotatingIcon::handleEnterFrame, titlebar_icon_g does not exist!");
		
			
		}

		
	}
}