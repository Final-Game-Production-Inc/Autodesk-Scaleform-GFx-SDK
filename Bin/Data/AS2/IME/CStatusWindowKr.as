/**************************************************************************

Filename    :   CStatusWindowKr.as
Content     :   Status Window Korean ActionScript object
Created     :   9/11/2008 
Authors     :   Ankur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

class CStatusWindowKr extends CStatusWindowBase
{

	public var toggleSymbol:Boolean;

	public var toggleShape:Boolean;

	public var toggleInputMode:Boolean;
	
	public var parent_mc:MovieClip;

	public function CStatusWindowKr(mc: MovieClip)
	{
		toggleSymbol = false;
	
		toggleShape = false;
	
		toggleInputMode = false;
		
		parent_mc = mc;
		
		mc.Controller = this;
	}
	
	public function ChangeBackgroundColor(frameNumber:Number, mc:MovieClip)
	{
		// Shade the border in a slightly darker shade..
		/*
		if (mc._name.indexOf("backgroundBorder")!= -1)
		{
			var col:Number 		= BGColor;
			var red:Number 		= ((col & 0xFF0000) >> 16)/1.2 < 1? 1: ((col & 0xFF0000) >> 16)/1.2;
			var green:Number 	= ((col & 0x00FF00) >> 8)/1.2 < 1? 1: ((col & 0x00FF00) >> 8)/1.2;
			var blue:Number 	= (col & 0x0000FF)/1.2 < 1 ? 1: (col & 0x0000FF)/1.2;
			
			var new_color = (red << 16) + (green << 8) +  blue;
			var change_color:Color = new Color(mc);
			change_color.setRGB(new_color);
		}
		else
		{
			var change_color:Color = new Color(mc);
			if ((frameNumber == 1) || (frameNumber == 3))
				change_color.setRGB(BGColor);
				
			if ((frameNumber == 2) || (frameNumber == 4))
				change_color.setRGB(BGColorOnover);	
		}	
		*/
		
		
		var col:Number 		= BGColor;
		var red:Number 		= ((col & 0xFF0000) >> 16)/1.2 < 1? 1: ((col & 0xFF0000) >> 16)/1.2;
		var green:Number 	= ((col & 0x00FF00) >> 8)/1.2 < 1? 1: ((col & 0x00FF00) >> 8)/1.2;
		var blue:Number 	= (col & 0x0000FF)/1.2 < 1 ? 1: (col & 0x0000FF)/1.2;
			
		var new_color = (red << 16) + (green << 8) +  blue;
		var change_color:Color = new Color(mc.inputModeBorder);
		change_color.setRGB(new_color);
		
		var change_color:Color = new Color(mc.inputModeBG);
		if ((frameNumber == 1) || (frameNumber == 3))
			change_color.setRGB(BGColor);
				
		if ((frameNumber == 2) || (frameNumber == 4))
			change_color.setRGB(BGColorOnover);		
				
		if ((frameNumber == 1) || (frameNumber == 2))		
			change_color = new Color(mc.inputIcon1);
			
		if ((frameNumber == 3) || (frameNumber == 4))		
			change_color = new Color(mc.inputIcon2);
			
		if ((frameNumber == 1) || (frameNumber == 3))
			change_color.setRGB(TextColor);
				
		if ((frameNumber == 2) || (frameNumber == 4))
			change_color.setRGB(TextColorSelected);		
		
		if ((frameNumber == 1) || (frameNumber == 3))
		{
			var oldTextFormat:TextFormat = mc.tf.getTextFormat();
			oldTextFormat.color = TextColor;
			mc.tf.setTextFormat(oldTextFormat);
			mc.tf.setNewTextFormat(oldTextFormat);
		}
		if ((frameNumber == 2) || (frameNumber == 4))
		{
			var oldTextFormat:TextFormat = mc.tf.getTextFormat();
			oldTextFormat.color = TextColorSelected;
			mc.tf.setTextFormat(oldTextFormat);
			mc.tf.setNewTextFormat(oldTextFormat);
		}
	}
	
	public function SetBackgroundColor(color1:Number, color2:Number):Void
	{
		super.SetBackgroundColor(color1, color2);
		if(parent_mc._currentframe == 1)
		{
			// This check is important to prevent AS errors since inputmode_mc, shape_mc etc are only available on 
			// the first frame of the parent movie clip. When the movie is first created, the playhead is positioned 
			// at the second frame to make the status window invisible. 
			ChangeBackgroundColor(parent_mc.InputMode_mc._currentframe, parent_mc.InputMode_mc);
		}
	}

	public function SetTextColor(color1:Number, color2:Number):Void
	{
		super.SetTextColor(color1, color2);
		if(parent_mc._currentframe == 1)
		{
			ChangeBackgroundColor(parent_mc.InputMode_mc._currentframe, parent_mc.InputMode_mc);
		}
	}
	
	public function ChangeIconColor(frameNumber:Number, mc:MovieClip)
	{
		var change_color:Color = new Color(mc);
		if ((frameNumber == 1) || (frameNumber == 3))
			change_color.setRGB(TextColor);
				
		if ((frameNumber == 2) || (frameNumber == 4))
			change_color.setRGB(TextColorSelected);
	}
	
	public function DisplayStatusWindow(bgColor1:Number, bgColor2:Number, textColor1:Number, textColor2:Number):Void
	{
		super.SetBackgroundColor(bgColor1, bgColor2);
		super.SetTextColor(textColor1, textColor2);
		SetBackgroundColor(bgColor1, bgColor2);
		if(parent_mc._currentframe != 1)
		{
			parent_mc.gotoAndPlay(1);
		}
	}
	
	public function RemoveStatusWindow()
	{
		if(parent_mc._currentframe != 2)
		{
			parent_mc.gotoAndPlay("empty");
		}
	}
	
	public function SetInputMode(mode:String)
	{
		if (mode == "Native")
		{
			toggleInputMode = false;
			if (parent_mc.InputMode_mc != null && parent_mc.InputMode_mc != undefined && parent_mc.InputMode_mc._currentframe != 1) // Native
			{
				parent_mc.InputMode_mc.gotoAndPlay(1);
			}
		}
		
		if (mode == "Alphanumeric")
		{
			toggleInputMode = true;
			if (parent_mc.InputMode_mc != null && parent_mc.InputMode_mc != undefined && parent_mc.InputMode_mc._currentframe != 3)  // Alphanumeric
			{
				parent_mc.InputMode_mc.gotoAndPlay(3);
			}
		}
	}
	
}