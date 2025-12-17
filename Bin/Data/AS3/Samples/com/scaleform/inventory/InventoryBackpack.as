package com.scaleform.inventory 
{	
	// Imports
	import flash.display.MovieClip;
	import flash.events.Event;
	import flash.events.FocusEvent;
	
	public class InventoryBackpack extends MovieClip 
	{
		// Constants
		static public var NUM_BACKPACK_BTNS:Number = 4;
		
		// Stage Objects
		public var heading1:MovieClip;
		public var heading2:MovieClip;
		
		public var item_btn1:MovieClip;
		public var item_btn2:MovieClip;
		public var item_btn3:MovieClip;
		public var item_btn4:MovieClip;
		public var item_btn5:MovieClip;
		public var item_btn6:MovieClip;
		public var item_btn7:MovieClip;
		public var item_btn8:MovieClip;
		public var item_btn9:MovieClip;
		public var item_btn10:MovieClip;
		
		public var tab2_mc:MovieClip;
		public var infoMC_G:MovieClip;

		public var stealDefaultFocusButton:MovieClip;
		
		//Member Variables
		private var selectedBtn:MovieClip;
			
		// Constructor
		public function InventoryBackpack() {
			addEventListener(Event.FRAME_CONSTRUCTED, initBackpack, false, 0, true);
		}
		
		// Setup elements once class instance is loaded
		private function initBackpack(ev:Event):void  {
			removeEventListener(Event.FRAME_CONSTRUCTED, initBackpack, false);
		
			for (var i:Number = 1; i < NUM_BACKPACK_BTNS; i++) {
				var itemBtn:MovieClip = getChildByName("item_btn" + i) as MovieClip;
				if(itemBtn != null) {
					setupBackpackButton( itemBtn );
					
					if(itemBtn.icon != null && itemBtn.icon.item_g != null)
						itemBtn.icon.item_g.gotoAndPlay(i + 11);
					else
						trace("Error: InventoryBackpack::initBackpack, itemBtn.icon or itemBtn.icon.item_g in item_btn"+i+" does not exist!");
					itemBtn.data = i;
				}
				else {
					trace("Error: InventoryBackpack::initBackpack, item_btn"+i+" does not exist!");
				}
			}
		}

		private function setupBackpackButton(mc:MovieClip):void {
			if(mc != null) {
				mc.addEventListener(FocusEvent.FOCUS_IN, onFocusChange, false, 0, true);
				mc.addEventListener(FocusEvent.FOCUS_IN, onFocusInUpdateInfo, false, 0, true);

				mc.addEventListener(FocusEvent.FOCUS_IN, onFocusInBackpackTab, false, 0, true);
				mc.addEventListener(FocusEvent.FOCUS_OUT, onFocusOut, false, 0, true);
			}
			else {
				trace("Error: InventoryBackpack::setupBackpackButton, mc does not exist!");
			}
		}

		private function onFocusInUpdateInfo(p_event:Object):void {
			var curParent:MovieClip = parent as MovieClip;
			if (curParent != null && curParent.info != null && 
				curParent.info.info_mc != null && curParent.info.info_mc.info_mc_g != null) {
					var infoMC_G:MovieClip = curParent.info.info_mc.info_mc_g;
					if(infoMC_G != null)
						infoMC_G.setItem(p_event.target.data);
					else
						trace("Error: InventoryArsenal::onFocusInUpdateInfo, infoMC_G does not exist!");
			}
			else {
				trace("Error: InventoryArsenal::onFocusInUpdateInfo, parent.info.info_mc.info_mc_g or some subset of that path does not exist!");
			}
		}

		private function onFocusInBackpackTab(p_event:Object):void {
			(parent as MovieClip).backpackToFront();
		}

		private function onFocusChange(p_event:Object):void {
			selectedBtn = p_event.target as MovieClip;
			startSelectionTween();
		}

		private function onFocusOut(p_event:Event = null):void {
			var lastSelected:MovieClip = p_event.target as MovieClip;
		}

		private function startSelectionTween(p_event:Object = null):void {
			var iconMC:MovieClip = selectedBtn["icon"]["item_g"] as MovieClip;
		}

		private function ContinueSelectionTween(p_event:Object):void {
			var iconMC:MovieClip = selectedBtn["icon"]["item_g"] as MovieClip;
		}
	}
}