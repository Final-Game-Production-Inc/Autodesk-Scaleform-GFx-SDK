/**************************************************************************

Filename    :   LoaderWithParams.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
 
package com.scaleform.system {
    
    import flash.display.Loader;

    public class LoaderWithParams extends Loader {
        public var loaderParams:Object;
        public var tweenData:Object;

        public function LoaderWithParams() {
            super();
            loaderParams = {};
            tweenData = {};
        }
    }
}