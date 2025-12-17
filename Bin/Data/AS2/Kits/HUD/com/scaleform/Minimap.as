/**************************************************************************

Filename    :   Minimap.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

/**
 * This class drives the minimap symbol.
 */
import flash.external.ExternalInterface;
import gfx.core.UIComponent;
import com.scaleform.MinimapControls;

class com.scaleform.Minimap extends UIComponent {
	
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
	var iconCounts:Object;

	public function Minimap() {
    //
    }	
	
	public function configUI():Void {	
		
		// Register minimap values with the app
		//
		// Params: 
		//			minimap movieclip (this),
		//			view radius, 
		//
		ExternalInterface.call("registerMiniMapView", this, 124);
		
		map.loadMovie(mapImagePath);
	}
	
}