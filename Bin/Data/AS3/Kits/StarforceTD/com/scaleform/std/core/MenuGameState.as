/**************************************************************************

Filename    :   MenuGameState.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.core
{
    import flash.display.Loader;
    import flash.display.MovieClip;
    import flash.display.Stage;
    import flash.events.Event;
    import flash.net.URLRequest;
    import flash.system.LoaderContext;
    import flash.system.ApplicationDomain;
    
    import com.scaleform.std.core.GameState;

    public class MenuGameState extends GameState
    {
        protected var _menuLoader:Loader;
        
        public function MenuGameState( stage:Stage ) {
            super( stage );
        }
        
        override public function init():void {
            loadMenu();
        }
        
        override public function destroy():void {
            _stage.removeChild( _menuLoader );
            _menuLoader.unloadAndStop( true );
        }
        
        protected function loadMenu():void {
            if ( _menuLoader == null ) {
                _menuLoader = new Loader();
            }
            
            var url:URLRequest = new URLRequest( "StarforceMenu.swf" ); 
            var context:LoaderContext = new LoaderContext( false, ApplicationDomain.currentDomain );
            _menuLoader.contentLoaderInfo.addEventListener( Event.COMPLETE, handleMenuLoadComplete, false, 0, true );
            _menuLoader.load( url, context );
        }

        protected function handleMenuLoadComplete( event:Event ):void {
            _stage.addChildAt( _menuLoader, 0 );
        }
    }

}