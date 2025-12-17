package com.scaleform.generator
{	
	// Imports
	import flash.display.MovieClip;
	import flash.events.Event;
	import flash.events.MouseEvent;
	import flash.events.KeyboardEvent;
	import scaleform.clik.controls.ButtonGroup;

	import fl.transitions.easing.*;
	
	import scaleform.clik.controls.Button;
	import scaleform.clik.controls.Label;
	import scaleform.clik.events.ButtonEvent;
	import scaleform.clik.motion.Tween;
	import scaleform.gfx.Extensions;

	public class GeneratorMain extends MovieClip 
	{
		// Stage Objects
		public var buttonpanel:MovieClip;
		public var turbine:MovieClip;
		public var bar:MovieClip;
		public var control_panel:MovieClip;
		
		// Member Variables
		public var label1:Label;
		public var label2:Label;
		public var label3:Label;
		public var label4:Label;
		
		private var currSelection:MovieClip;
		private var maskTween:Tween;
		
		private var _root:MovieClip;

		public function GeneratorMain() {
			Extensions.enabled = true;
			addEventListener(Event.FRAME_CONSTRUCTED, initGenerator, false, 0, true);
		}
 
		private function initGenerator(ev:Event):void {
			removeEventListener(Event.FRAME_CONSTRUCTED, initGenerator, false);
			_root = root as MovieClip;
			
			if (bar != null) {
				bar.z = -5;
				if(bar.mask != null)
					bar.mask.rotation = 0;
				else
					trace("Error: GeneratorMain::initGenerator, bar.mask does not exist!");
			}
			else {
				trace("Error: GeneratorMain::initGenerator, bar does not exist!");
			}
			
			if(control_panel != null)
				control_panel.z = -25;
			else 
				trace("Error: GeneratorMain::initGenerator, control_panel does not exist!");

			// ------------------------------------------------------------
			animateMask(null);

			if(_root != null) {
				_root.powerStateBank = new Array(true, true, true, true, true);
				if(_root.powerState == null)
					_root.powerState = new Object();
	
				if (buttonpanel != null) {
					if (buttonpanel.menubtn1 != null) {
						buttonpanel.menubtn1.z = -25;
						buttonpanel.menubtn1.focused = 1;
						currSelection = buttonpanel.menubtn1;
						_root.powerState = { label:buttonpanel.menubtn1.label, idx:0 };
						buttonpanel.menubtn1.addEventListener( ButtonEvent.CLICK, handleButtonClick, false, 0, true );
					}
					else {
						trace("Error: GeneratorMain::initGenerator, buttonpanel.menubtn1 does not exist!");
					}
					
					if (buttonpanel.menubtn2 != null) {
						buttonpanel.menubtn2.z = -25;
						buttonpanel.menubtn2.addEventListener( ButtonEvent.CLICK, handleButtonClick, false, 0, true );
					}
					else {
						trace("Error: GeneratorMain::initGenerator, buttonpanel.menubtn2 does not exist!");
					}
					
					if (buttonpanel.menubtn3 != null) {
						buttonpanel.menubtn3.z = -25;
						buttonpanel.menubtn3.addEventListener( ButtonEvent.CLICK, handleButtonClick, false, 0, true );
					}
					else {
						trace("Error: GeneratorMain::initGenerator, buttonpanel.menubtn3 does not exist!");
					}
					
					if (buttonpanel.menubtn4 != null) {
						buttonpanel.menubtn4.z = -25;
						buttonpanel.menubtn4.addEventListener( ButtonEvent.CLICK, handleButtonClick, false, 0, true );
					}
					else {
						trace("Error: GeneratorMain::initGenerator, buttonpanel.menubtn4 does not exist!");
					}
					
					if (buttonpanel.menubtn5 != null) {
						buttonpanel.menubtn5.z = -25;
						buttonpanel.menubtn5.addEventListener( ButtonEvent.CLICK, handleButtonClick, false, 0, true );
					}
					else {
						trace("Error: GeneratorMain::initGenerator, buttonpanel.menubtn5 does not exist!");
					}
				}
				else {
					trace("Error: GeneratorMain:initGenerator, buttonpanel does not exist!");
				}
			}
			else {
				trace("Error: GeneratorMain:initGenerator, _root does not exist!");
			}
			
		}

		public function handleButtonClick(e:Event):void {
			if (e.target == currSelection) { return; }
			var powerIdx:Number = parseInt(e.target.data);

			if(e.target != null) {
				var btn:Button = null;
				if (Extensions.isScaleform) {
					btn = (e.target as ButtonGroup).getButtonAt(powerIdx);
				}
				else {
					btn = (e.target as Button);
				}

				if (_root != null) {
					if (_root.powerState == null)
						_root.powerState = new Object();

					if(btn != null)
						_root.powerState = { label:btn.label, idx:powerIdx };
					else
						trace("Error: GeneratorMain:handleButtonClick, btn does not exist!");
				}
				
				currSelection = e.target as MovieClip;
			}
			else {
				trace("Error: GeneratorMain::handleButtonClick, e.target does not exist!");
			}

			if(control_panel != null)
				control_panel.gotoAndPlay("reset");
			else
				trace("Error: GeneratorMain::handleButtonClick, control_panel does not exist!");			
		}

		private function getRandomNum(min:Number, max:Number):Number {
			return (Math.random() * (max - min)) + min;
		}

		private function updatePowerText():void {
			var pct:Number = 0;
			if(bar != null && bar.mask != null) {
				var rot:Number = bar.mask.rotation;
				pct = Math.round(((135 - rot) / 135) * 100);
			}
			else {
				trace("Error: GeneratorMain::updatePowerText, bar or bar.mask does not exist!");
			}
			
			if (turbine != null) {
				if(turbine.turbine_inner_circle != null && turbine.turbine_inner_circle.circle != null) {
					var mc:MovieClip = turbine.turbine_inner_circle.circle;
					var frame:Number = mc.currentFrame;
					if (pct < 15 && frame == 1) {
						mc.gotoAndPlay("on");
						if(turbine.power_text != null)
							turbine.power_text.enabled = false;
						else
							trace("Error: GeneratorMain::updatePowerText, turbine.power_text does not exist!");
					} else if (pct >= 15 && frame != 1) {
						mc.gotoAndPlay("off");
						if(turbine.power_text != null)
							turbine.power_text.enabled = true;
						else
							trace("Error: GeneratorMain::updatePowerText, turbine.power_text does not exist!");
					}
				}
				else {
					trace("Error: GeneratorMain::updatePowerText, turbine.turbine_inner_circle or turbine.turbine_inner_circle.circle does not exist!");
				}
				turbine.power_text.text = pct + "%";
			}
			else {
				trace("Error: GeneratorMain::updatePowerText, turbine does not exist!");
			}
		}

		public function animateMask(t:Tween):void {
			if(bar != null && bar.mask != null) {
				var newAngle:Number = getRandomNum(0, 135);
				var rotDuration:Number = Math.abs(newAngle - bar.mask.rotation) * 100;
				maskTween = new Tween(rotDuration, bar.mask, {rotation:newAngle}, {ease:Strong.easeInOut, onChange:maskTweenChange, onComplete:animateMask});
			}
			else {
				trace("Error: GeneratorMain::animateMask, bar or bar.mask does not exist!");
			}
			updatePowerText();	
		}
		
		public function maskTweenChange(t:Tween):void {
			updatePowerText();
		}
		
		public function stopMaskTween():void {
			maskTween.paused = true;
		}
	}
}