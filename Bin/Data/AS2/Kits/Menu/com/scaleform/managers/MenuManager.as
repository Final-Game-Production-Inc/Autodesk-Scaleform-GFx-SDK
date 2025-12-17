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
import gfx.core.UIComponent;
import System.capabilities;

import com.scaleform.views.View;

import gfx.motion.Tween;
import gfx.ui.InputDetails;
import gfx.ui.NavigationCode;

import mx.transitions.easing.*;
import flash.external.ExternalInterface;

class com.scaleform.managers.MenuManager extends UIComponent {

// Public Properties:
    /** Reference to the MovieClip for the view at the top of the viewStack (currently being displayed. */
    public var currentView:MovieClip;
    
// Private Properties:
    /** Default parameters for the standard 'push view/pop view' tweens. */
    private var _tweenInitParams:Object;
    /** The stack of views which are currently being displayed. _viewStack[_viewStack.length-1] is the topmost view. */
    private var _viewStack:Array;
    /** Cursor enabled flag **/
    private var _cursorEnabled:Boolean;
    
// UI Elements:
    /** Reference to the MovieClip to which all the other views are attached. */
    public var manager:MovieClip;
    /** Reference to the cursor's MovieClip. */
    public var cursor:MovieClip;
   
// Initialization:
    public function MenuManager() { 
        super();
        
        _viewStack = [];
        Tween.init();

        manager._z = 450;
        manager._x += 40;
        manager._yrotation = -8;
        
        // Removed _perspfov change until fix in GFx core is added for _perspfov mouse-picking.
        // manager._perspfov = 30;
        
        _tweenInitParams = { _alpha:0, _z: -15000, _x:750 };
        
        if (capabilities.os == "Windows" || capabilities.os == "Mac OS X" || capabilities.os == "Linux") 
            _cursorEnabled = true;
        else 
            _cursorEnabled = false;
            
        showCustomCursor(true);
    }
    
    /**  Invoked from the application when the swf is put in background mode. We yield system cursor to something else. */
    public function onPause(val:Boolean):Void {
        showCustomCursor(!val);
    }
    
    /** Invoked from the application when the swf is destroyed. This method is NOT called automatically by GFx since _root is not in timeline. */
    public function onUnload():Void {
        showCustomCursor(false);
    }
    
    /** Retrieve a reference to the top most view on the _viewStack. */
    public function getCurrentView():MovieClip { return _viewStack[_viewStack.lengh-1] };
    
    public function get viewStack():Array { return _viewStack; }
    public function set viewStack(value:Array):Void {
        _viewStack = value;
    }
    
    /** Sets focus to the target MovieClip using Selection.setFocus(). */
    public function setSelectionFocus(mc:MovieClip):Void {
        Selection.setFocus(mc);
    }
    
    /** Push a view on to the _viewStack using the standard 3Di tween animation and update the position / alpha for all the views in the viewStack. */
    public function pushStandardView(targetView:View):Void {        
        if (!targetView) { return; }    // Don't bother pushing views that point to null / undefined MovieClips.
        pushViewImpl(targetView, {z:-15000, x:-750});
    }
    
    /** Pushes the dialog provided on to the _viewStack using a unique tween animation for dialogs and update existing views on the stack. */
    public function pushDialogView(targetView:View):Void {
        if (!targetView) { return; }    // Don't bother pushing views that point to null / undefined MovieClips.
        pushViewImpl(targetView, {z:-1000, x:-50});
    }
    
