/**************************************************************************

Filename    :   TechData.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.tech
{
    public class TechData 
    {
    // Constants:
        
    // Protected Properties
        protected var _name:String = "";
        protected var _description:String = "";
        protected var _iconPath:String = "";
        protected var _value:Number = 0;
        protected var _isActive:Boolean = true;
        
    // Initialization
        public function TechData() {
            loadData();
        }
        
    // Public Functions:
        public function getName():String {
            return _name;
        }
        public function getDesc():String {
            return _description;
        }
        public function getIconPath():String {
            return _iconPath;
        }
        public function getValue():Number {
            return _value;
        }
        
        public function isActive():Boolean {
            return _isActive;
        }
        public function setActive( val:Boolean ):void {
            _isActive = val;
        }
    
    // Protected Functions
        protected function loadData():void {}
        
    }

}