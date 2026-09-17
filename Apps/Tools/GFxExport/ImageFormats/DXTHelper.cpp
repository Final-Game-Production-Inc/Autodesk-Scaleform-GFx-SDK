/**************************************************************************
Filename    :   GDXTHelper.cpp
Content     :   Interface to external compression libraries
Created     :   September, 2006
Authors     :   Artyom Bolgar, Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.
**************************************************************************/
#define NOMINMAX

#include <nvtt/nvtt.h>

#include "Kernel/SF_File.h"
#include "Render/Render_Image.h"
#include "Kernel/SF_String.h"
#include "GFx/GFx_Stream.h"

#include "Render/Render_ResizeImage.h"
#include "Render/Render_TextureUtil.h"

#include "DXTHelper.h"

using namespace Scaleform;
using namespace GFx;
using namespace Render;

struct nvColor32 {
    UInt8 b, g, r, a;
};

int GDXTHelper::LastError = nvtt::Error_Unknown;

// 【修复点1】补充 GOutputHandler 缺失的纯虚函数实现
// 修复后：严格匹配 NVTT 2.1.2 基类签名，保留 override（C++20 安全）
struct GOutputHandler : public nvtt::OutputHandler
{
    GOutputHandler() : Total(0), Progress(0), Percentage(0), pOutFile(NULL) {}
    GOutputHandler(File* pFile) : Total(0), Progress(0), Percentage(0), pOutFile(pFile) {}
    virtual ~GOutputHandler() {}

    // 1. 修复：参数改为 float，匹配基类，override 生效
    virtual void setTotal(float t)
    {
        Total = t;
    }

    // 2. 修复：参数改为 float，匹配基类，override 生效
    virtual void setProgress(float p)
    {
        Progress = p;
        // 浮点计算，避免整数除法错误
        Percentage = Total > 0.0f ? (p * 100.0f) / Total : 0.0f;
    }

    virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel) override
    {
        SF_UNUSED6(size, width, height, depth, face, miplevel);
    }

    virtual void endImage() override
    {
        // 空实现，满足纯虚函数要求
    }

    virtual bool writeData(const void* data, int size) override
    {
        return pOutFile != nullptr && ((int)size == pOutFile->Write((const UByte*)data, (int)size));
    }

    // 3. 优化：成员变量改为 float，匹配 NVTT 类型（避免类型转换警告）
    float   Total;
    float   Progress;
    float   Percentage;
    File* pOutFile;
};

struct GErrorHandler : public nvtt::ErrorHandler
{
    virtual void error(nvtt::Error e) override
    {
        GDXTHelper::LastError = e;
        SF_ASSERT(0);
    }
};

static void GImage2NvColor32Buff(const ImageData& srcImg, nvColor32* nvImage)
{
    SF_ASSERT(nvImage);
    for (unsigned y = 0; y < srcImg.GetSize().Height; y++)
    {
        const UByte* pscanline = srcImg.GetScanline(y);
        switch (srcImg.Format)
        {
        case Image_R8G8B8:
        {
            for (unsigned x = 0; x < srcImg.GetWidth(); x++, pscanline += 3)
            {
                nvColor32& pix = nvImage[y * srcImg.GetWidth() + x];
                pix.r = pscanline[0];
                pix.g = pscanline[1];
                pix.b = pscanline[2];
                pix.a = 0xFF;
            }
        }
        break;

        case Image_R8G8B8A8:
        {
            for (unsigned x = 0; x < srcImg.GetWidth(); x++, pscanline += 4)
            {
                nvColor32& pix = nvImage[y * srcImg.GetWidth() + x];
                pix.r = pscanline[0];
                pix.g = pscanline[1];
                pix.b = pscanline[2];
                pix.a = pscanline[3];
            }
        }
        break;

        case Image_A8:
        {
            for (unsigned x = 0; x < srcImg.GetWidth(); x++, pscanline += 1)
            {
                nvColor32& pix = nvImage[y * srcImg.GetWidth() + x];
                pix.r = pscanline[0];
                pix.g = pscanline[0];
                pix.b = pscanline[0];
                pix.a = pscanline[0];
            }
        }
        break;

        default:
            SF_ASSERT(0);
        }
    }
}

