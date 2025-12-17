/**************************************************************************

Filename    :   AttackEvent.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.events 
{
    import flash.events.Event;
    
    import com.scaleform.std.entities.MoveEntity;
    
    public class AttackEvent extends Event 
    {
    // Constants:
        public static const TOWER_ATTACK:String = "towerAttack";
        public static const UNIT_ATTACK:String = "unitAttack";
        public static const ABILITY_ATTACK:String = "abilityAttack";
        
    // Public Properties:
        public var attackType:uint = 0;
        public var attackTarget:MoveEntity = null;
        public var damage:Number = 0;
        public var isCrit:Boolean = false;
        public var effectType:uint = 0;
        public var effectValue:Number = 0;
        public var effectDuration:Number = 0;
        public var effectTickSpeed:Number = 0;
        public var sourceId:uint = 0;
        
    // Protected Properties:
        
    // Initialization:
        public function AttackEvent(type:String, bubbles:Boolean = true, cancelable:Boolean = true,
                                    attackTarget:MoveEntity = null, attackType:uint = 0, damage:Number = 0,
                                    sourceId:uint = 0, effectType:uint = 0, effectValue:Number = 0, 
                                    effectDuration:Number = 0, effectTickSpeed:Number = 0, isCrit:Boolean = false)
        {
            super(type, bubbles, cancelable);
            
            this.attackTarget = attackTarget;
            this.attackType = attackType;
            
            this.damage = damage;
            this.isCrit = isCrit;
            
            this.sourceId = sourceId;
            
            this.effectType = effectType;
            this.effectValue = effectValue;
            this.effectDuration = effectDuration;
            this.effectTickSpeed = effectTickSpeed;
        }
        
    // Public Methods:
        override public function clone():Event {
            return new AttackEvent(type, bubbles, cancelable, attackTarget, attackType, damage, sourceId, effectType, effectValue, effectDuration, effectTickSpeed);
        }
        
        override public function toString():String {
            return formatToString("AttackEvent", "type", "bubbles", "cancelable", "attackTarget", "attackType", "damage", "sourceId", "effectType", "effectValue", "effectDuration", "effectTickSpeed");
        }
        
    // Protected Methods:
        
    }
    
}