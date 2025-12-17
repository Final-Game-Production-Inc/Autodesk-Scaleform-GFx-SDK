/**************************************************************************

Filename    :   BasicList.as
Content    :   A reduced version of the ScrollingList class. Creats 
                a renderer for every item in the dataProvider with no
                optimization.
                
Authors     :  Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
 
package com.scaleform.mmo.core {
    
    import flash.display.DisplayObject;
    import flash.display.Sprite;
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
        
    public class BasicList extends UIComponent {
        
    // Constants:
        
    // Public Properties:
        
    // Protected Properties:
        protected var _itemRendererClassName = "CLIKListItemRenderer";
        protected var _itemRenderer:Class;
        protected var _dataProvider:IDataProvider;
        
        protected var renderers:Vector.<IListItemRenderer>;
        protected var totalRenderers:uint = 0; // Usable renderers.
        protected var rendererHeight:uint = 0;
        protected var _newFrame:String;
        
        protected var _selectedIndex:int = -1;
        protected var _newSelectedIndex:int = -1;
        
        protected var state:String = "default";
        
    // UI Elements:
        protected var container:Sprite;
        
    // Initialization:
        public function BasicList() {
            super();
        }
        
        override protected function initialize():void {
            dataProvider = new DataProvider(); // Default Data.
            super.initialize();
        }
        
    // Public getter / setters:
        public function get dataProvider():IDataProvider { return _dataProvider; }
        public function set dataProvider(value:IDataProvider):void {
            // NFM: If they set the dataProvider, even if the pointer is the same invalidateData() and then return.
            if (_dataProvider == value) { 
                invalidateData();
                return; 
            }
            
            if (_dataProvider != null) {
                _dataProvider.removeEventListener(Event.CHANGE, handleDataChange);
            }
            
            _dataProvider = value;
            if (_dataProvider == null) { return; }
            _dataProvider.addEventListener(Event.CHANGE, handleDataChange);
            totalRenderers = dataProvider.length;
            invalidate(InvalidationType.DATA, InvalidationType.RENDERERS, InvalidationType.SIZE);
        }
        
        override public function get enabled():Boolean { return super.enabled; }
        override public function set enabled(value:Boolean):void {
            super.enabled = value;
            if (!renderers) return;
            for (var i:uint = 0; i < renderers.length; i++) {
                renderers[i].enabled = super.enabled;
            }
        }
        
        [Inspectable(name="itemRenderer", defaultValue="CLIKListItemRenderer")]
        public function set itemRendererName(value:String):void {
            // NFM: I'm not sure why that's here.
            if ( /*(_inspector && value == "CLIKListItemRenderer") ||*/ value == "") { return; }
            _itemRendererClassName = value;
            
            // Get a reference to the class for the itemRenderer.
            _itemRenderer = getDefinitionByName(_itemRendererClassName) as Class;
            
            var renderer:IListItemRenderer = createRenderer(0);
            rendererHeight = renderer.height;
            cleanUpRenderer(renderer);
        }
        
        public function get selectedIndex():int { return _selectedIndex; }
        public function set selectedIndex(value:int):void {
            if (value == _selectedIndex || value == _newSelectedIndex) { return; }
            _newSelectedIndex = value;
            invalidateSelectedIndex();
        }
        
        public function setSelectedIndexImpl(value:int):void {
            if (_selectedIndex == value) { return; }
            _selectedIndex = value;
            invalidateSelectedIndex();
            dispatchEventAndSound(new ListEvent(ListEvent.INDEX_CHANGE, true, false, _selectedIndex, -1, -1, getRendererAt(_selectedIndex), false));
        }
    
    // Public Methods:
        override protected function configUI():void {
            super.configUI();
            
            // Create the container that we'll be attaching our itemRenderers to.
            if (container == null) { 
                container = new Sprite();
                addChild(container);
            }
            
            tabEnabled = true; 
            tabChildren = false;
            
            addEventListener(InputEvent.INPUT, handleInput, false, 0, true);
        }
        
        public function invalidateRenderers():void {
            invalidate(InvalidationType.RENDERERS);
        }
        
        public function invalidateSelectedIndex():void {
            invalidate(InvalidationType.SELECTED_INDEX);
        }
    
        // Protected Methods:
        override protected function draw():void {
            if (isInvalid(InvalidationType.SELECTED_INDEX)) {
                updateSelectedIndex();
            }
            
            if (isInvalid(InvalidationType.STATE)) {
                if (_newFrame) {
                    gotoAndPlay(_newFrame);
                    _newFrame = null;
                }
            }
            
            var i:uint, l:uint, renderer:IListItemRenderer, displayObject:DisplayObject;
            
            // Remove old internal renderers
            if (isInvalid(InvalidationType.RENDERERS)) {
                if (renderers != null) {
                    l = renderers.length;
                    for (i=0; i<l; i++) {
                        renderer = getRendererAt(i);
                        cleanUpRenderer(renderer);
                        displayObject = renderer as DisplayObject;
                        if (container.contains(displayObject)) { container.removeChild(displayObject); }
                    }
                }
                renderers = new Vector.<IListItemRenderer>();
                invalidateData();
            }
            
            // Counter-scale to ensure base component is the right size.
            if (isInvalid(InvalidationType.SIZE)) {
                removeChild(container);
                setActualSize(_width, _height);
                container.scaleX = 1 / scaleX;
                container.scaleY = 1 / scaleY;
                totalRenderers = dataProvider.length;
                addChild(container);
                invalidateData();
            }
            
            // Create/Destroy renderers
            if (isInvalid(InvalidationType.RENDERERS, InvalidationType.SIZE)) {
                drawRenderers(totalRenderers);
                drawLayout();
            }
            
            if (isInvalid(InvalidationType.DATA)) {
                refreshData();
            }
        }
        
        public function getHeight():uint {
            return renderers.length * rendererHeight;
        }
        
        override public function handleInput(event:InputEvent):void {
            if (event.handled) { return; } // Already Handled.
            
            // Pass on to selected renderer first
            var renderer:IListItemRenderer = getRendererAt(_selectedIndex);
            if (renderer != null) {
                renderer.handleInput(event); // Since we are just passing on the event, it won't bubble, and should properly stopPropagation.
                if (event.handled) { return; }
            }
            
            // Only allow actions on key down, but still set handled=true when it would otherwise be handled.
            var details:InputDetails = event.details;
            var keyPress:Boolean = (details.value == InputValue.KEY_DOWN || details.value == InputValue.KEY_HOLD);
            switch(details.navEquivalent) {
                case NavigationCode.UP:
                    if (selectedIndex == -1) {
                        if (keyPress) { selectedIndex = totalRenderers - 1; }
                    } else if (_selectedIndex > 0) {
                        if (keyPress) { selectedIndex--; }
                    } else {
                        return;
                    }
                    break;
                    
                case NavigationCode.DOWN:
                    if (_selectedIndex == -1) {
                        if (keyPress) selectedIndex = 0;
                    } else if (_selectedIndex < _dataProvider.length-1) {
                        if (keyPress) { selectedIndex++; }
                    } else {
                        return;
                    }
                    break;        
    
                case NavigationCode.END:
                    if (!keyPress) {
                        selectedIndex = _dataProvider.length-1;
                    }
                    break;
                
                case NavigationCode.HOME:
                    if (!keyPress) { selectedIndex = 0; }
                    break;
                
                case NavigationCode.PAGE_UP:
                    if (keyPress) { selectedIndex = Math.max(0, _selectedIndex - totalRenderers); }
                    break;
                
                case NavigationCode.PAGE_DOWN:
                    if (keyPress) { selectedIndex = Math.min(_dataProvider.length-1, _selectedIndex + totalRenderers); }
                    break;
                
                default:
                    return;
            }
            
            event.handled = true;
            
        }
        
        protected function drawLayout():void {
            var dataWillChange:Boolean = isInvalid(InvalidationType.DATA);
            var l:uint = renderers.length;
            for (var i:uint = 0; i < l; i++) {
                var renderer:IListItemRenderer = getRendererAt(i);
                renderer.x = 0;
                renderer.y = i * rendererHeight + 1; // +1 here to help avoid overlapping.
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
                    trace("Renderer class not defined."); return;
                }
            }
            
            var i:int, l:int, renderer:IListItemRenderer, displayObject:DisplayObject;
            for (i=renderers.length; i<totalRenderers; i++) {
                renderer = createRenderer(i);
                if (renderer == null) { break; }
                renderers.push(renderer);
                container.addChild(renderer as DisplayObject);
            }
            l = renderers.length;
            for (i=l-1; i>=totalRenderers; i--) {
                renderer = getRendererAt(i);
                if (renderer != null) {
                    cleanUpRenderer(renderer);
                    displayObject = renderer as DisplayObject;
                    if (container.contains(displayObject)) { container.removeChild(displayObject); }
                }
                renderers.splice(i, 1);
            }
        }
        
        protected function updateSelectedIndex():void {
            if (_selectedIndex == _newSelectedIndex) { return; }//trace(_selectedIndex,_newSelectedIndex);
            if (totalRenderers == 0) { return; }    // Return if there are no renderers
        
            var renderer:IListItemRenderer = getRendererAt(_selectedIndex);
            if (renderer != null) {
                renderer.selected = false; // Only reset items in range
                renderer.displayFocus = false;
                renderer.validateNow();
            }
            
            setSelectedIndexImpl(_newSelectedIndex);    // Reset the new selected index value if we found a renderer instance
            
            if (_selectedIndex < 0 || _selectedIndex >= _dataProvider.length) { return; }
            renderer = getRendererAt(_selectedIndex);
            if (renderer != null) {
                renderer.selected = true; // Item is in range. Just set it.
                renderer.displayFocus = true;    
            } else {
                // scrollToIndex(_selectedIndex); // Will redraw
                renderer = getRendererAt(_selectedIndex);
                renderer.displayFocus = true;
            }
        }
        
        protected function calculateRendererTotal(width:Number, height:Number):uint {        
            var renderer:IListItemRenderer = createRenderer(0);
            rendererHeight = renderer.height;
            cleanUpRenderer(renderer);
            return (_height / rendererHeight) >> 0;
        }
        
        protected function createRenderer(index:uint):IListItemRenderer {
            var renderer:IListItemRenderer = new _itemRenderer() as IListItemRenderer;
            if (renderer == null) {
                trace("Renderer class could not be created."); return null;
            }
            setupRenderer(renderer);
            return renderer;
        }
        
        protected function getRendererAt(index:uint, offset:int=0):IListItemRenderer {
            if (renderers == null) { return null; }
            var newIndex:uint = index - offset;
            if (newIndex >= renderers.length) { return null; }
            return renderers[newIndex] as IListItemRenderer;
        }
        
        protected function setupRenderer(renderer:IListItemRenderer):void {
            renderer.owner = this;
            renderer.focusTarget = this;
            renderer.addEventListener(ButtonEvent.PRESS, dispatchItemEvent, false, 0, true);
            renderer.addEventListener(ButtonEvent.CLICK, handleItemClick, false, 0, true);
            renderer.addEventListener(MouseEvent.DOUBLE_CLICK, dispatchItemEvent, false, 0, true);
            renderer.addEventListener(MouseEvent.ROLL_OVER, dispatchItemEvent, false, 0, true);
            renderer.addEventListener(MouseEvent.ROLL_OUT, dispatchItemEvent, false, 0, true);
        }
        
        protected function cleanUpRenderer(renderer:IListItemRenderer):void {
            renderer.owner = null;
            renderer.focusTarget = null;
            renderer.tabEnabled = true;
            renderer.removeEventListener(ButtonEvent.PRESS, dispatchItemEvent);
            renderer.removeEventListener(ButtonEvent.CLICK, handleItemClick);
            renderer.removeEventListener(MouseEvent.DOUBLE_CLICK, dispatchItemEvent);
            renderer.removeEventListener(MouseEvent.ROLL_OVER, dispatchItemEvent);
            renderer.removeEventListener(MouseEvent.ROLL_OUT, dispatchItemEvent);
        }
        
        protected function dispatchItemEvent(event:Event):Boolean {
            var type:String;
            switch (event.type) {
                case ButtonEvent.PRESS:
                    type = ListEvent.ITEM_PRESS; break;
                case ButtonEvent.CLICK:
                    type = ListEvent.ITEM_CLICK; break;
                case MouseEvent.ROLL_OVER:
                    type = ListEvent.ITEM_ROLL_OVER; break;
                case MouseEvent.ROLL_OUT:
                    type = ListEvent.ITEM_ROLL_OUT; break;
                case MouseEvent.DOUBLE_CLICK:
                    type = ListEvent.ITEM_DOUBLE_CLICK; break;
                default:
                    return true;
            }
            
            var renderer:IListItemRenderer = event.currentTarget as IListItemRenderer;
            var isKeyboard:Boolean = false;
            if (event is ButtonEvent) { isKeyboard = (event as ButtonEvent).isKeyboard; }
            var newEvent:ListEvent = new ListEvent(type, false, true, renderer.index, 0, renderer.index, renderer, isKeyboard);
                
            return dispatchEventAndSound(newEvent);
        }
        
        protected function handleDataChange(event:Event):void {
            invalidate(InvalidationType.DATA);
        }
        
        protected function handleItemClick(event:ButtonEvent):void {
            var index:Number = (event.currentTarget as IListItemRenderer).index;
            if (isNaN(index)) { return; } // If the data has not been populated, but the listItemRenderer is clicked, it will have no index.
            if (dispatchItemEvent(event)) {
                selectedIndex = index;
            }
        }
        
        override protected function changeFocus():void {
            if (_focused || _displayFocus) {
                setState("focused", "default");
            } else {
                setState("default");
            }
            
            var renderer:IListItemRenderer = getRendererAt(_selectedIndex);
            if (renderer != null) {
                renderer.displayFocus = (focused > 0);
                renderer.validateNow();
            }
        }
        
        protected function refreshData():void {
            selectedIndex = Math.min(_dataProvider.length - 1, _selectedIndex);
            updateSelectedIndex();
            
            _dataProvider.requestItemRange(0, _dataProvider.length - 1, populateData);
        }
    
        protected function populateData(data:Array):void {
            var dl:uint = data.length;
            var l:uint = renderers.length;
            for (var i:uint = 0; i < l; i++) {
                var renderer:IListItemRenderer = getRendererAt(i);
                var index:uint = i;
                var listData:ListData = new ListData(index, "item"+i, _selectedIndex == index);
                renderer.setListData(listData);
                renderer.setData(data[i]);
                renderer.enabled = (i >= dl) ? false : true;
                renderer.validateNow();
            }
        }
        
        protected function setState(...states:Array):void {
            if (states.length == 1) {
                var onlyState:String = states[0].toString();
                if (state != onlyState && _labelHash[onlyState]) { 
                    state = _newFrame = onlyState;
                    invalidateState();
                }
                return;
            }
            var l:uint = states.length;
            for (var i:uint=0; i<l; i++) {
                var thisState:String = states[i].toString();
                if (_labelHash[thisState]) {
                    state = _newFrame = thisState;
                    invalidateState();
                    break;
                }
            }
        }
    }
}
