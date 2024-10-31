#ifndef HASHUTIL_H
#define HASHUTIL_H

#include <string>

class HashUtil
{
    HashUtil();
public:
    static std::string generateMD5(const std::string& input);
};

#endif // HASHUTIL_H
