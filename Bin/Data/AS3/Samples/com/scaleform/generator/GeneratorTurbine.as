package com.scaleform.generator
{	
	// Imports
	import flash.display.MovieClip;
	import fl.transitions.easing.*; 		
	import flash.events.Event;
	import scaleform.clik.motion.Tween;
	
	public class GeneratorTurbine extends MovieClip 
	{
		// Stage Objects
		public var turbine_outline_circle:MovieClip;
		public var turbine_outline_dot:MovieClip;
		public var turbine_ticks:MovieClip;
		public var turbine_inner_ticks:MovieClip;
		public var turbine_circle:MovieClip;
		public var turbine_inner_circle:MovieClip;
		public var power_text:MovieClip;
		
		// Member Variables
		private var turbineTween1:Tween;
		private var turbineTween2:Tween;
		
		private var turbineInnerTween1:Tween;
		private var turbineInnerTween2:Tween;
		
		private var turbineCircleTween1:Tween;
		private var turbineCircleTween2:Tween;
		
		private var turbineCircleInnerTween1:Tween;
		private var turbineCircleInnerTween2:Tween;
		
		//---------------------------------------------------
		// Constructor and initialization
		public function GeneratorTurbine() {
			addEventListener(Event.FRAME_CONSTRUCTED, initGeneratorTurbine, false, 0, true);
		}

		private function initGeneratorTurbine(ev:Event):void {
			removeEventListener(Event.FRAME_CONSTRUCTED, initGeneratorTurbine, false);
			
			if(turbine_outline_circle != null)
				turbine_outline_circle.z = -12;
			else
				trace("Error: GeneratorTurbine::initGeneratorTurbine, turbine_outline_circle does not exist!");
				
			if(turbine_outline_dot != null)
				turbine_outline_dot.z = -25;
			else
				trace("Error: GeneratorTurbine::initGeneratorTurbine, turbine_outline_dot does not exist!");
				
			// Start animations
			animateTurbineTicks();
			animateTurbineCircle();
			animateTurbineInnerTicks();
			animateTurbineInnerCircle();
		}

		// ------------------------------------------------------
		// Public Functions accessed by other objects in fla
		public function animateTurbineInnerCircle():void {
			animateTurbineInnerCircle1(null);
		}
		
		public function animateTurbineInnerTicks():void {
			animateTurbineInnerTicks1(null);
		}
		
		public function stopTurbineInnerCircleTween():void {
			if(turbineCircleInnerTween1 != null)
				turbineCircleInnerTween1.paused = true;
			else
				trace("Error: GeneratorTurbine::stopTurbineInnerCircleTween, turbineCircleInnerTween1 does not exist!");
				
			if(turbineCircleInnerTween2 != null)
				turbineCircleInnerTween2.paused = true;
			else
				trace("Error: GeneratorTurbine::stopTurbineInnerCircleTween, turbineCircleInnerTween2 does not exist!");	
		}
		
		public function stopTurbineTicksTween():void {
			if(turbineTween1 != null)
				turbineTween1.paused = true;
			else
				trace("Error: GeneratorTurbine::stopTurbineTicksTween, turbineTween1 does not exist!");
				
			if(turbineTween2 != null)
				turbineTween2.paused = true;
			else
				trace("Error: GeneratorTurbine::stopTurbineTicksTween, turbineTween2 does not exist!");	
		}

		public function stopTurbineInnerTicksTween():void {
			if(turbineInnerTween1 != null)
				turbineInnerTween1.paused = true;
			else
				trace("Error: GeneratorTurbine::stopTurbineInnerTicksTween, turbineInnerTween1 does not exist!");
				
			if(turbineInnerTween2 != null)
				turbineInnerTween2.paused = true;
			else
				trace("Error: GeneratorTurbine::stopTurbineInnerTicksTween, turbineInnerTween2 does not exist!");	
		}
		
		public function stopTurbineCircleTween():void {
			if(turbineCircleTween1 != null)
				turbineCircleTween1.paused = true;
			else
				trace("Error: GeneratorTurbine::stopTurbineCircleTween, turbineCircleTween1 does not exist!");
				
			if(turbineCircleTween2 != null)
				turbineCircleTween2.paused = true;
			else
				trace("Error: GeneratorTurbine::stopTurbineCircleTween, turbineCircleTween2 does not exist!");	
		}
		
		// ------------------------------------------------------------------
		// Private functions only referenced within this object
		private function getRandomNum(min:Number, max:Number):Number {
			return (Math.random() * (max - min)) + min;
		}

		private function animateTurbineTicks():void {
			animateTurbineTicks1(null);
		}

		private function animateTurbineTicks1(tween:Tween):void {
			if(turbine_ticks != null) {
				var mc:MovieClip = turbine_ticks;
				turbineTween1 = new Tween(3000, mc, 
											 {z: getRandomNum(0, 50)}, 
											 {ease:Back.easeInOut, onComplete:animateTurbineTicks2 } );
			}
			else {
				trace("Error: GeneratorTurbine::animateTurbineTicks1, turbine_ticks does not exist!");
			}
		}
		
		private function animateTurbineTicks2(tween:Tween):void {
			if(turbine_ticks != null && turbine_ticks.circle != null) {
				var angleDelta:Number=getRandomNum(-180,180);
				var rotDuration:Number=Math.abs(angleDelta)*0.05;
				var mc:MovieClip=turbine_ticks.circle;
				turbineTween2 = new Tween(rotDuration*1000, mc, 
											 {rotation: mc.rotation+angleDelta}, 
											 {ease:Strong.easeInOut, onComplete:animateTurbineTicks1 } );
			}
			else {
				trace("Error: GeneratorTurbine::animateTurbineTicks2, turbine_ticks or turbine_ticks.circle does not exist!");
			}
		}

		private function animateTurbineInnerTicks1(tween:Tween):void {
			if(turbine_inner_ticks != null) {
				var mc:MovieClip=turbine_inner_ticks;
				turbineInnerTween1 = new Tween(3000, mc, 
											 {z: getRandomNum(-50, 0)}, 
											 {ease:Back.easeInOut, onComplete:animateTurbineInnerTicks2 } );
			}
			else {
				trace("Error: GeneratorTurbine::animateTurbineInnerTicks1, turbine_inner_ticks does not exist!");
			}
		}
		
		private function animateTurbineInnerTicks2(tween:Tween):void {
			if(turbine_inner_ticks != null && turbine_inner_ticks.circle != null) {
				var angleDelta:Number=getRandomNum(-180,180);
				var rotDuration:Number=Math.abs(angleDelta)*0.1;
				var mc:MovieClip=turbine_inner_ticks.circle;
				turbineInnerTween2 = new Tween(rotDuration*1000, mc, 
											 {rotation: mc.rotation+angleDelta}, 
											 {ease:Strong.easeInOut, onComplete:animateTurbineInnerTicks1});		
			}
			else {
				trace("Error: GeneratorTurbine::animateTurbineInnerTicks2, turbine_inner_ticks or turbine_inner_ticks.circle does not exist!");
			}
		}

		private function animateTurbineCircle():void {
			animateTurbineCircle1(null);
		}
		
		private function animateTurbineCircle1(tween:Tween):void {
			if(turbine_circle != null) {
				var mc:MovieClip=turbine_circle;
				turbineCircleTween1 = new Tween(3000, mc, 
											 {z: getRandomNum(-25, 25)}, 
											 {ease:Back.easeInOut, onComplete:animateTurbineCircle2 } );
			}
			else {
				trace("Error: GeneratorTurbine::animateTurbineCircle1, turbine_circle does not exist!");
			}
		}
		
		private function animateTurbineCircle2(tween:Tween):void {
			if(turbine_circle != null && turbine_circle.circle != null) {
				var angleDelta:Number=getRandomNum(-180,180);
				var rotDuration:Number=Math.abs(angleDelta)*0.025;
				var mc:MovieClip=turbine_circle.circle;
				turbineCircleTween2 = new Tween(rotDuration*1000, mc, 
											 {rotation: mc.rotation+angleDelta}, 
											 {ease:Strong.easeInOut, onComplete:animateTurbineCircle1 } );
			}
			else {
				trace("Error: GeneratorTurbine::animateTurbineCircle2, turbine_circle or turbine_circle.circle does not exist!");
			}
		}
		
		private function animateTurbineInnerCircle1(tween:Tween):void {
			if(turbine_inner_circle != null) {
			var mc:MovieClip=turbine_inner_circle;
			turbineCircleInnerTween1 = new Tween(3000, mc, 
										 {z: getRandomNum(25, 75)}, 
										 {ease:Back.easeInOut, onComplete:animateTurbineInnerCircle2});	
			}
			else {
				trace("Error: GeneratorTurbine::animateTurbineInnerCircle1, turbine_inner_circle does not exist!");
			}
		}
		
		private function animateTurbineInnerCircle2(tween:Tween):void {
			if(turbine_inner_circle != null && turbine_inner_circle.circle != null) {
			var angleDelta:Number=getRandomNum(-180,180);
			var rotDuration:Number=Math.abs(angleDelta)*0.012;
			var mc:MovieClip=turbine_inner_circle.circle;
			turbineCircleInnerTween2 = new Tween(rotDuration*1000, mc, 
										 {rotation: mc.rotation+angleDelta}, 
										 {ease:Strong.easeInOut, onComplete:animateTurbineInnerCircle1});		
			}
			else {
				trace("Error: GeneratorTurbine::animateTurbineInnerCircle2, turbine_inner_circle or turbine_inner_circle.circle does not exist!");
			}
		}
	}
}