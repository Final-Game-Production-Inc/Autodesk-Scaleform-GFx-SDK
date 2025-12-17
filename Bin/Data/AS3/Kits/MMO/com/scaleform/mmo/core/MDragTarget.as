/**************************************************************************

Filename    :   MDragTarget.as
Content    :   
                


Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo.core {

	import com.scaleform.mmo.IGameSlot;
    import flash.display.Sprite;
    import flash.events.Event;
    import flash.text.TextField;
    import flash.utils.getDefinitionByName;
    import flash.display.MovieClip;
    import fl.motion.Color;
    import flash.geom.ColorTransform;
	import scaleform.gfx.InteractiveObjectEx;
    
    import scaleform.clik.controls.DragSlot;
    import scaleform.clik.events.DragEvent;
    import scaleform.clik.managers.DragManager;
    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.data.DataBinding;
    import com.scaleform.mmo.events.TooltipEvent;
	import com.scaleform.mmo.GameDataModel;
    
    public class MDragTarget extends DragSlot implements IGameSlot {
        
    // Constants:
    
    // Public Properties:
        
    // Protected Properties:
        protected var _tooltip:String;
        protected var _binding:String;
        protected var _index:uint;
        
        protected var _dragIconGhost:MovieClip;
        protected var _lastIconPath:String;
        
    // UI Elements:
        public var dropIndicator:MovieClip;
        public var slotUse:MovieClip;
        public var coolDown:MovieClip;
        public var valueAmount:TextField;
        
    // Initialization:
        public function MDragTarget() {
            super();
            addEventListener(Event.REMOVED_FROM_STAGE, onRemovedFromStage, false, 0, true);
            mouseChildren = false;
			if (slotUse) {
				slotUse.mouseEnabled = false;
				slotUse.mouseChildren = false;
			}
			
			InteractiveObjectEx.setHitTestDisable(slotUse, true);
			InteractiveObjectEx.setHitTestDisable(coolDown, true);
        }
        
    // Public Getter / Setters:
        public function get tooltip():String    {
			if (data) {
				return GameDataModel.getSlotTooltip(_binding);
			} else {
				return null;
			}
		}
        
        public function get index():uint        { return _index;    }
        public function set index(value:uint):void { 
            _index = value; 
        }
        
        override public function get enabled():Boolean { return super.enabled; }
        override public function set enabled(value:Boolean):void {
            super.enabled = value;
            setState(enabled ? "up" : "disabled");
        }
        
        public function showAcceptDrop(value:Boolean):void {
            if (dropIndicator) {
                dropIndicator.gotoAndPlay( (value) ? "show" : "hide" );
            }
        }
        
        public function get binding():String { return _binding; }
        public function set binding(value:String):void {
			deleteBinding(null);
            _binding = value;
			createBinding(null);
        }        

		protected function createBinding(e:Event):void {
			if (!_binding) return;
			DataBinding.createBinding(_binding, setItemSlotData, this);
			addEventListener(Event.REMOVED_FROM_STAGE, deleteBinding);			
		}
		protected function deleteBinding(e:Event):void {
			if (!_binding) return;
			DataBinding.deleteBinding(_binding, setItemSlotData, this);
			removeEventListener(Event.REMOVED_FROM_STAGE, deleteBinding);
		}
		
        protected function setItemSlotData(data:Object):void {
            this.data = data;
			clearCooldown();
			if (data != null) {
				dispatchEvent( new TooltipEvent( TooltipEvent.REGISTER_ELEMENT, true, false, this ) );
			} else {
				dispatchEvent( new TooltipEvent( TooltipEvent.UNREGISTER_ELEMENT, true, false, this) );
			}
			invalidate(InvalidationType.DATA);
			validateNow();			
        }
		public function clearCooldown():void {
			if (coolDown) coolDown.gotoAndStop(1);
			if (slotUse) {
				slotUse.visible = false;
			}			
		}		
		
		// IGameSlot implementation
		public function highlight():void { if (dropIndicator) dropIndicator.gotoAndPlay("show"); }
		public function unhighlight():void { if (dropIndicator) dropIndicator.gotoAndPlay("hide"); }
		public function updateCooldown(pct:Number):void {
			if (coolDown) {
				var frame:uint = Math.floor((coolDown.totalFrames - 2) * pct) + 3;
				coolDown.gotoAndStop(frame);
			}
		}
		public function endCooldown():void {
			if (coolDown) coolDown.gotoAndStop(1);
			if (slotUse) {
				slotUse.visible = true;
				slotUse.gotoAndPlay("on");
			}
		}
               
    // Public Methods:
        override public function handleDragStartEvent(e:DragEvent):void {
            // A START_DRAG event that originated from this DragSlot was just fired.
            if (data == null) { return; }
            if (data is Object && !(data is String) && data.iconPath && data.iconPath != "") {
                var def:Class = getDefinitionByName("Icon") as Class;
                var spr:Icon = new def() as Icon;
                
                // Grayscale the copy of the icon.
                var _tintColor:Color = new Color();
                _tintColor.setTint( 0x000000, .5 );
                spr.transform.colorTransform = _tintColor;
                
                _dragIconGhost = spr;
                updateIconWithData( _dragIconGhost as Icon );
                contentCanvas.addChild( _dragIconGhost );
            }
        }
        
        override public function handleDropEvent(e:DragEvent):Boolean {
            // An DROP event just occured with this as the DropTarget.
            return true;
        }
        
        override public function handleDragEndEvent(e:DragEvent, wasValidDrop:Boolean):void {
            // A dragEvent that originated from this DragSlot just ended.
            if ( _dragIconGhost ) {
                contentCanvas.removeChild( _dragIconGhost );
                content = _dragIconGhost;
                _dragIconGhost = null;
            }
        }
        
    // Protected Methods:        
        override protected function draw():void {
            super.draw();
            if (isInvalid(InvalidationType.DATA)) {
                if (data != null && data is Object && !(data is String)) { 
                    if (valueAmount) valueAmount.text = (data.count != null && data.count > 1) ? data.count: "";
                    if (data.iconPath != null && data.iconPath != "") {
                        
                        if (data.iconPath != _lastIconPath) {
                            var def:Class = getDefinitionByName("Icon") as Class;
                            var spr:Icon = new def() as Icon;
                            updateIconWithData(spr);
                            content = spr;                            
                        }
                        
                        if (content) {
                            var tc:Color = new Color();
                            if (data.count == 0) {
                                tc.setTint ( 0x000000, .5 );
                            }
                            content.transform.colorTransform = tc;
                        }
                    }                    
                    _lastIconPath = data.iconPath;
                }
				else
				{
					if (valueAmount) valueAmount.text = "";
					content = null;
                    _lastIconPath = null;
				}
            }
        }
        
        protected function updateIconWithData( icon:Icon ):void {
            // NFM: This stuff is extremely temporary. Need to replace FakeIcon. Move tf_Count into MDragTarget.
            if (icon && data is Object) { 
                icon.loadImage( (data.iconPath != null ) ? data.iconPath : "" );
            }
        }
        
        protected function onRemovedFromStage(e:Event):void {
            // TODO
        }
        
        override public function toString():String { 
            return "[MMOKit MDragTarget " + name + "]";
        }
    }
}