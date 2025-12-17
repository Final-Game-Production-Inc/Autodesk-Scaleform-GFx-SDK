/**************************************************************************

Filename    :   AbilityData.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.abilities
{
    
    public class AbilityData 
    {
        
    // Constants:
        
    // Protected Properties
        protected var _minDamage:Number     = 0;
        protected var _maxDamage:Number     = 0;
        protected var _duration:Number      = 0;
        protected var _armorPen:Number      = 0;
        protected var _attackSpeed:Number   = 0;
        protected var _splashRadius:Number  = 0;
        protected var _health:Number        = 0;
        protected var _range:Number         = 0;
        protected var _cooldown:Number      = 0;
        
    // Initialization
        public function AbilityData(data:XMLList) {
            loadXmlStats(data);
        }

        protected function loadXmlStats(data:XMLList):void {        
            _minDamage = data.attribute("minDamage");
            _maxDamage = data.attribute("maxDamage");
            _duration = data.attribute("duration");
            _armorPen = data.attribute("armorPen");
            _attackSpeed = data.attribute("attackSpeed");
            _splashRadius = data.attribute("splashRadius");
            _health = data.attribute("health");
            _range = data.attribute("range");
            _cooldown = data.attribute("cooldown");
        }
        
    // Public Functions:
        public function getMinDamage():Number {
            return _minDamage;
        }
        public function getMaxDamage():Number {
            return _maxDamage;
        }
        public function getAttackSpeed():Number {
            return _attackSpeed;
        }
        public function getDuration():Number {
            return _duration;
        }
        public function getArmorPen():Number {
            return _armorPen;
        }
        public function getSplashRadius():Number {
            return _splashRadius;
        }
        public function getHealth():Number {
            return _health;
        }
        public function getRange():Number {
            return _range;
        }
        public function getCooldown():Number {
            return _cooldown;
        }
    }

}