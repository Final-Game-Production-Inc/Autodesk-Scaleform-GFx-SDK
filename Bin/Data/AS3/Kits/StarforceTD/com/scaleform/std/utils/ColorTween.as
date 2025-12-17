/**************************************************************************

Filename    :   ColorTween.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.utils
{
    import flash.display.MovieClip;        
    import flash.events.MouseEvent;    
    import flash.geom.ColorTransform;
    
    import scaleform.clik.motion.Tween;
    import fl.transitions.easing.Strong;
    import fl.transitions.TweenEvent;
    
    
    public class ColorTween extends MovieClip    
    {
        private var startColor:ColorTransform; // default color, no tint
        private var endColor:ColorTransform;    // end change color                
        private var tweenColor:Tween;    // tweener object sets the tweening of the movieclip
        private var endTransparency:Number = 1; // sets the transparency of the color
        private var tweenMC:MovieClip;        
        private var movieClipHt:Number = 250;
        private var movieClipWt:Number = 400;
        
        private const endColorOver:String = "0x313031";    // end color while doing mouse over
        private const endColorOut:String = "0xCDFFFF";    // end color while doing mouse out
        private const tweenTime:Number = 500;    // tween duration
        private const initColorPos:Number = 0.1;    // this is the initial position of color tween
        
        public function ColorTween() {
            tweenMC = new MovieClip();
            tweenMC.graphics.beginFill(0xCDFFFF);
            tweenMC.graphics.drawRect(0, 0, movieClipWt, movieClipHt);
            tweenMC.graphics.endFill();
            this.addChild(tweenMC);
        }
        
        public function createColorTween( mc:MovieClip, duration:Number, color:Number ) {
            
        }
        
        // function executes when do mouse over or mouse out on the movieclip
        private function changeColor( evt:MouseEvent )
        {    
            // changes color on mouser over of the movieclip
            if (evt.type == MouseEvent.MOUSE_OVER)
            {    
                startColor = new ColorTransform(); // default color, no tint
                endColor = new ColorTransform();
                endColor.color = uint(endColorOver); // end color of 
                tweenColor = new Tween(evt.currentTarget, "", Strong.easeOut, 0, 0, initColorPos);            
                evt.currentTarget.transform.colorTransform = interpolateColor(startColor, endColor, initColorPos);
                
                tweenColor = new Tween(evt.currentTarget, "", Strong.easeOut, 0, endTransparency, tweenTime);            
                tweenColor.addEventListener( TweenEvent.MOTION_CHANGE, tweenTransformOver );
            }
            
            //changes color on mouse out of the movieclip
            else if (evt.type == MouseEvent.MOUSE_OUT) {
                startColor = new ColorTransform(); // default color, no tint
                endColor = new ColorTransform();
                endColor.color = uint( endColorOut ); // end color
                evt.currentTarget.transform.colorTransform = interpolateColor(startColor, endColor, 1);                
            }        
        }
        
        // does the tweening
        public function tweenTransformOver(event:TweenEvent):void {
            var result:ColorTransform = new ColorTransform();
            tweenMC.transform.colorTransform = interpolateColor(startColor, endColor, tweenColor.position);
        }
        
        // does the interpolation of color
        public function interpolateColor( start:ColorTransform, end:ColorTransform, currPos:Number ):ColorTransform {
            var currentColor:ColorTransform = new ColorTransform();
            currentColor.redMultiplier = start.redMultiplier + (end.redMultiplier - start.redMultiplier) * currPos;
            currentColor.greenMultiplier = start.greenMultiplier + (end.greenMultiplier - start.greenMultiplier) * currPos;
            currentColor.blueMultiplier = start.blueMultiplier + (end.blueMultiplier - start.blueMultiplier) * currPos;
            currentColor.alphaMultiplier = start.alphaMultiplier + (end.alphaMultiplier - start.alphaMultiplier) * currPos;
            currentColor.redOffset = start.redOffset + (end.redOffset - start.redOffset) * currPos;
            currentColor.greenOffset = start.greenOffset + (end.greenOffset - start.greenOffset) * currPos;
            currentColor.blueOffset = start.blueOffset + (end.blueOffset - start.blueOffset) * currPos;
            currentColor.alphaOffset = start.alphaOffset + (end.alphaOffset - start.alphaOffset) * currPos;
            return currentColor;
        }

    }
}