package com.scaleform.inventory 
{	
	// Imports
	import flash.display.MovieClip;
	import flash.events.Event;
	import flash.events.FocusEvent;
	import flash.text.TextField;
	
	import scaleform.clik.controls.Button;
	import scaleform.clik.events.ButtonEvent;
	
	public class InventoryArsenal extends MovieClip 
	{
		// Constants
		private static var NUM_BUTTONS:Number = 10;
		
		// Stage Objects
		public var tab1_mc:MovieClip;
		public var equipped:MovieClip;
		
		public var heading1:MovieClip;
		public var heading2:MovieClip;
		
		public var weapons_panel1:MovieClip;
		public var weapons_panel2:MovieClip;
		
		// Weapon buttons on stage
		public var weapon_btn1:Button;
		public var weapon_btn2:Button;
		public var weapon_btn3:Button;
		public var weapon_btn4:Button;
		public var weapon_btn5:Button;
		public var weapon_btn6:Button;
		public var weapon_btn7:Button;
		public var weapon_btn8:Button;
		public var weapon_btn9:Button;
		public var weapon_btn10:Button;
		
		// Member Variables
		protected var selectedBtn:MovieClip;

		// Constructor
		public function InventoryArsenal() {
			addEventListener(Event.FRAME_CONSTRUCTED, initArsenal, false, 0, true);
		}
		
		// Setup elements once class instance is loaded
		private function initArsenal(ev:Event):void {
			removeEventListener(Event.FRAME_CONSTRUCTED, initArsenal, false);
			
			if(tab1_mc != null)
				tab1_mc.selectTab();
			else
				trace("Error: InventoryArsenal::initArsenal, tab1_mc does not exist!");

			for (var i:Number = 1; i < NUM_BUTTONS+1; i++) {
				var weaponBtn:MovieClip = getChildByName("weapon_btn" + i) as MovieClip;	
				setupArsenalButton( weaponBtn );
				if (weaponBtn) {
					var weaponIcon:MovieClip = weaponBtn.getChildByName("icon") as MovieClip;
					if(weaponIcon != null) {
						var icon_g:MovieClip = weaponIcon.getChildByName("weapon_icon_g") as MovieClip;
						if(icon_g != null)
							icon_g.gotoAndPlay(i + 1);
						else
							trace("Error: InventoryArsenal::initArsenal, icon in weapon_btn"+i+" does not exist!");
					} 
					else {
						trace("Error: InventoryArsenal::initArsenal, icon.weapon_icon_g in weapon_btn"+i+" does not exist!");
					}
					if(weaponBtn.tfContainer != null) {
						var tf:TextField = weaponBtn.tfContainer.getChildByName("textField") as TextField;
						if (tf != null) {
							if (i < 10)
								tf.text = String("0" + String(i));
							else
								tf.text = String(i);
						}
						else {
							trace("Error: InventoryArsenal::initArsenal, weaponBtn.tfContainer.textField in weapon_btn"+i+" does not exist!");
						}
					}
					else {
						trace("Error: InventoryArsenal::initArsenal, weaponBtn.tfContainer in weapon_btn"+i+" does not exist!");
					}
					
					weaponBtn.data = i;
				}
				else {
					trace("Error: InventoryArsenal::initArsenal, weapon_btn"+i+" does not exist!");
				}
			}
		}

		// Setup button event listeners
		private function setupArsenalButton(mc:MovieClip):void {
			if(mc != null) {
				mc.addEventListener(FocusEvent.FOCUS_IN, onFocusInArsenalTab, false, 0, true);
				mc.addEventListener(FocusEvent.FOCUS_IN, onFocusInUpdateInfo, false, 0, true);
				
				mc.addEventListener(FocusEvent.FOCUS_IN, onFocusInTweenIcon, false, 0, true);
				mc.addEventListener(FocusEvent.FOCUS_OUT, onFocusOutStopTween, false, 0, true);

				mc.addEventListener(ButtonEvent.PRESS, onClickChangeWeapon, false, 0, true);
			}
			else {
				trace("Error: InventoryArsenal::setupArsenalButton, mc does not exist!");
			}
		}

		private function onFocusInUpdateInfo(p_event:FocusEvent):void {
			var curParent:MovieClip = parent as MovieClip;
			if (curParent != null && curParent.info != null && 
				curParent.info.info_mc != null && curParent.info.info_mc.info_mc_g != null) {
					var infoMC_G:MovieClip = curParent.info.info_mc.info_mc_g;
					if(infoMC_G != null)
						infoMC_G.setWeapon(p_event.target.data);
					else
						trace("Error: InventoryArsenal::onFocusInUpdateInfo, infoMC_G does not exist!");
			}
			else {
				trace("Error: InventoryArsenal::onFocusInUpdateInfo, parent.info.info_mc.info_mc_g or some subset of that path does not exist!");
			}
		}

		private function onFocusInArsenalTab(p_event:FocusEvent):void {
			(parent as MovieClip).arsenalToFront();
		}

		private function onClickChangeWeapon(p_event:ButtonEvent):void {
			if (equipped != null)
				equipped.setEquipped(p_event.target.data);
			else
				trace("Error: InventoryArsenal::onClickChangeWeapon, equipped does not exist!");
		}

		private function onFocusInTweenIcon(p_event:FocusEvent):void {
			selectedBtn = p_event.target as MovieClip;
			startSelectionTween();	
		}

		private function onFocusOutStopTween(p_event:FocusEvent):void {
			var lastSelected:MovieClip = p_event.target as MovieClip;
		}

		private function startSelectionTween(p_event:Event = null):void {
			var iconMC:MovieClip = selectedBtn["weapon_icon"] as MovieClip;
		}

		private function continueSelectionTween(p_event:Event = null):void {
			var iconMC:MovieClip = selectedBtn["weapon_icon"] as MovieClip;	
		}
				
		//
		////////////////////////////////////////////////////////////////////////////
		// Called from Timeline
		////////////////////////////////////////////////////////////////////////////
		//
		public function setEquippedWeapon():void {
			if (equipped != null) {
				equipped.gotoAndPlay("open");
				
				if (weapon_btn1 != null) {
					selectedBtn = weapon_btn1;
					weapon_btn1.focused = 1;
				}
				else {
					trace("Error: InventoryArsenal::setEquippedWeapon, weapon_btn1 does not exist");
				}
				
				if(equipped.equipped_weapon != null && equipped.equipped_weapon.equipped_weapon_g != null)
					equipped.equipped_weapon.equipped_weapon_g.gotoAndStop(2);
				else
					trace("Error: InventoryArsenal::setEquippedWeapon, equipped.equipped_weapon or equipped.equipped_weapon.equipped_weapon_g does not exist");
				
				if (equipped.equipped_text1 != null && equipped.equipped_text1.textField != null)
					equipped.equipped_text1.textField.text = "1";
				else
					trace("Error: InventoryArsenal::setEquippedWeapon, equipped.equipped_text1 or equipped.equipped_text1.textField does not exist");
				
				if (equipped.equipped_text2 != null && equipped.equipped_text2.textField != null)
					equipped.equipped_text2.textField.text = "1";
				else
					trace("Error: InventoryArsenal::setEquippedWeapon, equipped.equipped_text2 or equipped.equipped_text2.textField does not exist");
			}
			else {
				trace("Error: InventoryArsenal::setEquippedWeapon, equipped does not exist");
			}
		}
	}
}