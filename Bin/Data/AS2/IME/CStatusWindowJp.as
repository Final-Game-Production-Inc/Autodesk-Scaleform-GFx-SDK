/**************************************************************************

Filename    :   CStatusWindowJp.as
Content     :   Status Window Chinese ActionScript object
Created     :   9/11/2008
Authors     :   Ankur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

class CStatusWindowJp extends CStatusWindowBase
{

	public var toggleSymbol:Boolean;

	public var toggleShape:Boolean;

	public var toggleInputMode:Boolean;
	
	public var parent_mc:MovieClip;

	public function CStatusWindowJp(mc: MovieClip)
	{
		toggleSymbol = false;
	
		toggleShape = false;
	
		toggleInputMode = false;
		
		parent_mc = mc;
		
		mc.Controller = this;
	}

	public function  DisplayStatusWindow(bgColor1:Number, bgColor2:Number, textColor1:Number, textColor2:Number):Void
	{
		parent_mc.CreateInputModeTab("DirectInput", bgColor1, bgColor2, textColor1, textColor2);
	}
	
	public function SetBackgroundColor(color1:Number, color2:Number):Void
	{
		parent_mc.SetBackgroundColor(color1, color2);
	}
	
	public function SetTextColor(color1:Number, color2:Number):Void
	{
		parent_mc.SetTextColor(color1, color2);
	}
	
	public function RemoveStatusWindow()
	{
		if (parent_mc.InputModeList_mc != undefined)
			removeMovieClip(parent_mc.InputModeList_mc);
	} 
	
	public function SetInputMode(mode:String)
	{
		// close input mode list
		parent_mc.CloseList(parent_mc.NumRows);
		
		if (mode == "Alphanumeric_FullShape")
		{
			parent_mc.InputModeTab_mc.tf.text = "Full-width Alphanumeric";
		}
		
		if (mode == "Alphanumeric_HalfShape")
		{
			parent_mc.InputModeTab_mc.tf.text = "Half-width Alphanumeric";
		}
		
		if (mode == "Katakana_HalfShape")
		{
			parent_mc.InputModeTab_mc.tf.text = "Half-width Katakana";
		}
		
		if (mode == "Katakana_FullShape")
		{
			parent_mc.InputModeTab_mc.tf.text = "Full-width Katakana";
		}
		
		if (mode == "Hiragana_FullShape")
		{
			parent_mc.InputModeTab_mc.tf.text = "Hiragana";
		}
		
		if (mode == "DirectInput")
		{
			parent_mc.InputModeTab_mc.tf.text = "Direct Input";
		}
		var width:Number = parent_mc.InputModeTab_mc.tf.textWidth + 4;
		parent_mc.InputModeTab_mc.tf._width = width;
		parent_mc.Resize(width, parent_mc.InputModeTab_mc);
	}
}