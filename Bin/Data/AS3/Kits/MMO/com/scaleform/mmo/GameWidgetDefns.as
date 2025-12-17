/**************************************************************************

Filename    :   GameWidgetDefns.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo {
    
    import com.scaleform.mmo.data.Widget;
    import flash.external.ExternalInterface;
    
    public class GameWidgetDefns {
        
        public static const WIDGET_INVENTORY:uint       = 0;
        public static const WIDGET_MINIMAP:uint         = 1;
        public static const WIDGET_QUEST_LOG:uint       = 2;
        public static const WIDGET_ACTION_BARS:uint     = 3;
        public static const WIDGET_PAPER_DOLL:uint      = 4;
        public static const WIDGET_WINDOW_MENU:uint     = 5;
        public static const WIDGET_PLAYER_UF:uint       = 6;
        public static const WIDGET_TARGET_UF:uint       = 7;
        public static const WIDGET_SPELLBOOK:uint       = 8;
        public static const WIDGET_CHAT_LOG:uint        = 9;
        public static const WIDGET_QUEST_TRACKER:uint   = 10;
        public static const WIDGET_CAST_BAR:uint        = 11;
        
        private static var instance:GameWidgetDefns;
        private static var allowInstantiation:Boolean;
        
        public var widgetDefs:Array = [];
        public var fileExtension:String = ".swf";
        
        public function GameWidgetDefns():void {
            if (!allowInstantiation) {
                throw new Error("Error: Instantiation failed: Use GameWidgetDefns.getInstance() instead of new.");
            }
            
            // Checks to see whether this is a Shipping Build. If so, load exported and packed .gfx files rather than .swf files.
            if (ExternalInterface.available) {
                var returnValue:Object = { };
                ExternalInterface.call("isShippingBuild", returnValue)
                fileExtension = (returnValue.isShipping) ? ".gfx" : ".swf";
            }
            
            generateData();
        }
        
        public static function getInstance():GameWidgetDefns {
            if (instance == null) {
                allowInstantiation = true;
                instance = new GameWidgetDefns();
                allowInstantiation = false;
            }
            return instance;
        }
        
        protected function generateData() {
            var inventory:Widget    = new Widget(   WIDGET_INVENTORY,
                                                    "widgets/InventoryView" + fileExtension,
                                                    "Inventory",
                                                    false,
                                                    1064,
                                                    648);
            widgetDefs.push(inventory);
                                                
            var minimap:Widget      = new Widget(   WIDGET_MINIMAP,
                                                    "widgets/MinimapWidget" + fileExtension,
                                                    "Minimap",
                                                    false,
                                                    1080,
                                                    0 );
            widgetDefs.push(minimap);
                                                
            var questlog:Widget     = new Widget(   WIDGET_QUEST_LOG,
                                                    "",
                                                    "Quest Log",
                                                    true );
            widgetDefs.push(questlog);
                                                
            var actionbars:Widget   = new Widget(   WIDGET_ACTION_BARS,
                                                    "widgets/ActionBarsView" + fileExtension,
                                                    "Action Bars",
                                                    false );
            widgetDefs.push(actionbars);
                                                    
            var paperdoll:Widget    = new Widget(   WIDGET_PAPER_DOLL,
                                                    "widgets/PaperDollView" + fileExtension,
                                                    "Paper Doll",
                                                    true );
            widgetDefs.push(paperdoll);
                                                    
            var windowmenu:Widget   = new Widget(   WIDGET_WINDOW_MENU,
                                                    "widgets/WindowMenuWidget" + fileExtension,
                                                    "Window Menu",
                                                    false,
                                                    10,
                                                    650 );
            widgetDefs.push(windowmenu);
            
            var playeruf:Widget   = new Widget(     WIDGET_PLAYER_UF,
                                                    "widgets/PlayerUnitFrameWidget" + fileExtension,
                                                    "Player Unit Frame",
                                                    false,
                                                    0,
                                                    0 );
            widgetDefs.push(playeruf);
            
            var targetuf:Widget   = new Widget(     WIDGET_TARGET_UF,
                                                    "widgets/TargetUnitFrameWidget" + fileExtension,
                                                    "Target Unit Frame",
                                                    false,
                                                    300,
                                                    0 );
            widgetDefs.push(targetuf);
            
            var spellbook:Widget   = new Widget(    WIDGET_SPELLBOOK,
                                                    "widgets/SpellbookWidget" + fileExtension,
                                                    "Spellbook",
                                                    true );
            widgetDefs.push(spellbook);
            
            var chatlog:Widget   = new Widget(      WIDGET_CHAT_LOG,
                                                    "widgets/ChatlogWidget" + fileExtension,
                                                    "Chatlog",
                                                    false,
                                                    10,
                                                    410 );
            widgetDefs.push(chatlog);
            
            var questtracker:Widget   = new Widget( WIDGET_QUEST_TRACKER ,
                                                    "widgets/QuestTrackerWidget" + fileExtension,
                                                    "Quest Tracker",
                                                    false,
                                                    1000,
                                                    250 );
            widgetDefs.push(questtracker);
            
            var castbar:Widget        = new Widget( WIDGET_CAST_BAR,
                                                    "widgets/CastbarWidget" + fileExtension,
                                                    "Cast Bar",
                                                    false,
                                                    500,
                                                    500 );
            widgetDefs.push(castbar);
        }
    }
}