static nvtt::MipmapFilter NvFilterConst(GDXTHelper::MipFilterTypes filter)
{
    switch (filter)
    {
    case GDXTHelper::nvFilterBox:         return nvtt::MipmapFilter_Box;
    case GDXTHelper::nvFilterTriangle:    return nvtt::MipmapFilter_Triangle;
    case GDXTHelper::nvFilterKaiser:      return nvtt::MipmapFilter_Kaiser;
    }
    return nvtt::MipmapFilter_Box;
}

static nvtt::Quality NvQualityConst(GDXTHelper::QualitySetting qual)
{
    switch (qual)
    {
    case GDXTHelper::QualityFastest:    return nvtt::Quality_Fastest;
    case GDXTHelper::QualityNormal:     return nvtt::Quality_Normal;
    case GDXTHelper::QualityProduction: return nvtt::Quality_Production;
    case GDXTHelper::QualityHighest:    return nvtt::Quality_Highest;
    }
    SF_ASSERT(0);
    return nvtt::Quality_Fastest;
}

UInt32 GDXTHelper::CalculateDDSSize(UInt32 srcW, UInt32 srcH, ImageFormat format, int dxtN,
    bool isUncompressedDDS, bool doGenMipMap, bool isDxt1Allowed)
{
    nvtt::InputOptions       inputOptions;
    nvtt::CompressionOptions compressionOptions;
    nvtt::Compressor         compressor;
    inputOptions.setTextureLayout(nvtt::TextureType_2D, srcW, srcH);
    inputOptions.setWrapMode(nvtt::WrapMode_Clamp);
    inputOptions.setNormalMap(false);
    inputOptions.setConvertToNormalMap(false);
    inputOptions.setNormalizeMipmaps(false);
    inputOptions.setMipmapGeneration(doGenMipMap);

    switch (format)
    {
    case Image_R8G8B8A8:
        if (isUncompressedDDS)
            compressionOptions.setFormat(nvtt::Format_RGBA);
        else
        {
            if (dxtN == 1)
                compressionOptions.setFormat(nvtt::Format_DXT1a);
            else if (dxtN == 3)
                compressionOptions.setFormat(nvtt::Format_DXT3);
            else
                compressionOptions.setFormat(nvtt::Format_DXT5);
        }
        break;

    case Image_R8G8B8:
        if (isUncompressedDDS)
        {
            compressionOptions.setFormat(!doGenMipMap ? nvtt::Format_RGB : nvtt::Format_RGBA);
        }
        else
        {
            if (isDxt1Allowed)
                compressionOptions.setFormat(nvtt::Format_DXT1);
            else if (dxtN == 3)
                compressionOptions.setFormat(nvtt::Format_DXT3);
            else
                compressionOptions.setFormat(nvtt::Format_DXT5);
        }
        break;

    case Image_A8:
        compressionOptions.setFormat(nvtt::Format_RGBA); // NVTT 无 A8 格式，映射到 RGBA
        break;

    default:
        break;
    }
    return compressor.estimateSize(inputOptions, compressionOptions);
}

