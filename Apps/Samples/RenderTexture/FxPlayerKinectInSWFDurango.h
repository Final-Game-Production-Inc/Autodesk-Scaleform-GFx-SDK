/**********************************************************************

PublicHeader:   Render
Filename    :   FxPlayerKinectInSWFDurango.h
Content     :   
Created     :   2013/11/26
Authors     :   Bart Muzzin

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_FxPlayerKinectInSWFDurango_h
#define INC_FxPlayerKinectInSWFDurango_h

#include "FxPlayerTextureInSWF.h"

// NOTE: the appropriate D3D10 or D3D11 headers will be included D3D1x_Config.h. This includes
// creating macros which replace the function names with appropriate ones, depending on which one
// is being compiled (for example, the ID3D1x macros used below).
#include "Render/D3D1x/D3D1x_Config.h"

#include "Render/D3D1x/D3D1x_HAL.h"

using namespace Windows::Kinect;

class FxPlayerKinectInSWFAppDurango : public FxPlayerTextureInSWFApp
{
public:
    virtual void                     RenderMesh();
    virtual void                     InitGraphicsResources();

protected:

    Ptr<ID3D1x(Device)>              pDevice;               // The device (used for creating resources)
    Ptr<ID3D1x(DeviceContext)>       pDeviceContext;        // The device context (used for rendering, maps to 'pDevice' in D3D10).

    Ptr<ID3D1x(Texture2D)>           pCameraTexture;        // The kinect color texture.
    Ptr<ID3D1x(ShaderResourceView)>  pCameraTextureView;    // The kinect color texture view.
    Ptr<ID3D1x(Texture2D)>           pRTDepthStencilBuf;    // The depth/stencil surface used when rendering the rendered texture.
    Ptr<ID3D1x(DepthStencilView)>    pRTDepthStencil;       // The depth/stencil surface view.

    KinectSensor^                    pSensor;               // The kinect sensor.
    ColorFrameReader^                pFrameReader;          // The kinect color frame reader.
    FrameDescription^                pColorFrameDesc;       // The description of the camera color frame.
};



#endif
