/**************************************************************************

Filename    :   MultiItemList.as
Content    :   A list that supports multiple item renderers as part of
                the dataProvider.
                
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
    import flash.utils.Dictionary;
    import flash.utils.getDefinitionByName;
    
    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.data.ListData;
    import scaleform.clik.events.InputEvent;
    import scaleform.clik.events.ListEvent;
    import scaleform.clik.events.ButtonEvent;
    import scaleform.clik.interfaces.IDataProvider;
    import scaleform.clik.interfaces.IListItemRenderer;
        
    public class MultiItemList extends BasicList {
    // Constants:
        
    // Public Properties:
        
    // Protected Properties:
        protected var rendererData:Dictionary;
    
    // UI Elements:
        
    // Initialization:
        public function MultiItemList() {
            super();
            
            rendererData = new Dictionary();
        }
        
    // Public getter / setters:
       
    // Public Methods:
       
    // Protected Methods:
        override protected function draw():void {
            if (isInvalid(InvalidationType.DATA)) {
                updateRendererData();
                totalRenderers = dataProvider.length;
            }
            super.draw();
        }
        
        protected function updateRendererData():void {
            if (dataProvider == null || dataProvider.length < 1) { 
                return;
            }
            
            for (var i:uint = 0; i < dataProvider.length; i++) { 
                var rendererClassName:String = dataProvider[i].rendererClassName;
                if (rendererClassName == null) { 
                    trace(">> " + this + ", Error: dataProvider[" + i + "].rendererClass == null. Renderer creation will fail.");
                    return;
                }
                
                // Information for this renderer (class / height) has already been collected.
                if (rendererData[rendererClass] != undefined) {
                    continue;
                }
                
                // Get a reference to the class for the itemRenderer.
                var rendererClass:Class = getDefinitionByName(rendererClassName) as Class;
                
                // Create an instance of the renderer to measure its height.
                var renderer:IListItemRenderer = createRendererByClass(rendererClass, 0, false);
                
                var rendererHeight:uint = renderer.height;
                var rendererInfo:Object = { rclass:rendererClass, rheight:rendererHeight };
                rendererData[rendererClassName] = rendererInfo;
                
                renderer = null; // NFM: This should mark the renderer for GC.
            }
        }
            
            
        override protected function drawRenderers(total:Number):void {
            var i:int, l:int, renderer:IListItemRenderer, displayObject:DisplayObject;
            for (i = renderers.length; i < totalRenderers; i++) {
                var rendererClassName:String = dataProvider[i].rendererClassName;
                renderer = createRendererByClass(rendererData[rendererClassName].rclass, i);
                if (renderer == null) { break; }
                renderers.push(renderer);
                container.addChild(renderer as DisplayObject);
            }
            l = renderers.length;
            for (i = l-1; i >= totalRenderers; i--) {
                renderer = getRendererAt(i);
                if (renderer != null) {
                    cleanUpRenderer(renderer);
                    displayObject = renderer as DisplayObject;
                    if (container.contains(displayObject)) { container.removeChild(displayObject); }
                }
                renderers.splice(i, 1);
            }
        }
        
        protected function createRendererByClass(rclass:Class, index:uint, setup:Boolean=true):IListItemRenderer {
            var renderer:IListItemRenderer = new rclass() as IListItemRenderer;
            if (renderer == null) {
                trace(">> " + this + ", Error: Renderer class " + rclass + " could not be created."); 
                return null;
            }
            if (setup) { 
                setupRenderer(renderer);
            }
            return renderer;
        }
        
        override protected function drawLayout():void {
            var dataWillChange:Boolean = isInvalid(InvalidationType.DATA);
            var l:uint = renderers.length;
            var ycoord:int = 0;
            for (var i:uint = 0; i < l; i++) {
                // This data could also be stored in the IListItemRenderer. Potential optimization.
                var renderer:IListItemRenderer = getRendererAt(i);
                
                renderer.x = 0;
                
                // discover the height of the previous renderer.
                var preRendHeight:int = 0;
                if (i > 0) { 
                    var rendererClassName:String = dataProvider[i - 1].rendererClassName;
                    preRendHeight = rendererData[rendererClassName].rheight;
                }
                
                renderer.y = ycoord + preRendHeight; // NFM: Can add a +1 here to help avoid overlapping.
                ycoord = renderer.y;
                
                if (!dataWillChange) { renderer.validateNow(); }
            }
        }
    }
}
