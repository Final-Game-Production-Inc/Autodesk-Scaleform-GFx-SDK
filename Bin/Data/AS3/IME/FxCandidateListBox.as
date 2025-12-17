
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
package 
{
import flash.display.*
	
public class FxCandidateListBox
{	 
	private var ListObj:MovieClip;
	private var ItemName:String;

	private var SelectedItemIndex:Number;
	
	private var DisplayItems:Array;
	private var DisplayItemIndex:Number;
	
	private var Items:Array;
	private var ItemDataMap:Array;
	
	// CANDIDATE LIST PROPERTIES
	// color of unselected text
	private var	 TextColor:Number;
	
	// color of selected text
	private var	 SelectedTextColor:Number;
	
	// color of selected text background movie clip
	private var  SelectedBackgroundColor:Number;
	
	// color of unselected text background movie clip
	private var	 BackgroundColor:Number;
	
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
		ListObj 		= null;
		ItemName 		= null;
		
		SelectedItemIndex = -1;
		
		DisplayItems	 = new Array();
		DisplayItemIndex = 0;
		
		Items			= new Array();		
		
		VisibleCount 	= 0;
		
		DisplayCallback = null;
		
	}
	
	 // Setting default values for candidate list
	public function InitCandidateListDefaults(styles:Object):void 
	{
			SelectedBackgroundColor		= (styles.selectedBackgroundColor 	== undefined)? 0x94DB93: styles.selectedBackgroundColor;
			IndexBackgroundColor		= (styles.indexBackgroundColor 			== undefined)? 0xBEC9D1: styles.indexBackgroundColor;
			SelectedIndexBackgroundColor= (styles.selectedIndexBackgroundColor 	== undefined)? 0x6AB861: styles.selectedIndexBackgroundColor;
			SelectedTextColor 			= (styles.selectedTextColor == undefined)? 0xffffff: styles.selectedTextColor;
			TextColor 					= (styles.textColor 		== undefined)? 0x000000: styles.textColor;
			BackgroundColor 			= (styles.backgroundColor 	== undefined)? 0xEFF2F4: styles.backgroundColor;
			FontSize 					= (styles.fontSize 			== undefined)? 26: styles.fontSize;
		
	}
	
	public function SetCandidateListProps(_textColor:Number, selectedTextColor:Number, _backgroundColor:Number,
										  selectedBackgroundColor:Number, fontSize:Number, 
										  indexBackgroundColor:Number, selectedIndexBackgroundColor:Number)
	{
	/*	
		trace(selectedTextColor + " " +  _textColor + " " + selectedTextBackgroundColor + " " +
										  textBackgroundColor + " " + indexBackgroundColor + " " +
										  selectedIndexBackgroundColor + " " + fontSize);
	*/
		SelectedBackgroundColor		= (selectedBackgroundColor >= 0)	? selectedBackgroundColor	: SelectedBackgroundColor;
		SelectedIndexBackgroundColor= (selectedIndexBackgroundColor >= 0)	? selectedIndexBackgroundColor	: SelectedIndexBackgroundColor;
		BackgroundColor 			= (_backgroundColor >= 0)	? _backgroundColor	: BackgroundColor;
		IndexBackgroundColor		= (indexBackgroundColor >= 0)	? indexBackgroundColor	: IndexBackgroundColor;
		FontSize 					= (fontSize >= 0)				? fontSize				: FontSize;
		SelectedTextColor 			= (selectedTextColor >= 0)		? selectedTextColor		: SelectedTextColor;
		TextColor 					= (_textColor >= 0)				? _textColor				: TextColor;
		//trace("FontSize = " + FontSize);

	}
	
	// ---- Create a list dynamically
	public function CreateCandidateList(list_mc:MovieClip, numRows:Number, itemName:String, x:Number, y:Number, rowMovie:String, 
										dataMap:Array, styles:Object)
	{	
		InitCandidateListDefaults(styles);
		for (var i=0; i < numRows; i++)
		{
			var nmc:MovieClip = new ListRow(this);
			list_mc.addChild(nmc);
			nmc.name = itemName + i; 
			nmc.x = x;
			nmc.y = y;
			nmc.ID.autoSize = true;
			nmc.Value.autoSize = true;
			y += (nmc.height) - 3;
		}
	
		SetListObject(list_mc);
		SetItemDataMap(dataMap);	
		SetItemName(itemName);	
		SetVisibleCount(numRows);	
	}		
	
	public function DestroyList():void
	{
		for (var i=0; i < VisibleCount; i++)
		{
			DisplayItems[i] = null;
		}	
	}
	
	// ---- Clear the list (reset the item index)
	public function ClearList():void
	{
	//	DisplayItems.splice(0); // Removes All items
		DisplayItemIndex = 0;
		SelectedItemIndex = -1;
	}
	
	// ---- Add a new list item
	public function Add():void
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
	public function SetVisibleCount(items:Number):void
	{
		VisibleCount = items;
		// Generate the display items array
		DisplayItems.splice(0);
		for (var i=0; i < items; i++)
		{
			var item = new FxCandidateListItemInterface(MovieClip(ListObj.getChildByName(ItemName+i)));		
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
	public function SetList():void
	{
		if (ListObj == null)
		{
			trace("FxListBoxWidget::SetList - ListObj must be defined.");
			return;
		}
		
		if (ItemName == null)
		{
			trace("FxListBoxWidget::SetList - ItemName is undefined.");
			return;
		}
	
		var l = ItemDataMap.length;
		for (var i:Number=0; i < VisibleCount; i++)
		{
			var tmp:FxCandidateListItemInterface =  DisplayItems[i] as FxCandidateListItemInterface;
			var lobj = (tmp).ItemObj;
			if (i < DisplayItemIndex)
			{
				lobj.visible = true;
				if (SelectedItemIndex == i)
				{
					if (lobj != null )
					{
						//lobj.gotoAndPlay(SelectedFrame);
						lobj.GotoOnState();
					}
				}
				else
				{
					if (lobj != null )
					{					
					//	lobj.gotoAndPlay(UnselectedFrame);
						lobj.GotoOffState();
					}
				}
			}
			if (i >= DisplayItemIndex)
			{
				lobj.visible = false;
				// set the correct frame
				
			}
			else
			{			
			//	lobj.visible = true;
			//	var item = DisplayItems[i] as FxCandidateListItemInterface;
			//	var itemData = item.ItemData;
			//	item.DisplayInfo();		
			}
		}
		if (DisplayCallback != null)
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
		 MovieClip(ListObj.parent).SendIMEMessage("CandidateList_OnSetItemDataMap", itemDataNames);
	//	_global.imecommand("CandidateList_OnSetItemDataMap", itemDataNames);
	}
	
	// ---- Display info for a list item
	public function DisplayInfo(name:String)
	{
		var item:FxCandidateListItemInterface;
		
		item = GetItem(name);
		if (item != null)
			item.DisplayInfo();
	}

	// ---- Get a list item by name
	private function GetItem(name:String):FxCandidateListItemInterface
	{
		if (ItemName == null)
			return null;
		
		if (name == null)
			return null;
		
		var i = parseInt(name.substring(ItemName.length));
		return DisplayItems[i];
	}	
	
	// ---- Select a list item
	public function SelectItem(name:String)
	{
		SelectedItemIndex = parseInt(name.substring(ItemName.length));
		MovieClip(ListObj.parent).SendIMEMessage("CandidateList_OnItemSelect", SelectedItemIndex);
	//	_global.imecommand("CandidateList_OnItemSelect", SelectedItemIndex);
	//	SetList();	
	}
	
	// ---- Select an item by its index
	public function SelectDataItem(idx:Number)
	{		
		SelectedItemIndex = idx;		
	}
	
	// ---- Set the list object
	public function SetListObject(listobj:MovieClip)
	{
		ListObj = listobj;
	}
	
	// ---- Set the item name prefix of each list item object
	public function SetItemName(itemname:String)
	{
		ItemName = itemname;
	}

	// ---- Register a function that will be called after the list items have been updated
	// ---- Useful for playing movieclips within each item (status flags, etc.)
	public function RegisterPostDisplayCallback(func:Function)
	{
		DisplayCallback = func;
	}
	
}
}

	

