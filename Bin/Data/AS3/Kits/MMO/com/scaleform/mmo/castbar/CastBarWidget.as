package com.scaleform.mmo.castbar {
    
    import flash.display.MovieClip;
    
    import com.scaleform.mmo.GameDataModel;
    
    public class CastBarWidget extends MovieClip {
        
    // Constants:
        
    // Public Properties:
        
    // Protected Properties:
        
    // UI Elements:
        public var castbar:MovieClip;
        
    // Initialization:
        public function CastBarWidget() {
            super();
            
            if (GameDataModel.registerCastBar != null) GameDataModel.registerCastBar(castbar);
        }
        
    // Public getter / setters:
        
        
    // Public Methods:
    
    // Protected Methods:
        protected function configUI():void {
            //
        }
    }
}