bool GDXTHelper::CompressToFile(const Render::Image* psrcimage, File* pdstFile, CompressionSettinns* psettings, GFxExportListener* logger)
{
    SF_UNUSED(logger);

    DXTSettings* ps = (DXTSettings*)psettings;
    ExportImageHelper::RescaleTypes rescale = ps->Rescale;
    bool isUncompressedDDS = ps->IsUncompressed;

    if (rescale == RescaleNone && (!isUncompressedDDS || ps->DoGenMipMap))
        rescale = RescaleBiggestPower2;

    ImageData srcimgData;
    Ptr<RawImage> ptempImage = *RawImage::Create(Image_R8G8B8A8, 1, psrcimage->GetSize(), 0);
    ptempImage->GetImageData(&srcimgData);
    psrcimage->Decode(&srcimgData, GetImageConvertFunc(Image_R8G8B8A8, psrcimage->GetFormat()));

    Ptr<RawImage> scaledImage = *CreateResizedImage(&srcimgData, rescale, ps->RescaleFilter, psettings->bSquare);
    if (!scaledImage)
        scaledImage = ptempImage;

    // 小于 4x4 的图片强制不压缩
    if ((scaledImage->GetSize().Width < 4) || (scaledImage->GetSize().Height < 4))
        isUncompressedDDS = true;

    nvtt::InputOptions       inputOptions;
    nvtt::CompressionOptions compressionOptions;
    nvtt::Compressor         compressor;

    nvtt::MipmapFilter  nvMipFilter = NvFilterConst(ps->MipFilter);
    nvtt::Quality       nvQuality = NvQualityConst(ps->Quality);

    ImageData scaledImageData;
    scaledImage->GetImageData(&scaledImageData);
    nvColor32* pimgBuff = (nvColor32*)SF_ALLOC(
        scaledImageData.GetWidth() * scaledImageData.GetHeight() * sizeof(nvColor32),
        Stat_Image_Mem
    );
    GImage2NvColor32Buff(scaledImageData, pimgBuff);

    inputOptions.setTextureLayout(nvtt::TextureType_2D, scaledImageData.GetWidth(), scaledImageData.GetHeight());
    inputOptions.setMipmapData(pimgBuff, scaledImageData.GetWidth(), scaledImageData.GetHeight());
    SF_FREE(pimgBuff);

    inputOptions.setWrapMode(nvtt::WrapMode_Clamp);
    inputOptions.setNormalMap(false);
    inputOptions.setConvertToNormalMap(false);
    inputOptions.setGamma(2.2f, 2.2f);
    inputOptions.setNormalizeMipmaps(false);
    inputOptions.setMipmapGeneration(ps->DoGenMipMap);
    if (ps->DoGenMipMap)
        inputOptions.setMipmapFilter(nvMipFilter);

    compressionOptions.setQuality(nvQuality);
    compressor.enableCudaAcceleration(false);
    compressionOptions.setColorWeights(1, 1, 1);

    // 【修复点2】替换 Format_A8 为 Format_RGBA
    switch (psrcimage->GetFormat())
    {
    case Image_R8G8B8A8:
        if (isUncompressedDDS)
            compressionOptions.setFormat(nvtt::Format_RGBA);
        else
        {
            if (ps->DxtN == 1)
                compressionOptions.setFormat(nvtt::Format_DXT1a);
            else if (ps->DxtN == 3)
                compressionOptions.setFormat(nvtt::Format_DXT3);
            else
                compressionOptions.setFormat(nvtt::Format_DXT5);
        }
        break;

    case Image_R8G8B8:
        if (isUncompressedDDS)
        {
            compressionOptions.setFormat(!ps->DoGenMipMap ? nvtt::Format_RGB : nvtt::Format_RGBA);
        }
        else
        {
            if (ps->IsDxt1Allowed)
                compressionOptions.setFormat(nvtt::Format_DXT1);
            else if (ps->DxtN == 3)
                compressionOptions.setFormat(nvtt::Format_DXT3);
            else
                compressionOptions.setFormat(nvtt::Format_DXT5);
        }
        break;

    case Image_A8:
        compressionOptions.setFormat(nvtt::Format_RGBA); // 移除 Format_A8，改用 RGBA
        break;

    default:
        break;
    }

    GOutputHandler outputHandler(pdstFile);
    GErrorHandler  errorHandler;
    outputHandler.setTotal(static_cast<float>(compressor.estimateSize(inputOptions, compressionOptions)));

    nvtt::OutputOptions outputOptions;
    outputOptions.setOutputHandler(&outputHandler);
    outputOptions.setErrorHandler(&errorHandler);

    compressor.process(inputOptions, compressionOptions, outputOptions);

    return true;
}

const char* GDXTHelper::GetLastErrorString()
{
    return nvtt::errorString((nvtt::Error)GDXTHelper::LastError);
}