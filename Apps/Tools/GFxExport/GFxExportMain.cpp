/**************************************************************************

Filename    :   GFxExportMain.cpp
Content     :   SWF to GFX resource extraction and conversion tool
Created     :   October, 2006
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFxExport.h"
#include "ConsoleSupport.h"

GFxImageExporterFactoryBase* GFxImageExporterFactoriesArray[GFX_MAX_NUM_OF_IMAGE_EXPORTERS];
unsigned                     GFxImageExporterFactoriesArrayCount = 0; 

// Print comma-delimited integer number
static void commaprint(unsigned long n)
{
    char retbuf[30];
    char *p = &retbuf[sizeof(retbuf)-1];
    int i = 0;
    *p = '\0';
    do {
        if(i%3 == 0 && i != 0)
            *(--p) = ',';
        *(--p) = (char)('0' + n % 10);
        n /= 10;
        i++;
    } while(n != 0);
    printf("%9s",p);
}

class DefaultLog : public GFxExportListener
{
public:
    void            SetInfoMode(bool infoMode)
    {
        InfoMode = infoMode;
    }
    virtual void    OnOutput(OutputType type, const OutputPayload &data)
    {
        switch (type)
        {
        case OUT_ImageFile:
            {
                if (InfoMode)
                {
                    printf("  %-20s ", data.Name.ToCStr());
                    printf("%4dx%4d  ", data.ImageWidth, data.ImageHeight);
                    printf("estimated size (bytes) : ");
                    commaprint(data.ImageEstSize);
                    printf("\n");
                }
                break;
            }
        case OUT_SoundFile:
            {
                if (InfoMode)
                {
                    printf("  %-20s ", data.Name.ToCStr());
                }
                break;
            }
        case OUT_SpriteSoundFile:
            {
                if (InfoMode)
                {
                    printf("  %-20s ", data.Name.ToCStr());
                }
                break;
            }
        case OUT_GFxFile:
        case OUT_CFxFile:
            {
                printf("Saving stripped SWF file as '%s'\n", data.FilePath.ToCStr());
                if (data.GFxTotalImagesWritten || data.GFxEstMemSize || data.GFxEstFileSize)
                {
                    printf("Total images : %d \n", data.GFxTotalImagesWritten);
                    printf("Estimated total size (bytes)%29s : "," ");
                    commaprint(data.GFxEstMemSize);printf("\n");
                    printf ("\nEstimated gfx file size (bytes)%26s : "," ");
                    commaprint(data.GFxEstFileSize);printf("\n");
                }
                break;
            }
        case OUT_FontList:
            {
                printf("Saving list of fonts as '%s'\n", data.FilePath.ToCStr());
                break;
            }
        case OUT_EditTextfieldList:
        case OUT_DynamicTextfieldList:
            {
                printf("Saving list of unique default dynamic textfield text as '%s'\n", data.FilePath.ToCStr());
                break;
            }
        case OUT_FileList:
            {
                printf("Saving list of generated files as '%s'\n", data.FilePath.ToCStr());
                break;
            }
        case OUT_AbcFile:
            {
                printf("Exporting AS3 byte-code as '%s'\n", data.FilePath.ToCStr());
                break;
            }
        default:
            {
                // TODO: Other types
            }
        }
    }
    virtual void    OnProcBegin(ProcMode type, const ProcData& data)
    {
        switch (type)
        {
        case PROC_Swf:
            {
                printf("Loading SWF file: %s - ", data.SwfFilename.ToCStr());
                break;
            }
        }
    }
    virtual void    OnProcProgress(ProcMode type, int progress)
    {
        SF_UNUSED(type);
        printf("%3d%%\b\b\b\b", progress);
    }
    virtual void    OnProcEnd(ProcMode type, const ProcData& data)
    {
        SF_UNUSED2(type, data);
    }
    virtual void    OnMessage(MessageType type, const char *msg)
    {
        switch (type)
        {
        case MSG_Warning:
        case MSG_Error:
            {
                fprintf(stderr, msg);
                fprintf(stderr, "\n");
                break;
            }
        default:
            printf(msg);
            printf("\n");
        }
    }
private:
    bool    InfoMode;
};

int main(int argc, char *argv[])
{
    GFx::System gfxInit;

    DefaultLog  logger;

    setvbuf(stdout, NULL, _IONBF, 0);

    DetectConsoles();

    if (argc > 0)
    {
        GFxDataExporter exporter(&logger);
        for (unsigned i = 0; i < GFxImageExporterFactoriesArrayCount; ++i)
        {
            // fill 'exporter' by all registered image exporters.
            GFxImageExporter* pexp = GFxImageExporterFactoriesArray[i]->Create();
            exporter.AddImageExporter(pexp->GetFormatId(), pexp);
        }

        if (argc == 1)
        {
            exporter.ShowHelpScreen();
            return -1;
        }

        if (exporter.ParseCommandLine(argc, argv))
        {
            logger.SetInfoMode(exporter.IsInfoMode());
            if (!exporter.Process())
                return -1;
        }
        else
            return -1;
    }
    return 0;
}
