/**************************************************************************

Filename    :   MenuManager.as
Content     :  Implementation of the Manager that manages the _viewStack
                including loading / unloading menu views and animating the 
                menu.
                
Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.managers {

    import flash.display.Loader;
    import flash.display.MovieClip;
    import flash.display.DisplayObject;
    import flash.display.DisplayObjectContainer;
    import flash.display.Sprite;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.external.ExternalInterface;
    import flash.net.URLRequest;	
    import flash.system.LoaderContext;
    import flash.system.ApplicationDomain;
    import flash.system.SecurityDomain;
    import flash.utils.getDefinitionByName;
    
    import fl.transitions.easing.*;
    
    import scaleform.gfx.Extensions;
    import scaleform.gfx.FocusManager;
    
    import scaleform.clik.core.CLIK;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.controls.ScrollingList;
    import scaleform.clik.events.ListEvent;
    import scaleform.clik.events.ButtonEvent;
    import scaleform.clik.interfaces.IListItemRenderer;
    import scaleform.clik.motion.Tween;
    import scaleform.clik.managers.FocusHandler;
    
    import com.scaleform.views.View;
    import com.scaleform.system.LoaderWithParams;
    
    public class MenuManager extends UIComponent {
    // Public Properties:
        /** Reference to the MovieClip for the view at the top of the viewStack (currently being displayed. */
        public var currentView:MovieClip;
        
    // Protected Properties:
        /** Default parameters for the standard 'push view/pop view' tweens. */
        protected var _tweenInitParams:Object;
        /** The stack of views which are currently being displayed. _viewStack[_viewStack.length-1] is the topmost view. */
        protected var _viewStack:Array;
        protected var _loadedViews:Array;
        
    // UI Elements:
        /** Reference to the MovieClip to which all the other views are attached. */
        public var manager:MovieClip;
        /** Reference to the cursor's MovieClip. */
        public var cursor:MovieClip;
        
    // Initialization:
        public function MenuManager() { 
            super();
            
            _viewStack = [];
            // Initialize loadedViews Array if it wasn't done before (might have been done during a loadView call).
            if (_loadedViews == null) { _loadedViews = []; }
            _loadedViews = [];
            
            manager.x += 40;
            manager.rotationY = -8;
            
            // Removed _perspfov change until fix in GFx core is added for _perspfov mouse-picking.
            // manager._perspfov = 30;
            
            _tweenInitParams = { alpha:0, z: -1500, x:750 };
        }
        
        /** Retrieve a reference to the top most view on the _viewStack. */
        public function getCurrentView():MovieClip { return _viewStack[_viewStack.lengh-1] };

        public function get viewStack():Array { return _viewStack; }
        public function set viewStack(value:Array):void {
            _viewStack = value;
        }
        
        /** Sets focus to the target MovieClip using Selection.setFocus(). */
        public function setSelectionFocus(mc:MovieClip):void {
            if (mc) { FocusHandler.getInstance().setFocus ( mc ); }
        }
        
        /** Displays or hides the Flash cursor MovieClip based on the parameter. */
        public function showCursor(value:Boolean) {
            cursor.visible = value;
        }
        
        /** Push a view on to the _viewStack using the standard 3Di tween animation and update the position / alpha for all the views in the viewStack. */
        public function pushStandardView(targetView:View):void {        
            if (!targetView) { return; }    // Don't bother pushing views that point to null / undefined MovieClips.
            pushViewImpl(targetView, {z:-1500, x:-750});
        }
        
        /** Pushes the dialog provided on to the _viewStack using a unique tween animation for dialogs and update existing views on the stack. */
        public function pushDialogView(targetView:View):void {
            if (!targetView) { return; }    // Don't bother pushing views that point to null / undefined MovieClips.
            pushViewImpl(targetView, {z:-100, x:-50});
        }
        
        /** Removes the target view from the stack and update the position / alpha for all the views in the viewStack.*/
        public function popView(targetView:View):void {
            if (_viewStack.length <= 1) { return; } // We can't pop a view if there's no view beneath it.
            popViewImpl();
        }
        
        public function loadView(viewName:String, viewURL:String):Loader {
            var containerMC:Sprite = new Sprite();
            containerMC.name = viewName + "Container";

            var _loader = new LoaderWithParams();
            var url:URLRequest = new URLRequest(viewURL); 
            var context:LoaderContext = new LoaderContext(false, ApplicationDomain.currentDomain);
            _loader.contentLoaderInfo.addEventListener(Event.COMPLETE, initHandler, false, 0, true);
            
            _loader.loaderParams.canvas = containerMC;
            _loader.loaderParams.viewName = viewName;
            
            _loader.load(url, context);
            _loader.visible = false;
            
            // Store a reference to the new _loader.
            if (_loadedViews == null) { _loadedViews = []; }
            _loadedViews.push(_loader);
            
            return _loader;
        }

        private function initHandler(event:Event):void {
            var _loader:LoaderWithParams = event.target.loader;
            var _params:Object = _loader.loaderParams;
            var _viewName:String = _params.viewName;
            var _canvas:DisplayObjectContainer = _params.canvas as DisplayObjectContainer;
            var _content:MovieClip = _loader.content as MovieClip;
            
            _loader.tabEnabled = _loader.mouseEnabled = false;
            _canvas.addChild(_loader); 
            
            // trace(">>> View Init Callback( " + _viewName + ", " + CLIK.getTargetPathFor(_content) + ", " + _content + " )");
            if (Extensions["CLIK_addedToStageCallback"]) {
                // Use the viewName that we provided during loadView() to the loader's loaderParams.
                // Prepend the path for the _content with the manager's path since we're about to addChild() the content onto the 
                // the manager and thus its children will fire callbacks with the manager in their path.
                Extensions["CLIK_addedToStageCallback"](_viewName, (CLIK.getTargetPathFor(manager) + "." + CLIK.getTargetPathFor(_content)), _content);
            }
            
            manager.addChild(_canvas as Sprite);
        }
        
    // Private Methods:
        /** Pushes a view onto the stack, tweening it in and configuring its modal properties. */
        private function pushViewImpl(targetView:View, pushParams:Object):void {
            // trace("\nAS MenuManager :: pushViewImpl( " + targetView + " )");
            targetView["tweenPushParams"] = pushParams;
            FocusManager.setModalClip(targetView);
            
            // Intro tween new screen
            var pushedContainer:DisplayObject = targetView.parent as DisplayObject;
            pushedContainer.visible = true;
            
            var newViewLoader:LoaderWithParams = pushedContainer as LoaderWithParams;
            if (newViewLoader.tweenData) {
                newViewLoader.tweenData = { z:0, x:0 };
            }
            
            if (pushedContainer) {
                pushedContainer.alpha = 0;
                pushedContainer.z = -1500;
                pushedContainer.x = 750;
                
                pushedContainer.visible = true;
                
                if (targetView.activeTween) { targetView.activeTween.paused = true; }
                var newViewTween:Tween = new Tween( 300, 
                                                    pushedContainer, 
                                                    { alpha:1, z:0, x:0 },
                                                    { ease:Strong.easeOut } );
                
                targetView.activeTween = newViewTween;
            }
            
            // Push in existing screens incrementally
            var zPush:Number = pushParams.z;
            var xPush:Number = pushParams.x;
            for (var i:Number = 0; i < _viewStack.length; i++) {
                var container:DisplayObject = (_viewStack[i] as DisplayObject).parent as DisplayObject;
                var invIdx:Number = (_viewStack.length - i - 1) + 1;
                
                var oldViewLoader:LoaderWithParams = container as LoaderWithParams;
                if (oldViewLoader) {
                    var targetZ:Number = oldViewLoader.tweenData.z - zPush;
                    var targetX:Number = oldViewLoader.tweenData.x + xPush;
                    var targetAlpha:Number = (100 / (invIdx * 25 + 1)) / 100;
                    
                    var oldView:View = _viewStack[i] as View;
                    if (oldView.activeTween) { oldView.activeTween.paused = true; }
                    
                    var oldViewTween:Tween = new Tween( 300, 
                                                        oldViewLoader, 
                                                        { z:targetZ, x:targetX, alpha:targetAlpha},
                                                        {ease:Strong.easeOut} ); // @todo: Add onComplete.
                    
                    oldView.activeTween = oldViewTween;
                    oldViewLoader.tweenData = { z:targetZ, x:targetX };
                }
            }
            
            _viewStack.push(targetView);
            if (_viewStack.length > 1) {
                if (Extensions["CLIK_playSound"]) Extensions["CLIK_playSound"](this, "default", "viewPushed");
            }
            
            for (var j:Number = 0; j < _viewStack.length-1; j++) {
                var view:View = _viewStack[j] as View;
                view.mouseEnabled = false;
                view.mouseChildren = false;
            }
        }
        
        /** Pops a view from the viewStack, tweening it out and configuring the properties of the new top-most view. */
        private function popViewImpl():MovieClip {
            // Restore next screen in stack
            var poppedView:View = _viewStack.pop() as View;
            var latestView:MovieClip = MovieClip(_viewStack[_viewStack.length - 1]);
            FocusManager.setModalClip(latestView);
            
            var popParams:Object = poppedView.tweenPushParams;
            var zPull:Number = popParams.z;
            var xPull:Number = popParams.x;
            
            // Outro tween popped screen
            var poppedViewLoader:LoaderWithParams = poppedView.parent as LoaderWithParams;
            if (poppedView.activeTween) { poppedView.activeTween.paused = true; }
            var viewTweenOut:Tween = new Tween( 300,
                                                poppedViewLoader, 
                                                { x:750, alpha:0 }, //_tweenInitParams,
                                                {ease:Strong.easeOut, onComplete:hideViewPostTween } );
            poppedView.activeTween = viewTweenOut;
            
            for (var i:Number = 0; i < _viewStack.length; i++) {
                var viewLoader:LoaderWithParams = _viewStack[i].parent as LoaderWithParams;
                
                var invIdx:Number = _viewStack.length - i - 1;
                var targetZ:Number = viewLoader.tweenData.z + zPull;
                var targetX:Number = viewLoader.tweenData.x - xPull;
                var targetAlpha:Number = (100 / (invIdx * 25 + 1)) / 100;
                
                var oldView:View = _viewStack[i] as View;
                if (oldView.activeTween) { oldView.activeTween.paused = true; }
                
                var cTween:Tween = new Tween(   500,
                                                viewLoader, 
                                                { z:targetZ, x:targetX, alpha:targetAlpha },
                                                {ease:Strong.easeOut} );
                oldView.activeTween = cTween;
                viewLoader.tweenData = { z:targetZ, x:targetX };
            }
            
            if (Extensions["CLIK_playSound"]) Extensions["CLIK_playSound"](this, "default", "viewPopped");
            
            latestView.mouseEnabled = true;
            latestView.mouseChildren = true;
            
            return poppedView;
        }
        
        public function hideViewPostTween(viewTween:Tween):void {
            viewTween.onComplete = null; // Clear the onComplete call so it's not fired again during some other tween.
            viewTween.target.visible = false;
        }
    }
}