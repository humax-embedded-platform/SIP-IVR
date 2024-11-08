#include "FileUtil.h"
#include <filesystem>
#include <gst/gst.h>
#include "util/Log.hpp"

FileUtil::FileUtil() {}

std::string FileUtil::absolutePath(std::string path)
{

    std::filesystem::path p(path);
    if (path[0] == '~') {
        const gchar* userDataDir = g_get_home_dir();
        if (userDataDir) {
            p = std::filesystem::path(userDataDir) / path.substr(2);
        }
    }
    return p.string();
}
