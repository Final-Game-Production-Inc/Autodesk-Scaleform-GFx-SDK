/**************************************************************************

Filename    :   TreeList.as
Content    :   A "tree view" with multiple root nodes. Nodes with 
                children can be expanded and contracted.
                
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
    
    import scaleform.clik.core.InvalidationType;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.data.ListData;
    import scaleform.clik.events.InputEvent;
    import scaleform.clik.events.ListEvent;
    import scaleform.clik.events.ButtonEvent;
    
    import com.scaleform.mmo.core.TreeListDataProvider;
        
    public class TreeList extends MultiItemList {
        
    // Constants:
        public static const TYPE_LEAF:Number = 0x00>>0;	
        public static const TYPE_OPEN:Number = 0x01>>0;
        public static const TYPE_CLOSED:Number = 0x02>>0;	
            
        public static const ICON_LINE_STRAIGHT:Number = 0x01>>0;
        public static const ICON_LINE_MIDDLE:Number = 0x02>>0;
        public static const ICON_LINE_BOTTOM:Number = 0x03>>0;	
        
        public static const ICON_BUTTON_ALONE:Number = 0x10>>0;
        public static const ICON_BUTTON_TOP:Number = 0x20>>0;
        public static const ICON_BUTTON_MIDDLE:Number = 0x30>>0;
        public static const ICON_BUTTON_BOTTOM:Number = 0x40>>0;
        
        public static const ICON_FOLDER_ROOT:Number = 0x100>>0;
        public static const ICON_FOLDER_LEAF:Number = 0x200>>0;
        
    // Public Properties:
        
    // Protected Properties:
        
    // UI Elements:
        
    // Initialization:
        public function TreeList() {
            super();
        }
        
    // Public getter / setters:
       
    // Public Methods:
       
    // Protected Methods:
        override protected function configUI():void {
            super.configUI();
            addEventListener(ListEvent.ITEM_CLICK, "expandContract", false, 0, true);
            
            // Debugging
            // var dp:TreeViewDataProvider = new TreeViewDataProvider(root);
        }
        
        function expandContract(e:ListEvent):void {
            trace("expandContract: " + e.target.data);
            // Basically, negate the open/closed state, invalidate the DP, and invalidate the tree.
            
            /*
            if (e.item && e.item.type!=TreeViewConstants.TYPE_LEAF) {
                // flip open/closed state
                e.item.type = (e.item.type==TreeViewConstants.TYPE_OPEN)?TreeViewConstants.TYPE_CLOSED:TreeViewConstants.TYPE_OPEN;
                dp.validateLength();
                tree.invalidateData();
            }
            */
        }

    
    }
}
