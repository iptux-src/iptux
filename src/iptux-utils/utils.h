#ifndef IPTUX_UTILS_UTILS_H
#define IPTUX_UTILS_UTILS_H

#include <string>

namespace iptux {
namespace utils {

/**
 * @brief calculate the file or directory size;
 *
 * return 0 if not exist.
 *
 * @param fileOrDirName
 * @return int64_t
 */
int64_t fileOrDirectorySize(const std::string& fileOrDirName);

}
}

#endif
