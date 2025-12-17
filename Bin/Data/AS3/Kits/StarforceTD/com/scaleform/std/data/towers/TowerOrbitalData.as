/**************************************************************************

Filename    :   TowerOrbitalData.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.towers
{
    public class TowerOrbitalData extends TowerData 
    {
        
    // Constants
        
    // Initilaization
        public function TowerOrbitalData(data:XMLList) {
            super(data);
        }
        
    // Protected Functions
        override protected function loadXmlStats(data:XMLList):void {
            super.loadXmlStats(data);
            
            // Effect
            effectType = TowerData.EFFECT_AMP_DMG;
        }
    }

}