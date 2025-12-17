/**************************************************************************

Filename    :   MissionInfoView.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.menu
{
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.events.TouchEvent;
    import flash.text.TextField;
    
    import com.scaleform.std.events.MapEvent;
    import com.scaleform.std.controls.SBButton;
    
    import scaleform.clik.events.ButtonEvent;
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.events.GameEvent;

    public class MissionInfoView extends MovieClip 
    {        
        // Star label animations
        public static const LABEL_ANIMATION_STAR_OFF:String = "off";
        public static const LABEL_ANIMATION_STAR_ON:String = "on";
        
        // UI Scene
        public var PlanetData:PlanetDataDisplay; // All of the information related to the selected planet.
        public var PopupTitleText:MovieClip; // The textfield at the top-left that displays the name of the level.
        
        public var StarPoints:MovieClip; // The stars beside the "X" button that show the high score for the level.
        public var Star1:MovieClip; // StarPoints.Star1. First star in the StarPoints list.
        public var Star2:MovieClip; // StarPoints.Star2. Second star in the StarPoints list.
        public var Star3:MovieClip; // StarPoints.Star3. Third star in the StarPoints list.
        public var PopupPanel:MovieClip; // The surrounding border panel. 

		
        public var BtnGo:SBButton; // The large green button at the bottom of the view that launches the game.
        public var BtnClose:SBButton; // "X" button at the top-right that closes the view.
        
        protected var _selectedGalaxy:uint = 0;
        protected var _selectedMission:uint = 0;
        
        public function MissionInfoView() {
            super();
            
            mouseEnabled = true;
            
            Star1 = StarPoints.Star1;
            Star2 = StarPoints.Star2;
            Star3 = StarPoints.Star3;
            
            Star1.gotoAndPlay("off");
            Star2.gotoAndPlay("off");
            Star3.gotoAndPlay("off");
        }
        
        public function showMissionData( gId:uint, mId:uint ):void {           
            _selectedGalaxy = gId;
            _selectedMission = mId;
            
            PopupTitleText.titleText.text = GameData.getMissionSystemName( mId );
            
            var numStars:uint = GameData.getMissionNumStars( mId );
            switch ( numStars ) {
                case 3:
                    Star3.gotoAndPlay( LABEL_ANIMATION_STAR_ON );
                case 2:
                    Star2.gotoAndPlay( LABEL_ANIMATION_STAR_ON );
                case 1:
                    Star1.gotoAndPlay( LABEL_ANIMATION_STAR_ON );
                default:
                    break;
            }
            
            BtnGo.label = "LAUNCH MISSION";
            BtnGo.addEventListener( ButtonEvent.CLICK, handleButtonGoPress, false, 0, true );
            
            PlanetData.setInfo( GameData.getMissionDescription( mId ) );
            
            var planetFrame:String = GameData.getMissionPlanetFrame( mId );
            var pathFrame:String = GameData.getMissionPathFrame( mId );
            
            var planetClimate:String = GameData.getMissionClimate( mId );
            var planetTemp:String = GameData.getMissionTemp( mId );
            var planetMass:String = GameData.getMissionMass( mId );
            
            PlanetData.PlanetIcon.gotoAndPlay( planetFrame );
            PlanetData.PlanetPath.gotoAndPlay( pathFrame );
            
            PlanetData.setStats( planetClimate + "\n" + planetTemp + "\n" + planetMass );
            PlanetData.setLabels("CLIMATE\nAVG TEMP\nMASS");
            PlanetData.setTitle( GameData.getMissionPlanetName( mId ) );
        }
        
        protected function handleButtonGoPress( be:ButtonEvent ):void {
            disableButtons();
            stage.dispatchEvent( new GameEvent( GameEvent.LAUNCH_MISSION, true, true, _selectedGalaxy, _selectedMission ) ) ;
        }
        
        public function enableButtons():void {
            BtnGo.enabled = true;
            BtnClose.enabled = true;
        }
        public function disableButtons():void {
            BtnGo.enabled = false;
            BtnClose.enabled = false;
        }
    }

}