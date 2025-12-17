/**************************************************************************

Filename    :   DragTargetList.as
Content    :   
                


Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo.core {
    
    import flash.display.DisplayObject;
    import flash.display.MovieClip;
    import flash.display.Sprite;
    import flash.display.Stage;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.geom.Rectangle;
    import flash.utils.getDefinitionByName;
    
    import scaleform.clik.events.InputEvent;
    import scaleform.clik.constants.InputValue;
    import scaleform.clik.ui.InputDetails;
    import scaleform.clik.constants.NavigationCode;
    
    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.data.ListData;
    import scaleform.clik.events.InputEvent;
    import scaleform.clik.events.ListEvent;
    import scaleform.clik.events.ButtonEvent;
    import scaleform.clik.interfaces.IDataProvider;
    import scaleform.clik.interfaces.IListItemRenderer;
       
    public class DragTargetList extends UIComponent {
        
        public var totalRowsOverride:int = -1;
        public var totalColumnsOverride:int = -1;
        
        protected var _itemRendererClassName = "";
        protected var _itemRenderer:Class;
        protected var _dataProvider:IDataProvider;
        
        protected var _renderers:Array;
        protected var _totalRenderers:uint = 0; // Usable renderers.
        
//        protected var _slots:uint = 0;
        protected var _totalRows:uint = 0;
        protected var _totalColumns:uint = 0;
        
        protected var _rendererHeight:uint = 0;
        protected var _rendererWidth:uint = 0;
        
        protected var state:String = "default";
        protected var _stageRef:Stage = null;
        
    // UI Elements:
        protected var container:Sprite;
        public var bg:MovieClip;
        
    // Initialization:
        public function DragTargetList() {
            super();
        }
        
        override protected function initialize():void {
            dataProvider = new DataProvider(); // Default Data.
            super.initialize();
        }
        
    // Public Getter / Setters:
        public function get stageRef():Stage { return _stageRef; }
        public function set stageRef(value:Stage):void {
            _stageRef = value;
        }
        
//        public function get slots():uint { return _slots; }
//        public function set slots(value:uint):void {
//            _slots = value;
//            invalidate(InvalidationType.DATA, InvalidationType.RENDERERS);
//        }
        
        public function get totalColumns():uint { return _totalColumns; }
        public function set totalColumns(value:uint):void {
            if (_totalColumns == value) { return; }
            _totalColumns = value;
            invalidateSize();
        }
        
        public function get totalRows():uint { return _totalRows; }
        public function set totalRows(value:uint):void {
            if (_totalRows == value) { return; }
            _totalRows = value;
            invalidateSize();
        }
        
        public function get dataProvider():IDataProvider { return _dataProvider; }
        public function set dataProvider(value:IDataProvider):void {
            // NFM: If they set the dataProvider, even if the pointer is the same invalidateData() and then return.
            if (_dataProvider == value) { 
                invalidateData();
                return; 
            }
            
            if (_dataProvider != null) {
                _dataProvider.removeEventListener(Event.CHANGE, handleDataChange, false);
            }
            
            _dataProvider = value;
            if (_dataProvider == null) { return; }
            _dataProvider.addEventListener(Event.CHANGE, handleDataChange, false, 0, true);
            
            invalidateData();
        }
        
        public function set itemRendererName(value:String):void {
            if ( value == "") { return; }
            _itemRendererClassName = value;
            
            // Get a reference to the class for the itemRenderer.
            if (_itemRendererClassName == null) { return; }
            _itemRenderer = getDefinitionByName( _itemRendererClassName ) as Class;
            
            if (_itemRenderer) { 
                var renderer:DisplayObject = new _itemRenderer() as DisplayObject;
                _rendererHeight = renderer.height;
                _rendererWidth = renderer.width;
            }
        }
    
    // Public Methods:
        override protected function configUI():void {
            super.configUI();
            
            // Create the container that we'll be attaching our itemRenderers to.
            if (container == null) { 
                container = new Sprite();
                
                if (bg) { removeChild(bg); }
                addChild(container);
                if (bg) { addChild(bg); }
            }
            
            tabEnabled = tabChildren = false;
        }
        
        public function invalidateRenderers():void {
            invalidate(InvalidationType.RENDERERS);
        }
    
        // Protected Methods:
        override protected function draw():void {
            var i:uint, l:uint, renderer:DisplayObject, displayObject:DisplayObject;
            
            // NFM: We won't be changing the dataProvider in such a way that renderers need to be recreated.
            // Remove old internal renderers
            if (isInvalid(InvalidationType.RENDERERS)) {
                if (_renderers != null) {
                    l = _renderers.length;
                    for (i = 0; i < l; i++) {
                        renderer = getRendererAt(i);
                        // cleanUpRenderer(renderer);
                        displayObject = renderer as DisplayObject;
                        if (container.contains(displayObject)) { container.removeChild(displayObject); }
                    }
                }
                _renderers = [];
                invalidateData();
            }
            
            // Counter-scale to ensure base component is the right size.
            if (isInvalid(InvalidationType.SIZE)) {
                removeChild(container);
                
                setActualSize(_width, _height);
                container.scaleX = 1 / scaleX;
                container.scaleY = 1 / scaleY;
                
//                if (_slots == 0) { trace("\n>>> " + this + " Warning: _slots == 0. No renderers will be drawn. Intended?"); }
                if (_dataProvider.length == 0) { trace("\n>>> " + this + " Warning: _slots == 0. No renderers will be drawn. Intended?"); }
//                _totalRenderers = _slots;
                _totalRenderers = _dataProvider.length;
                if (totalColumnsOverride == -1 || totalRowsOverride == -1) {
                    _totalColumns = 4;
                    _totalRows = _dataProvider.length / 4;
                } else {
                    _totalColumns = totalColumnsOverride;
                    _totalRows = totalRowsOverride;
                }
                
                addChild(container);
                invalidateData();
            }
            
            // Create/Destroy renderers
            if (isInvalid(InvalidationType.RENDERERS, InvalidationType.SIZE)) {
                drawRenderers(_totalRenderers);
                drawLayout();
                
                if (bg) {
                    bg.width = container.width;
                    bg.height = container.height;
                }
            }
            
            if (isInvalid(InvalidationType.DATA)) {
                _dataProvider.requestItemRange(0, _dataProvider.length - 1, populateData);
            }
        }
        
        protected function drawLayout():void {
            var dataWillChange:Boolean = isInvalid(InvalidationType.DATA);
            var l:uint = _renderers.length;
            for (var i:uint = 0; i < l; i++) {
                var renderer:UIComponent = getRendererAt(i) as UIComponent;
                
                // Data will always be displayed horizontally.
                renderer.x = (i % _totalColumns) * _rendererWidth; // + 1; // +1 here to help avoid overlapping.
                renderer.y = (i / _totalColumns >> 0) * _rendererHeight;// + 1; // +1 here to help avoid overlapping.
                
                if (!dataWillChange) { renderer.validateNow(); }
            }
        }
            
        protected function drawRenderers(total:Number):void {
            // NFM: Added this since inspectables are not always applied to the component. No idea why this occurs.
            if (_itemRenderer == null) {
                if (_itemRendererClassName != undefined) {
                    itemRendererName = _itemRendererClassName;
                }
                if (_itemRenderer == null) {
                    trace( this + " :: drawRenderers Error, Renderer class not defined."); 
                    return;
                }
            }
            
            var i:int, l:int, renderer:DisplayObject, displayObject:DisplayObject;
            for (i = _renderers.length; i < _totalRenderers; i++) {
                renderer = createRenderer(i);
                if (renderer == null) { break; }
                _renderers.push(renderer);
                container.addChild(renderer as DisplayObject);
            }
            
            l = _renderers.length;
            for (i=l-1; i>=_totalRenderers; i--) {
                renderer = getRendererAt(i);
                if (renderer != null) {
                    // cleanUpRenderer(renderer);
                    displayObject = renderer as DisplayObject;
                    if (container.contains(displayObject)) { container.removeChild(displayObject); }
                }
                _renderers.splice(i, 1);
            }
        }
        
        protected function createRenderer(index:uint):DisplayObject {
            var renderer:DisplayObject = new _itemRenderer() as DisplayObject;
            if (renderer == null) {
                trace("Renderer class could not be created."); return null;
            }
            setupRenderer(renderer, index);
            return renderer;
        }
        
        protected function setupRenderer(mc:DisplayObject, index:int = -1):void { }
        
        protected function getRendererAt(index:uint, offset:int=0):DisplayObject {
            if (_renderers == null) { return null; }
            var newIndex:uint = index - offset;
            if (newIndex >= _renderers.length) { return null; }
            return _renderers[newIndex] as DisplayObject;
        }
        
        protected function handleDataChange(event:Event):void {
            invalidate(InvalidationType.DATA);
        }
        
        protected function populateData(data:Array):void {
            var dl:uint = data.length;
            var l:uint = _renderers.length;
            for (var i:uint = 0; i < l && data[i] != null; i++) {
                var renderer:IListItemRenderer = getRendererAt(i) as IListItemRenderer;
                var index:uint = i;
                renderer.setData(data[i]);
                renderer.validateNow();
            }
        }
        
        override public function toString():String { 
            return "[MMOKit DragTargetList " + name + "]";
        }
    }
}