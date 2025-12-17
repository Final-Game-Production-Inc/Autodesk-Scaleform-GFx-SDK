/**************************************************************************

Filename    :   ImageScroller.as
Content     :  Implementation of a list of images the scroll on the Z.
                Used in the Start Game view of the Menu Kit.
                
Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.controls {
    
    import flash.display.Loader;
    import flash.display.MovieClip;
    import flash.display.Sprite;
    import flash.display.DisplayObject;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.external.ExternalInterface;
    import flash.net.URLRequest;
    import flash.system.LoaderContext;
    import flash.utils.getDefinitionByName;
    
    import fl.transitions.easing.*;
    
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.controls.ScrollingList;
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.events.ListEvent;
    import scaleform.clik.events.ButtonEvent;
    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.interfaces.IDataProvider;
    import scaleform.clik.interfaces.IListItemRenderer;
    import scaleform.clik.motion.Tween;
    
    import com.scaleform.views.View;
    
    public class ImageScroller extends UIComponent {
        
    // Constants:
        
    // Public Properties:
        public var loaderWidth:Number = 462;
        public var loaderHeight:Number = 254;
        
        public var xmargin:Number = 350;
        public var ymargin:Number = 0;
        public var zmargin:Number = 550; // 10000;
        public var alphaMargin:Number = 25;
        
    // Private Properties:
        protected var _activeTween:Tween;
        protected var _rowCount:Number;
        
        protected var _dataProvider:IDataProvider;
        protected var _itemRenderer:String = "DifficultyItemRenderer";
        protected var _itemRendererClass:Class;
        protected var _selectedIndex:Number = -1;
        
        protected var _drawnRendererCount:Number = 5;
        protected var renderers:Array;
        protected var totalRenderers:Number;
        
    // UI Elements:
        protected var container:Sprite;
        
    // Initialization:
        /**
         * The constructor is called when a ScrollingList or a sub-class of ScrollingList is instantiated on stage or by using {@code attachMovie()} in ActionScript. This component can <b>not</b> be instantiated using {@code new} syntax. When creating new components that extend ScrollingList, ensure that a {@code super()} call is made first in the constructor.
         */
        public function ImageScroller() {
            enableInitCallback = true;
            super();
        }
        
        override protected function initialize():void {
            dataProvider = new DataProvider(); // Default Data.
            renderers = [];
            super.initialize();
        }
        
    // Public Getters / Setters
        /**
         * The data model displayed in the component. The dataProvider must implement the 
         * {@code IDataProvider} interface. When a new DataProvider is set, the {@code selectedIndex}
         * property will be reset to 0.
         */
        public function get dataProvider():IDataProvider { return _dataProvider; }
        public function set dataProvider(value:IDataProvider):void {
            if (_dataProvider != null) {
                _dataProvider.removeEventListener(Event.CHANGE, handleDataChange, false);
            }
            
            _dataProvider = value;
            if (_dataProvider == null) { return; }
            _dataProvider.addEventListener(Event.CHANGE, handleDataChange, false, 0, true);
            
            invalidate();
        }
        
        // When the selectedIndex changes we will shift it to the middle and do some fancy
        // code tween. Do not check if the new selectedIndex == newSelectedIndex so that we
        // ensure the lists animates properly moving between screens and when focus comes in and 
        // out of the list.
        public function get selectedIndex():Number { return _selectedIndex; }
        public function set selectedIndex(value:Number):void {
            if (value != _selectedIndex && _selectedIndex < renderers.length) {
                var renderer:MovieClip = renderers[_selectedIndex] as MovieClip;
                if (renderer != null) { renderer.gotoAndPlay("off"); }
            }
            
            var lastIndex:Number = _selectedIndex;
            _selectedIndex = value;
            dispatchEventAndSound(new ListEvent(ListEvent.INDEX_CHANGE, true, false, _selectedIndex, -1, -1, getRendererAt(_selectedIndex) as IListItemRenderer, false));        
            
            if (!renderers || renderers.length <= 0) { return; }
            
            // Load the data for the renderers that are now being displayed.
            for (var i = _selectedIndex - 2; i < selectedIndex + 2; i++) {
                if ( i >= 0 && i < renderers.length) {
                    loadDataForRenderer(i);
                }
            }
            
            // Animate the renderers into their appropriate position
            animateRenderers(_selectedIndex);
            
            if (totalRenderers == 0) { return; }
            renderer = getRendererAt(_selectedIndex) as MovieClip;
            if (renderer != null) {
                if (value != lastIndex) {
                    renderer.gotoAndPlay("on");
                }
            }
        }
        
    // Public Methods:
        public function animateRenderers(value:Number):void {
            for (var i:Number = 0; i < renderers.length; i++) {
                var renderer:MovieClip = renderers[i] as MovieClip;
                
                var targetZ:Number = 0;
                if (i < value)      { targetZ = (value - i) * zmargin; }
                else if (i > value) { targetZ = (i - value) * -zmargin; }
                
                var targetX:Number = 0;
                if (i < value)      { targetX = (value - i) * -xmargin; }
                else if (i > value) { targetX = (i - value) * xmargin; }
                
                var targetAlpha:Number = 0;
                var relIndex:Number = (i - value);
                if (relIndex <= 0) {
                    if (relIndex == -1)         { targetAlpha = .25; }
                    else if (relIndex == -2)    { targetAlpha = .15; }
                    else if (relIndex == 0)     { targetAlpha = 1; }
                }

                				// This concerves memory and keeps the screen from failing to render. ~khirsch
				if (targetAlpha == 0) {
					renderer.visible = false;
				}
				else {
					renderer.visible = true;
				}

                // If the renderer is already tweening, stop the original tween.
                if (renderer["activeTween"]) {
                    renderer["activeTween"].paused = true;
                }
                
                // Track each tween within its respective renderer so we can pause it if we need to start a new
                // tween before this tween has finished.
                renderer["activeTween"] = new Tween(    1000,
                                                        renderer,
                                                        { z:targetZ, x:targetX, alpha:targetAlpha },
                                                        { ease:Strong.easeOut } );
                
                // Hide renderers that are offscreen to the right or out of view to the the left.
                // var isHidden:Boolean = ((i > selectedIndex + 2) || (i < selectedIndex - 4));
                // renderer.visible = isHidden;
            }
        }
        
        /** @exclude */
        override public function toString():String {
            return "[MenuKit ImageScroller "+ name +"]";
        }
        
    // Protected Functions:
        protected function getRendererAt(index:uint, offset:int = 0):MovieClip {
            if (renderers == null) { return null; }
            var newIndex:uint = index - offset;
            if (newIndex >= renderers.length) { return null; }
            return renderers[newIndex] as MovieClip;
        }
        
        protected function handleDataChange(event:Event):void {
            invalidate(InvalidationType.DATA);
        }
        
        // NFM: This logic should be moved into the renderer's draw() and out of ImageScroller.
        protected function loadDataForRenderer(index:Number):void {
            if (!renderers || renderers.length == 0) { return; }
            
            var renderer:DifficultyItemRenderer = renderers[index] as DifficultyItemRenderer;
            if (renderer != null) {
                var rendData:Object = dataProvider[index];
                renderer.setData( rendData );
            }
        }
    
        /**
         * Called by sub-classes to create a single renderer based on an index.  The renderer is specified by the {@code itemLinkage} property.
         * @param index The index in the dataProvider
         * @returns The newly-created itemRenderer
         */
        protected function createRenderer(index:uint):MovieClip {
            _itemRendererClass = getDefinitionByName( _itemRenderer ) as Class
            var renderer:MovieClip = new _itemRendererClass() as MovieClip;
            if (renderer == null) {
                trace("Renderer class could not be created."); return null;
            }
            return renderer;
        }
        
        /**
         * Create new renderers and destroy old renderers to ensure there are a specific number of renderers in the component.
         * @param totalRenderers The number of renderers that the component needs.
         */
        protected function drawRenderers(total:Number):void {
            if (_itemRenderer == null) {
                trace("Renderer class not defined."); return;
            }
            
            var i:int, l:int, renderer:MovieClip, displayObject:DisplayObject;
            for (i = renderers.length; i < totalRenderers; i++) {
                renderer = createRenderer(i);
                if (renderer == null) { break; }
                renderers.push(renderer);
                container.addChild(renderer as DisplayObject);
            }
            
            l = renderers.length;
            for (i = l-1; i >= totalRenderers; i--) {
                renderer = getRendererAt(i);
                if (renderer != null) {
                    displayObject = renderer as DisplayObject;
                    if (container.contains(displayObject)) { container.removeChild(displayObject); }
                }
                renderers.splice(i, 1);
            }
        }
        
        override protected function configUI():void {
            super.configUI();
            
            if (container == null) { 
                container = new Sprite();
                addChild(container);
            }
            
            tabEnabled = tabChildren = false;
            mouseEnabled = mouseChildren = false;
        }
        
        override protected function draw():void {
            var i:uint, l:uint, renderer:MovieClip, displayObject:DisplayObject;
            
            // Remove old internal renderers
            if (isInvalid(InvalidationType.RENDERERS)) {
                if (renderers != null) {
                    l = renderers.length;
                    for (i = 0; i < l; i++) {
                        renderer = getRendererAt(i);
                        displayObject = renderer as DisplayObject;
                        if (container.contains(displayObject)) { container.removeChild(displayObject); }
                    }
                }
                renderers = [];
                invalidateData();
            }
            
            // Counter-scale to ensure base component is the right size.
            if (isInvalid(InvalidationType.SIZE) || isInvalid(InvalidationType.DATA)) {
                removeChild(container);
                container.scaleX = 1 / scaleX;
                container.scaleY = 1 / scaleY;
                totalRenderers = _dataProvider.length;
                addChild(container);
                invalidateData();
            }
            
            // Create / destroy renderers
            if (isInvalid(InvalidationType.RENDERERS, InvalidationType.SIZE)) {
                drawRenderers(totalRenderers);
                drawLayout();
            }
            
            if (isInvalid(InvalidationType.DATA)) {
                refreshData();
            }
        }
        
        protected function refreshData():void {
            selectedIndex = Math.min(_dataProvider.length - 1, _selectedIndex);
            _dataProvider.requestItemRange(0, _dataProvider.length - 1, populateData);
        }
        
        protected function drawLayout():void {
            for (var i:Number = 0; i < renderers.length; i++) {
                var renderer:MovieClip = renderers[i] as MovieClip;
                renderer.z = (i * -zmargin); 
                renderer.x = (i * 350);
                
                // Ensure they are the enforced width / height.
                // renderers[i].width = rendererWidth;
                // renderers[i].height = rendererHeight;
            }
        }
        
        // Only fired at the start.
        protected function populateData(data:Array):void {
            loadDataForRenderer(0);
            for (var i:Number = 0; i < renderers.length; i++) {
                var renderer:MovieClip = renderers[i] as MovieClip;
                if (renderer) {
                    renderer.gotoAndPlay( (i == selectedIndex) ? "on" : 0 );
                }
            }
        }
    }
}