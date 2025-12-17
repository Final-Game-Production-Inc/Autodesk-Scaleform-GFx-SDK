package com.scaleform.inventory 
{	
	// Imports
	import flash.display.MovieClip;
	
	public class InventoryEquipped extends MovieClip 
	{
		// Stage Objects
		public var equipped_weapon:MovieClip;
		public var equipped_text1:MovieClip;
		public var equipped_text2:MovieClip;
		
		
		public function setEquipped(data:Number):void {
			if(equipped_weapon != null && equipped_weapon.equipped_weapon_g != null) 
				equipped_weapon.equipped_weapon_g.gotoAndStop(data + 1);
			else
				trace("Error: InventoryEquipped::setEquipped, equipped_weapon or equipped_weapon.equipped_weapon_g does not exist!");
			
			gotoAndPlay("equipped");
			
			var num:String;
			if (data < 10)
				num = String("0" + String(data));
			else
				num = String(data);
			
			if(equipped_text1 != null && equipped_text1.textField != null)
				equipped_text1.textField.text = num;
			else
				trace("Error: InventoryEquipped::setEquipped, equipped_text1 or equipped_text1.textField does not exist!");
			
			if(equipped_text2 != null && equipped_text2.textField != null)
				equipped_text2.textField.text = num;
			else
				trace("Error: InventoryEquipped::setEquipped, equipped_text2 or equipped_text2.textField does not exist!");
		}
	}
}