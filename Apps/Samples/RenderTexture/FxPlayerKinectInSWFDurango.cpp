/**************************************************************************

Filename    :   FxPlayerKinectInSWFDurango.cpp
Content     :   Sample showing replacement of a .SWF texture with the Kinect color output.
Created     :   21/11/2013
Authors     :   Bart Muzzin

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

Portions of this sample are taken/adapted from the 'SimpleNuiViewer' sample provided
by Microsoft on the Durango development portal.

**************************************************************************/

#include "FxPlayerKinectInSWFDurango.h"
#include <wrl.h>
#include <robuffer.h>

using namespace Windows::Kinect;
using namespace Windows::Storage::Streams;
using namespace Microsoft::WRL;

//--------------------------------------------------------------------------------------
// Name: GetPointerToBufferData()
// Desc: Returns the pointer to the IBuffer data.
// NOTE: The pointer is only valid for the lifetime of the governing IBuffer^
//--------------------------------------------------------------------------------------
void* GetPointerToBufferData( IBuffer^ buffer )
{
    // Obtain IBufferByteAccess from IBuffer
    ComPtr<IUnknown> pBuffer( (IUnknown*)buffer );
    ComPtr<IBufferByteAccess> pBufferByteAccess;
    pBuffer.As( &pBufferByteAccess );

    // Get pointer to data
    byte* pData = nullptr;
    if ( FAILED( pBufferByteAccess->Buffer( &pData ) ) )
    {
        // Buffer is not annotated with _COM_Outpr, so if it were to fail, then the value of pData is undefined
        pData = nullptr;
    }
    return pData;
}

void FxPlayerKinectInSWFAppDurango::RenderMesh()
{
    if (!pFrameReader)
        return;

    ColorFrame^ frame = pFrameReader->AcquireLatestFrame();
    if (frame == nullptr)
        return;

    // From VisualizeKinect.cpp in ATG framework.
    const size_t size = pColorFrameDesc->Width * pColorFrameDesc->Height * pColorFrameDesc->BytesPerPixel;
    IBuffer^ buffer = frame->LockRawImageBuffer();
    void* pcolorData = GetPointerToBufferData( buffer );

    D3D11_MAPPED_SUBRESOURCE streamData;
    pDeviceContext->Map( pCameraTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &streamData );
    memcpy( streamData.pData, pcolorData, size );
    pDeviceContext->Unmap( pCameraTexture, 0 );
}

// Initializes the graphics resources used by the sample (this does not include the ones used by GFx).
// This is essentially just setting up the Kinect sensor to read a ColorSource, and creating the texture
// that it will update.
void FxPlayerKinectInSWFAppDurango::InitGraphicsResources()
{
    // Store the device pointer (for ease of access).
    SF_DEBUG_ASSERT(pPlatformHAL, "Unexpected NULL pPlatformHAL in InitGraphicsResources.");
    D3D1x::HAL* phal = reinterpret_cast<D3D1x::HAL*>(pPlatformHAL);
    pDevice = phal->GetDevice();
    pDeviceContext = phal->pDeviceContext;

    // Initialize the Kinect sensor.
    pSensor = KinectSensor::GetDefault();
    SF_DEBUG_ASSERT(pSensor, "Unexpected NULL KinectedSensor::GetDefault().");
    if (!pSensor)
        return;
    pSensor->Open();

    pColorFrameDesc = pSensor->ColorFrameSource->CreateFrameDescription(Windows::Kinect::ColorImageFormat::Yuy2);
    SF_DEBUG_ASSERT(pColorFrameDesc, "Unexpected NULL return from KinectSensor::ColorFrameSource->CreateFrameDescription().");
    if (!pColorFrameDesc)
        return;

    pFrameReader = pSensor->ColorFrameSource->OpenReader();
    SF_DEBUG_ASSERT(pFrameReader, "Unexpected NULL return from KinectSensor::ColorFrameSource->OpenReader().");
    if (!pFrameReader)
        return;

    // Create texture and texture view for color stream. The color stream has a YUV4:2:2 data which can
    // be presented as either YUY2 or G8R8_G8B8
    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory( &textureDesc, sizeof( textureDesc ) );
    textureDesc.Width           = pColorFrameDesc->Width;
    textureDesc.Height          = pColorFrameDesc->Height;
    textureDesc.MipLevels       = 1;
    textureDesc.ArraySize       = 1;
    textureDesc.Format          = DXGI_FORMAT_G8R8_G8B8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage           = D3D11_USAGE_DYNAMIC;
    textureDesc.BindFlags       = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags  = D3D11_CPU_ACCESS_WRITE;
    textureDesc.MiscFlags       = 0;
    pDevice->CreateTexture2D( &textureDesc, NULL, &pCameraTexture.GetRawRef() );
    pDevice->CreateShaderResourceView( pCameraTexture, NULL, &pCameraTextureView.GetRawRef() );

    // Override the RTWidth/RTHeight from the base class, as they were assuming a 256x256 RT, but we are
    // using the one the size that the Kinect reports (1920x1080).
    RTWidth = pColorFrameDesc->Width;
    RTHeight = pColorFrameDesc->Height;

    // Now create the GFx texture, that will actually be applied, from our FBO.
    D3D1x::TextureManager * pmanager = (D3D1x::TextureManager*)pPlatformHAL->GetTextureManager(); 
    pMyHWTexture = *pmanager->CreateTexture( pCameraTexture, ImageSize(RTWidth, RTHeight));
}

//------------------------------------------------------------------------
// ***** Main APP implementation (This macro handles the main function, on all platforms).

SF_PLATFORM_SYSTEM_APP(KinectInSWF, Scaleform::GFx::System, FxPlayerKinectInSWFAppDurango)
