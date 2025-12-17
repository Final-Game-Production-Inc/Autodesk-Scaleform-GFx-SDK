/**************************************************************************

Filename    :   PlayerTechTree.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data
{
    import flash.utils.Dictionary;
    
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.system.SaveSystem;
    import com.scaleform.std.data.tech.TechData;
    
    import com.scaleform.std.data.tech.mech.Tech_Mech_Cost;
    import com.scaleform.std.data.tech.mech.Tech_Mech_Spd_Dmg;
    
    public class PlayerTechTree 
    {
    // Static Constants
        public static const TECH_MECH_COST:String = "mCost";
        public static const TECH_MECH_SPD_DMG:String = "mSpdDmg";
        public static const TECH_MECH_31:String = "mech31";
        public static const TECH_MECH_32:String = "mech32";
        public static const TECH_MECH_41:String = "mech41";
        public static const TECH_MECH_42:String = "mech42";
        
        public static var techMechIds:Array = null;
        public static var techTankIds:Array = null;
        public static var techAPCIds:Array = null;
        public static var techOrbitalIds:Array = null;
        public static var techGeneralIds:Array = null;
        
    // Protected Variables
        protected static var _techTree:Dictionary = null;
        
    // Initialization
        public function PlayerTechTree() {
            //
        }
        
        public static function init():void {
            loadTechTreeData();
            loadTechTreeProgress();
        }
        
    // Public Functions
        public static function learnTech( techId:String ):void {
            var td:TechData = _techTree[ techId ];
            if ( td != null ) {
                td.setActive( true );
            }
        }
        public static function resetTechTree():void {
            for ( var key:String in _techTree ) {
                var td:TechData = _techTree[ key ] as TechData;
                if ( td != null ) {
                    td.setActive( false );
                }
            }
        }
        
        public static function getTechValue( techId:String ):Number {
            var td:TechData = _techTree[ techId ];
            return ( td != null && td.isActive() ) ? td.getValue() : 0;
        }
        
        public static function saveTechTreeProgress():void {
            SaveSystem.saveTechTree( getFlatTechTree() );
        }
        
    // Protected Functions
        protected static function getFlatTechTree():Array {
            var sharedObjectArray:Array = [];
            var idx:uint = 0;
            for ( var key:String in _techTree ) {
                sharedObjectArray[ idx++ ] = key;
                sharedObjectArray[ idx++ ] = _techTree[ key ].isActive();
            }
            // trace("Saving TechTree... sharedObjectArray: " + sharedObjectArray);
            return sharedObjectArray;
        }
        
        protected static function loadTechTreeData():void {
            _techTree = new Dictionary();
            _techTree[ TECH_MECH_COST ] = new Tech_Mech_Cost();
            _techTree[ TECH_MECH_SPD_DMG ] = new Tech_Mech_Spd_Dmg();
            
            techAPCIds = [];
            techMechIds = [ TECH_MECH_COST, TECH_MECH_SPD_DMG, TECH_MECH_31, TECH_MECH_32, TECH_MECH_41, TECH_MECH_42 ];
            techTankIds = [];
            techOrbitalIds = [];
            techGeneralIds = [];
        }
        
        protected static function loadTechTreeProgress():void {
            var sharedObjectArray:Array = SaveSystem.loadTechTree();
            if ( sharedObjectArray != null ) {
                for ( var i:uint = 0; i < sharedObjectArray.length; i++ ) {
                    var techId:String = sharedObjectArray[i];
                    var td:TechData = _techTree[ techId ] as TechData;
                    if ( td != null ) {
                        td.setActive( sharedObjectArray[++i] );
                    }
                }
            }
        }
    }
}