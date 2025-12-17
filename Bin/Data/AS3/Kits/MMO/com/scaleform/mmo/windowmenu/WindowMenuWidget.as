/**************************************************************************

Filename    :   WindowMenuWidget.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo.windowmenu {
    
    import com.scaleform.mmo.events.WidgetEvent;
    import flash.display.Sprite;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.external.ExternalInterface;
    
    import scaleform.clik.controls.Button;
    import scaleform.clik.events.ButtonEvent;
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.managers.DragManager;
    
    import com.scaleform.mmo.GameWidgetDefns;
    import com.scaleform.mmo.MMOKit;
    
    public class WindowMenuWidget extends MovieClip {
    
    // Public Properties:
        
    // Protected Properties:
        protected var _widgetDefns:GameWidgetDefns;
        protected var _activeButtons:uint = 0; // NFM: We can replace this with hardcoded / dynamic once we have assets defined.
        protected var _buttons:Array;
        
    // UI Elements:
        public var windowMenu:MovieClip;
        
        public function WindowMenuWidget() {
            super();
                        
            _buttons = [windowMenu.btn0, windowMenu.btn1, windowMenu.btn2, windowMenu.btn3, windowMenu.btn4, windowMenu.btn5, windowMenu.btn6, windowMenu.btn7, windowMenu.btn8, windowMenu.btn9];
           
            addEventListener(Event.RENDER, configUI, false, 0, true);
        }
        
        protected function configUI(e:Event):void {
            removeEventListener(Event.RENDER, configUI, false);
            
            stage.addEventListener(WidgetEvent.OPEN_WIDGET, handleOpenWidgetEvent, true, 0, true);
            stage.addEventListener(WidgetEvent.CLOSE_WIDGET, handleCloseWidgetEvent, true, 0, true);
            
            tabEnabled = tabChildren = false;
           
            for (var i:uint = 0; i < _buttons.length; i++)
            {
                _buttons[i].menuButtonIcon.gotoAndStop(i+1);
                _buttons[i].enabled = false;
            }
            
            _buttons[1].enabled = true;
            _buttons[1].data = 4; // Paperdoll
            _buttons[1].addEventListener( Event.SELECT, handleMenuButtonClick, false, 0, true );
            
            _buttons[2].enabled = true;
            _buttons[2].data = 8; // Spell book
            _buttons[2].addEventListener( Event.SELECT, handleMenuButtonClick, false, 0, true );
            
            /*
            _widgetDefns = GameWidgetDefns.getInstance();
            var widgetsLength:uint = _widgetDefns.widgetDefs.length;
            for (var i:uint = 0; i < widgetsLength; i++) {
                if (!_widgetDefns.widgetDefs[i].isWindow) { continue; }
                
                var btn:Button = windowMenu.getChildByName( "btn" + _activeButtons++ ) as Button;
                if (btn == null) { break; }
                
                btn.data = i;
                btn.toggle = true;
                btn.label = _widgetDefns.widgetDefs[i].fname;
                btn.addEventListener( Event.SELECT, handleMenuButtonClick, false, 0, true );
                _buttons.push(btn);
            }
            */            
        }
        
        // maybe they should be toggle buttons? or maybe this thing should just keep track of them.
        protected function handleMenuButtonClick(e:Event):void {
            if (e.target.selected) {
                dispatchEvent( new WidgetEvent(WidgetEvent.OPEN_WIDGET, true, false, e.target.data) );
            } else {
                dispatchEvent( new WidgetEvent(WidgetEvent.CLOSE_WIDGET, true, false, e.target.data) );
            }
        }
        
        protected function handleOpenWidgetEvent(e:WidgetEvent):void {
            for (var i:uint = 0; i < _buttons.length; i++) {
                if (_buttons[i].data == e.widgetID) {
                    stage.removeEventListener(WidgetEvent.OPEN_WIDGET, handleOpenWidgetEvent, true);
                    _buttons[i].selected = true;
                    stage.addEventListener(WidgetEvent.OPEN_WIDGET, handleOpenWidgetEvent, true, 0, true);
                }
            }
        }
        
        protected function handleCloseWidgetEvent(e:WidgetEvent):void {
            for (var i:uint = 0; i < _buttons.length; i++) {
                if (_buttons[i].data == e.widgetID) {
                    stage.removeEventListener(WidgetEvent.CLOSE_WIDGET, handleCloseWidgetEvent, true);
                    _buttons[i].selected = false;
                    stage.addEventListener(WidgetEvent.CLOSE_WIDGET, handleCloseWidgetEvent, true, 0, true);
                }
            }
        }
    }
}
