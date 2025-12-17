/**************************************************************************

Filename    :   fxplayer.as
Content     :   FxPlayer HUD flash interface.
                
Created     :   November, 2008
Authors     :   Dmitry Polenur, Boris Rayskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

_global.gfxExtensions 		= true;
Stage.scaleMode				= "noScale";
Stage.align 				= "TL";

var stageHeight:Number		= Stage.height;

hudText.selectable 			= false;
var currentLine:Number 		= 1;
var selectionEnabled:Boolean= true;

hudText.html 				= false;
f11.caption.html 			= true;
f12.caption.html 			= true;
refreshBtn.caption.html 	= true;

messageText.html 			= true;
hudText.autoSize 			= false;
hudText.noAutoSelection 	= true;
//hudText.border = true;

//hudText.selectionBkgColor = 0x7f007fbf;

messageText.autoSize 		= true;

var pointsArray:Array 		= new Array;
var stageListener:Object 	= new Object();

var systemType:String; //i.e. pc, xbox360 etc.
var focusBtnBmp:Object 		= new Object;
var refreshBtnBmp:Object 	= new Object;
var closeBtnBmp:Object 		= new Object;

focusBtnBmp["xbox360"] 		= "xbox_back";
refreshBtnBmp["xbox360"]	= "xbox_y";
closeBtnBmp["xbox360"] 		= "xbox_x";

focusBtnBmp["ps"] 			= "ps3_select";
refreshBtnBmp["ps"] 		= "ps3_tri";
closeBtnBmp["ps"] 			= "ps3_squ";

focusBtnBmp["wii"] 			= "wii_2";
refreshBtnBmp["wii"] 		= "empty";
closeBtnBmp["wii"] 			= "wii_1";

var minimumTransp:Number	= 40;
var maximumTransp:Number	= 100;
var transpInc:Number		= 5;
var transparency:Number		= 90;

// Listeners for the interaction.
var mouseListener:Object 	= new Object();
var keyListener:Object 		= new Object();

Mouse.addListener(mouseListener);
Key.addListener(keyListener);
Stage.addListener(stageListener);

var hudTextHeight 	        = hudText._height;
var panelHeight         	= panel._height;
var hudTextY 	        	= hudText._y;
var panelY                  = panel._y;
var refreshY                = refreshBtn._y;


function init() : Void
{
    panel._x                = 0;
    panel._y                = 28;
	panel._width 		    = 512;

	tabRow.tabMemFrag._y    = tabRow.tabMemChart._y = 
    tabRow.tabMemGram._y	= tabRow.tabMemTree._y 	= 28;
	
	var panelXMax           = panel._x + panel._width;
	
	pageUpBtn._x 	        = pageDnBtn._x 		= 
	scrollUpBtn._x 	        = scrollDownBtn._x 	= 
	Legend1._x 		        = Legend2._x 		= 
	Legend3._x 		        = Legend4._x 		= 
	Legend5._x 		        = Legend6._x 		= 
	Legend7._x 		        = Legend8._x		=
	Legend9._x		        = SysLegend1._x 	=
	SysLegend2._x 	        = SysLegend3._x 	=
	SysLegend4._x 	        = panelXMax;
	
	pageUpBtn._y 		    = panel._y + 10;
	scrollUpBtn._y 		    = pageUpBtn._y      + pageUpBtn._height;
	scrollDownBtn._y 	    = scrollUpBtn._y    + scrollUpBtn._height;
	pageDnBtn._y 		    = scrollDownBtn._y  + scrollDownBtn._height;

	Legend1._y		        = tabRow.tabMemFrag._y + 
                              tabRow.tabMemFrag._height + 1;
	Legend2._y		        = Legend1._y + Legend1._height;
	Legend3._y		        = Legend2._y + Legend2._height;
	Legend4._y		        = Legend3._y + Legend3._height;
	Legend5._y		        = Legend4._y + Legend4._height;
	Legend6._y		        = Legend5._y + Legend5._height;
	Legend7._y		        = Legend6._y + Legend6._height;
	Legend8._y		        = Legend7._y + Legend7._height;
	//Legend9._y	        = Legend8._y + Legend8._height;
	
	SysLegend1._y	        = Legend8._y + Legend9._height;
	SysLegend2._y	        = SysLegend1._y + SysLegend1._height;
	SysLegend3._y	        = SysLegend2._y + SysLegend2._height;
	//SysLegend4._y	        = SysLegend3._y + SysLegend3._height;
	
	refreshBtn._x 	        = panel._x + panel._width - refreshBtn._width - 1;

    TextureBorder._x        = tabRow.tabMemTree._x;
    TextureBorder._y        = tabRow.tabMemTree._y + tabRow.tabMemTree._height;
    TextureBorder._width    = 512;
    //TextureBorder._height   = 512;

    LegendBorder._x         = TextureBorder._x + TextureBorder._width;
    LegendBorder._y         = TextureBorder._y;
    LegendBorder._width     = Legend1._width;
    LegendBorder._height    = SysLegend3._y + SysLegend3._height - 
                              Legend1._y + 1;
							  
	memoGramGrid1._y		= TextureBorder._height / 4 + TextureBorder._y;
	memoGramGrid2._y		= TextureBorder._height / 2 + TextureBorder._y;
	memoGramGrid3._y		= memoGramGrid1._y + memoGramGrid2._y - TextureBorder._y;
}


function setSystem(system:String)
{
	systemType = system;
	if (systemType != "pc")
	{		
		f11.caption.htmlText 		= 	"Hud Focus   <img src='" +
							   			focusBtnBmp[systemType] + 
							   			"' width='16' height='16' align='baseline' vspace='-3'>";
		
		f12.caption.htmlText 		= 	"    <img src='" +
										closeBtnBmp[systemType] + 
										"' width='16' height='16' align='baseline' vspace='-5'>";

		refreshBtn.caption.htmlText = 	" <img src='" +
										refreshBtnBmp[systemType] + 
										"' width='16' height='16' align='baseline' vspace='-3'>";
	}
}


stageListener.onResize = function () 
{
	// DBG
	//trace("Stage size is now " + Stage.width + " by " + Stage.height);
	centermessageText();
	
	stageHeight		= Stage.height;
	
	f12._x 			= Stage.width - f12._width - 1;
	f11._x 			= f12._x - f11._width - 1;
	refreshBtn._x   = panel._x + panel._width - refreshBtn._width - 1;
	header._width 	= Stage.width - 2;
	
	resizePanel();
}


function resizePanel() : Void
{
    var panelMinBottom:Number = pageDnBtn._y + pageDnBtn._height;
    var panelMaxBottom:Number = Stage.height - panel._y;
                                //hudText._y + hudText._height; // DBG
	var textBottom:Number = hudText._y + hudText.textHeight + 10;
	
	if (textBottom >= panelMaxBottom)
	{
		panel._height = panelMaxBottom;
	}
	else if (textBottom < panelMinBottom)
	{
		panel._height = panelMinBottom - panel._y;
	}
	else
	{
		panel._height = textBottom - panel._y;
	}
	hudText._height = panel._y + panel._height - hudText._y;
}


function centermessageText()
{
	messageText._x = (Stage.width  - messageText._width)  / 2;
	messageText._y = (Stage.height - messageText._height) / 2;
}


function setMessageText(textArg : String)
{
	messageText.htmlText = textArg;
	centermessageText();
}


function showHud(show:Boolean)
{
	// Make all items except graph visible/invisible, messageText should appear 
	// when hud is "off"
	header._visible 		= show;
	panel._visible 			= show;
	hudText._visible 		= show;
	tabRow._visible 		= show;
	pageUpBtn._visible 		= show;
	scrollUpBtn._visible	= show;
	scrollDownBtn._visible 	= show;
	pageDnBtn._visible 		= show;
	f11._visible 			= show;
	f12._visible 			= show;
	refreshBtn._visible 	= show;
	
    // DBG
	//for (var t in _root)
	//{
	//	if ((_root[t] instanceof TextField) || (_root[t] instanceof MovieClip))
	//	{
	//		_root[t]._visible = show;
	//	}
	//}

	messageText._visible = !show;
	if(show)
	{
		stageListener.onResize();
	}
}


function setHudFocus(focus:Boolean)
{
	var frame:String = "flashFocus";
	if (focus)
	{
		frame = "hudFocus";
		hudText.selectionBkgColor = 0x7f007fbf;
	}
	else
	{
		hudText.selectionBkgColor = 0x7f000000;
	}
	
	panel.gotoAndStop(frame);
	//graphPanelMC.graphPanel.gotoAndStop(frame);
	header.gotoAndStop(frame);
	f11.gotoAndStop(frame);
	
	if (tabRow.tabGroup != undefined)
	{
		tabRow.tabGroup.setEnabledState(focus);
	}
}


function setRefresh(needsRefresh:Boolean)
{
	if (needsRefresh)
	{
		refreshBtn.refreshBtnSmall.gotoAndPlay("refresh");
	}
	else
	{
		refreshBtn.refreshBtnSmall.gotoAndStop("norm");
	}
}


function setHudText(textArg:String)
{
	var currentScroll:Number = hudText.scroll;
	hudText.text = textArg;
	//hudText.scroll = currentScroll;
	//panel._height = hudText._height + 10;
	//panel._width = hudText._width + 10;
}


// DBG
//function setHudFontSize(fontSize : Number)
//{
//	var tf = new TextFormat;
//	tf.size = fontSize;
//	hudText.setTextFormat(tf);
//	hudText.fontSize = fontSize;
////	setHudSize(hudText._width + (hudText._x - Hud._x) * 2,
////				hudText._height + (hudText._y - Hud._y) * 2);	
//
//}


function enableSelection(enable : Boolean)
{
	selectionEnabled = enable;
}


function selectLine(line : Number)
{
	hudText.selectable  = true;
    lineStart           = hudText.getLineOffset(line);
	lineEnd             = hudText.getLineOffset(line + 1);
	Selection.setFocus("hudText");     
	Selection.setSelection(lineStart, lineEnd);
	hudText.scroll      = 1;
	hudText.selectable  = false;
}


function selectCurrentLine()
{
	// Restore scroll, so text will not jump if current 
	// line is not shown
	var savedScroll:Number = hudText.scroll;
	selectLine(currentLine);
	hudText.scroll = savedScroll;
}


keyListener.onKeyDown = function()
{
	if (selectionEnabled)
	{
		switch (Key.getCode())
		{
		case Key.SPACE :
		case Key.ENTER :
			fscommand("click", currentLine);
			selectLine(currentLine);
		break;
		case Key.UP :
			if (currentLine > 0)
			{
				currentLine = currentLine - 1;
			}
			else
			{
				currentLine = 0;
			}
			//trace("Line:" + currentLine); 
			selectLine(currentLine);
		break;
		case Key.DOWN :
			//trace("hudText.numLines" + hudText.numLines);
			if (currentLine < hudText.numLines - 2)
			{
				currentLine = currentLine + 1;
			}
			else
			{
				currentLine = hudText.numLines - 2;
			}
			//trace("Line:" + currentLine); 
			selectLine(currentLine);
		break;
		case Key.LEFT :
			fscommand("left", currentLine);
		break;
		case Key.RIGHT :
			fscommand("right", currentLine);
		break;
// 		DBG		
//		case Key.INSERT :
//			setHudFocus(true);
//		break;
//		case Key.DELETEKEY :
//			setHudFocus(false);
//		break;		
		}
	}
}


mouseListener.onMouseWheel = function(delta:Number)
{
	hudText.scroll -= delta;
}


panel.onPress = function() 
{
	// DBG
    //trace("(" + _xmouse + ","+_ymouse + ")\n");
	//setmessageText("(" + hudText._xmouse + ","+hudText._ymouse + ")\n");
	if(selectionEnabled)
	{
		var line:Number = hudText.getLineIndexAtPoint(hudText._xmouse, hudText._ymouse);
		if (line > -1)
		{
			if (line == currentLine)
			{
				fscommand("click", currentLine);
			}
			else
			{
				currentLine = line;
				selectLine(currentLine);
			}
		}
	}
}


function onTabChanged()
{
	Selection.setSelection(0,0);
	hudText.scroll = 1;
}


function setAlpha(a:Number) : Void
{
	tabRow.tabMemFrag._alpha 	= a;
	tabRow.tabMemChart._alpha 	= a;
	tabRow.tabMemGram._alpha 	= a;
	tabRow.tabMemTree._alpha 	= a;
	
	Legend1._alpha 				= a;
	Legend2._alpha 				= a;
	Legend3._alpha 				= a;
	Legend4._alpha 				= a;
	Legend5._alpha 				= a;
	Legend6._alpha 				= a;
	Legend7._alpha 				= a;
	Legend8._alpha 				= a;
	//Legend9._alpha 			= a;
	
	SysLegend1._alpha			= a;
	SysLegend2._alpha			= a;
	SysLegend3._alpha			= a;
	//SysLegend4._alpha			= a;
	
	panel._alpha				= a;
	hudText._alpha				= a;
	
	pageUpBtn._alpha			= a;
	scrollUpBtn._alpha			= a;
	pageDnBtn._alpha			= a;
	scrollDownBtn._alpha		= a;
	
	refreshBtn._alpha			= a;

    memoGramGrid1._alpha        = a;
    memoGramGrid2._alpha        = a;
    memoGramGrid3._alpha        = a;
}


function setGraphBtnVisible(b:Boolean) : Void
{
    tabRow.tabMemTree._visible	= b;
	tabRow.tabMemFrag._visible 	= b;
	tabRow.tabMemChart._visible	= b;
	tabRow.tabMemGram._visible 	= b;
}


function setMemoGramGridVisible(b:Boolean) : Void
{
    memoGramGrid1._visible      = b;
    memoGramGrid2._visible      = b;
    memoGramGrid3._visible      = b;
}


function setMemoGramGridText1(s:String) : Void
{
    memoGramGrid1.text          = s;
}

function setMemoGramGridText2(s:String) : Void
{
    memoGramGrid2.text          = s;
}

function setMemoGramGridText3(s:String) : Void
{
    memoGramGrid3.text          = s;
}


function setMemoTreeVisible(b:Boolean) : Void
{
	hudText._visible 		    = b;
	panel._visible			    = b;
	refreshBtn._visible 	    = b;
	pageUpBtn._visible 		    = b;
	scrollUpBtn._visible 	    = b;
	scrollDownBtn._visible	    = b;
	pageDnBtn._visible 		    = b;
}


function increaseTransparency() : Void
{
	if (transparency > minimumTransp)
	{
		transparency -= transpInc;
	}
	setAlpha(transparency);
}


function decreaseTransparency() : Void
{
	if (transparency < maximumTransp)
	{
		transparency += transpInc;
	}
	setAlpha(transparency);
}


function setLegend1(heapColor:Number, heapName:String) : Void
{
	Legend1.Text.text = heapName;
	color = new Color(Legend1.Box);
	color.setRGB(heapColor);
}


function setLegend2(heapColor:Number, heapName:String) : Void
{
	Legend2.Text.text = heapName;
	color = new Color(Legend2.Box);
	color.setRGB(heapColor);
}


function setLegend3(heapColor:Number, heapName:String) : Void
{
	Legend3.Text.text = heapName;
	color = new Color(Legend3.Box);
	color.setRGB(heapColor);
}


function setLegend4(heapColor:Number, heapName:String) : Void
{
	Legend4.Text.text = heapName;
	color = new Color(Legend4.Box);
	color.setRGB(heapColor);
}


function setLegend5(heapColor:Number, heapName:String) : Void
{
	Legend5.Text.text = heapName;
	color = new Color(Legend5.Box);
	color.setRGB(heapColor);
}


function setLegend6(heapColor:Number, heapName:String) : Void
{
	Legend6.Text.text = heapName;
	color = new Color(Legend6.Box);
	color.setRGB(heapColor);
}


function setLegend7(heapColor:Number, heapName:String) : Void
{
	Legend7.Text.text = heapName;
	color = new Color(Legend7.Box);
	color.setRGB(heapColor);
}

function setLegend8(heapColor:Number, heapName:String) : Void
{
	Legend8.Text.text = heapName;
	color = new Color(Legend8.Box);
	color.setRGB(heapColor);
}

function setLegend9(heapColor:Number, heapName:String) : Void
{
	Legend9.Text.text = heapName;
	color = new Color(Legend9.Box);
	color.setRGB(heapColor);
}

function setSysHeapLegend1(heapColor:Number, heapName:String) : Void
{
	SysLegend1.Text.text = heapName;
	color = new Color(SysLegend1.Box);
	color.setRGB(heapColor);
}


function setSysHeapLegend2(heapColor:Number, heapName:String) : Void
{
	SysLegend2.Text.text = heapName;
	color = new Color(SysLegend2.Box);
	color.setRGB(heapColor);
}


function setSysHeapLegend3(heapColor:Number, heapName:String) : Void
{
	SysLegend3.Text.text = heapName;
	color = new Color(SysLegend3.Box);
	color.setRGB(heapColor);
}


//function setSysHeapLegend4(heapColor:Number, heapName:String) : Void
//{
//	SysLegend4.Text.text = heapName;
//	color = new Color(SysLegend4.Box);
//	color.setRGB(heapColor);
//}


function setLegendVisible(b:Boolean) : Void
{
	Legend1._visible 	    = b;
	Legend2._visible 	    = b;
	Legend3._visible 	    = b;
	Legend4._visible 	    = b;
	Legend5._visible 	    = b;
	Legend6._visible 	    = b;
	Legend7._visible 	    = b;
	Legend8._visible 	    = b;
	//Legend9._visible 	    = b;
	
	SysLegend1._visible     = b;
	SysLegend2._visible     = b;
	SysLegend3._visible     = b;
	//SysLegend4._visible   = b;

    TextureBorder._visible  = b;
    LegendBorder._visible   = b;
}


function setPanelPosition(b:Boolean) : Void
{
	if (b)
	{
        panel._y            = tabRow.tabMemTree._y + tabRow.tabMemTree._height;
	    panel._height      += tabRow.tabMemTree._height;
		hudText._y          = panel._y + 8;
		hudText._height    += tabRow.tabMemTree._height;

        refreshBtn._y      = refreshY + tabRow.tabMemTree._height;  
	}
	else
	{
		hudText._y 		    = hudTextY; 
		hudText._height     = hudTextHeight;
		panel._height 	    = panelHeight;
        panel._y            = panelY;

        refreshBtn._y       = refreshY;
	}

	pageUpBtn._y 		    = panel._y + 8;
	scrollUpBtn._y 		    = pageUpBtn._y      + pageUpBtn._height;
	scrollDownBtn._y 	    = scrollUpBtn._y    + scrollUpBtn._height;
	pageDnBtn._y 		    = scrollDownBtn._y  + scrollDownBtn._height;

}


// Main

init();

//memoGramGrid1.text = "aaaaaa";

setLegendVisible(false);
Legend9._visible 	= false;
SysLegend4._visible = false;

setMemoGramGridVisible(false);
stageListener.onResize();
// DBG
//testText = " 1aa\n 2aa\n 3aa\n 4sdf\n 5sadf\n 6asdf\n 7ff\n 8dd\n 1sdf\n 2\n 3\n 4\n 5\n 6\n 7\n 8\n";
//hudText.text = testText;

showHud(false);
setAlpha(transparency);
setGraphBtnVisible(false);

// DBG
//setHudFocus(false);
//setRefresh(true);
//setRefresh(false);
//setSystem("wii");

stop();
