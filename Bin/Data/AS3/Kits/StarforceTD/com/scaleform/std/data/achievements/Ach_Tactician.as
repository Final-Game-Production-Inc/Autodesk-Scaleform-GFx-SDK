/**************************************************************************

Filename    :   Ach_Tactician.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.achievements
{
    public class Ach_Tactician extends AchieveData
    {
        
    // Constants:
        
    // Protected Properties
        
    // Initialization
        public function Ach_Tactician() {
            super();
        }
        
    // Public Functions:
    
    // Protected Functions
        override protected function loadData():void {
            _name = "Tactician";
            _description = "Defeat 100 Enemies";
            _iconPath = "tacitician.png";
            _maxProgress = 100;
        }
    }
}