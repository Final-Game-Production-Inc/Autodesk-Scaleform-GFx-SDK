/**************************************************************************

Filename    :   CStatusWindowCh.as
Content     :   Status Window Chinese ActionScript object
Created     :   9/11/2008
Authors     :   Ankur 

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package
{
import flash.display.MovieClip;
import flash.geom.*;
import flash.events.MouseEvent;
import scaleform.gfx.*;

class CStatusWindowCh extends flash.display.MovieClip
{
	public var 	BGColor:Number 			= 0xDEE7EE;
	public var	BGColorOnover:Number 	= 0x55C0DC;
	public var 	TextColor:Number        = 0x555759;
	public var 	TextColorSelected:Number= 0xFFFFFF;
	public var 	toggleSymbol:Boolean;
	public var 	toggleShape:Boolean;
	public var 	toggleInputMode:Boolean;
	public var 	parent_mc:MovieClip;
	public var 	inputModeBG:MovieClip;
	public var 	shapeBG:MovieClip;
	public var 	symbolBG:MovieClip;
	public var 	backgroundBorder:MovieClip;
	public var 	inputIcon1:MovieClip;
	public var 	inputIcon2:MovieClip;
	public var 	shapeIcon1:MovieClip;
	public var 	shapeIcon2:MovieClip;
	public var 	symbolIcon1:MovieClip;
	public var 	symbolIcon2:MovieClip;
	
	public function CStatusWindowCh(mc: MovieClip = null)
	{
		toggleSymbol 	= false;
		toggleShape 	= false;
		toggleInputMode = false;
		parent_mc = mc;
		mouseChildren = false;
		if (mc != null)
		{
			mc.Controller = this;
		}
		addEventListener(MouseEvent.MOUSE_OVER, OnRollOver);
		addEventListener(MouseEvent.MOUSE_OUT, 	OnRollOut);
		addEventListener(MouseEvent.MOUSE_UP, 	OnClick);
		super();
	}
	
	public function ChangeBackgroundColor(frameNumber:Number)
	{
		// Shade the border in a slightly darker shade..
		
		var col:Number 		= BGColor;
		var red:Number 		= ((col & 0xFF0000) >> 16)/1.2 < 1? 1: ((col & 0xFF0000) >> 16)/1.2;
		var green:Number 	= ((col & 0x00FF00) >> 8)/1.2 < 1? 1: ((col & 0x00FF00) >> 8)/1.2;
		var blue:Number 	= (col & 0x0000FF)/1.2 < 1 ? 1: (col & 0x0000FF)/1.2;
		if (backgroundBorder != null)
		{
			backgroundBorder.transform.colorTransform = new ColorTransform(0,0,0,1, red, green, blue, 0);
		}
		var pos:Number;
		// Detect which kind of movie it is
		if (name.indexOf("Input")!= -1)
		{
			var inputIcon:MovieClip;
			
			if ((frameNumber == 1) || (frameNumber == 2))		
				inputIcon = inputIcon1;
			
			if ((frameNumber == 3) || (frameNumber == 4))		
				inputIcon = inputIcon2;
				
			if ((frameNumber == 1) || (frameNumber == 3))
			{
				inputModeBG.transform.colorTransform = new ColorTransform(0,0,0,1, (BGColor & 0xFF0000) >> 16, (BGColor & 0x00FF00) >> 8, (BGColor & 0x0000FF), 0);
				inputIcon.transform.colorTransform = new ColorTransform(0,0,0,1, (TextColor & 0xFF0000) >> 16, (TextColor & 0x00FF00) >> 8, (TextColor & 0x0000FF), 0);
			}
					
			if ((frameNumber == 2) || (frameNumber == 4))
			{
				inputModeBG.transform.colorTransform = new ColorTransform(0,0,0,1, (BGColorOnover & 0xFF0000) >> 16, (BGColorOnover & 0x00FF00) >> 8, (BGColorOnover & 0x0000FF), 0);
				inputIcon.transform.colorTransform = new ColorTransform(0,0,0,1, (TextColorSelected & 0xFF0000) >> 16, 
														(TextColorSelected & 0x00FF00) >> 8, (TextColorSelected & 0x0000FF), 0);
			}
		}
		
		if (name.indexOf("Shape")!= -1)
		{
			var shapeIcon:MovieClip;
			
			if ((frameNumber == 1) || (frameNumber == 2))		
				shapeIcon = shapeIcon1;
			
			if ((frameNumber == 3) || (frameNumber == 4))		
				shapeIcon = shapeIcon2;
	
			if ((frameNumber == 1) || (frameNumber == 3))
			{
				shapeBG.transform.colorTransform = new ColorTransform(0,0,0,1, (BGColor & 0xFF0000) >> 16, (BGColor & 0x00FF00) >> 8, (BGColor & 0x0000FF), 0);
				shapeIcon.transform.colorTransform = new ColorTransform(0,0,0,1, (TextColor & 0xFF0000) >> 16, (TextColor & 0x00FF00) >> 8, (TextColor & 0x0000FF), 0);
			}
					
			if ((frameNumber == 2) || (frameNumber == 4))
			{
				shapeBG.transform.colorTransform = new ColorTransform(0,0,0,1, (BGColorOnover & 0xFF0000) >> 16, (BGColorOnover & 0x00FF00) >> 8, (BGColorOnover & 0x0000FF), 0);
				shapeIcon.transform.colorTransform = new ColorTransform(0,0,0,1, (TextColorSelected & 0xFF0000) >> 16, 
														(TextColorSelected & 0x00FF00) >> 8, (TextColorSelected & 0x0000FF), 0);
			}
		}
		
		if (name.indexOf("Symbol")!= -1)
		{
			var symbolIcon:MovieClip;
			
			if ((frameNumber == 1) || (frameNumber == 2))		
				symbolIcon = symbolIcon1;
			
			if ((frameNumber == 3) || (frameNumber == 4))		
				symbolIcon = symbolIcon2;
				
			if ((frameNumber == 1) || (frameNumber == 3))
			{
				symbolBG.transform.colorTransform = new ColorTransform(0,0,0,1, (BGColor & 0xFF0000) >> 16, (BGColor & 0x00FF00) >> 8, (BGColor & 0x0000FF), 0);
				symbolIcon.transform.colorTransform = new ColorTransform(0,0,0,1, (TextColor & 0xFF0000) >> 16, (TextColor & 0x00FF00) >> 8, (TextColor & 0x0000FF), 0);
			}
					
			if ((frameNumber == 2) || (frameNumber == 4))
			{
				symbolBG.transform.colorTransform = new ColorTransform(0,0,0,1, (BGColorOnover & 0xFF0000) >> 16, (BGColorOnover & 0x00FF00) >> 8, (BGColorOnover & 0x0000FF), 0);
				symbolIcon.transform.colorTransform = new ColorTransform(0,0,0,1, (TextColorSelected & 0xFF0000) >> 16, 
														(TextColorSelected & 0x00FF00) >> 8, (TextColorSelected & 0x0000FF), 0);
			}
		}
	}
	
	public function SetBackgroundColor(color1:Number, color2:Number):void
	{
		BGColorOnover 	= color2;
		BGColor 		= color1;
		ChangeBackgroundColor(currentFrame);
	}
	
	public function SetTextColor(color1:Number, color2:Number):void
	{
		TextColor			= color1;
		TextColorSelected 	= color2;
		ChangeBackgroundColor(currentFrame);
	}
	
	public function DisplayStatusWindow(bgColor1:Number, bgColor2:Number, textColor1:Number, textColor2:Number):void
	{
		SetBackgroundColor(bgColor1, bgColor2);
		SetTextColor(textColor1, textColor2);
		if(currentFrame != 1)
		{
			gotoAndPlay(1);
		}
	}
	
	public function Hide():void
	{
		if(currentFrame != 2)
		{
			gotoAndPlay("empty");
		}
	}
	
	public function SetInputMode(mode:String)
	{
		if (mode == "Native")
		{
			toggleInputMode = false;
			if (currentFrame != 1) // Native
			{
				gotoAndPlay(1);
			}
		}
		
		if (mode == "Alphanumeric")
		{
			toggleInputMode = true;
			if (currentFrame != 3)  // Alphanumeric
			{
				gotoAndPlay(3);
			}
		}
	}
	
	public function SetSymbolMode(mode:String)
	{
		if (mode == "full")
		{
			toggleSymbol = false;
			if (currentFrame != 3) // Native
			{
				gotoAndPlay(3);
			}
		}
		
		if (mode == "half")
		{
			toggleSymbol = true;
			if (currentFrame != 1)  // Alphanumeric
			{
				gotoAndPlay(1);
			}
		}
	}
	
	public function SetShapeMode(mode:String)
	{
		if (mode == "full")
		{
			toggleShape = false;
			if (currentFrame != 3) // Native
			{
				gotoAndPlay(3);
			}
		}
		
		if (mode == "half")
		{
			toggleShape = true;
			if (currentFrame != 1)  // Alphanumeric
			{
				gotoAndPlay(1);
			}
		}
	}
	
	public function OnRollOver(event:MouseEvent)
	{
		if(currentFrame == 1)
			gotoAndPlay(2);
	
		if(currentFrame == 3)
			gotoAndPlay(4);	
	}
	
	public function OnRollOut(event:MouseEvent)
	{
		if(currentFrame == 2)
			gotoAndPlay(1);
		
		if(currentFrame == 4)
			gotoAndPlay(3);
	}
	
	function SelectItem(name:String) 
	{
		if (name == "Shape_mc")	
			IMEEx.SendLangBarMessage(this, "StatusWindow_OnShape", toggleShape == true? "true":"false");
			
		if (name == "InputMode_mc")	
			IMEEx.SendLangBarMessage(this, "StatusWindow_OnInputMode", toggleInputMode == true? "true":"false");
			
		if (name == "Symbol_mc")	
			IMEEx.SendLangBarMessage(this, "StatusWindow_OnSymbol", toggleSymbol == true? "true":"false");
	}
	
	public function OnClick(event:MouseEvent)
	{
		SelectItem(name);
	}
}
}