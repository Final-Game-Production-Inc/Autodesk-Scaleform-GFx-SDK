/**************************************************************************

Filename    :   FxCandidateListItemInterface.as
Content     :   List item ActionScript object
Created     :   2/23/2007
Authors     :   David Cook, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

class FxCandidateListItemInterface
{ 
	public var  ItemObj:Object;
	public var  ItemDataMap:Array;
	public var 	ItemData:Array;	

	// ---- Constructor
	public function FxCandidateListItemInterface(mc:MovieClip)
	{
		ItemObj		= mc;
	}
	
	// ---- Get the data for a specific key
	public function GetData(key:String)
	{
		for (var i=0; i < ItemDataMap.length; i++)
		{
			if (ItemDataMap[i] == key)
				return ItemData[i];
		}
	}
	
	// ---- Display the item data
	public function DisplayInfo()
	{
		for (var j=0; j < ItemDataMap.length; j++)
		{
			// set the text values of the movie this relates to
			var obj = ItemObj[ItemDataMap[j]];
			if (obj != undefined)
			{
				obj.replaceText(0, obj.length, ItemData[j]);	
			}
		}		
	}
}