
#include "ConsoleSupport.h"

#ifdef SF_OS_WIN32

ConsoleImageSupport Consoles[] = 
{
    {"PS3", "Sony PlayStation 3", ImageTarget_PS3, 0},
    {"Wii", "Nintendo Wii",       ImageTarget_Wii, 0},
    {"WiiU", "Nintendo WiiU",    ImageTarget_WiiU, 0},
    {"PSVita", "Sony PlayStation Vita",    ImageTarget_PSVita, 0},
    {"Xbox360", "Microsoft Xbox 360",    ImageTarget_X360, 0},
    {"3DS", "Nintendo 3DS",       ImageTarget_3DS, 0},
    {"Adreno", "Qualcomm Adreno",  ImageTarget_Adreno, 0},
    {"DX11", "DirectX11 block compression",  ImageTarget_DX11, 0},
    {"PS4", "Sony Playstation 4",  ImageTarget_Orbis, 0},
    {0,0,ImageTarget_Any,0}
};
#else
ConsoleImageSupport Consoles[] = 
{
    {0,0,ImageTarget_Any,0}
};
#endif

void DetectConsoles()
{
#ifdef SF_OS_WIN32
    ConsoleImageSupport* pcons = Consoles;
    while (pcons->Name)
    {
        String dllname = "GFxExport_";
        dllname += pcons->Name;
#ifdef SF_BUILD_DEBUG
        dllname += "_Debug";
#endif
        dllname += ".dll";
        HMODULE hconsupp = LoadLibraryA(dllname.ToCStr());
        if (hconsupp)
        {
            GetConsoleSupportFunc pfunc = (GetConsoleSupportFunc)GetProcAddress(hconsupp, "GetConsoleSupport");
            if (pfunc)
                pcons->pSupport = pfunc(0);
        }
        pcons++;
    }
#endif
}


GFxExportConsolePlugin* FindConsole(ImageTarget target)
{
    ConsoleImageSupport* pcons = Consoles;
    while (pcons->Name)
    {
        if (target == pcons->ImgTarget && pcons->pSupport)
            return pcons->pSupport;
        pcons++;
    }
    return 0;
}
