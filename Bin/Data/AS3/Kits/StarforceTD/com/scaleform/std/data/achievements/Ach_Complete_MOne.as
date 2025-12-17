/**************************************************************************

Filename    :   Ach_Complete_MOne.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.achievements
{
    public class Ach_Complete_MOne extends AchieveData
    {
        
    // Constants:
        
    // Protected Properties
        
    // Initialization
        public function Ach_Complete_MOne() {
            super();
        }
        
    // Public Functions:
    
    // Protected Functions
        override protected function loadData():void {
            _name = "Athena Victory";
            _description = "Complete the Athena Mission with 3 Stars";
            _iconPath = "m1Complete.png";
            _maxProgress = 1;
        }
    }
}