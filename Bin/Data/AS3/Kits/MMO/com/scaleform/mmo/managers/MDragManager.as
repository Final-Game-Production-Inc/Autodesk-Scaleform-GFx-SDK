/**************************************************************************

Filename    :   MDragManager.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

﻿package com.scaleform.mmo.managers {
    
    import flash.display.DisplayObject;
    import flash.display.DisplayObjectContainer;
    import flash.display.MovieClip;
    import flash.display.Sprite;
    import flash.display.Stage;
    import flash.events.Event;
    import flash.events.EventDispatcher;
    import flash.events.MouseEvent;
    import flash.geom.Point;
    import flash.display.Bitmap;
    import flash.display.IBitmapDrawable;
    import flash.display.BitmapData;
    
    import scaleform.gfx.FocusManager;
    
    import scaleform.clik.events.DragEvent;
    import scaleform.clik.controls.DragTarget;
    import scaleform.clik.interfaces.IDragTarget;
    import scaleform.clik.managers.DragManager;
    
    public class MDragManager extends DragManager {
        
    // Constants:
        
    // Protected Properties:

    // Initialization:
        public static function init(stage:Stage):void {
            super.init(stage);
        }
        
    // Public Methods:
        public static function handleStartDragEvent( e:DragEvent ):void {
            super.handleStartDragEvent(e);
            GameDataModel.onStartSlotDrag( _origDragTarget.id );
        }
        
        public static function handleEndDragEvent( e:MouseEvent ):void {
            super.handleEndDragEvent(e);
            GameDataModel.onEndDrag( _origDragTarget.id, dropTarget.id );
        }
        
    // Protected Methods:
    
    }
}