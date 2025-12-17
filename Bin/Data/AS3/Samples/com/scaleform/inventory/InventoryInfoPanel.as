package com.scaleform.inventory 
{	
	// Imports
	import flash.display.MovieClip;
	
	public class InventoryInfoPanel extends MovieClip 
	{
		// Stage Objects
		public var weapon:MovieClip;
		public var info_arrow:MovieClip;
		public var info_title:MovieClip;
		public var info_text:MovieClip;
		
		public function InventoryInfoPanel()
		{
			if(weapon != null) 
				weapon.gotoAndStop(1);
			else 
				trace("Error: InventoryInfoPanel::ctor, weapon does not exist!");
				
			if(info_arrow != null)
				info_arrow.visible = false;
			else	
				trace("Error: InventoryInfoPanel::ctor, info_arrow does not exist!");
			
			if(info_title != null && info_title.textField != null)
				info_title.textField.text = "";
			else
				trace("Error: InventoryInfoPanel::ctor, info_title or info_title.textField does not exist!");
			
			if(info_text != null && info_text.textField != null)
				info_text.textField.text = "";
			else
				trace("Error: InventoryInfoPanel::ctor, info_text or info_text.textField does not exist!");
		}
		
		public function setWeapon(data:Number):void {
			if(weapon != null) {
				if (weapon.currentFrame == data+1) 
					return;

				if(weapon.weapon_icon_g != null)
					weapon.weapon_icon_g.gotoAndStop(data + 1);
				else
					trace("Error: InventoryInfoPanel::setWeapon, weapon.weapon_icon_g does not exist!");
			}
			else {
				trace("Error: InventoryInfoPanel::setWeapon, weapon does not exist!");
			}
				
			if(info_arrow != null)
				info_arrow.visible = true;
			else	
				trace("Error: InventoryInfoPanel::setWeapon, info_arrow does not exist!");
			
			if(info_title != null && info_title.textField != null)
				info_title.textField.text = "Weapon Title: [" + data + "]";
			else
				trace("Error: InventoryInfoPanel::setWeapon, info_title or info_title.textField does not exist!");
			
			if(info_text != null && info_text.textField != null)
				info_text.textField.text = "Weapon Desc [" + data + "]";
			else
				trace("Error: InventoryInfoPanel::setWeapon, info_text or info_text.textField does not exist!");
			
			gotoAndPlay("on");
		}
		
		public function setItem(data:Number):void {	
			if(weapon != null) {
				if(weapon.weapon_icon_g != null)
					weapon.weapon_icon_g.gotoAndStop(data + 11);
				else
					trace("Error: InventoryInfoPanel::setItem, weapon.weapon_icon_g does not exist!");
			}
			else {
				trace("Error: InventoryInfoPanel::setItem, weapon does not exist!");
			}
			
			if(info_arrow != null)
				info_arrow.visible = false;
			else	
				trace("Error: InventoryInfoPanel::setItem, info_arrow does not exist!");
			
			if(info_title != null && info_title.textField != null)
				info_title.textField.text = "Item Name: ["+data+"]";
			else
				trace("Error: InventoryInfoPanel::setItem, info_title or info_title.textField does not exist!");
			
			if(info_text != null && info_text.textField != null)
				info_text.textField.text = "Item Description: ["+data+"]";
			else
				trace("Error: InventoryInfoPanel::setItem, info_text or info_text.textField does not exist!");
			
			gotoAndPlay("on");
		}
	}
}