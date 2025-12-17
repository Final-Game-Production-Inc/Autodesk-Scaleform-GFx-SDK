/**************************************************************************

Filename    :   Minimap.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform {

/**
 * This class drives the minimap symbol.
 */

import flash.display.MovieClip;
import flash.display.Loader;
import flash.events.*;
import flash.events.ProgressEvent;
import flash.external.ExternalInterface;
import flash.net.URLRequest;

public class Minimap extends MovieClip {
	
	// The entities, layers and controls of the minimap
	public var player:MovieClip;
	public var icons_waypoint:MovieClip;
	public var icons_flag:MovieClip;
	public var icons_diamond:MovieClip;
	public var compass:MovieClip;
	public var icons_player_red:MovieClip;
	public var icons_player_blue:MovieClip;
	public var map:MovieClip;
	
	// Path to load the terrain map. This variable can be overwritten from the application
	// during the registerMiniMapView XI.call to change the loading path.
	public var mapImagePath:String = "map.jpg";
	
	// Keeps track of generated icon counts
	private var iconCounts:Object;

	public function Minimap() {
    	addEventListener(Event.ADDED_TO_STAGE, configUI);
    }	
	
	private function configUI(e:Event):void {	
		if (e.eventPhase == EventPhase.AT_TARGET && e.target == this)
		{
		    //trace("configUI "+e);
			removeEventListener(Event.ADDED_TO_STAGE, configUI)

			var l:Loader = new Loader();
			l.contentLoaderInfo.addEventListener(Event.COMPLETE, registerMiniMapView);
			map.addChild(l);
			l.load(new URLRequest(mapImagePath));
		
		}
	}
	
	private function registerMiniMapView(e:Event):void {
		// Register minimap values with the app
		//
		// Params: 
		//			minimap movieclip (this),
		//			view radius, 
		//
		ExternalInterface.call("registerMiniMapView", this, 124);		
	}
	
} // class Minimap

} // package com.scaleform
