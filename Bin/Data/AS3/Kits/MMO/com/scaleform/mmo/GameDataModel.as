/**************************************************************************

Filename    :   GameDataModel.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo {
    
    import flash.external.ExternalInterface;
    
    public class GameDataModel {
        
        public static function initialize():void {
            ExternalInterface.call("gameDataModelInit", GameDataModel);
        }
        
        // Item slots (all parameters are binding IDs)
        public static var notifySlotDragStart:Function;	    // params: (binding:String)
        public static var notifySlotDragEnd:Function;       // params: (srcBinding:String, destBinding:String)
        public static var notifySlotUse:Function;           // params: (binding:String)
        
        // Inventory
        public static var getBagSlotBindings:Function;      // params: (id:String) - bag entity id
        
        // Spellbook
        public static var getSpellbookGroups:Function;      // params: none
        public static var getSpellsByGroup:Function;        // params: (groupid:Number)
        
        // Tooltip
        public static var getSlotTooltip:Function;          // params: (binding:String)
		
		// Castbar
		public static var registerCastBar:Function;			// params: (castbarMc:MovieClip)
		
		// Chat logs
		public static var getMessageLogs:Function;					// params: none
		public static var registerMessageLogListener:Function;		// params: (handlerClosure:Function)
		public static var unregisterMessageLogListener:Function;	// params: (handlerClosure:Function)
		public static var getMessageLogData:Function;				// params: (logId:String)
        
        // Chat input
        public static var submitLogMessage:Function;                // params: (logId:String, msg:String)
        public static var getSubmitChannelInfo:Function;                // params: (logId:String)
    }
}
