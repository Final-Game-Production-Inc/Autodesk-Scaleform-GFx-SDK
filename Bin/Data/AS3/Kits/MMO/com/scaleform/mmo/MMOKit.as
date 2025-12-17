/**************************************************************************

Filename    :   MMOKit.as
Content    :   Primary class for the MMO Kit. DocumentClass for MMOKit.fla.
                Registers itself with C++ and loads the appropriate widgets (SWFs).
                

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
 
package com.scaleform.mmo {
    
    import flash.display.DisplayObject;
    import flash.display.Loader;
    import flash.display.MovieClip;
    import flash.display.Sprite;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.external.ExternalInterface;
    import flash.net.URLRequest;
    import flash.system.LoaderContext;
    import flash.text.TextField;
    import flash.ui.Mouse;
    import flash.utils.Dictionary;
	import scaleform.clik.core.CLIK;

    import scaleform.clik.controls.Button;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.events.DragEvent;
    import scaleform.clik.managers.DragManager;
    import scaleform.clik.utils.Padding;
    import scaleform.clik.data.DataBinding;
    
    import com.scaleform.mmo.GameDataModel;
    import com.scaleform.mmo.core.MDragTarget;
    import com.scaleform.mmo.core.MWindow;
    import com.scaleform.mmo.data.Widget;
    import com.scaleform.mmo.events.WidgetEvent;
    import com.scaleform.mmo.events.TooltipEvent;
    import com.scaleform.mmo.events.WindowEvent;
    import com.scaleform.mmo.managers.WindowManager;
    import com.scaleform.mmo.utils.WidgetLoader;
    import com.scaleform.mmo.utils.TooltipManager;
    
    import scaleform.gfx.InteractiveObjectEx;
    
    public class MMOKit extends MovieClip {
    
    // Public Properties:
        
    // Protected Properties:
        protected var _widgetDefns:GameWidgetDefns;
        
        /** Hash table of all loaded widgets. Mapping of widgetID (key) to widget reference (value). */
        protected var _loadedWidgets:Dictionary;
        /** Array of widgetIDs that correspond to external widgets currently being loaded. */
        protected var _widgetsLoading:Array;

    // UI Elements:
        /** Reference to Sprite to which all external widgets will be attached. */
        protected var _widgetCanvas:Sprite;
        /** Reference to the custom cusor symbol. */
        protected var myCursor:MCursor;
        
        public function MMOKit() {
            super();
            
			CLIK.disableTextFieldToNullFocusMoves = false;
			
            _loadedWidgets = new Dictionary();
            _widgetsLoading = [];
            
            addEventListener(Event.ENTER_FRAME, configUI, false, 0, true);
        }
        
        protected function configUI(e:Event):void {
            removeEventListener(Event.ENTER_FRAME, configUI, false);
            _widgetCanvas = new Sprite();
            addChild(_widgetCanvas);
            
            // NFM: Just for general debugging.
            // stage.addEventListener(MouseEvent.MOUSE_DOWN, traceMouseDown, false, 0, true);
            
            // Initialize the static managers.
            TooltipManager.init(stage);
            
            WindowManager.init(stage);
            stage.addEventListener(WidgetEvent.OPEN_WIDGET, handleOpenWidgetEvent, false, 0, true);
            stage.addEventListener(WidgetEvent.CLOSE_WIDGET, handleCloseWidgetEvent, false, 0, true);
            
            DragManager.init(stage);
            stage.addEventListener(DragEvent.DRAG_START, onStartDragAlertModel, false, 0, true);
            stage.addEventListener(DragEvent.DRAG_END, onEndDragAlertModel, false, 0, true);
            
            // Initialize the data bindings
            DataBinding.initialize();
            // Initialize the game UI adapter
            GameDataModel.initialize();
            
            // Setup the custom cursor.
            Mouse.hide();
            myCursor = new MCursor();
            stage.addChild(myCursor); // MOUSE_DEPTH.
            myCursor.mouseEnabled = false;
            myCursor.mouseChildren = false;
            InteractiveObjectEx.setHitTestDisable(myCursor, true);
            stage.addEventListener(MouseEvent.MOUSE_MOVE, handleCursorMouseMove, false, 0, true);            
            
            // Load UI widgets.
            _widgetDefns = GameWidgetDefns.getInstance();
            loadWidget(GameWidgetDefns.WIDGET_CHAT_LOG);
            //loadWidget(GameWidgetDefns.WIDGET_MINIMAP);
            //loadWidget(GameWidgetDefns.WIDGET_QUEST_TRACKER);
            
            loadWidget(GameWidgetDefns.WIDGET_WINDOW_MENU);
            loadWidget(GameWidgetDefns.WIDGET_INVENTORY);
            loadWidget(GameWidgetDefns.WIDGET_ACTION_BARS);
            
            loadWidget(GameWidgetDefns.WIDGET_PLAYER_UF);
            loadWidget(GameWidgetDefns.WIDGET_TARGET_UF);
            
            loadWidget(GameWidgetDefns.WIDGET_CAST_BAR);
        }
        
        public function loadWidget(widgetID:uint):void {
            // Don't let them try to load a widget that is already open / being loaded right now.
            if (isWidgetLoaded(widgetID) || isWidgetLoading(widgetID)) { 
                return; 
            }
            
            if (_widgetDefns.widgetDefs[widgetID].isWindow) { 
                openWindow( widgetID );
            }
            else {
                var widget:Widget = _widgetDefns.widgetDefs[widgetID];
                var widgetPath:String = widget.path;
                if (widgetPath) {
                    var loader:WidgetLoader = new WidgetLoader();
                    var url:URLRequest = new URLRequest(widgetPath); 
                    var context:LoaderContext = new LoaderContext(false, null);
                    loader.widgetID = widgetID;
                    loader.xcoord = widget.xcoord;
                    loader.ycoord = widget.ycoord;
                    loader.contentLoaderInfo.addEventListener(Event.COMPLETE, handleWidgetLoadComplete, false, 0, true);
                    trace("MMOKit :: loading widget... " + widget.path);
                    loader.load(url, context);
                    _widgetsLoading.push(widgetID);
                }
            }
        }
        
        /** 
         * Widgets must be unloaded by their widgetID, otherwise we can't remove them from the Dictionary
         * since you can't find a key using a value.
         */
        public function unloadWidget( widgetID:uint ):void {
            // Cannot unload a widget if it's not currently loaded.
            if ( !isWidgetLoaded(widgetID) ) { 
                return; 
            }
            
            if (_widgetDefns.widgetDefs[widgetID].isWindow) { 
                WindowManager.closeWindowByRef( _loadedWidgets[widgetID] );
            }
            else {
                // Remove the widget from the Stage.
                var widgetRef:DisplayObject = _loadedWidgets[ widgetID ];
                if (widgetRef.parent != null) {
                    widgetRef.parent.removeChild( widgetRef );
                }
            }
            
            // Clean up the _loadedWidgets dictionary.
            _loadedWidgets[ widgetID ] = null;
            delete _loadedWidgets[ widgetID ]
        }
        
        protected function handleWidgetLoadComplete(e:Event):void {
            var loader:WidgetLoader = e.target.loader as WidgetLoader;
            var content:DisplayObject = loader.content;
            
            content.x = loader.xcoord;
            content.y = loader.ycoord;
            
            _loadedWidgets[ loader.widgetID ] = content;
            _widgetCanvas.addChild( content );
            
            // Remove the widgetID from the widgetsLoading array since it is now loaded.
            for (var i:uint = 0; i < _widgetsLoading.length; i++) {
                if ( _widgetsLoading[i] == loader.widgetID ) { 
                    _widgetsLoading.splice(i, 1);
                }
            }
        }
        
        public function openWindow( widgetID:uint ):void {
            if (isWidgetLoaded(widgetID) || isWidgetLoading(widgetID)) { 
                return; 
            }
            
            var windowRef:MWindow = WindowManager.spawnWindow( widgetID, _widgetDefns.widgetDefs[widgetID].path, "SkinnedWindow", _widgetDefns.widgetDefs[widgetID].fname );
            if (windowRef != null) { 
                _loadedWidgets[ widgetID ] = windowRef;
            }
        }
        
        protected function isWidgetLoading( widgetID:uint ):Boolean {
            var isLoading:Boolean = false;
            for (var i:uint = 0; i < _widgetsLoading.length; i++) {
                if ( _widgetsLoading[i] == widgetID ) { 
                    isLoading = true;
                    break;
                }
            }
            return isLoading;
        }
        
        protected function isWidgetLoaded( widgetID:uint ):Boolean {
            var isLoaded:Boolean = false;
            for (var key:Object in _loadedWidgets) {
                if (key == widgetID) {
                    isLoaded = true;
                    break;
                }
            }
            return isLoaded;
        }
        
        protected function traceMouseDown(e:MouseEvent):void {
            trace("e: " + e.target + " :: " + e.target.name );
        }
        
        protected function handleOpenWidgetEvent(e:WidgetEvent):void {
            var widgetID:int = e.widgetID;
            if (widgetID != -1) {
                loadWidget( widgetID );
            }
        }
        
        protected function handleCloseWidgetEvent(e:WidgetEvent):void {
            var widgetID:int = e.widgetID;
            if (widgetID != -1) {
                unloadWidget( widgetID );
            }
        }
        
        protected function onStartDragAlertModel(e:DragEvent):void {
            var dragTarget:MDragTarget = e.dragTarget as MDragTarget;
            GameDataModel.notifySlotDragStart(dragTarget.binding);
        }
        
        protected function onEndDragAlertModel(e:DragEvent):void {
            var dragTarget:MDragTarget = e.dragTarget as MDragTarget;
            var dropTarget:MDragTarget = e.dropTarget as MDragTarget;
            GameDataModel.notifySlotDragEnd(dragTarget.binding, dropTarget ? dropTarget.binding : null); 
        }
        
        protected function handleCursorMouseMove(e:MouseEvent):void {
            myCursor.x = mouseX;
            myCursor.y = mouseY;
        }
    }
}
