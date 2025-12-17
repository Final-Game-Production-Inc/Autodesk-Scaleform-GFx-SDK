/**************************************************************************

Filename    :   D3D1x_Config.h
Content     :   Facilitates switching interfaces between DX10 and DX11.
Created     :   Mar 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_D3D1X_CONFIG_H
#define INC_SF_D3D1X_CONFIG_H
#pragma once

#if !defined(SF_D3D_VERSION)
#error SF_D3D_VERSION must be defined, and must be 10 or 11.
#endif

#if defined(SF_DURANGO_MONOLITHIC)
    // In Durango July 2013 XDK, the monolithic runtime was added. It conflicts with any 'stock' D3D include files.
    #include <d3d11_x.h>
#elif defined(SF_OS_WINMETRO) || (_MSC_VER >= 1700)   
    // In VS2011+ d3d11_1.h should exist. Include d3d11_1.h unconditionally (even in D3D10), because
    // d3d10.h does not include the common ID3DUserAnnotation interfaces.
    #include <d3d11_1.h>
#endif

#if !defined(SF_DURANGO_MONOLITHIC)
    #include <d3dcommon.h>
#endif

#if (SF_D3D_VERSION == 10 )
    #include <d3d10_1.h>
    #include <d3d10.h>

    #define D3D10(...)      __VA_ARGS__
    #define D3D11(...)      
    #define D3D10_11(x,y)   x

    #define D3D1x(x)      D3D10_##x
    #define ID3D1x(x)     ID3D10##x
    #define IID_ID3D1x(x) IID_ID3D10##x

    #define D3D1xMapBuffer( pDeviceContext, Resource, SubResource, MapType, MapFlag, MappedBuffer) \
        Resource->Map(MapType, MapFlag, (void**)MappedBuffer)
    #define D3D1xUnmapBuffer( pDeviceContext, Resource, SubResource) \
        Resource->Unmap()
    #define D3D1xMapTexture2D( pDeviceContext, Resource, SubResource, MapType, MapFlag, MappedBuffer) \
        Resource->Map(SubResource, MapType, MapFlag, MappedBuffer)
    #define D3D1xUnmapTexture2D( pDeviceContext, Resource, SubResource) \
        Resource->Unmap(SubResource)
    #define D3D1xCreateVertexShader( pDevice, ByteCode, Length, ShaderPtr) \
        pDevice->CreateVertexShader( ByteCode, Length, ShaderPtr)
    #define D3D1xCreatePixelShader( pDevice, ByteCode, Length, ShaderPtr) \
        pDevice->CreatePixelShader( ByteCode, Length, ShaderPtr)
    #define D3D1xVSSetShader( pDeviceContext, Shader ) \
        pDeviceContext->VSSetShader( Shader );
    #define D3D1xPSSetShader( pDeviceContext, Shader ) \
        pDeviceContext->PSSetShader( Shader );
    #define D3D1xCSSetShader( pDeviceContext, Shader) // No Compute Shader D3D10.
    #define D3D1xDSSetShader( pDeviceContext, Shader) // No Domain Shader D3D10.
    #define D3D1xGSSetShader( pDeviceContext, Shader) \
        pDeviceContext->GSSetShader(Shader)
    #define D3D1xHSSetShader( pDeviceContext, Shader) // No Hull Shader D3D10
    #define D3D1xEndAsynchronous( pDeviceContext, Query ) \
        Query->End();
    #define D3D1xGetDataAsynchronous( pDeviceContext, Query, Data, DataSize, Flags ) \
        Query->GetData(Data, DataSize, Flags)

    typedef ID3D10Device1 ID3D10DeviceContext;                      // No ID3D10DeviceContext, map the methods directly to the device.
    #define ID3D10Device  ID3D10Device1                             // Always use 10.1 device type (may be created with 10.0 feature level).
    #define IID_ID3D10ShaderReflection IID_ID3D10ShaderReflection1  // Always use 10.1 shader reflection class.
    struct D3D10_MAPPED_BUFFER 
    { 
        void* pData; 
        operator void* () { return pData;}
    };

#elif (SF_D3D_VERSION == 11 )
    #if !defined(__ID3DUserDefinedAnnotation_FWD_DEFINED__) && !defined(SF_DURANGO_MONOLITHIC)
        #include <d3d11.h>
    #endif

    #if defined(SF_DURANGO_MONOLITHIC)
//        typedef ID3D11DeviceX           ID3D11Device;
//        typedef ID3D11DeviceContextX    ID3D11DeviceContext;
    #endif

    #define D3D10(...)    
    #define D3D11(...)        __VA_ARGS__
    #define D3D10_11(x,y)   y

    #define D3D1x(x)      D3D11_##x
    #define ID3D1x(x)     ID3D11##x
    #define IID_ID3D1x(x) IID_ID3D11##x

    #define D3D1xMapBuffer( pDeviceContext, Resource, SubResource, MapType, MapFlag, MappedBuffer) \
        pDeviceContext->Map(Resource, SubResource, MapType, MapFlag, MappedBuffer)
    #define D3D1xUnmapBuffer( pDeviceContext, Resource, SubResource) \
        pDeviceContext->Unmap(Resource, SubResource)
    #define D3D1xMapTexture2D( pDeviceContext, Resource, SubResource, MapType, MapFlag, MappedBuffer) \
        pDeviceContext->Map(Resource, SubResource, MapType, MapFlag, MappedBuffer)
    #define D3D1xUnmapTexture2D( pDeviceContext, Resource, SubResource) \
        pDeviceContext->Unmap(Resource, SubResource)
    #define D3D1xCreateVertexShader( pDevice, ByteCode, Length, ShaderPtr) \
        pDevice->CreateVertexShader( ByteCode, Length, 0, ShaderPtr)
    #define D3D1xCreatePixelShader( pDevice, ByteCode, Length, ShaderPtr) \
        pDevice->CreatePixelShader( ByteCode, Length, 0, ShaderPtr)
    #define D3D1xVSSetShader( pDeviceContext, Shader ) \
        pDeviceContext->VSSetShader( Shader, 0, 0)
    #define D3D1xPSSetShader( pDeviceContext, Shader ) \
        pDeviceContext->PSSetShader( Shader, 0, 0)
    #define D3D1xCSSetShader( pDeviceContext, Shader) \
        pDeviceContext->CSSetShader(Shader, 0, 0)
    #define D3D1xDSSetShader( pDeviceContext, Shader) \
        pDeviceContext->DSSetShader(Shader, 0, 0)
    #define D3D1xGSSetShader( pDeviceContext, Shader) \
        pDeviceContext->GSSetShader(Shader, 0, 0)
    #define D3D1xHSSetShader( pDeviceContext, Shader) \
        pDeviceContext->HSSetShader(Shader, 0, 0)
    #define D3D1xEndAsynchronous( pDeviceContext, Query ) \
        pDeviceContext->End(Query);
    #define D3D1xGetDataAsynchronous( pDeviceContext, Query, Data, DataSize, Flags ) \
        pDeviceContext->GetData(Query, Data, DataSize, Flags)

    typedef D3D11_MAPPED_SUBRESOURCE D3D11_MAPPED_TEXTURE2D;
    typedef D3D11_MAPPED_SUBRESOURCE D3D11_MAPPED_BUFFER;

#else
    #error SF_D3D_VERSION must be 10 or 11.
#endif

// Required for PIX events.
#if !defined(SF_BUILD_SHIPPING) && !defined(SF_OS_WINMETRO)
#include <d3d9types.h>
#endif

#endif // INC_SF_D3D1X_CONFIG_H
