/**************************************************************************

Filename    :   WindowManager.as
Content    :   Window Manager for the MMO Kit.
 
                Manages creation/destruction of all windows spawned by 
                the UI. Also handles reflowing windows' locations based on
                the currently open windows.
                
                This class should have some sort of interface for opening 
                windows via index or name? In other words, the caller should
                not have to provide a reference to the MovieClip for the window.
                

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo.managers {
    
    import com.scaleform.mmo.core.MWindow;
    import com.scaleform.mmo.events.WidgetEvent;
    import flash.display.DisplayObject;
    import flash.display.DisplayObjectContainer;
    import flash.display.MovieClip;
    import flash.display.Sprite;
    import flash.display.Stage;
    import flash.events.Event;
    import flash.geom.Point;
    import flash.utils.getDefinitionByName;
    
    import scaleform.gfx.FocusManager;
    import scaleform.clik.utils.Padding;
    
    import com.scaleform.mmo.events.WindowEvent;
    
    public class WindowManager {
        
    // Constants:
        
    // Protected Properties:
        protected static var _stage:Stage;
        protected static var _canvas:MovieClip;
        protected static var _maxWindows:uint = 1;
        protected static var _openWindows:Array;
        
        // NFM: Maybe we should change this init() to be like the other managers. A getInstace() type of thing.
        public static function init(stage:Stage):void {
             WindowManager._stage = stage;
             WindowManager._openWindows = [];
             
            _canvas = new MovieClip();
            _stage.addChildAt(_canvas, 1);
            
            _stage.addEventListener(Event.ADDED, WindowManager.handleStageAddedEvent, false, 0, true);
            _stage.addEventListener(WindowEvent.WINDOW_READY, WindowManager.handleWindowReadyEvent);// , false, 0, true);// , 0, true);
        }
        
    // Public Methods:
        public static function showModal(mc:Sprite, bg:Sprite = null, controllerIdx:uint = 0):void {
            if (!_stage) { trace("WindowManager has not been initialized. Automatic initialization has not occured or has failed; call WindowManager.init() manually."); return; }
            
            // Remove from existing parents
            if (mc.parent) { mc.parent.removeChild(mc); }
            if (bg && bg.parent) { bg.parent.removeChild(mc); }
            
            // Reparent to popup canvas layer
            if (bg) {
                _canvas.addChild(bg);
            }
            _canvas.addChild(mc);
            
            FocusManager.setModalClip(mc, controllerIdx);
        }
        
        public static function closeWindowByRef( mc:MWindow ):Boolean {
            for (var i:uint = 0; i < _openWindows.length; i++) {
                if (_openWindows[i] == mc) {
                    _openWindows.splice(i, 1);
                    _canvas.removeChild(mc);
                    return true;
                }
            }
            return false;
        }
        
        public static function getNumWindowsOpen():uint {
            return _openWindows.length;
        }
        
        public static function spawnWindow( widgetID:uint, source:String, windowClass:String, windowTitle = ""):MWindow {
            var symbolClass:Class = getDefinitionByName( windowClass ) as Class;
            if (symbolClass == null) { return null; }
            
            var targetWindow:MWindow = new symbolClass() as MWindow;
            targetWindow.name = ("window" + (WindowManager._openWindows.length + 1));
            
            targetWindow.width = 500;
            targetWindow.height = 500;
            
            targetWindow.title = windowTitle;
            targetWindow.widgetID = widgetID;
            targetWindow.contentPadding = new Padding(43, 16, 16, 16);
            
            targetWindow.source = source;
            targetWindow.stageRef = WindowManager._stage;
            targetWindow.validateNow()
            
            var xcoord:Number = 40;
            var ycoord:Number = (_stage.stageHeight / 2) - 250; // arbitrary pos.
            var lastWindow = WindowManager._openWindows[_openWindows.length - 1];
            if (lastWindow) { 
                xcoord = lastWindow.x + lastWindow.width + 40; // 40 == buffer.
                if (xcoord > _stage.stageWidth) { xcoord = 40; } // let's keep everything on screen.
            }
            
            targetWindow.y = ycoord;
            
            _openWindows.push(targetWindow);
            return targetWindow;
        }
        
    // Protected Methods:
        protected static function handleStageAddedEvent(e:Event):void {
            // NFM: This needs to be thought-out for MMO Kit.
            // _stage.setChildIndex(_canvas, _stage.numChildren - 1);
            // _stage.addChildAt(_canvas, 0);
        }
        
        protected static function handleWindowReadyEvent(e:WindowEvent):void {
            _canvas.addChild(e.windowRef);
        }
    }
    
}