/**************************************************************************

Filename    :   NextWaveEnemyIcon.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.hud
{
    import flash.display.MovieClip;

    public class NextWaveEnemyIcon extends MovieClip
    {
        public static const LABEL_INFANTRY_ICON:String = "inf";
        public static const LABEL_INFANTRY_ICON:String = "ainf";
        public static const LABEL_INFANTRY_ICON:String = "drone";
        public static const LABEL_INFANTRY_ICON:String = "mech";
        public static const LABEL_INFANTRY_ICON:String = "tank";
        
        
        public function NextWaveEnemyIcon() {
            super();
        }
        
        public function getFrameForType( enemyType:uint ):void {
            
        }
    }

}