package com.scaleform.gesturedemos
{
    
    import flash.display.MovieClip;
    import flash.display.Sprite;
    import flash.display.StageScaleMode;
    import flash.events.Event;
    import flash.events.GestureEvent;
    import flash.events.GesturePhase;
    import flash.events.MouseEvent;
    import flash.events.PressAndTapGestureEvent;
    import flash.events.TransformGestureEvent;
    import flash.text.TextField;
    import flash.ui.Multitouch;
    import flash.ui.MultitouchInputMode;
    import flash.geom.Point;
    
    public class Suite extends MovieClip
    {
        public var ContainerMC: MovieClip;
        public var DemoArea:MovieClip;
        
        public var MaskArea:MovieClip;
        public var MaskAreaTwo:MovieClip;
        
        public var debugText:TextField;
        
        public var origDemoWidth:int;
        public var origDemoHeight:int;
        
        public var TabPan:MovieClip;
        public var TabZoom:MovieClip;
        public var TabRotate:MovieClip;
        public var TabSwipe:MovieClip;
        public var TabTap:MovieClip;
        public var TabAll:MovieClip;
        
        public var currentTab:MovieClip;
        public var currentDemoMC: MovieClip;
        
        public var transitionDemoMC: MovieClip;
        public var transitionTime: Number = 0;
        public var doingTransition: Boolean = false;
        public var transitionX: Number = 0;
        public var transitionY: Number = 0;
        
        public function Suite()
        {
            addEventListener(Event.ENTER_FRAME, configUI);
        }
        
        //
        
        public function configUI(e:Event):void
        {
            removeEventListener(Event.ENTER_FRAME, configUI);
            addEventListener(Event.ENTER_FRAME, update);
            
            stage.scaleMode = StageScaleMode.EXACT_FIT;
            ContainerMC.x = 0;
            ContainerMC.y = 0;
            
            origDemoWidth = ContainerMC.width;
            origDemoHeight = ContainerMC.height;
            
            Multitouch.inputMode = MultitouchInputMode.GESTURE;
            
            
            ContainerMC.x = (1280 - stage.stageWidth)*.5;
            
            ContainerMC.Panel.TabPan.addEventListener(MouseEvent.MOUSE_UP, clickTab);
            ContainerMC.Panel.TabZoom.addEventListener(MouseEvent.MOUSE_UP, clickTab);
            ContainerMC.Panel.TabRotate.addEventListener(MouseEvent.MOUSE_UP, clickTab);
            ContainerMC.Panel.TabSwipe.addEventListener(MouseEvent.MOUSE_UP, clickTab);
            ContainerMC.Panel.TabTap.addEventListener(MouseEvent.MOUSE_UP, clickTab);
            ContainerMC.Panel.TabAll.addEventListener(MouseEvent.MOUSE_UP, clickTab);
            
            ContainerMC.Image2.visible = false;
            currentDemoMC = ContainerMC.Image1;
            
            debugText = ContainerMC.Panel.helpText;
            
            debugText.text = "Welcome to the gesture demos suite.";
            debugText.multiline = true;
            debugText.width = ContainerMC.width;
            
            currentTab = ContainerMC.Panel.TabPan;
            createCurrentDemo();
        }
        
        public var lastTime: Number = 0;
        
        public function update(e:Event):void
        {
            
            
            if (doingTransition)
            {
                var newTime: Number = new Date().time;
                var deltaTime: Number = (newTime - lastTime)/1000;
                
                transitionTime += deltaTime*4;
                
                
                if (transitionTime >= 1)
                    transitionTime = 1;
                
                transitionDemoMC.x = transitionFromX + (transitionToX - transitionFromX) * transitionTime;
                transitionDemoMC.y = transitionFromY + (transitionToY - transitionFromY) * transitionTime;
                currentDemoMC.x = transitionToX + (transitionToX - transitionFromX) * transitionTime;
                currentDemoMC.y = transitionToY + (transitionToY - transitionFromY) * transitionTime;
                
                currentDemoMC.visible = true;
                transitionDemoMC.visible = true;
                
                
                if (transitionTime >= 1)
                {
                    currentDemoMC.visible = false;
                    currentDemoMC = transitionDemoMC;
                    transitionTime = 0;
                    doingTransition = false;
                }
                
                lastTime = newTime;
            }
            
        }
        
        public function clickTab(me:MouseEvent):void
        {
            if (currentTab == me.target)
                return;
            
            destroyCurrentDemo();
            currentTab = me.target as MovieClip;
            createCurrentDemo();
        }
        
        //
        
        public function createCurrentDemo():void
        {
            switch (currentTab)
            {
                case ContainerMC.Panel.TabPan: 
                    createPan();
                    break;
                case ContainerMC.Panel.TabZoom: 
                    createZoom();
                    break;
                case ContainerMC.Panel.TabRotate: 
                    createRotate();
                    break;
                case ContainerMC.Panel.TabSwipe: 
                    createSwipe();
                    break;
                case ContainerMC.Panel.TabTap: 
                    createTap();
                    break;
                case ContainerMC.Panel.TabAll: 
                    createAll();
                    break;
                default: 
                    return;
            }
        }
        
        public function destroyCurrentDemo():void
        {
            switch (currentTab)
            {
                case ContainerMC.Panel.TabPan: 
                    destroyPan();
                    break;
                case ContainerMC.Panel.TabZoom: 
                    destroyZoom();
                    break;
                case ContainerMC.Panel.TabRotate: 
                    destroyRotate();
                    break;
                case ContainerMC.Panel.TabSwipe: 
                    destroySwipe();
                    break;
                case ContainerMC.Panel.TabTap: 
                    destroyTap();
                    break;
                case ContainerMC.Panel.TabAll: 
                    destroyAll();
                    break;
                default: 
                    return;
            }
        }
        
        //////////////////
        
        public var panTexture:MovieClip;
        
        public function createPan():void
        {
            debugText.text = "Drag two fingers across the screen.";
            stage.addEventListener(TransformGestureEvent.GESTURE_PAN, onPan);
            
            panTexture = currentDemoMC;
        }
        
        private function onPan(evt:TransformGestureEvent):void
        {
            panTexture = currentDemoMC;
            if (evt.phase == GesturePhase.BEGIN)
            {
                //debugText.text = "Pan begin";
            }
            
            if (evt.phase == GesturePhase.UPDATE)
            {
                //debugText.text = "Pan update";
            }
            
            if (evt.phase == GesturePhase.END)
            {
                //debugText.text = "Pan end";
            }
            
            //debugText.text = evt.phase;
            
            debugText.text = "Pan successful.";
            
            panTexture.x += evt.offsetX;
            panTexture.y += evt.offsetY;
            
        }
        
        public function destroyPan():void
        {
            stage.removeEventListener(TransformGestureEvent.GESTURE_PAN, onPan);
        }
        
        //////////////////
        
        public var zoomTexture:MovieClip;
        public var zoomTextureTwo:MovieClip;
        
        public function createZoom():void
        {            
            debugText.text = "Pinch two fingers on the screen.";
            stage.addEventListener(TransformGestureEvent.GESTURE_ZOOM, onZoom);
            
            zoomTexture = currentDemoMC;
            
        }
        
        private function onZoom(e:TransformGestureEvent):void
        {
            zoomTexture = currentDemoMC;
            if (e.phase == GesturePhase.BEGIN)
            {
                //debugText.text = "Zoom begin";
            }
            
            if (e.phase == GesturePhase.UPDATE)
            {
                //debugText.text = "Pan update";
            }
            
            if (e.phase == GesturePhase.END)
            {
                //debugText.text = "Zoom end";
            }
            
            debugText.text = "Zoom successful.";
            
            var scaleMag: Number = e.scaleX;
            
            zoomTexture.scaleX *= scaleMag;
            zoomTexture.scaleY *= scaleMag;
            
        }
        
        public function destroyZoom():void
        {
            stage.removeEventListener(TransformGestureEvent.GESTURE_ZOOM, onZoom);
            
        }
        
        //////////////////
        
        public var rotateSpiral:MovieClip;
        
        public function createRotate():void
        {
            rotateSpiral = currentDemoMC;
            //rotateSpiral.x = origDemoWidth * .5;
            //rotateSpiral.y = origDemoHeight * .5;
            
            debugText.text = "Rotate two fingers on the screen.";
            stage.addEventListener(TransformGestureEvent.GESTURE_ROTATE, rothandler);
        }
        
        private function rothandler(evt:TransformGestureEvent):void
        {
            rotateSpiral = currentDemoMC;
            if (evt.phase == GesturePhase.BEGIN)
            {
                //debugText.text = "Rotating";
            }
            
            if (evt.phase == GesturePhase.UPDATE)
            {
                //debugText.text = "Pan update";
            }
            
            if (evt.phase == GesturePhase.END)
            {
                //debugText.text = "Rotate end";
            }
            //debugText.text = evt.phase;
            
            debugText.text = "Rotation successful.";
           // debugText.text = "Rotate " + evt.rotation;
            rotateSpiral.rotation += evt.rotation;
        }
        
        public function destroyRotate():void
        {
            stage.removeEventListener(TransformGestureEvent.GESTURE_ROTATE, rothandler);
        }
        
        //////////////////
        
        public var arrowCircles:Vector.<MovieClip>;
        public var arrowVelocities:Vector.<Point>;
        public var arrowDirections:Vector.<int>;
        
        public function createSwipe():void
        {      
            stage.addEventListener(TransformGestureEvent.GESTURE_SWIPE, swipehandler);
            
            debugText.text = "Use one finger to swipe across the screen.";
        }
        
        function DistanceTwoPoints(x1:Number, x2:Number, y1:Number, y2:Number):Number
        {
            var dx:Number = x1 - x2;
            var dy:Number = y1 - y2;
            return Math.sqrt(dx * dx + dy * dy);
        }
        
        public var transitionToX: Number = 0;
        public var transitionToY: Number = 0;
        public var transitionFromX: Number = 0;
        public var transitionFromY: Number = 0;
        
        private function swipehandler(evt:TransformGestureEvent):void
        {
            if (doingTransition)
                return;
                
            debugText.text = "Swipe successful.";
            //debugText.text = "Swiped";
            
            lastTime = new Date().time;
            doingTransition = true;
            transitionX = -evt.offsetX;
            transitionY = -evt.offsetY;
            
            transitionTime = 0;
            
            if (currentDemoMC == ContainerMC.Image2)
            {
                transitionDemoMC = ContainerMC.Image1;
            }
            else
            {
                transitionDemoMC = ContainerMC.Image2;
            }
            
            transitionDemoMC.scaleX = currentDemoMC.scaleX;
            transitionDemoMC.scaleY = currentDemoMC.scaleY;
            transitionDemoMC.rotation = currentDemoMC.rotation;
            
            transitionToX = currentDemoMC.x;
            transitionToY = currentDemoMC.y;
            transitionDemoMC.x = currentDemoMC.x + stage.stageWidth*transitionX*2;
            transitionDemoMC.y = currentDemoMC.y + stage.stageHeight*transitionY*2;
            transitionFromX = transitionDemoMC.x;
            transitionFromY = transitionDemoMC.y;
            
            
        }
        
        
        public function destroySwipe():void
        {
           
            stage.removeEventListener(TransformGestureEvent.GESTURE_SWIPE, swipehandler);
            
        }
        
        //////////////////
        
        private var radialMenu:MovieClip;
        private var menuOpened:Boolean = false;
        private var menuMouseDown: Boolean = false;
        private var menuOpenedTime:Number;
        private var menuX:Number;
        private var menuY:Number;
        
        public function createTap():void
        {
            //radialMenu = new RadialMenu();
            //this.stage.addChild(radialMenu);
            //radialMenu.gotoAndPlay("off");
                        
            debugText.text = "Tap the screen with 2 fingers.";
            //stage.addEventListener(MouseEvent.MOUSE_DOWN, openRadialMenu);
            //stage.addEventListener(MouseEvent.MOUSE_UP, closeRadialMenu);
            //stage.addEventListener(PressAndTapGestureEvent.GESTURE_PRESS_AND_TAP, taphandler);
            stage.addEventListener(GestureEvent.GESTURE_TWO_FINGER_TAP, twotaphandler);
        }
        
        private function openRadialMenu(e:MouseEvent):void
        {
            menuMouseDown = true;
            this.stage.addEventListener(Event.ENTER_FRAME, updateRadial);
            menuOpenedTime = new Date().time;
            menuX = e.stageX;
            menuY = e.stageY;
        }
        
        private function updateRadial(e:Event):void
        {
            if (radialMenu == null || !menuMouseDown)
            {
                this.stage.removeEventListener(Event.ENTER_FRAME, updateRadial);
                return;
            }
            
            trace("diff " + (new Date().time - menuOpenedTime));
            
            if (new Date().time - menuOpenedTime > 350)
            {
                this.stage.removeEventListener(Event.ENTER_FRAME, updateRadial);
                menuOpened = true;
                radialMenu.gotoAndPlay("open");
                radialMenu.x = menuX;
                radialMenu.y = menuY;
                trace("opened radial menu");
            }
        }
        
        private function closeRadialMenu(e:MouseEvent):void
        {
            menuMouseDown = false;
            if (menuOpened)
            {
                radialMenu.gotoAndPlay("close");
                menuOpened = false;
                trace("closed radial menu");
            }
        }
        
        private function taphandler(evt:PressAndTapGestureEvent):void
        {
            //debugText.text = "Press " + evt.stageX + " " + evt.stageY + " and tap " + evt.tapStageX + " " + evt.tapStageY;
        }
        
        private function twotaphandler(evt:GestureEvent):void
        {
            debugText.text = "Two-finger tap successful.";
            //debugText.text = "2 finger tap " + evt.stageX + " " + evt.stageY;
            var twoTapMC:MovieClip = new TwoTapRipple();
            twoTapMC.x = evt.stageX;
            twoTapMC.y = evt.stageY;
            this.stage.addChild(twoTapMC);
        }
        
        public function destroyTap():void
        {
            menuOpened = false;
            //this.stage.removeChild(radialMenu);
            //radialMenu = null;
            //stage.removeEventListener(MouseEvent.MOUSE_DOWN, openRadialMenu);
            //stage.removeEventListener(MouseEvent.MOUSE_UP, closeRadialMenu);
            //stage.removeEventListener(PressAndTapGestureEvent.GESTURE_PRESS_AND_TAP, taphandler);
            stage.removeEventListener(GestureEvent.GESTURE_TWO_FINGER_TAP, twotaphandler);
        
        }
		
		var allVector: MovieClip;
		
		public function createAll():void
        {
            debugText.text = "Use any of the above gestures simultaneously.";
            
            allVector = currentDemoMC;
            //allVector.x = origDemoWidth * .5;
            //allVector.y = origDemoHeight * .5;
			
            stage.addEventListener(GestureEvent.GESTURE_TWO_FINGER_TAP, twotaphandler);
            stage.addEventListener(TransformGestureEvent.GESTURE_SWIPE, swipehandler);
            stage.addEventListener(TransformGestureEvent.GESTURE_PAN, onPanAll);
            stage.addEventListener(TransformGestureEvent.GESTURE_ZOOM, onZoomAll);
            stage.addEventListener(TransformGestureEvent.GESTURE_ROTATE, onRotateAll);
        }
		
		
        private function onPanAll(evt:TransformGestureEvent):void
        {
            allVector = currentDemoMC;
            allVector.x += evt.offsetX;
            allVector.y += evt.offsetY;
            
        }
        private function onZoomAll(e:TransformGestureEvent):void
        {
			var scaleMag: Number = e.scaleX;
           	allVector.scaleX *= scaleMag;
            allVector.scaleY *= scaleMag;
            
        }
        private function onRotateAll(evt:TransformGestureEvent):void
        {
            allVector = currentDemoMC;
            allVector.rotation += evt.rotation;
        }
		
		public function destroyAll():void
        {
            stage.removeEventListener(GestureEvent.GESTURE_TWO_FINGER_TAP, twotaphandler);
            stage.removeEventListener(TransformGestureEvent.GESTURE_SWIPE, swipehandler);
            stage.removeEventListener(TransformGestureEvent.GESTURE_PAN, onPanAll);
            stage.removeEventListener(TransformGestureEvent.GESTURE_ZOOM, onZoomAll);
            stage.removeEventListener(TransformGestureEvent.GESTURE_ROTATE, onRotateAll);
            //DemoArea.removeChild(allVector);
			//allVector = null;
        }
    }

}
