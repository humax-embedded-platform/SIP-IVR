#ifndef FILEUTIL_H
#define FILEUTIL_H


#include <string>

namespace gstmediaserver {

class FileUtil
{
    FileUtil();
public:
    static std::string absolutePath(std::string path);
};

} // namespace gstmediaserver

#endif // FILEUTIL_H
