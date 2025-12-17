/**************************************************************************

Filename    :   Tech_Mech_Cost.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.tech.mech
{
    import com.scaleform.std.data.tech.TechData;
    
    public class Tech_Mech_Cost extends TechData
    {
    // Constants:
        
    // Protected Properties
        
    // Initialization
        public function Tech_Mech_Cost() {
            super();
        }
        
    // Public Functions:
    
    // Protected Functions
        override protected function loadData():void {
            _name = "Upgrade 1";
            _description = "Decreases the cost of all Mech upgrades by 20%.";
            _iconPath = "mech1.png";
            _value = 0.20;
        }
    }
}