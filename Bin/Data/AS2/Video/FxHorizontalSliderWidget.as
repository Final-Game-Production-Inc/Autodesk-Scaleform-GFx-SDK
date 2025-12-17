/**************************************************************************

Filename    :   FxHorizontalSliderWidget.as
Content     :   Horizontal Slider widget
Created     :   10/23/2007
Authors     :   David Cook, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

class FxHorizontalSliderWidget extends FxWidget
{
	private var Dragging:Boolean; 
	private var OldX:Number;
	private var GripMargin:Number;
	private var TrackLeft:Number;
	private var TrackRight:Number;
	private var GripOffsetLeft:Number;
	private var GripOffsetRight:Number;	

	private var IntervalIds:Array;
	private var Started:Boolean;
	private var Delay:Number;
	private var Repeat:Number;
	
	// Scrollbar ItemInfo
	private var ItemIndex:Number;
	private var ItemCount:Number;
	
	private var Slider:MovieClip;
	
	private var GripMC:MovieClip;
	private var GripHitTest:MovieClip;
	
	private var TrackMC:MovieClip;
	private var TrackHitTest:MovieClip;
	
	private var HitTestEnabled:Boolean;
	
	public var  onGripChange:Function;
	
	// ---- Constructor
	public function FxHorizontalSliderWidget(tag:String, hittestEnable:Boolean)
	{
		super(tag);
		
		// Scrollbar vars
		Dragging	= false;
		OldX		= 0;
		TrackLeft	= 0;
		TrackRight	= 0;
		GripOffsetLeft	= 0;
		GripOffsetRight = 0;		
		GripMargin	= 0;
			
		// item vars
		ItemIndex	= 0;
		ItemCount	= 0;
		
		// Timer vars
		IntervalIds = Array();
		Started		= false;
		Delay		= 300;
		Repeat		= 90;
		
		HitTestEnabled = hittestEnable;
		
		onGripChange = undefined;
	}
	
	// ---- Set the movie clips
	public function SetSlider(mc:MovieClip)
	{	
		Slider = mc;
		SetSliderObjects(mc.Grip, mc.Track);
		
		// setup slider
		Slider.Controller = this;
		Slider.onSetFocus = function()
		{
			this.Controller.AcquireFocus();
		}
		Slider.onKillFocus = function()
		{
			this.Controller.ReleaseFocus();
		}
		Slider.tabEnabled = true;
		Slider.tabChildren = false;
		Slider.focusEnabled = true;

		// setup grip
		GripHitTest.Controller = this;
		GripMC.Controller = this;		
		GripMC.onMouseMove = function()
		{
			this.Controller.DragGrip(this._x);
		}
		GripHitTest.onRelease = function()
		{
			if (this.Enabled != 0)
			{
				if (this.onReleaseCustom)
					this.onReleaseCustom();
			}
			this.Controller.Release();
		}
		GripHitTest.onReleaseOutside = function()
		{
			if (this.Enabled != 0)
			{
				this.Controller.Release();
				if (this.onReleaseOutsideCustom)
					this.onReleaseOutsideCustom();
			}
		}
		GripHitTest.onPress = function()
		{
			if (this.Enabled != 0)
			{
				if (this.onPressCustom)
					this.onPressCustom();
				this.Controller.Press(this);
			}
		}
		GripHitTest.onRollOver = function()
		{
			if (this.Enabled != 0)
			{
				if (!this.Controller.HasFocus())
				{
					this.Controller.Release();
				}
				if (this.onRollOverCustom)
					this.onRollOverCustom();
			}
		}
		GripHitTest.onRollOut = function()
		{
			if (this.Enabled != 0)
			{
				if (this.onRollOutCustom)
					this.onRollOutCustom();
			}
		}
		GripHitTest.onDragOut = function()
		{
			if (this.Enabled != 0)
			{
				if (!this.Controller.IsGripDragging())
				{
					if (this.onDragOutCustom)
						this.onDragOutCustom();
				}
			}
		}
		GripHitTest.onDragOver = function()
		{
			if (this.Enabled != 0)
			{
				if (!this.Controller.IsGripDragging())
				{
					if (this.onDragOverCustom)
						this.onDragOverCustom();
				}
			}
		}
		
		// setup track
		TrackHitTest.Controller = this;
		TrackHitTest.onPress = function()
		{
			// need the scale mult because the track's _yscale changed during UI construction
			this.Controller.Press(this, this._xmouse * (this._xscale / 100));
		}
		TrackHitTest.onRelease = function()
		{
			this.Controller.Release();
		}
		TrackHitTest.onRollOver = function()
		{
			if (!this.Controller.HasFocus())
			{
				this.Controller.Release();
			}			
		}
		TrackHitTest.onDragOut = function()
		{
			this.Controller.Release();
		}		
	}	
	
	// ---- Set the movie clips
	public function SetSliderObjects(grip, track)
	{
		if (grip == undefined ||
			track == undefined )
		{
			trace("Slider does not follow required structure!");
		}
		
		GripMC		= grip;
		TrackMC		= track;
		
		if (HitTestEnabled)
		{
			GripHitTest = GripMC.HitTest;
			TrackHitTest = TrackMC.HitTest;
			if (GripHitTest == undefined)
				trace(GripMC + ".HitTest not found!");
			if (TrackHitTest == undefined)
				trace(TrackMC + ".HitTest not found!");
		}		
		else
		{
			GripHitTest = GripMC;
			TrackHitTest = TrackMC;
		}
		
		// default grip reg point is MIDDLE
		SetGripRegPoint("MIDDLE");
		
		SetTrackBounds();
	}

	// ---- Set the grip margin
	public function SetGripMargin(margin)
	{
		GripMargin = margin;
		SetTrackBounds();
	}
	
	// ---- Set the grip registration point (TOP, MIDDLE, BOTTOM)
	public function SetGripRegPoint(regpoint:String)
	{
		if (regpoint != "TOP" && regpoint != "MIDDLE" && regpoint != "BOTTOM")
		{
			trace("Slider::SetGripRegPoint - Registration point must be TOP, MIDDLE, or BOTTOM.");
			return;
		}
		
		if (GripHitTest == undefined || typeof(GripHitTest) != "movieclip")
		{
			trace("Slider::SetGripRegPoint - Grip must be set to a MovieClip before SetGripRegPoint can be called.");
			return;
		}
		
		if (regpoint == "TOP")
		{
			GripOffsetLeft = 0;
			GripOffsetRight = GripHitTest._width;
		}
		else if (regpoint == "MIDDLE")
		{
			GripOffsetLeft = GripOffsetRight = (GripHitTest._width/2);
		}
		else
		{
			GripOffsetLeft = GripHitTest._width;
			GripOffsetRight = 0;
		}
			
		SetTrackBounds();
	}
	
	// ---- Set the track bounds
	public function SetTrackBounds()
	{
		TrackLeft	= TrackHitTest._x + GripOffsetLeft + GripMargin;
		TrackRight	= TrackHitTest._x + TrackHitTest._width - GripOffsetRight - GripMargin;
		
		SetGripPos();
	}

	// ---- Drag the grip
	public function DragGrip(mousex):Boolean
	{
		if (Dragging && OldX != mousex)
		{
			OldX 	= mousex;
			FireScrollDragEvent( ((mousex - TrackLeft) / (TrackRight - TrackLeft)) );
			return true;
		}
		return false;
	}

	// ---- Set the grip position (consequently the state of the uparrow & downarrow)
	public function SetGripPos()
	{	
		if (GripMC == undefined) return;
		
		if (ItemCount > 0)
		{
			GripMC.Enabled = true;
			if (GripMC._currentframe != 1)
			{
				GripMC.gotoAndPlay("off");
			}
		}
		else
		{
			GripMC.Enabled = false;
			GripMC.gotoAndPlay("disable");
		}
			
		if (GripMC.Enabled == true && !Dragging)
		{
			var OldX1 = Math.floor( TrackLeft + ((TrackRight - TrackLeft) * (ItemIndex / (ItemCount - 1))) );
			var OldX2 = Math.floor( TrackLeft + ((TrackRight - TrackLeft) * ((ItemIndex+1) / (ItemCount - 1))) );
			if (GripHitTest._x < OldX1 || GripHitTest._x >= OldX2)
				GripMC._x = OldX1;
		}		
	}
	
	// ---- Returns true if grip is being dragged
	public function IsGripDragging()
	{
		return Dragging;
	}

	// ---- Press event
	public function Press(obj:Object, mousex:Number)
	{
		if (obj == GripHitTest)
		{
			Dragging = true;
			startDrag(GripMC, false, TrackLeft, GripMC._y, TrackRight, GripMC._y);
		}
		else if (obj == TrackHitTest)
		{
			TrackPress(mousex);
		}	
	}

	// ---- Release event
	public function Release(noFocus:Boolean)
	{	
		if (noFocus == undefined || noFocus == false)
			Selection.setFocus(Slider);
		
		if (Dragging)
		{
			Dragging = false;
			stopDrag();
		}
		else
		{
			// clear all intervals set by the timers
			var id;
			Started = false;
			while(id = IntervalIds.pop())
			{
				clearInterval(id);
			}
		}
	}

	public function LeftPress()
	{
		ScrollLeft();
		IntervalIds.push(setInterval(this, "OnHoldLeft", Delay));
	}
	
	// ---- Scroll up event (and helper methods)
	public function ScrollLeft()
	{
		FireScrollEvent(-1);
	}
	
	public function OnHoldLeft()
	{		
		if (!Started)
		{
			ScrollLeft();
			IntervalIds.push(setInterval(this, "ScrollLeft", Repeat));
			Started = true;
		}
		if (!HasFocus())
		{
			Release();
		}		
	}
	
	public function RightPress()
	{
		ScrollRight();
		IntervalIds.push(setInterval(this, "OnHoldRight", Delay));
	}	
	
	// ---- Scroll down event (and helper methods)
	public function ScrollRight()
	{
		FireScrollEvent(1);
	}
	
	public function OnHoldRight()
	{
		if (!Started)
		{
			ScrollRight();
			IntervalIds.push(setInterval(this, "ScrollRight", Repeat));
			Started = true;
		}
		if (!HasFocus())
		{
			Release();
		}				
	}
	
	// ---- Track press event 
	public function TrackPress(mousex) 
	{
		mousex += GripOffsetLeft; 
		// figure out new index
		var newIdx = (mousex / TrackHitTest._width) * ItemCount;  
		if (GripHitTest._xmouse < -1)
		{
			TrackScrollLeft(newIdx);
			IntervalIds.push(setInterval(this, "OnHoldTrackLeft", Delay, newIdx));
		}
		else
		{
			TrackScrollRight(newIdx);
			IntervalIds.push(setInterval(this, "OnHoldTrackRight", Delay, newIdx));
		}
	}
	
	// ---- Track scroll up event (and helper methods)
	public function TrackScrollLeft(newIdx)  
	{
		if (ItemIndex > newIdx)
		{
			FireScrollEvent(-1);
		}
		else
		{
			Release();
		}
	}
	
	public function OnHoldTrackLeft(newIdx)
	{
		if (!Started)
		{
			TrackScrollLeft(newIdx);
			IntervalIds.push(setInterval(this, "TrackScrollLeft", Repeat, newIdx));
			Started = true;
		}
	}
	
	// ---- Track scroll down event (and helper methods)
	public function TrackScrollRight(newIdx)
	{  
		if (ItemIndex < newIdx)
		{
			FireScrollEvent(1);
		}
		else
		{
			Release();
		}
	}
	
	public function OnHoldTrackRight(newIdx)
	{
		if (!Started)
		{
			TrackScrollRight(newIdx);
			IntervalIds.push(setInterval(this, "TrackScrollRight", Repeat, newIdx));
			Started = true;
		}
	}
	
	// ---- Set the item count
	public function SetItemCount(items:Number)
	{
		ItemCount = items;
		SetGripPos();
		
		if (this.onGripChange)
			this.onGripChange(ItemIndex, ItemCount);
	}
	
	// ---- Get the item count
	public function GetItemCount():Number
	{
		return ItemCount;
	}
	
	// ---- Set the current item index
	public function SetItemIndex(idx:Number)
	{
		ItemIndex = idx;
		SetGripPos();
		
		if (this.onGripChange)
			this.onGripChange(ItemIndex, ItemCount);
	}
	public function SetItemIndexByPercentage(pct:Number)
	{
		ItemIndex = (ItemCount - 1) * pct;
		SetGripPos();
		
		if (this.onGripChange)
			this.onGripChange(ItemIndex, ItemCount);
	}	

	// ---- Get the item index
	public function GetItemIndex():Number
	{
		return ItemIndex;
	}

	// ---- Set the item info in a single call
	public function SetItemInfo(index:Number, items:Number)	
	{	
		ItemIndex	= index;
		ItemCount	= items;
				
		SetGripPos();
		
		if (this.onGripChange)
			this.onGripChange(ItemIndex, ItemCount);
	}
	
	// ---- Mouse wheel listener
	public function onMouseWheel(delta, target)
	{
		if (target != undefined && 
			 HasTarget(target))
		{
			delta = -delta;
			if ((delta > 0) ||
				(delta < 0))
				FireScrollEvent(delta);
		}
	}	
	
	// ---- [ACTION] Key down	
	public function onKeyDown()
	{
		if (Focus == true)
		{
			var code = Key.getCode();
			if (code == Key.LEFT)
			{
				Selection["captureFocus"](false); // stop arrow keys from moving focus	
				LeftPress();
			}
			else if (code == Key.RIGHT)
			{
				Selection["captureFocus"](false); // stop arrow keys from moving focus	
				RightPress();
			}
			else
			{
				if (KeyDownCallback != undefined)
				{
					KeyDownCallback(code);
				}
				Selection["captureFocus"](true);				
			}
		}
	}
	
	public function onKeyUp()
	{
		if (Focus == true)
		{
			var code = Key.getCode();
			if (code == Key.LEFT || code == Key.RIGHT)
			{
				Release();
			}
		}
	}
	
	// ---- Returns true if the target contains the scrollbar widget
	public function HasTarget(target)
	{
		return (target.indexOf(GripHitTest+"") >= 0	||
				target.indexOf(TrackHitTest+"") >= 0)
	}
	
	// ---- Fire a scroll event
	public function FireScrollEvent(dev:Number)  
	{
		ItemIndex += dev;
		if (ItemIndex < 0)
			ItemIndex = 0;
		else if (ItemIndex >= ItemCount)
			ItemIndex = ItemCount - 1;
		else
		{
			if (FsCommandsEnabled)
			{
				fscommand(Tag + "_OnScroll", dev);
			}			
		}
		SetGripPos();		
		
		if (this.onGripChange)
			this.onGripChange(ItemIndex, ItemCount);
	}
		
	// ---- Fire a scroll drag event
	public function FireScrollDragEvent(dev:Number)
	{
		ItemIndex = dev * (ItemCount - 1);
		
		if (FsCommandsEnabled)
		{
			fscommand(Tag + "_OnScrollDrag", dev);
		}
		
		if (this.onGripChange)
			this.onGripChange(ItemIndex, ItemCount);	
	}
	
	// ---- OVERRIDEN Initialize
	public function Initialize(path:String)
	{
		super.Initialize(path);
		if (this.onGripChange)
			this.onGripChange(ItemIndex, ItemCount);
	}
	
	// ---- OVERRIDEN Register widget as a system event listener
	public function RegisterListeners()
	{
		super.RegisterListeners();
		Mouse.addListener(this);		
	}
	
	// ---- OVERRIDEN Unregister widget as a system event listener
	public function UnregisterListeners()
	{
		super.UnregisterListeners();
		Mouse.removeListener(this);
	}		
}