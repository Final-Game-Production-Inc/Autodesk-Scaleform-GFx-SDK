/**************************************************************************

Filename    :   CStatusWindowBase.as
Content     :   Status Window ActionScript object
Created     :   9/11/2008
Authors     :   Ankur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

class CStatusWindowBase
{ 
	public var 	BGColor:Number 			= 0xDEE7EE;
	public var	BGColorOnover:Number 	= 0x55C0DC;
	public var 	TextColor:Number        = 0x555759;
	public var 	TextColorSelected:Number= 0xFFFFFF
	
	public function DisplayStatusWindow(bgColor1:Number, bgColor2:Number, textColor1:Number, textColor2:Number):Void
	{
	}
	
	public function RemoveStatusWindow()
	{
	}
	
	public function SetInputMode(mode:String)
	{
	}
	
	public function SetSymbolMode(mode:String)
	{
	}
	
	public function SetShapeMode(mode:String)
	{
	}
	
	public function SetBackgroundColor(color1:Number, color2:Number):Void
	{
		BGColorOnover 	= color2;
		BGColor 		= color1;
	}
	
	public function SetTextColor(color1:Number, color2:Number):Void
	{
		TextColor			= color1;
		TextColorSelected 	= color2;
	}
} 