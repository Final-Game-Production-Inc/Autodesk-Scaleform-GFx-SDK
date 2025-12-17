/**************************************************************************

Filename    :   ChatLogWidget.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo.chatlog {
    
    import flash.display.MovieClip;
    import flash.events.EventPhase;
    import flash.events.FocusEvent;
    import flash.events.KeyboardEvent;
    import flash.text.TextField;
    import flash.events.MouseEvent;
    import flash.events.Event;
    import flash.text.TextFormat;
    import flash.ui.Keyboard;
    
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.controls.ScrollBar;
    import scaleform.clik.controls.ButtonBar;
    import scaleform.clik.controls.Button;
    import scaleform.clik.controls.TextInput;
    import scaleform.clik.utils.Constraints;
    import scaleform.clik.constants.ConstrainMode;
    import scaleform.clik.events.ButtonEvent;
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.events.IndexEvent;
    
    import scaleform.gfx.TextFieldEx;
    
    import com.scaleform.mmo.MMOKit;
    import com.scaleform.mmo.GameDataModel;
    
    public class ChatLogWindow extends UIComponent {
    
    // Public Properties:
    
    // Protected Properties:
        protected var minWidth:Number = 200;
        protected var maxWidth:Number = 800;
        protected var minHeight:Number = 200;
        protected var maxHeight:Number = 800;
        protected var _dragProps:Array;
        protected var _hidden:Boolean = false;
        protected var _lockToBottom:Boolean = true;
        
    // UI Elements:
        public var scrollBar:ScrollBar;
        public var textField:TextField;
        public var tabBar:ButtonBar;
        public var hideBtn:Button;
        public var tfInput:ChatLogTextInput;
        public var sendBtn:Button;
        public var sizeGrip:Button;
        public var background:MovieClip;
        
        public function ChatLogWindow() {
            super();
        }
        
        override protected function preInitialize():void {
            constraints = new Constraints(this, ConstrainMode.REFLOW);
        }
        
        override protected function configUI():void {
            super.configUI();
            
            constraints.addElement("scrollBar", scrollBar, Constraints.TOP | Constraints.BOTTOM);
            constraints.addElement("textField", textField, Constraints.ALL);
            constraints.addElement("tabBar", tabBar, Constraints.TOP | Constraints.LEFT | Constraints.RIGHT);
            constraints.addElement("background", background, Constraints.ALL);
            constraints.addElement("hideBtn", hideBtn, Constraints.LEFT | Constraints.BOTTOM);
            constraints.addElement("tfInput", tfInput, Constraints.LEFT | Constraints.RIGHT | Constraints.BOTTOM);
            constraints.addElement("sendBtn", sendBtn, Constraints.RIGHT | Constraints.BOTTOM);
            constraints.addElement("sizeGrip", sizeGrip, Constraints.RIGHT | Constraints.BOTTOM);
            
            sizeGrip.addEventListener(MouseEvent.MOUSE_DOWN, handleResizeMouseDown, false, 0, true);
            tabBar.addEventListener(MouseEvent.MOUSE_DOWN, handleMoveMouseDown, false, 0, true);
            background.addEventListener(MouseEvent.MOUSE_DOWN, handleMoveMouseDown, false, 0, true);
            hideBtn.addEventListener(ButtonEvent.CLICK, handleWidgetHide, false, 0, true);
            scrollBar.addEventListener(Event.SCROLL, handleScroll, false, 0, true);
            tfInput.addEventListener(KeyboardEvent.KEY_UP, checkInputKeyUp, true, 0, true);
            sendBtn.addEventListener(ButtonEvent.CLICK, handleSendButtonClick, false, 0, true);
            
            tabEnabled = tabChildren = false;            
            
            TextFieldEx.setVerticalAlign(textField, TextFieldEx.VALIGN_BOTTOM);
            
            if (GameDataModel.getMessageLogs != null) {
                tabBar.dataProvider = new DataProvider( GameDataModel.getMessageLogs() );
            }
            tabBar.validateNow();            
            
            tabBar.addEventListener(IndexEvent.INDEX_CHANGE, handleTabIndexChange, false, 0, true);
            tabBar.selectedIndex = 0;
            
            if (GameDataModel.registerMessageLogListener != null) {
                GameDataModel.registerMessageLogListener(onNewLogMessage);
            }
            addEventListener(Event.REMOVED_FROM_STAGE, unregisterListeners);
            
            tfInput.addEventListener(FocusEvent.FOCUS_IN, handleInputFocusIn, false, 0, true);
            
            stage.addEventListener(KeyboardEvent.KEY_UP, focusInput, false, 0, true);
        }
            
        protected function handleInputFocusIn(e:FocusEvent):void {
            refreshPrefix();
        }
        protected function refreshPrefix():void {
            var info:Object = GameDataModel.getSubmitChannelInfo(tabBar.dataProvider[tabBar.selectedIndex].id);
            var color:Number = Number( "0x" + info.color.substring( 1 ) );
            tfInput.inputTextFormat.color = color;
            tfInput.setPrefix(info.name + ": ");
        }
        
        protected function focusInput(e:KeyboardEvent):void {
            if (e.keyCode == Keyboard.ENTER && !tfInput.focused) {
                tfInput.focused = 1;
            }
        }
        
        protected function handleSendButtonClick(e:ButtonEvent):void {
            if (tfInput.text.length > 0) {
                submitText();
            }
        }        
        
        protected function checkInputKeyUp( e:KeyboardEvent ):void {
            if (e.keyCode ==  Keyboard.ENTER ) {
                if (tfInput.text.length > 0) {
                    submitText();
                    e.stopImmediatePropagation();
                }
            } 
            else if (e.keyCode == Keyboard.ESCAPE) {
                tfInput.clearPrefix();
                tfInput.text = "";                
                stage.focus = null;
                e.stopImmediatePropagation();
            }             
        }
        
        protected function submitText():void {
            GameDataModel.submitLogMessage(tabBar.dataProvider[tabBar.selectedIndex].id, tfInput.text);
            tfInput.text = "";         
            refreshPrefix();
        }
        
        protected function unregisterListeners(e:Event):void {
            GameDataModel.unregisterMessageLogListener(onNewLogMessage);
        }
        
        protected function handleScroll(e:Event):void {
            _lockToBottom = (scrollBar.position >= textField.maxScrollV);
        }        
        
        protected function handleTabIndexChange(e:IndexEvent):void {
            var lastIdx:int = e.lastIndex;
            var newIdx:int = e.index;
            
            if (newIdx >= 0) {
                var data:Object = tabBar.dataProvider[newIdx];
                if (!data) return;
                var logId:String = data.id;
                textField.htmlText = GameDataModel.getMessageLogData(logId);
                _lockToBottom = true;
                updateLogState();
                
                tfInput.clearPrefix();
                tfInput.text = "";                
            }
        }
        
        protected function onNewLogMessage(logId:String, msg:String):void {
            var selectedLog:String = tabBar.dataProvider[tabBar.selectedIndex].id;
            if (selectedLog == logId) {
                addToChannelLog(msg);
            }
        }
        
        protected function addToChannelLog(s:String):void {
            TextFieldEx.appendHtml(textField, s);
            // Keep scroll at the bottom always
            updateLogState();
        }
               
        protected function updateLogState():void {            
            strip();
            if (_lockToBottom)
                textField.scrollV = textField.maxScrollV;
        }
        
        protected function strip():void {
            while (textField.numLines > 1000) {
                var start:int = textField.getFirstCharInParagraph(0);
                var len:int = textField.getParagraphLength(0);
                textField.replaceText(start, start + len, "");
            }
        }
        
        override protected function draw():void {
            // Resize and update constraints
            if (isInvalid(InvalidationType.SIZE)) {
                constraints.update(_width, _height);
                updateLogState();
            }
        }
        
        protected function handleWidgetHide(e:Event):void {
            if (_hidden) { showView(); }
            else { hideView(); }
            _hidden = !_hidden;
        }
        
        protected function hideView():void {
            scrollBar.visible = false;
            textField.visible = false;
            tabBar.visible = false;
            tfInput.visible = false;
            sendBtn.visible = false;
            sizeGrip.visible = false;
            background.visible = false;
        }
        protected function showView():void {
            scrollBar.visible = true;
            textField.visible = true;
            tabBar.visible = true;
            tfInput.visible = true;
            sendBtn.visible = true;
            sizeGrip.visible = true;
            background.visible = true;
        }
        
        protected function handleResizeMouseDown(e:Event):void {
            stage.addEventListener(MouseEvent.MOUSE_UP, handleResizeMouseUp, false, 0, true);
            _dragProps = [parent.mouseX - (x + width), parent.mouseY - (y + height)];
            stage.addEventListener(MouseEvent.MOUSE_MOVE, handleResizeMouseMove, false, 0, true);
        }
        
        protected function handleResizeMouseUp(e:Event):void {
            stage.removeEventListener(MouseEvent.MOUSE_MOVE, handleResizeMouseMove, false);
            stage.removeEventListener(MouseEvent.MOUSE_UP, handleResizeMouseUp, false);
        }
        
        protected function handleResizeMouseMove(e:Event) {
            var w:Number = Math.max(minWidth, Math.min(maxWidth, parent.mouseX - x - _dragProps[0]));
            var h:Number = Math.max(minHeight, Math.min(maxHeight, parent.mouseY - y - _dragProps[1]));
            if (w != _width || h != _height) {
                setSize(w, h);
            }
        }
        
        protected function handleMoveMouseDown(e:Event):void {
            stage.addEventListener(MouseEvent.MOUSE_UP, handleMoveMouseUp, false, 0, true);
            startDrag();
        }
        
        protected function handleMoveMouseUp(e:Event):void {
            stage.removeEventListener(MouseEvent.MOUSE_UP, handleMoveMouseUp, false);
            stopDrag();
        }		
    }
}
