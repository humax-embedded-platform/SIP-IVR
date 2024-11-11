#ifndef HASHUTIL_H
#define HASHUTIL_H


#include <string>

namespace gstmediaserver {

class HashUtil
{
    HashUtil();
public:
    static std::string generateMD5(const std::string& input);
};

} // namespace gstmediaserver

#endif // HASHUTIL_H
