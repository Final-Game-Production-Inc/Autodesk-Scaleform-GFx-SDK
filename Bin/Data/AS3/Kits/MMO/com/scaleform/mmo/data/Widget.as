/**************************************************************************

Filename    :   Widget.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo.data {
    
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.utils.Dictionary;
    
    public class Widget
    {
        public var id:uint;
        public var fname:String;
        public var path:String;
        public var isWindow:Boolean;
        public var xcoord:int;
        public var ycoord:int;
        
        public function Widget( wid:uint,
                                wpath:String,
                                wfname:String,
                                wwindow:Boolean = false, 
                                wxcoord:int = 0, 
                                wycoord:int = 0) {
            super();
            id = wid;
            fname = wfname;
            path = wpath;
            isWindow = wwindow;
            xcoord = wxcoord;
            ycoord = wycoord;
        }
    }
}