package com.scaleform.generator
{	
	// Imports
	import flash.display.MovieClip;
	import flash.events.Event;
	import flash.events.KeyboardEvent;
	import flash.events.MouseEvent;
	import flash.text.TextField;

	public class GeneratorWindow extends MovieClip 
	{
		// Stage Objects
		public var warning:MovieClip;
		public var btn_on:MovieClip;
		public var btn_off:MovieClip;
		
		public var title_text:MovieClip;
		public var powerLevel:TextField;
		
		// Member Variables
		private var _root:MovieClip;
		
		public function GeneratorWindow() {
			addEventListener(Event.FRAME_CONSTRUCTED, initGeneratorWindow, false, 0, true);
		}

		public function initGeneratorWindow(ev:Event):void {
			removeEventListener(Event.FRAME_CONSTRUCTED, initGeneratorWindow, false);
			_root = root as MovieClip;
			
			if(warning != null)
				warning.z = -15;
			else
				trace("Error: GeneratorWindow::initGeneratorWindow, warning does not exist!");
				
			if (btn_on != null) {
				btn_on.z = -15;
				btn_on.addEventListener(MouseEvent.CLICK, enablePowerState, false, 0, true);
			}
			else {
				trace("Error: GeneratorWindow::initGeneratorWindow, btn_on does not exist!");
			}
			
			if(btn_off != null) {
				btn_off.z = -15;
				btn_off.addEventListener(MouseEvent.CLICK, disablePowerState, false, 0, true);
			}
			else {
				trace("Error: GeneratorWindow::initGeneratorWindow, btn_off does not exist!");
			}
				
			if(title_text != null)
				title_text.text = _root.powerState.label;
			else
				trace("Error: GeneratorWindow::initGeneratorWindow, title_text does not exist!");
				
			if(_root != null && btn_on != null && btn_off != null) {
				if (_root.powerState != null && _root.powerStateBank[_root.powerState.idx]) {
					btn_on.selected = true;
					btn_off.selected = false
				} else {
					btn_on.selected = false;
					btn_off.selected = true;
				}
			}
			
			if(powerLevel != null) 
				powerLevel.text = (Math.round(Math.random() * 990) / 10) + "K";
		}

		public function enablePowerState(e:Event):void {
			if(_root != null)
				_root.setPowerState(true);
		}

		public function disablePowerState(e:Event):void {
			if(_root != null)
				_root.setPowerState(false);
		}
	}
}