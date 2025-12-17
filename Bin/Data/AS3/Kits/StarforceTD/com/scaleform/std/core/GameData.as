/**************************************************************************

Filename    :   GameData.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.core
{
    import flash.geom.Point;
    import flash.utils.Dictionary;
    
    import com.scaleform.std.data.galaxies.GalaxyData;
    import com.scaleform.std.data.galaxies.GalaxyOneData;
    import com.scaleform.std.data.galaxies.GalaxyZeroData;
	import com.scaleform.std.data.galaxies.GalaxyTwoData;
    import com.scaleform.std.data.missions.Mission_0_0_Data;
    import com.scaleform.std.data.missions.Mission_1_0_Data;
    import com.scaleform.std.data.missions.Mission_1_1_Data;
	import com.scaleform.std.data.missions.Mission_2_0_Data;
    import com.scaleform.std.data.towers.TowerAPCData;
    import com.scaleform.std.data.towers.TowerData;
    import com.scaleform.std.data.towers.TowerMechData;
    import com.scaleform.std.data.towers.TowerOrbitalData;
    import com.scaleform.std.data.towers.TowerTankData;
    import com.scaleform.std.data.PlayerTechTree;
    
    import com.scaleform.std.data.abilities.AbilityData;
    
    import com.scaleform.std.data.PlayerData;
    import com.scaleform.std.system.SaveSystem;
    import com.scaleform.std.data.missions.MissionData;
    import flash.events.Event;
    import flash.net.URLLoader;
    import flash.net.URLRequest;
    
    public class GameData 
    {
        
    // Constants:
        // Definitions so that other classes can access the data for certain types through GameData.
        // Tank type definitions
        public static const TOWER_TYPE_NONE:uint = 0;
        public static const TOWER_TYPE_APC:uint  = 1;
        public static const TOWER_TYPE_MECH:uint = 2;
        public static const TOWER_TYPE_TANK:uint = 3;
        public static const TOWER_TYPE_OST:uint  = 4;
        
        // Player ability definitions
        public static const ABILITY_NONE:uint       = 0;
        public static const ABILITY_STRIKE:uint     = 1;
        public static const ABILITY_DROPSHIP:uint   = 2;
        
        // Galaxy definitions
        public static const GALAXY_ZERO:uint  = 0;
        public static const GALAXY_ONE:uint   = 1;
        public static const GALAXY_TWO:uint   = 2;
        
        // Mission defintions
        public static const MISSION_0_0:uint = 0;
        // public static const MISSION_0_1:uint = 1;
        public static const MISSION_1_0:uint = 1;
        public static const MISSION_1_1:uint = 2;
        public static const MISSION_2_0:uint = 3;
        
        // Enemy definitions
        public static const ENEMY_INFANTRY:uint         = 0;
        public static const ENEMY_ARMORED_INFANTRY:uint = 1;
        public static const ENEMY_DRONE:uint            = 2;
        public static const ENEMY_TANK:uint             = 3;
        public static const ENEMY_MECH:uint             = 4;
		public static const ENEMY_TEST:uint				= 5;
        
    // Protected Properties
        protected static var _playerData:PlayerData = null;
    
        // protected var _galaxyDefinitions:Array = null;
        public static var galaxies:Array = null;
       
        // Temporary player data here
        public static var pGalaxiesUnlocked:uint = 0; // 0 is 1 in this case.
        public static var pMissionsUnlocked:Dictionary;
        
        protected static var _towerData:Dictionary = null; 
        protected static var _abilityData:Dictionary = null; 
        protected static var _galaxyData:Dictionary = null;
        protected static var _missionData:Dictionary = null;
        
        protected static var _initialized:Boolean = false;
        
        protected static var _selectedGalaxyId:uint = 0;
        protected static var _selectedMissionId:uint = 0;
        
        protected static var   _showMenuTutorial:Boolean = true;
        protected static var   _showGameTutorial:Boolean = true;        
        
        public function GameData() {
            throw( new Error( "Error :: GameData.as, Do not use the GameData ctor. Use GameData.init() or the accessors instead." ) );
        }
        
        public static function init():void {
            if ( _initialized ) { return; }
            
            loadGameData();
        }
        
        protected static function loadGameData():void {
			var xmlLoader:URLLoader = new URLLoader();
			xmlLoader.addEventListener(Event.COMPLETE, processGameData);
			xmlLoader.load(new URLRequest("conf/stats.xml"));
		}        
        
        protected static function processGameData(e:Event):void {
			var xml:XML = new XML(e.target.data);
           
            configureTowerStats(xml.towers);
            configureAbilityStats(xml.abilities);
            configureGalaxies();
            configureMissions();
            
            _playerData = new PlayerData();
            SaveSystem.load( _playerData );
            unpackPlayerData();
            
            _initialized = true;            
        }
        
        protected static function unpackPlayerData():void {
            var gProgress:Array = _playerData.getGalaxyProgress();
            if ( gProgress != null ) {
                var isMission:Boolean = false;
                for ( var i:uint = 0; i < gProgress.length; i++ ) {
                    var data:String = String( gProgress[i] );
                    if ( data.indexOf( "g" ) == 0) {
                        isMission = true;
                        continue;
                    }
                    else if ( isMission ) {
                        setMissionNumStars( uint(data), uint(gProgress[++i]), false );
                    }
                }
            }
        }
        
        public static function getShowMenuTutorial():Boolean { return _showMenuTutorial; }
        public static function setShowMenuTutorial( show:Boolean, save:Boolean = true ):void {
            _showMenuTutorial = show;
            if (save) SaveSystem.save(_playerData);
        }
        
        public static function getShowGameTutorial():Boolean { return _showGameTutorial; }
        public static function setShowGameTutorial( show:Boolean, save:Boolean = true ):void {
            _showGameTutorial = show;
            if (save) SaveSystem.save(_playerData);
        }
        
        public static function setCurrentGalaxyId( gId:uint ):void {
            _selectedGalaxyId = gId;
        }
        public static function setCurrentMissionId( mId:uint ):void {
            _selectedMissionId = mId;
        }
        
        public static function getCurrentGalaxyId():uint {
            return _selectedGalaxyId;
        }
        public static function getCurrentMissionId():uint {
            return _selectedMissionId;
        }
        
        public static function getNumStars():uint {
            var numStars:uint = 0;
            for each (var g:GalaxyData in _galaxyData) { 
                numStars += g.getNumStars();
            }  
            return numStars;
        }
        public static function getMaxStars():uint {
            var maxStars:uint = 0;
            for each (var g:GalaxyData in _galaxyData) { 
                maxStars += g.getMaxStars();
            }  
            return maxStars;
        }
        
        public static function getTowerData( type:uint ):TowerData {
            return _towerData[ type ];
        }
        public static function getTowerAttackSpeed( type:uint, grade:uint ):Number {
            var td:TowerData = _towerData[ type ];
            return td.getAttackSpeed( grade );
        }
        public static function getTowerMinDamage( type:uint, grade:uint ):Number {
            var td:TowerData = _towerData[ type ];
            var minDmg:Number = td.getMinDamage( grade );
            switch( type ) {
                case TOWER_TYPE_APC:
                    break;
                case TOWER_TYPE_MECH:
                    minDmg = minDmg * ( 1 + PlayerTechTree.getTechValue( PlayerTechTree.TECH_MECH_SPD_DMG ) );
                    break;
                case TOWER_TYPE_TANK:
                    break;
                case TOWER_TYPE_OST:
                    break;
            }
            return Math.floor( minDmg );
        }
        
        public static function getTowerMaxDamage( type:uint, grade:uint ):Number {
            var td:TowerData = _towerData[ type ];
            var maxDmg:Number = td.getMaxDamage( grade );
            switch( type ) {
                case TOWER_TYPE_APC:
                    break;
                case TOWER_TYPE_MECH:
                    maxDmg = maxDmg * ( 1 + PlayerTechTree.getTechValue( PlayerTechTree.TECH_MECH_SPD_DMG ) );
                    break;
                case TOWER_TYPE_TANK:
                    break;
                case TOWER_TYPE_OST:
                    break;
            }
            return Math.floor( maxDmg );
        }
        
        public static function getTowerRange( type:uint, grade:uint ):Number {
            var td:TowerData = _towerData[ type ];
            return td.getRange( grade );
        }
        public static function getTowerUpgradeCost( type:uint, grade:uint ):Number {
            var td:TowerData = _towerData[ type ];
            return td.getUpgradeCost( grade );
        }
        public static function getTowerRefund( type:uint, grade:uint ):Number {
            var td:TowerData = _towerData[ type ];
            return td.getRefund( grade );
        }
        public static function getTowerCritical( type:uint, grade:uint ):Number {
            var td:TowerData = _towerData[ type ];
            return td.getCritChance( grade );
        }
        
        // Tower Effects
        public static function getTowerEffectType( type:uint, grade:uint ):Number {
            var td:TowerData = _towerData[ type ];
            return td.getEffectType();
        }
        public static function getTowerEffectValue( type:uint, grade:uint ):Number {
            var td:TowerData = _towerData[ type ];
            return td.getEffectValue( grade );
        }
        public static function getTowerEffectDuration( type:uint, grade:uint ):Number {
            var td:TowerData = _towerData[ type ];
            return td.getEffectDuration( grade );
        }
        public static function getTowerEffectTickSpeed( type:uint, grade:uint ):Number {
            var td:TowerData = _towerData[ type ];
            return td.getEffectTickSpeed( grade );
        }
        public static function getTowerEffectUpgradeCost( type:uint, grade:uint ):Number {
            var td:TowerData = _towerData[ type ];
            return td.getEffectUpgradeCost( grade );
        }
        
        
        // APC
        public static function getTowerHealth( type:uint, grade:uint ):Number {
            var td:TowerData = _towerData[ type ];
            return td.getHealth( grade );
        }
        public static function getTowerArmor( type:uint, grade:uint ):Number {
            var td:TowerData = _towerData[ type ];
            return td.getArmor( grade );
        }
        
        // OST
        public static function getTowerSplashRadius( type:uint, grade:uint ):Number {
            var td:TowerData = _towerData[ type ];
            return td.getSplashRadius( grade );
        }
        
        public static function getAbilityData( type:uint ):AbilityData {
            return _abilityData[ type ];
        }
        public static function getAbilityMinDamage( type:uint ):Number {
            var ad:AbilityData = _abilityData[ type ];
            return ad.getMinDamage();
        }
        public static function getAbilityMaxDamage( type:uint ):Number {
            var ad:AbilityData = _abilityData[ type ];
            return ad.getMaxDamage();
        }
        public static function getAbilityAttackSpeed( type:uint ):Number {
            var ad:AbilityData = _abilityData[ type ];
            return ad.getAttackSpeed();
        }
        public static function getAbilityDuration( type:uint ):Number {
            var ad:AbilityData = _abilityData[ type ];
            return ad.getDuration();
        }
        public static function getAbilityArmorPen( type:uint):Number {
            var ad:AbilityData = _abilityData[ type ];
            return ad.getArmorPen();
        }
        public static function getAbilitySplashRadius( type:uint ):Number {
            var ad:AbilityData = _abilityData[ type ];
            return ad.getSplashRadius();
        }
        public static function getAbilityHealth( type:uint ):Number {
            var ad:AbilityData = _abilityData[ type ];
            return ad.getHealth();
        }
        public static function getAbilityRange( type:uint ):Number {
            var ad:AbilityData = _abilityData[ type ];
            return ad.getRange();
        }
        public static function getAbilityCooldown( type:uint ):Number {
            var ad:AbilityData = _abilityData[ type ];
            return ad.getCooldown();
        }
        
        // Galaxies (Nebulas)
        public static function getNumAvailableGalaxies():Number {
            // @TODO: Use PlayerData.
            var numGalaxies:uint = 0;
            for (var key:Object in _galaxyData) { 
                numGalaxies++;
            } 
            return numGalaxies;
        }
        public static function getGalaxyIsAvailable( galaxyId:uint ):Boolean {
            var gd:GalaxyData = _galaxyData[ galaxyId ];
            var gMissionsIds:Array = gd.getMissionIds();
            for ( var i:uint = 0; i < gMissionsIds.length; i++ ) {
                if ( getMissionIsAvailable( gMissionsIds[i] ) ) {
                    return true;
                }
            }
            return false;
        }
        public static function getGalaxyName( galaxyId:uint ):String {
            var gd:GalaxyData = _galaxyData[ galaxyId ];
            return gd.getName();
        }
        public static function getGalaxyNumStars( galaxyId:uint ):Number {
            // @TODO: Use PlayerData.
            var gd:GalaxyData = _galaxyData[ galaxyId ];
            return gd.getNumStars();
        }
        public static function getGalaxyMaxStars( galaxyId:uint ):Number {
            var gd:GalaxyData = _galaxyData[ galaxyId ];
            return gd.getMaxStars();
        }
        public static function getGalaxyMapCoords( galaxyId:uint ):Point {
            var gd:GalaxyData = _galaxyData[ galaxyId ];
            return gd.getMapCoords();
        }
        public static function getGalaxyMissionIds( galaxyId:uint ):Array {
            var gd:GalaxyData = _galaxyData[ galaxyId ];
            return gd.getMissionIds();
        }
        public static function getGalaxyNebulaFrame( galaxyId:uint ):uint {
            var gd:GalaxyData = _galaxyData[ galaxyId ];
            return gd.getNebulaFrame();
        }
        
        // Missions (Systems)
        public static function setMissionNumStars( missionId:uint, numStars:uint, save:Boolean = true ):void {
            var md:MissionData = _missionData[ missionId ];
            if ( md != null && md.getNumStars() < numStars ) { 
                md.setNumStars( numStars );
            }
            
            if ( save ) { 
                _playerData.updateProgress();
                SaveSystem.save( _playerData );
            }
        }
        public static function getMissionIsAvailable( missionId:uint ):Boolean {
            var md:MissionData = _missionData[ missionId ];
            return md.getIsAvailable();
        }
        public static function getMissionSystemName( missionId:uint ):String {
            var md:MissionData = _missionData[ missionId ];
            return md.getSystemName();
        }
        public static function getMissionPlanetName( missionId:uint ):String {
            var md:MissionData = _missionData[ missionId ];
            return md.getPlanetName();
        }
        public static function getMissionNumStars( missionId:uint ):Number {
            // @TODO: Use PlayerData.
            var md:MissionData = _missionData[ missionId ];
            return md.getNumStars();
        }
        public static function getMissionMaxStars( missionId:uint ):Number {
            var md:MissionData = _missionData[ missionId ];
            return md.getMaxStars();
        }
        public static function getMissionMapCoords( missionId:uint ):Point {
            var md:MissionData = _missionData[ missionId ];
            return md.getMapCoords();
        }
        public static function getMissionDescription( missionId:uint ):String {
            var md:MissionData = _missionData[ missionId ];
            return md.getDescription();
        }
        public static function getMissionTemp( missionId:uint ):String {
            var md:MissionData = _missionData[ missionId ];
            return md.getTempString();
        }
        public static function getMissionClimate( missionId:uint ):String {
            var md:MissionData = _missionData[ missionId ];
            return md.getClimateString();
        }
        public static function getMissionMass( missionId:uint ):String {
            var md:MissionData = _missionData[ missionId ];
            return md.getMassString();
        }
        public static function getMissionPathFrame( missionId:uint ):String {
            var md:MissionData = _missionData[ missionId ];
            return md.getPathFrame();
        }
        public static function getMissionPlanetFrame( missionId:uint ):String {
            var md:MissionData = _missionData[ missionId ];
            return md.getPlanetFrame();
        }
        
        protected static function configureTowerStats(towers:XMLList):void {
            _towerData = new Dictionary( true );
            _towerData[ TOWER_TYPE_APC  ] = new TowerAPCData(towers.apc);
            _towerData[ TOWER_TYPE_TANK ] = new TowerTankData(towers.tank);
            _towerData[ TOWER_TYPE_MECH ] = new TowerMechData(towers.mech);
            _towerData[ TOWER_TYPE_OST  ] = new TowerOrbitalData(towers.ost);
        }
        
        protected static function configureAbilityStats(abilities:XMLList):void {
            _abilityData = new Dictionary( true );
            _abilityData[ ABILITY_STRIKE ]      = new AbilityData(abilities.strike);
            _abilityData[ ABILITY_DROPSHIP ]    = new AbilityData(abilities.dropship);
        }
        
        protected static function configureGalaxies():void {
            _galaxyData = new Dictionary( true );
            _galaxyData[ GALAXY_ZERO ]  = new GalaxyZeroData();
            _galaxyData[ GALAXY_ONE ]   = new GalaxyOneData();
			_galaxyData[ GALAXY_TWO ] = new GalaxyTwoData();
        }
        
        protected static function configureMissions():void {
            _missionData = new Dictionary( true );
            _missionData[ MISSION_0_0 ] = new Mission_0_0_Data();
            _missionData[ MISSION_1_0 ] = new Mission_1_0_Data();
            _missionData[ MISSION_1_1 ] = new Mission_1_1_Data();
			_missionData[ MISSION_2_0 ] = new Mission_2_0_Data();
        }
    }
}