    /** Removes the target view from the stack and update the position / alpha for all the views in the viewStack.*/
    public function popView(targetView:View):Void {
        if (_viewStack.length <= 1) { return; } // We can't pop a view if there's no view beneath it.
        popViewImpl();
    }
    
// Private Methods:
    /** Pushes a view onto the stack, tweening it in and configuring its modal properties. */
    private function pushViewImpl(targetView:View, pushParams:Object):Void {
        // trace("\nAS MenuManager :: pushViewImpl( " + targetView + " )");
        
        targetView["tweenPushParams"] = pushParams;
        Selection["modalClip"] = targetView;
        
        // Intro tween new screen
        var pushedContainer:MovieClip = targetView._parent;
        pushedContainer._visible = true;    // PPS: Do we need this?
        pushedContainer._alpha = 0;
        pushedContainer._z = -15000;
        pushedContainer._x = 750;  
		pushedContainer["onTweenComplete"] = null;
        pushedContainer.tweenTo(0.3, 
        {
            _alpha: 100,
            _z: 0,
            _x: 0
        }, Strong.easeOut);
        pushedContainer.target = { z:0, x:0 };
        
        // Push in existing screens incrementally
        var zPush:Number = pushParams.z;
        var xPush:Number = pushParams.x;
        for (var i:Number = 0; i < _viewStack.length; i++) {
            var container:MovieClip = MovieClip(_viewStack[i])._parent;
            var invIdx:Number = (_viewStack.length - i - 1) + 1;
            var targetZ:Number = container.target.z - zPush;
            var targetX:Number = container.target.x + xPush;
            container.tweenTo(0.5, 
            { 
                _alpha: 100 / (invIdx * 25 + 1), 
                _z: targetZ,
                _x: targetX
            }, Strong.easeOut);
            container.target = { z:targetZ, x:targetX };
        }
        
        _viewStack.push(targetView);
		if (_viewStack.length > 1) {
			playSound("viewPushed");
		}
        
        // Attach a MovieClip ("modal") that will absord all Mouse events targeted at views behind the top-most view.
        if (_viewStack.length > 1) {
            targetView["modalBG"].attachMovie("modal", "modal", this.getNextHighestDepth());
        }
    }
    
    /** Pops a view from the viewStack, tweening it out and configuring the properties of the new top-most view. */
    private function popViewImpl():MovieClip {
        // trace("popView(" + poppedView + ")");
        
        // Restore next screen in stack
        var poppedView:MovieClip = MovieClip(_viewStack.pop());
        var latestView:MovieClip = MovieClip(_viewStack[_viewStack.length - 1]);
        Selection["modalClip"] = latestView;
        
        var popParams:Object = poppedView.tweenPushParams;
        var zPull:Number = popParams.z;
        var xPull:Number = popParams.x;
        
        // Outro tween popped screen
        var poppedContainer:MovieClip = poppedView._parent;
        poppedContainer.tweenTo(0.3, _tweenInitParams, Strong.easeOut);
		poppedContainer["onTweenComplete"] = hideViewPostTween;
        
        for (var i:Number = 0; i < _viewStack.length; i++) {
            var container:MovieClip = MovieClip(_viewStack[i])._parent;
            var invIdx:Number = _viewStack.length - i - 1;
            var targetZ:Number = container.target.z + zPull;
            var targetX:Number = container.target.x - xPull;
            container.tweenTo(0.5, 
            { 
                _alpha:100 / (invIdx * 25 + 1), 
                _z: targetZ,
                _x: targetX
            }, Strong.easeOut);
            container.target = { z:targetZ, x:targetX };
        }
		
		playSound("viewPopped");
        
        // Remove the previously attached MovieClip that absorbed Mouse events now that this view has been removed from the stack.
        poppedView["modalBG"].modal.removeMovieClip();
        return poppedView;
    }
	
	public function hideViewPostTween():Void {
		this["onTweenComplete"] = null;
		this._visible = false;
	}
    
    /** Interface to manager custom cursor visibility. */
    private function showCustomCursor(value:Boolean):Void {
        if (!_cursorEnabled) {
            cursor._visible = false;
            return;
        }
        
        cursor._visible = value;
        if (value) {
            Mouse.hide();
            cursor.hitTestDisable = true; 
            cursor._x = _root._xmouse;
            cursor._y = _root._ymouse;
        } else {
            Mouse.show();
        }
    }    
}