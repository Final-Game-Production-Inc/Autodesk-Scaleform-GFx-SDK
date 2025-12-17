/**************************************************************************

Filename    :   DifficultyItemRenderer.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
    
package com.scaleform.controls {
    
	import flash.events.Event;
    import flash.display.Bitmap;
    import flash.display.Loader;
    import flash.display.MovieClip;
    import flash.events.FocusEvent;
    import flash.net.URLRequest;
    import flash.system.LoaderContext
    
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.data.ListData;
    import scaleform.clik.interfaces.IListItemRenderer;
    
    public class DifficultyItemRenderer extends MovieClip {
        
    // Constants:
        
    // Public Properties:
        
    // Protected Properties:
        protected var _data:Object;
        protected var _loadedImage:String
        
    // UI Elements:
        public var image_text:MovieClip;
        public var image:MovieClip;
           
        public var imageLoader:Loader;
    
    // Initialization:
        public function DifficultyItemRenderer() {
            super();
            _loadedImage = "";
        }
        
    // Public Getter / Setters:
        
    // Public Methods:
        public function setData(data:Object):void {
            _data = data;
            draw();
        }
        
        override public function toString():String {
            return "[MenuKit DifficultyItemRenderer " + name + "]";
        }
    
    // Protected Methods:
        protected function draw():void {
            if (_data) {
                if (_data.label) {
                    image_text.mode.textField.text = _data.label;
                }
                
                if (_data.players) {
                    image_text.players.textField.text = _data.players;
                }
                
                if (_data.image && (_loadedImage != _data.image)) {
                    if (imageLoader == null) {
                        imageLoader = new Loader();
                        image.addChild(imageLoader);
                    }
                    
                    var url:URLRequest = new URLRequest(_data.image); 
                    var context:LoaderContext = new LoaderContext(false, null);
                    imageLoader.contentLoaderInfo.addEventListener(Event.COMPLETE, initHandler, false, 0, true);
                    imageLoader.load(url, context);
                    _loadedImage = _data.image;
                }
            }
        }
        
        protected function initHandler(event:Event):void {
            var loader:Loader = event.target.loader as Loader;
            var content:Bitmap = loader.content as Bitmap;
			if(content != null)
				content.smoothing = true;
        }
        
    }
    
}