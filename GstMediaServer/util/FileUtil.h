#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <string>

class FileUtil
{
    FileUtil();
public:
    static std::string absolutePath(std::string path);
};

#endif // FILEUTIL_H
