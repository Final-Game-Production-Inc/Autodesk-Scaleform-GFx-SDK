/**************************************************************************

Filename    :   TooltipManager.as
Content    :   Tooltip Manager for the MMO Kit.
                
                Displays MOUSE_OVER tooltips for UIComponents registered 
                with the TooltipManager instance with valid tooltip data.
                

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo.utils {
    
	import flash.display.DisplayObjectContainer;
    import flash.display.Loader;
    import flash.display.MovieClip;
    import flash.display.Sprite;
    import flash.display.Stage;
    import flash.net.URLRequest;
    import flash.system.LoaderContext;
    import flash.external.ExternalInterface;
    import flash.text.TextField;
    import flash.text.TextFieldAutoSize;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.utils.setInterval;
    import flash.utils.clearInterval;
    import flash.utils.Timer;
    import flash.geom.Point;
    
    import fl.transitions.easing.*;
    
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.motion.Tween;
    
    import com.scaleform.mmo.events.TooltipEvent;
    
    public class TooltipManager {
    
    // Public Properties:
        protected static var _allowInstantiation:Boolean;
        protected static var _instance:TooltipManager;
        
    // Protected Properties:
        protected var _tooltipRef:MovieClip;
        protected var _hoverMode:Boolean;
        protected var _delayTimerID:Number;
        protected var _resetTimerID:Number;
        protected var _stage:Stage;
        
    // UI Elements:
        protected var _canvas:Sprite;
        
        public function TooltipManager(stage:Stage):void {
            if (!_allowInstantiation) {
                throw new Error("Error: Instantiation failed: Use TooltipManager.getInstance() instead of new.");
            }
            _hoverMode = true;
            _canvas = new Sprite();
            _stage = stage;
            if (_stage) {
                _stage.addEventListener(TooltipEvent.REGISTER_ELEMENT, handleTooltipRegisterEvent, false, 0, true);
                _stage.addEventListener(TooltipEvent.UNREGISTER_ELEMENT, handleTooltipUnregisterEvent, false, 0, true);				
                _stage.addChild(_canvas);
            }
        }
        
        public static function init(stage:Stage):TooltipManager {
            if (_instance == null) {
                _allowInstantiation = true;
                _instance = new TooltipManager(stage);
                _allowInstantiation = false;
            }
            return _instance;
        }
        
        public static function isInitialized():Boolean {
            return (_instance != null);
        }
        
        public static function register(elem:UIComponent):void {
            _instance.registerElement(elem);
            // maybe store the tooltip on the movieclip here.
        }
        public static function unregister(elem:UIComponent):void {
            _instance.unregisterElement(elem);
        }
        
        protected function registerElement(elem:UIComponent):void {
            elem.addEventListener(MouseEvent.ROLL_OVER, showTooltip, false, 0, true);
            elem.addEventListener(MouseEvent.ROLL_OUT, hideTooltip, false, 0, true);
            elem.addEventListener(MouseEvent.MOUSE_DOWN, hideTooltip, false, 0, true);
        }
		protected function unregisterElement(elem:UIComponent):void {
            elem.removeEventListener(MouseEvent.ROLL_OVER, showTooltip, false);
            elem.removeEventListener(MouseEvent.ROLL_OUT, hideTooltip, false);
            elem.removeEventListener(MouseEvent.MOUSE_DOWN, hideTooltip, false);
        }
        
        protected function handleTooltipRegisterEvent(e:TooltipEvent):void {
            var element:UIComponent = e.elem as UIComponent;
            if (element != null) {
                TooltipManager.register( element );
            }
        }
		
		protected function handleTooltipUnregisterEvent(e:TooltipEvent):void {
			var element:UIComponent = e.elem as UIComponent;
            if (element != null) {
                TooltipManager.unregister( element );
            }
		}		
		
        protected function showTooltip(e:MouseEvent):void {
            var elem:MovieClip = e.target as MovieClip;
            if (!elem.tooltip) { return; }
            var tooltip:String = elem.tooltip;
            if (_tooltipRef) { _canvas.removeChild(_tooltipRef); }
            
            // Construct the tooltip.
            _tooltipRef = new Tooltip(); 
            var tf:TextField = _tooltipRef["textField"];
            tf.htmlText = elem.tooltip;
            
            // 220, 128.
            var window:MovieClip = _tooltipRef["window"];
			var maxTooltipWidth:Number = 200;
			var newTextfieldWidth = Math.max(maxTooltipWidth, tf.textWidth + 10);
			if (newTextfieldWidth > maxTooltipWidth) {
				tf.wordWrap = true;
				newTextfieldWidth = maxTooltipWidth;
			}
			tf.width = newTextfieldWidth;
            tf.height = tf.textHeight + 10;
			window.width = newTextfieldWidth + 37;
            window.height = tf.textHeight + 37;
            
			var ttWidth:Number = _tooltipRef.width;
			var ttHeight:Number = _tooltipRef.height;
            var targetTopLeft:Point = new Point(elem.x, elem.y);
			var targetBottomRight:Point = new Point(elem.x + elem.width, elem.y + elem.height);
			var elemParent:DisplayObjectContainer = elem.parent;
            var destTL:Point = elemParent.localToGlobal(targetTopLeft);
			var destBR:Point = elemParent.localToGlobal(targetBottomRight);
            
			setupTooltipLocation(ttWidth, ttHeight, destTL, destBR);
            _tooltipRef.mouseEnabled = _tooltipRef.mouseChildren = false;
            
            _canvas.addChild(_tooltipRef);
            
            if (_hoverMode) {
                showTooltipImpl();
            } else {
                _delayTimerID = setInterval(showTooltipImpl, 1000);
            }
        }
        
        protected function showTooltipImpl():void {
            // Stop the reset timer
            if (_resetTimerID) {
                clearInterval(_resetTimerID);
                _resetTimerID = undefined;
            }
            
            // Show the tooltip
            //clampToView( _tooltipRef.mouseX, _tooltipRef.mouseY + 20);
            _tooltipRef.visible = true;
            _tooltipRef.alpha = 0;
            
            var tween:Tween = new Tween(300, _tooltipRef, { alpha:1 }, { ease:Strong.easeOut } );
            clearInterval(_delayTimerID);
            _delayTimerID = undefined;
            
            // Enable hover mode
            _hoverMode = true;
        }
        
        protected function hideTooltip(e:Object):void {
            if (_tooltipRef) { 
                _canvas.removeChild(_tooltipRef);
                _tooltipRef = null;
                
                clearInterval(_delayTimerID);
                _delayTimerID = undefined;
                
                if (_resetTimerID) {
                    clearInterval(_resetTimerID);
                    _resetTimerID = undefined;
                }
                
                _resetTimerID = setInterval(resetHoverMode, 250);
            }
        }
        
        protected function resetHoverMode():void {
            if (_resetTimerID) {
                clearInterval(_resetTimerID);
                _resetTimerID = undefined;
            }
            
            // Disable hover mode
            _hoverMode = false;
        }
        
		protected function setupTooltipLocation(w:Number, h:Number, targetTopLeft:Point, targetBottomRight:Point):void {
			var padding = 0;
			var tx:Number = 0, ty:Number = 0;
			tx = targetTopLeft.x - w - padding;
			ty = targetTopLeft.y - h - padding;
			if (tx < 0) tx = targetBottomRight.x + padding;
			if (ty < 0) ty = targetBottomRight.y + padding;
			_tooltipRef.x = tx;
			_tooltipRef.y = ty;
		}
    }
}
