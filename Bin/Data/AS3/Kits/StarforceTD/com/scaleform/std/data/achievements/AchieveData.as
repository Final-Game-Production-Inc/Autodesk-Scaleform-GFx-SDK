/**************************************************************************

Filename    :   AchieveData.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.achievements
{
    
    public class AchieveData 
    {
        
    // Constants:
        
    // Protected Properties
        protected var _name:String = "";
        protected var _description:String = "";
        protected var _iconPath:String = "";
        protected var _progress:Number = 0;
        protected var _maxProgress:Number = 0;
        protected var _value:Number = 0;
        
    // Initialization
        public function AchieveData() {
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
        public function getProgress():Number {
            return _progress;
        }
        public function getMaxProgress():Number {
            return _maxProgress;
        }
        public function getValue():Number {
            return _value;
        }
        
        public function setProgress( val:Number ):Boolean {
            _progress = val;
            return ( _progress == _maxProgress );
        }
        public function isComplete():Boolean {
            return ( _progress == _maxProgress );
        }
    
    // Protected Functions
        protected function loadData():void {}
        
    }

}