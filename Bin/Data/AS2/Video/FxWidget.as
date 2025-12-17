/**************************************************************************

Filename    :   FxWidget.as
Content     :   Basic Widget ActionScript object
Created     :   5/16/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

class FxWidget
{
	private var Focus:Boolean;
	private var Tag:String;	// a unique tag defined by UI context, set by user	
	private var FsCommandsEnabled:Boolean;

	private var FocusTarget:MovieClip;
	private var FocusMovieClip:MovieClip;
	
	private var KeyDownCallback:Function;		
	
	// ---- Constructor
	public function FxWidget(tag:String)
	{
		Tag = tag;
		Focus = false;
		FsCommandsEnabled = true;
		FocusTarget = undefined;
		FocusMovieClip = undefined;		
	}
	
	// ---- Notify C++ to initialize the widget
	public function Initialize(widgetPath:String)
	{
		RegisterListeners();
		if (FsCommandsEnabled)
		{
			fscommand(Tag + "_OnInitialize", widgetPath);
		}
	}
	
	// ---- Notify C++ to uninitialize the widget
	public function Destroy()
	{
		UnregisterListeners();
		if (FsCommandsEnabled)
		{
			fscommand(Tag + "_OnDestroy");
		}
	}
	
	// ---- Get the widget's tag
	public function GetTag():String
	{
		return Tag;
	}
	
	// ---- Enable fscommands
	public function EnableFSCommands()
	{
		FsCommandsEnabled = true;
	}
	
	// ---- Disable fscommands
	public function DisableFSCommands()
	{
		FsCommandsEnabled = false;
	}
	
	// ---- Set the movie clip that displays the current focus 
	// ---- on the widget
	public function SetFocusMovie(fm:MovieClip)
	{
		FocusMovieClip = fm;
	}
	
	// ---- Acquire focus from the focus manager
	public function AcquireFocus()
	{
		Focus = true;
		
		if (FocusMovieClip != undefined)
		{
			FocusMovieClip.gotoAndPlay("on");
		}
	}
	
	// ---- Release focus from the focus manager
	public function ReleaseFocus()
	{	
		Focus = false;
		
		if (FocusMovieClip != undefined)
		{
			FocusMovieClip.gotoAndPlay("off");
		}
	}
	
	// ---- Does widget have focus?
	public function HasFocus():Boolean
	{
		return Focus;
	}
	
	// ---- Register widget as a system event listener
	public function RegisterListeners()
	{
		Key.addListener(this);
	}
	
	// ---- Unregister widget as a system event listener
	public function UnregisterListeners()
	{
		Key.removeListener(this);
	}	
	
	// ---- Register a function that will be called whenever a key is pressed,
	// ---- while the listbox has focus
	// ---- Callback function recieves one param: Key code
	public function RegisterKeyDownCallback(func:Function)
	{
		KeyDownCallback = func;
	}		
}
