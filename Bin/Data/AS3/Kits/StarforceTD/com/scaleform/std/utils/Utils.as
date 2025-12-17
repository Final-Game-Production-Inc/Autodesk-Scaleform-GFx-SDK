/**************************************************************************

Filename    :   Utils.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


package com.scaleform.std.utils
{
    import flash.geom.Point;
    
    import com.scaleform.std.entities.Entity;

    public class Utils 
    {
        public static var idCounter:uint = 0;
		
		// PPS: Should be in another file; exists here because I was lazy
		public static var	showIntro:Boolean = true;
        
        public static function getNewId():uint { return idCounter++; }
        public static function resetIds():void { idCounter = 0; }
        
        public static function calcDistanceEntity( e1:Entity, e2:Entity ):Number {
            return Math.sqrt( Math.pow( (e2.x - e1.x), 2 ) + Math.pow( (e2.y - e1.y), 2 ) );
        }
        
        public static function calcDistanceCoords( x1:Number, y1:Number, x2:Number, y2:Number ):Number {
            return Math.sqrt( Math.pow( (x2 - x1), 2 ) + Math.pow( (y2 - y1), 2 ) );
        }
        
        public static function calcDirectionEntity( e1:Entity, e2:Entity ):Number {
            var atheta:Number = 0;
            var btheta:Number = Math.atan2(e2.y - e1.y, e2.x - e1.x) / Constants.DEGTORAD;
            return btheta - atheta;
        }  
        
        public static function calcDirectionCoords( x1:Number, y1:Number, x2:Number, y2:Number ):Number {
            var atheta:Number = 0;
            var btheta:Number = Math.atan2(y2 - y1, x2 - x1) / Constants.DEGTORAD;
            return btheta - atheta;
        }
        
        public static function generateRandomBetween( a:Number, b:Number ):Number {
            return Math.floor( (Math.random() * (b-a)) + a );
        }
        
        public static function removeFromArray(arr:Array, obj:Object):void {
            var len:uint = arr.length;
            for (var i:uint = 0; i < len; i++) {
                if (arr[i] == obj) {
                    arr.splice(i, 1);
                    break;
                }
            }
        }
        
    }

}