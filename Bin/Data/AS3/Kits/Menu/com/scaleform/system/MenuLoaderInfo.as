/**************************************************************************

Filename    :   MenuLoaderInfo.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
 
package com.scaleform.system {
    
    import flash.display.LoaderInfo;
    import flash.events.Event;

    import com.scaleform.events.MenuLoaderEvent;

    public class MenuLoaderInfo extends LoaderInfo {
        public var loaderParams:Object;

        public function MenuLoaderInfo() {
            super();
            loaderParams = {};
        }

        override public function dispatchEvent(event:Event):Boolean {
            if (type == Event.INIT || type == Event.COMPLETE) {
                var menuEvent:MenuLoaderEvent = new MenuLoaderEvent(event.type, event.bubbles, event.cancelable, event.target, event.currentTarget, loaderParams);
                return super.dispatchEvent(menuEvent);
            }
            else {
                return super.dispatchEvent(event);
            }
        }
    }
}