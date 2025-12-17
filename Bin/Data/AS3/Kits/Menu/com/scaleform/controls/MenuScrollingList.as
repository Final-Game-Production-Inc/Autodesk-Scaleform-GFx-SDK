/**************************************************************************

Filename    :   MenuScrollingList.as
Content     :  Custom ScrollingList implementation for the Menu Kit with
                extra logic for handling focus changes from mouse input.
                
Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.controls {

    import flash.display.MovieClip;
    import flash.display.DisplayObject;
    import flash.events.Event;
    import flash.events.MouseEvent;
    
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.controls.ScrollingList;
    import scaleform.clik.events.ListEvent;
    import scaleform.clik.events.ButtonEvent;
    import scaleform.clik.interfaces.IListItemRenderer;
    
    import com.scaleform.controls.FocusItemRenderer;
    import com.scaleform.events.MenuListEvent;
    
    public class MenuScrollingList extends ScrollingList {
    // Constants:

    // Public Properties:
    
    // Private Properties:
        protected var lastSelectedIndex:Number = -1;
        
    // UI Elements:
    
    // Initialization:
        public function MenuScrollingList() {
            enableInitCallback = true;
            super();
        }
        
        override protected function dispatchItemEvent(event:Event):Boolean {
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
                case Event.CHANGE:
                    type = MenuListEvent.ITEM_CHANGE; break;
                default:
                    return true;
            }
            
            var renderer:IListItemRenderer = event.currentTarget as IListItemRenderer;
            var isKeyboard:Boolean = false;
            if (event is ButtonEvent) { isKeyboard = (event as ButtonEvent).isKeyboard; }
            var newEvent:ListEvent = new ListEvent(type, false, true, renderer.index, 0, renderer.index, renderer, isKeyboard);
            
            return dispatchEventAndSound(newEvent);
        }

        override protected function changeFocus():void {
            super.changeFocus();
            var renderer:IListItemRenderer = getRendererAt(_selectedIndex, _scrollPosition);
            if (renderer != null) {
                // if ( renderer.displayFocus == true ) { return; }
                // renderer.displayFocus = (focused > 0);
                
                var itemRenderer:FocusItemRenderer = renderer as FocusItemRenderer;
                var keyframe:String = (_focused > 0) ? "selected_over" : "up";
                itemRenderer.changeState( keyframe );
                
                // NFM: Let's move this into MultiControlItemRenderer.changeState().
                var optionStepper:DisplayObject = MovieClip(renderer).getChildByName("optionstepper");
                if (optionStepper != null) {
                    MovieClip(optionStepper).gotoAndPlay( (_focused) ? "focused" : "default" );
                }
            }
        }
        
        override protected function setupRenderer(renderer:IListItemRenderer):void {
            super.setupRenderer(renderer);
            renderer.addEventListener(Event.CHANGE, dispatchItemEvent, false, 0, true);
        }
        
        override protected function cleanUpRenderer(renderer:IListItemRenderer):void {
            super.cleanUpRenderer(renderer);
            renderer.removeEventListener(Event.CHANGE, dispatchItemEvent, false);
        }
        
        override protected function configUI():void {
            super.configUI();
            
            // Add event listener that gives a listItemRenderer focus when it on ITEM_ROLL_OVER event.
            addEventListener(ListEvent.ITEM_ROLL_OVER, onItemRollOver_ChangeSelected, false, 0, true);
        }
    
        protected function onItemRollOver_ChangeSelected( p_event:ListEvent ):void {
            if (!enabled) { return; }
            
            // If it's a rollOver event, remove the old SelectedIndex so we don't see it play an animation.
            if (!_focused) { 
                // Need to ensure that we reset the previous renderer properly here since _selectedIndex will be -1 in a moment
                // and therefore it won't be updated in updateSelectedIndex().
                var renderer:IListItemRenderer = getRendererAt(_selectedIndex, _scrollPosition);
                renderer.selected = false; 
                _selectedIndex = -1; 
            }

            // Set focus to the list so that the display indicator will be displayed when we do set the selectedIndex;
            // But don't use .focused or we'll call changeFocus() more than once.
            // stage.focus = this; // NFM, 12/27: This alone leaves this.focused = 0. This just doesn't work right. Might be a FocusHandler issue.
            this.focused = 1; // NFM, 12/27: stage.focus = null if we only do this. Using both commands for now.
        
            selectedIndex = p_event.index; // Set the selectedIndex (will be updated for updateSelectedIndex).
            updateSelectedIndex(); // Update the selectedIndex.
            changeFocus();
        }
    }
}