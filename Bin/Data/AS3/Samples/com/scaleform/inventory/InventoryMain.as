package com.scaleform.inventory 
{	
	// Imports
	import flash.display.MovieClip;
	
	public class InventoryMain extends MovieClip 
	{
		// Stage Objects
		public var arsenal:MovieClip;
		public var backpack:MovieClip;
		public var info:MovieClip;
		public var infoMC_G:MovieClip;
		public var left:MovieClip;
		public var titlebar:MovieClip;
		
		// Member Variables
		private var arsenalFocused:Boolean = false;
		private var backpackFocused:Boolean = false;

		public function arsenalToFront():void {
			if (!arsenalFocused)
			{
				if(arsenal != null && arsenal.tab1_mc != null)
					arsenal.tab1_mc.selectTab();
				else
					trace("Error: InventoryMain::arsenalToFront, arsenal or arsenal.tab1_mc does not exist!");
				
				if(backpack != null && backpack.tab2_mc != null)
					backpack.tab2_mc.deselectTab();
				else
					trace("Error: InventoryMain::arsenalToFront, backpack or backpack.tab2_mc does not exist!");

				backpackFocused = false;
				arsenalFocused = true;
			}
		}

		public function backpackToFront():void {
			if (!backpackFocused)
			{
				if(arsenal != null && arsenal.tab1_mc != null)
					arsenal.tab1_mc.deselectTab();
				else
					trace("Error: InventoryMain::backpackToFront, arsenal or arsenal.tab1_mc does not exist!");
				
				if(backpack != null && backpack.tab2_mc != null)
					backpack.tab2_mc.selectTab();
				else
					trace("Error: InventoryMain::backpackToFront, backpack or backpack.tab2_mc does not exist!");

				backpackFocused = true;
				arsenalFocused = false;
			}
		}
	}
}