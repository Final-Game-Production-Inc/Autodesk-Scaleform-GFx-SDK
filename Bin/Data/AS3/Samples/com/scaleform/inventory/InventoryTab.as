package com.scaleform.inventory 
{	
	// Imports
	import flash.display.MovieClip;
	
	public class InventoryTab extends MovieClip 
	{	
		// Stage Instance
		public var tab:MovieClip;
		
		// Functions called from Timeline
		public function selectTab():void {
			gotoAndPlay("on");
		}
		
		public function deselectTab():void {
			gotoAndPlay("off");
		}
		
	}
}