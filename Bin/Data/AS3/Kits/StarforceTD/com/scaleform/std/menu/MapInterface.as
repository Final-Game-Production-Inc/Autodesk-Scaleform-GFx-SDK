/**************************************************************************

Filename    :   MapInterface.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.menu
{
    
    import com.scaleform.std.events.MapEvent;
    import com.scaleform.std.events.TowerEvent;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.events.TouchEvent;
    import flash.geom.Point;
    import flash.utils.Dictionary;
    
    import fl.transitions.easing.*;
    
    import com.scaleform.std.controls.SBButton;
    
    import scaleform.clik.events.ButtonEvent;
    import scaleform.clik.motion.Tween;
    
    import flash.utils.getDefinitionByName;
    import com.scaleform.std.utils.Constants;
    import flash.display.DisplayObject;
    import com.scaleform.std.core.GameData;
    
    import com.scaleform.std.menu.controls.MapLocation;
    
    import com.scaleform.std.system.SoundSystem;

    public class MapInterface extends MovieClip 
    {
        
        // WMI
        protected var _startMouseX:Number, _startMouseY:Number;
        protected var _oldMouseX:Number;
        protected var _oldMouseY:Number;
        protected var _moving:Boolean;
        
        protected var _focusedLoc:MapLocation;
        protected var _lastMode:uint;
        
        protected var _centerTween:Tween;
        protected var _centerX:Number;
        protected var _centerY:Number;
        
        protected var _correctTween:Tween;
        protected var _btnTween:Tween;
        
        protected var _locations:Vector.<MapLocation>;
        protected var _active:Boolean;

        // UI Scene
        public var MissionTilt:MovieClip;
        public var GalaxyTilt:MovieClip;
        
        public var GalaxyLocations:MovieClip;
        public var MissionLocations:MovieClip;
        
        protected var _isTransitioning:Boolean = false;
        
        protected var _galaxyX:Number = 0;
        protected var _galaxyY:Number = 0;
        
        protected var _mx:Number = 0;
        protected var _my:Number = 0;
        
        protected var locButtonClass:Class = null;
        protected var locLabelClass:Class = null;
        
        protected var _galaxies:Array = null;
        protected var _galaxyMissions:Array = null;
        
        protected var _missions:Dictionary = null;
        
        protected var _selectedGalaxyId:int = 0;
        protected var _selectedMissionId:int = 0;
        
        public var GalaxyField:MovieClip;
        public var StarField:MovieClip;
        
        public var MapContainer:MovieClip;
        
        public var FlatG:MovieClip;
        public var FlatN:MovieClip;
        
        public var GalaxyMap:MovieClip;
        public var NebulaMap:MovieClip;
        
        public var GalaxyStars3D:MovieClip;
        public var NebulaStars3D:MovieClip;
        
        public var SelectedMap:MovieClip = null; // GalaxyMap or NebulaMap;
        public var SelectedStars:MovieClip = null;
        public var SelectedBG:MovieClip = null;
        
        public var SelectedGalaxy:SBButton;
        
        protected var _nebulaOriginalX:Number = 0;
        protected var _nebulaOriginalY:Number = 0;
		
		////
		
        protected var _panningBack:Boolean = false;
		private var _isPanning: Boolean = false;
		
		private var velX: Number = 0;
		private var velY: Number = 0;
		
		private var targetX: Number=0;
		private var targetY: Number=0;
        
        public function MapInterface() {
            super();
            
            _galaxies = [];
            _missions = new Dictionary( true );
            
            GameData.init(); // NFM: I don't want this here, but it won't hurt anything for testing the Menu in stand-alone mode.
            
            var numGalaxies:uint = GameData.getNumAvailableGalaxies();
            for ( var i:uint = 0; i < numGalaxies; i++ ) {
                if ( GameData.getGalaxyIsAvailable( i ) ) {
                    loadGalaxy( i );
                    var gMissionIds:Array = GameData.getGalaxyMissionIds( i );
                    var numMissions:uint = gMissionIds.length;
                    for ( var j:uint = 0; j < numMissions; j++ ) {
                        if ( GameData.getMissionIsAvailable( gMissionIds[ j ] ) ) {
                            loadMission( i, gMissionIds[ j ] );
                        }
                    }
                }
            }
            
            var p:MainMenuView = parent as MainMenuView;
            GalaxyField = p.GalaxyField;
            StarField = p.StarField;
            
            addEventListener( Event.ADDED_TO_STAGE, configUI, false, 0, true );
        }
		
		function distance(x1:Number, y1:Number, x2:Number, y2:Number): Number
		{
			var dx:Number = x1-x2;
			var dy:Number = y1-y2;
			return Math.sqrt(dx * dx + dy * dy);
		}
		
		function updateMap(evt:Event)
		{ 
			
			// dampen
			velX *= .565;
			velY *= .565;
			SelectedMap.x += velX;
			SelectedMap.y += velY;
			
			if ( SelectedMap == GalaxyMap ) { 
                GalaxyField.x = GalaxyField.x + velX / 10;
                GalaxyField.y = GalaxyField.y + velY / 10;
                GalaxyStars3D.x = GalaxyStars3D.x + velX / 4;
                GalaxyStars3D.y = GalaxyStars3D.y + velY / 4;
            }
            if ( SelectedMap == NebulaMap ) { 
                StarField.x = StarField.x + velX / 10;
                StarField.y = StarField.y + velY / 10;
                NebulaStars3D.x = NebulaStars3D.x + velX / 4;
                NebulaStars3D.y = NebulaStars3D.y + velY / 4;
            }
			
			
			if (_isPanning)
			{
				   
			}
			else
			{
				if (distance(0, 0, SelectedMap.x, SelectedMap.y) > 900)
				{
					velX -= (SelectedMap.x) * .125;
					velY -= (SelectedMap.y) * .125;
				}
			}

		}
        
        public function open():void {
            var eventType:String = Constants.isTouchEnabled() ? TouchEvent.TOUCH_BEGIN : MouseEvent.MOUSE_DOWN;
            parent.addEventListener( eventType, handleMapMouseDown, false, 0, true );
            
            eventType = Constants.isTouchEnabled() ? TouchEvent.TOUCH_END : MouseEvent.MOUSE_UP;
            parent.addEventListener( eventType, handleMapMouseUp, false, 0, true );
			
            addEventListener( MapEvent.GALAXY_SELECTED, handleLocationSelected, false, 0, true );   
			addEventListener(Event.ENTER_FRAME, updateMap);        
        }
        public function close():void {
            var eventType:String = Constants.isTouchEnabled() ? TouchEvent.TOUCH_BEGIN : MouseEvent.MOUSE_DOWN;
            parent.removeEventListener( eventType, handleMapMouseDown, false );
            
            eventType = Constants.isTouchEnabled() ? TouchEvent.TOUCH_END : MouseEvent.MOUSE_UP;
            parent.removeEventListener( eventType, handleMapMouseUp, false );
			
            removeEventListener( MapEvent.GALAXY_SELECTED, handleLocationSelected, false );      
			removeEventListener(Event.ENTER_FRAME, updateMap);      
        }
        
        protected function loadGalaxy( gId:uint ):void {
            var gCoords:Point = GameData.getGalaxyMapCoords( gId );
            var g:Object =  {   
                                id:gId,
                                label:GameData.getGalaxyName( gId ),
                                numStars:GameData.getGalaxyNumStars( gId ),
                                maxStars:(GameData.getGalaxyMissionIds( gId ).length * 3),
                                x:gCoords.x,
                                y:gCoords.y 
                            }
            _galaxies.push( g );
        }
        protected function loadMission( gId:uint, mId:uint ):void {
            var mCoords:Point = GameData.getMissionMapCoords( mId );
            var m:Object =  {   
                                id:mId,
                                label:GameData.getMissionSystemName( mId ),
                                numStars:GameData.getMissionNumStars( mId ),
                                x:mCoords.x,
                                y:mCoords.y 
                            }
            if ( _missions[ gId ] == null ) {
                _missions[ gId ] = [];
            }
            _missions[ gId ].push( m );
        }
        
        public function populateGalaxyStars( layer:MovieClip ):void {
            GalaxyStars3D = layer;
            layer.mouseEnabled = layer.mouseChildren = false;

            var minX:Number = -512;
            var maxX:Number = 512;

            var minY:Number = -256;
            var maxY:Number = 256;

            var minZ:Number = -2000;
            var maxZ:Number = 256;               
            
            var starClass:Class = getDefinitionByName("GalaxyStar") as Class;
            for ( var i:uint = 0; i < 100; i++ ) {
                
                var rx:Number = (minX) + (maxX - minX) * Math.random();
                var ry:Number = (minY) + (maxY - minY) * Math.random();
                var rz:Number = (minZ) + (maxZ - minZ) * Math.random();
                
                var cstar:MovieClip = new starClass();
                cstar.x = rx;
                cstar.y = ry;
                cstar.z = rz;
                
                layer.addChild( cstar );
            }
        }
        
        public function populateNebulaStars( layer:MovieClip ):void {
            NebulaStars3D = layer;
            // layer.rotationX = -50;
            layer.mouseEnabled = layer.mouseChildren = false;
            
            var starClass:Class = getDefinitionByName("GalaxyStar") as Class;
            for ( var i:uint = 0; i < 100; i++ ) {
                var minX:Number = -512;
                var maxX:Number = 512;
                
                var minY:Number = -256;
                var maxY:Number = 256;
                
                var minZ:Number = -2000;
                var maxZ:Number = 256;               
                
                var rx:Number = (minX) + (maxX - minX) * Math.random();
                var ry:Number = (minY) + (maxY - minY) * Math.random();
                var rz:Number = (minZ) + (maxZ - minZ) * Math.random();
                
                var cstar:MovieClip = new starClass();
                cstar.x = rx;
                cstar.y = ry;
                cstar.z = rz;
                // cstar.rotationX = 50;
                
                layer.addChild( cstar );
            }
        }
        
        protected function configUI(e:Event):void {
            removeEventListener( Event.ADDED_TO_STAGE, configUI, false );
            
            // WMI
            _active = true;
            
            // WMI: Was Terrain
            FlatG.MapContainer.z = -200;
            FlatG.MapContainer.rotationX = -50;
            
            FlatN.MapContainer.z = -200;
            FlatN.MapContainer.rotationX = -50;
            
            GalaxyMap = FlatG.MapContainer.MapG;
            NebulaMap = FlatN.MapContainer.MapN;
            
            _nebulaOriginalX = NebulaMap.x;
            _nebulaOriginalY = NebulaMap.y;
            
            SelectedMap = GalaxyMap;
            SelectedStars = GalaxyStars3D;
            SelectedBG = GalaxyField;
            
            FlatN.visible = false;
            // NebulaStars3D.visible = false;
            // WMI
            
            generateGalaxyButtons();
            
            stage.addEventListener( MapEvent.RETURN_TO_GALAX_MAP, handleReturnToGalaxyMap, false, 0, true );
        }
        
        protected function handleLocationSelected( e:Event ):void {
            if ( !_moving ) { 
                cleanupMove();
                var loc:MapLocation = e.target as MapLocation;
                if (loc != _focusedLoc) {
                    _focusedLoc = loc;
                    // updateAfterSelection();
                }
            }
        }
		
		private var moveTouchId: int = -1;
        
        protected function handleMapMouseDown( e:Event ):void {
            if ( !_isTransitioning  && !_isPanning) { 
                var mx:Number = e["stageX"];
                var my:Number = e["stageY"];
				
				if (Constants.isTouchEnabled())
					moveTouchId = (e as TouchEvent).touchPointID;
                
                _startMouseX = mx
                _startMouseY = my;
                
                _oldMouseX = _startMouseX;
                _oldMouseY = _startMouseY;
				
				_isPanning = true;
                
                var eventType:String = Constants.isTouchEnabled() ? TouchEvent.TOUCH_MOVE : MouseEvent.MOUSE_MOVE;
                parent.addEventListener( eventType, handleMapMove, false, 0, true );
                
                eventType = Constants.isTouchEnabled() ? TouchEvent.TOUCH_OUT : MouseEvent.MOUSE_OUT;
                parent.addEventListener( eventType, handleMapMouseOut, false, 0, true );    
            }
        }
        
        protected function handleMapMouseUp( e:Event ):void {
            if ( !_isTransitioning || (Constants.isTouchEnabled() && (e as TouchEvent).touchPointID==moveTouchId)) {
                cleanupMove();
            }
        }
        
        protected function handleMapMouseOut( e:Event ):void {
            if  ( !_isTransitioning || (Constants.isTouchEnabled() && (e as TouchEvent).touchPointID==moveTouchId)) { 
                if ( e is MouseEvent ) { 
                    var me:MouseEvent = e as MouseEvent;
                    if ( !me.buttonDown ) {
                        cleanupMove();
						//trace("mouse out");
                    }
                }
            }
        }
        
		private function clamp(val:Number, min:Number, max:Number){
			return Math.max(min, Math.min(max, val))
		}
		
        protected function cleanupMove():void {
            _moving = false;
			_isPanning = false;
            
            var eventType:String = Constants.isTouchEnabled() ? TouchEvent.TOUCH_MOVE : MouseEvent.MOUSE_MOVE;
            parent.removeEventListener( eventType, handleMapMove, false );
            
            eventType = Constants.isTouchEnabled() ? TouchEvent.TOUCH_OUT : MouseEvent.MOUSE_OUT;
            parent.removeEventListener( eventType, handleMapMouseOut, false );
                                   
        }
        
        protected function handleMapMove( e:Event ):void {
            if ( _isTransitioning || (Constants.isTouchEnabled() && (e as TouchEvent).touchPointID!=moveTouchId)) { return; }
            
            var mx:Number = e["stageX"];
            var my:Number = e["stageY"];
            
            var deltaX:Number = mx - _oldMouseX;
            var deltaY:Number = my - _oldMouseY;
            
            if (!_moving && (Math.abs(mx - _startMouseX) > 10 || Math.abs(my - _startMouseY) > 10)) {
                _moving = true;
            }
            
            
			velX += deltaX;
			velY += deltaY;

            
            _oldMouseX = mx;
            _oldMouseY = my;
        }
        
        protected function updateAfterSelection():void {
            // centerFocusedLocation(); // NFM: Single click takes us inside a galaxy -- handleGalaxyPress
            
            /*
            if (_lastMode != _focusedLoc.mode) {
                if (_focusedLoc.mode == WorldLocation.MODE_ACTIVE) {
                    stage.focus = enterBtn;
                    _btnTween = new Tween(500, enterBtn, { x:(854 + enterBtn.width)/2 - enterBtn.width + 96 }, { ease:Strong.easeOut } );
                } else {
                    stage.focus = null;
                    _btnTween = new Tween(500, enterBtn, { x:(854 + enterBtn.width)/2 }, { ease:Strong.easeIn } );
                }
            }
            _lastMode = _focusedLoc.mode;   
            
            GameInterface.selectMap(_focusedLoc.label);
            */
        }
        
        protected function centerFocusedLocation( onCompleteFunc:Function ):void {
            var map:DisplayObject = SelectedMap;                        
            if (_centerTween && !_centerTween.paused) {
                _centerTween.paused = true;
                map.x = _centerX;
                map.y = _centerY;
            }
            _centerX = -_focusedLoc.x;
            _centerY = -_focusedLoc.y;
            
            var deltaX:Number = _centerX - map.x;
            var deltaY:Number = _centerY - map.y;
            
            _centerTween = new Tween( 500, map, { x:_centerX, y:_centerY }, { ease:Strong.easeOut, onComplete:onCompleteFunc } );
            var t1:Tween = new Tween( 500, GalaxyField, { x:GalaxyField.x + deltaX / 10, y:GalaxyField.y + deltaY / 10 }, { ease:Strong.easeOut } );
            var t2:Tween = new Tween( 500, GalaxyStars3D, { x:GalaxyStars3D.x + deltaX / 4, y:GalaxyStars3D.y + deltaY / 4}, { ease:Strong.easeOut } );
        }
        
        protected function generateGalaxyButtons():void {
            var numGalaxies:uint = _galaxies.length;
            for (var i:uint = 0; i < numGalaxies; i++) {
                var g:Object = _galaxies[ i ];
                var gId:uint = _galaxies[ i ].id;
                var gButton:MapLocation = new MapLocation();
                gButton.x = g.x;
                gButton.y = g.y;
                gButton.data = gId;
                
                
                var mIds:Array = GameData.getGalaxyMissionIds( gId );
                var activeGalaxy:Boolean = ( GameData.getMissionNumStars( mIds[ mIds.length - 1] ) == 0 );
                gButton.selected = activeGalaxy;
                
                gButton.addEventListener( ButtonEvent.CLICK, handleGalaxyPress, false, 0, true );
                gButton.setData( g.label, true, g.numStars, g.maxStars );
                
                GalaxyMap.addChild( gButton );
            }
        }
        
        protected function generateMissionButtons():void {
            // Clean up the NebulaMap since it's shared between Nebulas.
            while ( NebulaMap.numChildren != 1 ) { 
                NebulaMap.removeChildAt( numChildren - 1 ); 
            }
            
            var numMissions:uint = _galaxyMissions.length;
            for (var i:uint = 0; i < numMissions; i++) {
                var m:Object = _galaxyMissions[ i ];
                
                var mButton:MapLocation = new MapLocation();
                mButton.x = m.x;
                mButton.y = m.y;
                mButton.data = m.id;
                
                if ( GameData.getMissionNumStars( m.id ) < 1 ) {
                    mButton.selected = true; 
                }
                
                mButton.addEventListener( ButtonEvent.CLICK, handleMissionPress, false, 0, true );
                mButton.setData( m.label, false, m.numStars, 3 );
                
                NebulaMap.addChild( mButton );
            }
        }
        
        protected function handleGalaxyPress( be:ButtonEvent ):void {
            handleLocationSelected( be );
            
            var gButton:SBButton = be.target as SBButton;
            SelectedGalaxy = gButton;
            var gId:uint = uint( gButton.data );
            
            // Get the center coordinates of the Button.
            // _galaxyX = gButton.x + ( gButton.width / 2 ) + stage.stageWidth / 2;
            // _galaxyY = gButton.y + ( gButton.height / 2 ) + stage.stageHeight / 2;
            
            _selectedGalaxyId = gId;
            showMissions( gId, true );
        }
        
        protected function handleMissionPress( be:ButtonEvent ):void {
            var mButton:SBButton = be.target as SBButton;
            var mId:uint = uint( mButton.data );
            
            _selectedMissionId = mId;
            dispatchEvent( new MapEvent( MapEvent.MISSION_SELECTED, true, true, _selectedGalaxyId, _selectedMissionId ) );
        }
        
        protected function configureMenu():void {
            // 
        }
        
        protected function handleReturnToGalaxyMap( me:MapEvent ):void {
            showGalaxies( true );
        }
        
        public function showGalaxies( animate:Boolean ):void {
            SelectedMap = GalaxyMap;
            SelectedStars = GalaxyStars3D;
            SelectedBG = GalaxyField;          
            
            if ( animate ) { 
                _isTransitioning = true;
                
                // We'll fade out the Galaxy and fade in the Mission.
                NebulaMap.alpha = 1;
                NebulaMap.z = 0;
                NebulaMap.visible = true;
                
                FlatG.alpha = 0.01;
                FlatG.x = 0;
                FlatG.y = 0;
                FlatG.z = -1000;
                FlatG.visible = true;
                
                GalaxyStars3D.alpha = 0.01;
                GalaxyStars3D.z = -1000;
                GalaxyStars3D.visible = true;
                
                mouseEnabled = mouseChildren = false;
                
                var gfTween:Tween = new Tween( 1000, GalaxyField, { alpha:1.0 } );
                var sfTween:Tween = new Tween( 1000, StarField, { alpha:0.0 } );
                
                var gTween:Tween = new Tween( 1000, FlatG, { z:0, x:_galaxyX, y:_galaxyY, alpha:1.0 }, { ease:Strong.easeOut, onComplete:onGalaxyTweenInComplete } );
                var gStarsTween:Tween = new Tween( 1000, GalaxyStars3D,  { z:0, alpha:1.0}, { ease:Strong.easeOut} );
                
                var nTween:Tween = new Tween( 1000, FlatN, { z:2000, alpha:0.0 }, { ease:Strong.easeOut, onComplete:onMissionTweenOutComplete } ) ;
                var nStarsTween:Tween = new Tween( 1000, NebulaStars3D,  { z:2000, alpha:0.0 }, { ease:Strong.easeOut } );
				
                var fade:Tween = new Tween( 100, SelectedGalaxy, { alpha:1.0 }, { ease:Strong.easeOut/*, delay: 500*/ });
                
                SoundSystem.playSound(SoundSystem.SOUND_FX, "GalaxyZoomOut.mp3");
            }
            else {
                FlatG.alpha = 1.0;
                FlatG.x = _galaxyX;
                FlatG.y = _galaxyY;
                FlatG.z = 0;
                FlatG.visible = true;
                
                GalaxyStars3D.alpha = 1;
                GalaxyStars3D.z = 0;
                GalaxyStars3D.visible = true;
                
                FlatN.visible = false;
                NebulaStars3D.visible = false;
            }
            
            FlatG.mouseEnabled = FlatG.mouseChildren = true;
        }
        
        public function showMissions( galaxyId:uint, animate:Boolean ):void {
            if ( animate ) {
                _isTransitioning = true;
                centerFocusedLocation( onGalaxyOutPart1 );
                
                // We'll fade out the Galaxy and fade in the Mission.
                FlatG.alpha = 1;
                
                FlatN.alpha = 0.01;
                // NebulaMap.scaleX = NebulaMap.scaleY = 0.01
                FlatN.z = 2000;
                FlatN.visible = true;
                
                NebulaStars3D.alpha = 0.01;
                NebulaStars3D.z = 2000;
                NebulaStars3D.visible = true;
                
                mouseEnabled = mouseChildren = false;
            } 
            else {
                FlatN.alpha = 1.0;
                FlatN.x = _galaxyX;
                FlatN.y = _galaxyY;
                FlatN.z = 0;
                FlatN.visible = true;
                
                NebulaStars3D.alpha = 1;
                NebulaStars3D.z = 0;
                NebulaStars3D.visible = true;
                
                FlatG.visible = false;
                GalaxyStars3D.visible = false;
            }
            
            SelectedMap = NebulaMap;
            SelectedStars = NebulaStars3D;
            SelectedBG = StarField;
            
            NebulaMap.BackgroundContainer.gotoAndStop( GameData.getGalaxyNebulaFrame( galaxyId ) );
            _galaxyMissions = _missions[ galaxyId ];
            generateMissionButtons();
            
            
            FlatN.mouseEnabled = FlatN.mouseChildren = true;
            
            dispatchEvent( new MapEvent( MapEvent.GALAXY_SELECTED, true, true, galaxyId ) );
        }
        
        protected function onGalaxyTweenInComplete( t:Tween ):void {
            //
        }
        protected function onGalaxyOutPart1( t:Tween ):void {
            var gTween:Tween = new Tween( 1000, FlatG, { z: -1000, alpha:0.0 }, { ease:Strong.easeOut, onComplete:OnGalaxyOutPart2 } );
            var gStarsTween:Tween = new Tween( 1000, GalaxyStars3D, { z: -1000, alpha:0.0 }, { ease:Strong.easeOut } );
            
            var nTween:Tween = new Tween( 1750, FlatN, { z:0, alpha:1.0 }, { ease:Strong.easeOut, onComplete:onMissionTweenInComplete } );
            var nStarsTween:Tween = new Tween( 1750, NebulaStars3D, { z:0, alpha:1.0 }, { ease:Strong.easeOut } );
            
            var sfTween:Tween = new Tween( 1000, StarField, { alpha:1.0 } );
            var gfTween:Tween = new Tween( 750, GalaxyField, { alpha:0.0 } );
            
            var fade:Tween = new Tween( 100, SelectedGalaxy, { alpha:0.0 }, { ease:Strong.easeIn/*, delay: 500*/ });
            
            SoundSystem.playSound(SoundSystem.SOUND_FX, "GalaxyZoomIn.mp3");			
        }
        protected function OnGalaxyOutPart2( t:Tween ):void {
            FlatG.visible = false;
            GalaxyStars3D.visible = false;
            FlatG.mouseEnabled = FlatG.mouseChildren = false;
            mouseEnabled = mouseChildren = true;
            _isTransitioning = false;
            
            dispatchEvent( new MapEvent( MapEvent.MISSION_TRANISITION_COMPLETE, true, true ) );
        }
        protected function onCleanupTweenComplete( t:Tween ):void {
            _isTransitioning = false;
        }
        
        // Mission Tween functions.
        protected function onMissionTweenInComplete( t:Tween ):void {
            //
        }
        protected function onMissionTweenOutComplete( t:Tween ):void {
            FlatN.visible = false;
            NebulaStars3D.visible = false;
            FlatN.mouseEnabled = FlatN.mouseChildren = false;
            NebulaMap.x = _nebulaOriginalX;
            NebulaMap.y = _nebulaOriginalY;
            mouseEnabled = mouseChildren = true;
            _isTransitioning = false;
            
            dispatchEvent( new MapEvent( MapEvent.GALAXY_TRANSITION_COMPLETE, true, true ) );
        }
        
    }

}
