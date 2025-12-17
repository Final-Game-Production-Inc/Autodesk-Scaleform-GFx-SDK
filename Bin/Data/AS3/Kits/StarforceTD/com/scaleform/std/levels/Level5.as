/**************************************************************************

Filename    :   Level5.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.levels
{   
    import flash.display.MovieClip;
    
    public class Level5 extends Level
    {
        public var atmo:MovieClip;
        
        public function Level5() {
            super("conf/level5.xml");
        }
		
		override protected function getAtmosphere():MovieClip { return atmo; }
        
    }
}