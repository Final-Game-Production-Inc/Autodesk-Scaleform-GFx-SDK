
#include "Platform/Platform.h"
#include "Kernel/SF_String.h"
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>

namespace Scaleform { namespace Platform {


class DirectoryImpl : public DirectoryImplBase
{
public:
    DirectoryImpl(const String& path, const String& pattern) :
      DirectoryImplBase(path, pattern) { }

    virtual void ReadDirectory(const String& path, const String& pattern);
};

void DirectoryImpl::ReadDirectory(const String& path, const String& pattern)
{
    Pattern = pattern;
    Path = path;
    DIR* dirp = opendir(Path);
    if (dirp != NULL) 
    {
        struct dirent *dp;
        while ((dp = readdir(dirp)) != NULL)
        {
            if (!fnmatch(Pattern.ToCStr(), dp->d_name, 0))
                Filenames.PushBack(dp->d_name);
        }
        closedir(dirp);
        CurFile = 0;
    }
}

Directory::Directory(const String& path, const String& pattern)
{
    pImpl = new DirectoryImpl(path, pattern);
}

Directory::~Directory()
{
    delete pImpl;
}

}}

