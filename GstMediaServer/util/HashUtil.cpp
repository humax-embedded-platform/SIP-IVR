#include "HashUtil.h"
#include <openssl/md5.h>
#include <sstream>
#include <iomanip>

HashUtil::HashUtil() {}

std::string HashUtil::generateMD5(const std::string &input)
{
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((unsigned char*)input.c_str(), input.size(), (unsigned char*)&digest);

    // Convert the digest to a hexadecimal string
    std::ostringstream oss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return oss.str();
}
