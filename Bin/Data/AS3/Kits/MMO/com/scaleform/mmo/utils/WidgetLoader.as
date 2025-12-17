/**************************************************************************

Filename    :   WidgetLoader.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo.utils {
    
    import flash.display.Loader;
    
    public class WidgetLoader extends Loader {
        
        /** x-coordinate where loaded content should be placed relative to context. */
        public var xcoord:int = 0;
        /** y-coordinate where loaded content should be placed relative to context. */
        public var ycoord:int = 0;
        /** The widget's id from GameWidgetDefns.as. */
        public var widgetID:int;
        
        public function WidgetLoader() {
            super();
        }
    }
}