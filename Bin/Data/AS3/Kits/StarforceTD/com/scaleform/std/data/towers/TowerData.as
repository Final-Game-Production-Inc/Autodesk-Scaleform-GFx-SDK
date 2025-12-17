/**************************************************************************

Filename    :   TowerData.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.towers
{
    public class TowerData 
    {
        
    // Constants:
        public static const EFFECT_NONE:uint    = 0;
        public static const EFFECT_SLOW:uint    = 1;
        public static const EFFECT_DOT:uint     = 2;
        public static const EFFECT_AMP_DMG:uint = 3;
        
        public static const GRADE_MAX = 2;
        public static const GRADE_EFFECT_MAX = 3;
        
    // Protected Properties
        // Stats by grade
        protected var upgradeCostByGrade:Array  = null;
        protected var refundByGrade:Array       = null;
        protected var rangeByGrade:Array        = null;
        protected var attackSpeedByGrade:Array  = null;
        protected var minDamageByGrade:Array    = null;
        protected var maxDamageByGrade:Array    = null;
        protected var critChanceByGrade:Array   = null;
        protected var armorPenByGrade:Array     = null;
        
        protected var healthByGrade:Array       = null;
        protected var armorByGrade:Array        = null;               
        
        protected var splashRadiusByGrade:Array    = null;
        
        protected var effectValueByGrade:Array          = null;
        protected var effectDurationByGrade:Array       = null;
        protected var effectTickSpeedByGrade:Array      = null;
        protected var effectUpgradeCostByGrade:Array    = null;
        
        // Effect of the tower on the enemies
        protected var effectType:uint = 0;
        
    // Initialization
        public function TowerData(data:XMLList) {           
            loadXmlStats(data);
        }

        protected function loadXmlStats(data:XMLList):void {
            upgradeCostByGrade  = [];
            refundByGrade       = [];
            rangeByGrade        = [];
            attackSpeedByGrade  = [];
            minDamageByGrade    = [];
            maxDamageByGrade    = [];
            critChanceByGrade   = [];
            armorPenByGrade     = [];
            
            healthByGrade       = []; // APC
            armorByGrade        = []; // APC      
            
            splashRadiusByGrade    = []; // OST
            
            effectTickSpeedByGrade      = [];
            effectDurationByGrade       = [];
            effectValueByGrade          = [];
            effectUpgradeCostByGrade    = [];
            
            var grades:XMLList = data.grade;
			for each (var grade:XML in grades) {
                var level:uint = grade.attribute("level");
                var cost:uint = grade.attribute("cost");
                var refund:uint = grade.attribute("refund");
                var range:uint = grade.attribute("range");
                var attSpd:Number = grade.attribute("attackSpeed");
                var minDmg:uint = grade.attribute("minDamage");
                var maxDmg:uint = grade.attribute("maxDamage");
                var crit:uint = grade.attribute("crit");
                var armorPen:uint = grade.attribute("armorPen");
                var effectTickSpd:uint = grade.attribute("effectTickSpeed");                
                var effectDuration:uint = grade.attribute("effectDuration");
                var effectVal:uint = grade.attribute("effectValue");
                var effectUpgrade:uint = grade.attribute("effectUpgradeCost");
                var health:uint = grade.attribute("health");
                var armor:uint = grade.attribute("armor");
                var splashRadius:uint = grade.attribute("splashRadius");

                upgradeCostByGrade[level] = cost;
                refundByGrade[level] = refund;
                rangeByGrade[level] = range;
                attackSpeedByGrade[level] = attSpd;
                minDamageByGrade[level] = minDmg;
                maxDamageByGrade[level] = maxDmg;
                critChanceByGrade[level] = crit;
                armorPenByGrade[level] = armorPen;
                effectTickSpeedByGrade[level] = effectTickSpd;
                effectDurationByGrade[level] = effectDuration;
                effectValueByGrade[level] = effectVal;
                effectUpgradeCostByGrade[level] = effectUpgrade;
                healthByGrade[level] = health;
                armorByGrade[level] = armor;
                splashRadiusByGrade[level] = splashRadius;
            }
        }
        
    // Public Functions:
        public function getUpgradeCost( grade:uint ):Number {
            return upgradeCostByGrade[ grade ];
        }
        public function getRefund( grade:uint ):Number {
            return refundByGrade[ grade ];
        }
        public function getRange( grade:uint ):Number {
            return rangeByGrade[ grade ];
        }
        public function getAttackSpeed( grade:uint ):Number {
            return attackSpeedByGrade[ grade ];
        }
        public function getMinDamage( grade:uint ):Number {
            return minDamageByGrade[ grade ];
        }
        public function getMaxDamage( grade:uint ):Number {
            return maxDamageByGrade[ grade ];
        }
        public function getCritChance( grade:uint ):Number {
            return critChanceByGrade[ grade ];
        }
        public function getArmorPen( grade:uint ):Number {
            return armorPenByGrade[ grade ];
        }
        public function getHealth( grade:uint ):Number {
            return healthByGrade[ grade ];
        }
        public function getArmor( grade:uint ):Number {
            return armorByGrade[ grade ];
        }
        public function getSplashRadius( grade:uint ):Number {
            return splashRadiusByGrade[ grade ];
        }
        
        public function getEffectType():Number {
            return effectType;
        }
        public function getEffectValue( eGrade:uint ):Number {
            return effectValueByGrade[ eGrade ];
        }
        public function getEffectUpgradeCost( eGrade:uint ):Number {
            return effectUpgradeCostByGrade[ eGrade ];
        }
        public function getEffectDuration( eGrade:uint ):Number {
            return effectDurationByGrade[ eGrade ];
        }
        public function getEffectTickSpeed( eGrade:uint ):Number {
            return effectTickSpeedByGrade[ eGrade ];
        }
    }

}