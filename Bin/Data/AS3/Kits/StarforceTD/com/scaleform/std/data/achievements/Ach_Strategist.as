/**************************************************************************

Filename    :   Ach_Strategist.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.achievements
{
    public class Ach_Strategist extends AchieveData
    {
        
    // Constants:
        
    // Protected Properties
        
    // Initialization
        public function Ach_Strategist() {
            super();
        }
        
    // Public Functions:
    
    // Protected Functions
        override protected function loadData():void {
            _name = "Strategist";
            _description = "Defeat 10 Enemies";
            _iconPath = "strategist.png";
            _maxProgress = 10;
        }
    }
}