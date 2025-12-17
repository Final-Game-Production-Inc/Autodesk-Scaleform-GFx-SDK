/**************************************************************************

Filename    :   FxCandidateListBox.as
Content     :   ListBox interface ActionScript object
Created     :   10/12/2007
Authors     :   David Cook, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


class FxCandidateListBox
{	 
	private var ListObj:Object;
	private var ItemName:String;

	private var SelectedItemIndex:Number;
	
	private var DisplayItems:Array;
	private var DisplayItemIndex:Number;
	
	private var Items:Array;
	private var ItemDataMap:Array;
	
	private var  UnselectedFrame:Number;
	private var  SelectedFrame:Number;
	
	// CANDIDATE LIST PROPERTIES
	// color of unselected text
	private var	 TextColor:Number;
	
	// color of selected text
	private var	 SelectedTextColor:Number;
	
	// color of selected text background movie clip
	private var  SelectedTextBackgroundColor:Number;
	
	// color of unselected text background movie clip
	private var	 TextBackgroundColor:Number;
	
	// color of unselected index background movie clip
	private var	 IndexBackgroundColor:Number;
	
	// color of selected index background movie clip
	private var	 SelectedIndexBackgroundColor:Number;
	
	private var  FontSize:Number; 
	
	private var VisibleCount:Number;
	
	private var DisplayCallback:Function;

	// ---- Constructor
	public function FxCandidateListBox()
	{
		ListObj 		= undefined;
		ItemName 		= undefined;
		
		SelectedItemIndex = -1;
		
		DisplayItems	 = Array();
		DisplayItemIndex = 0;
		
		Items			= Array();		
		
		VisibleCount 	= 0;
		
		DisplayCallback = undefined;
		
	}
	
	 // Setting default values for candidate list
	public function InitCandidateListDefaults(styles:Object):Void 
	{
			SelectedTextBackgroundColor	= (styles.selectedTextBackgroundColor 	== undefined)? 0x94DB93: styles.selectedTextBackgroundColor;
			IndexBackgroundColor		= (styles.indexBackgroundColor 			== undefined)? 0xBEC9D1: styles.indexBackgroundColor;
			SelectedIndexBackgroundColor= (styles.selectedIndexBackgroundColor 	== undefined)? 0x6AB861: styles.selectedIndexBackgroundColor;
			SelectedTextColor 			= (styles.selectedTextColor == undefined)? 0xffffff: styles.selectedTextColor;
			TextColor 					= (styles.textColor 		== undefined)? 0x000000: styles.textColor;
			TextBackgroundColor 		= (styles.backgroundColor 	== undefined)? 0xEFF2F4: styles.backgroundColor;
			FontSize 					= (styles.fontSize 			== undefined)? 26: styles.fontSize;
		
	}
	
	public function SetCandidateListProps(_textColor:Number, selectedTextColor:Number, textBackgroundColor:Number,
										  selectedTextBackgroundColor:Number, fontSize:Number, 
										  indexBackgroundColor:Number, selectedIndexBackgroundColor:Number)
	{
	/*	
		trace(selectedTextColor + " " +  _textColor + " " + selectedTextBackgroundColor + " " +
										  textBackgroundColor + " " + indexBackgroundColor + " " +
										  selectedIndexBackgroundColor + " " + fontSize);
	*/
		SelectedTextBackgroundColor	= (selectedTextBackgroundColor >= 0)	? selectedTextBackgroundColor	: SelectedTextBackgroundColor;
		SelectedIndexBackgroundColor= (selectedIndexBackgroundColor >= 0)	? selectedIndexBackgroundColor	: SelectedIndexBackgroundColor;
		TextBackgroundColor 		= (textBackgroundColor >= 0)	? textBackgroundColor	: TextBackgroundColor;
		IndexBackgroundColor		= (indexBackgroundColor >= 0)	? indexBackgroundColor	: IndexBackgroundColor;
		FontSize 					= (fontSize >= 0)				? fontSize				: FontSize;
		SelectedTextColor 			= (selectedTextColor >= 0)		? selectedTextColor		: SelectedTextColor;
		TextColor 					= (_textColor >= 0)				? _textColor				: TextColor;
		//trace("FontSize = " + FontSize);

	}
	// ---- Create a list dynamically
	public function CreateCandidateList(list_mc:MovieClip, numRows:Number, itemName:String, x:Number, y:Number, rowMovie:String, 
										dataMap:Array, frames:Array, styles:Object)
	{		
		InitCandidateListDefaults(styles);
		for (var i=0; i < numRows; i++)
		{
			var nmc:MovieClip = list_mc.attachMovie(rowMovie, itemName+i, list_mc.getNextHighestDepth());
			nmc._x = x;
			nmc._y = y;
			nmc.ID.autoSize = true;
			nmc.Value.autoSize = true;
			y += (nmc._height) - 3;
		}
		
		SetItemDataMap(dataMap);	
		SetFrames(frames[0], frames[1]);					
		SetListObject(list_mc);
		SetItemName(itemName);	
		SetVisibleCount(numRows);	
	}		
	
	
	// ---- Clear the list (reset the item index)
	public function ClearList():Void
	{
	//	DisplayItems.splice(0); // Removes All items
		DisplayItemIndex = 0;
		SelectedItemIndex = -1;
	}
	
	// ---- Add a new list item
	public function Add():Void
	{
		if (arguments.length < ItemDataMap.length)
		{
			trace("Not enough arguements passed to Add.");
			return;
		}
		if (DisplayItemIndex < DisplayItems.length)
		{
			DisplayItems[DisplayItemIndex].ItemData = arguments;
			DisplayItemIndex++;
		}
	}	
	
	// ---- Set the visible list item count
	public function SetVisibleCount(items:Number):Void
	{
		VisibleCount = items;
		// Generate the display items array
		DisplayItems.splice(0);
		for (var i=0; i < items; i++)
		{
			var item = new FxCandidateListItemInterface(ListObj[ItemName+i]);		
			item.ItemDataMap = ItemDataMap;
			DisplayItems.push(item);
		}	
	}
	
	public function GetVisibleCount():Number
	{
		return VisibleCount;
	}
	
	// ---- Get a list item by index
	private function GetItemAtIndex(idx:Number):FxCandidateListItemInterface
	{
		return DisplayItems[idx];
	}	
	
	// ---- Display the list
	public function SetList():Void
	{
		if (ListObj == undefined)
		{
			trace("FxListBoxWidget::SetList - ListObj must be defined.");
			return;
		}
		
		if (ItemName == undefined)
		{
			trace("FxListBoxWidget::SetList - ItemName is undefined.");
			return;
		}
	
		var l = ItemDataMap.length;
		for (var i:Number=0; i < VisibleCount; i++)
		{
			var lobj = DisplayItems[i].ItemObj;
			if (SelectedItemIndex == i)
			{
				if (lobj != undefined && lobj._currentframe != SelectedFrame)
				{
					lobj.gotoAndPlay(SelectedFrame);
				}
			}
			else
			{
				if (lobj != undefined && lobj._currentframe != UnselectedFrame)
				{					
					lobj.gotoAndPlay(UnselectedFrame);
				}
			}
			if (i >= DisplayItemIndex)
			{
				lobj._visible = false;
				// set the correct frame
				
			}
			else
			{			
				lobj._visible = true;
				var item = DisplayItems[i];
				var itemData = item.ItemData;
				item.DisplayInfo();		
			}
		}
		if (DisplayCallback != undefined)
			DisplayCallback(SelectedItemIndex);
	}
		
	// ---- Set the list's data map
	public function SetItemDataMap(map:Array)
	{
		ItemDataMap = map;
		
		var itemDataNames:String;
		if (ItemDataMap.length > 0)
		{
			itemDataNames = ItemDataMap[0];
			for (var i=1; i<ItemDataMap.length; i++)
				itemDataNames += "," + ItemDataMap[i];
		}
		trace("in setItemDataMap");
		// Must use _root.SendIMECommand as opposed to SendIMECommand in order to avoid compiler error.
		_root.SendIMEMessage("CandidateList_OnSetItemDataMap", itemDataNames);
	//	_global.imecommand("CandidateList_OnSetItemDataMap", itemDataNames);
	}
	
	// ---- Display info for a list item
	public function DisplayInfo(name:String)
	{
		var item:FxCandidateListItemInterface;
		
		item = GetItem(name);
		if (item != undefined)
			item.DisplayInfo();
	}

	// ---- Get a list item by name
	private function GetItem(name:String):FxCandidateListItemInterface
	{
		if (ItemName == undefined)
			return;
		
		if (name == undefined)
			return;
		
		var i = parseInt(name.substring(ItemName.length));
		return DisplayItems[i];
	}	
	
	// ---- Select a list item
	public function SelectItem(name:String)
	{
		SelectedItemIndex = parseInt(name.substring(ItemName.length));
		_root.SendIMEMessage("CandidateList_OnItemSelect", SelectedItemIndex);
	//	_global.imecommand("CandidateList_OnItemSelect", SelectedItemIndex);
	//	SetList();	
	}
	
	// ---- Select an item by its index
	public function SelectDataItem(idx:Number)
	{		
		SelectedItemIndex = idx;		
	}
	
	// ---- Set the list object
	public function SetListObject(listobj:Object)
	{
		ListObj = listobj;
	}
	
	// ---- Set the item name prefix of each list item object
	public function SetItemName(itemname:String)
	{
		ItemName = itemname;
	}
	
	// ---- Set the action frames for the list items	
	public function SetFrames(usf:Number, sf:Number)
	{
		UnselectedFrame = usf;
		SelectedFrame = sf;
	}
	
	// ---- Register a function that will be called after the list items have been updated
	// ---- Useful for playing movieclips within each item (status flags, etc.)
	public function RegisterPostDisplayCallback(func:Function)
	{
		DisplayCallback = func;
	}
	
}
	

