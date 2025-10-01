#include <string>
#include <sys/types.h>

/// @brief Test
struct SearchResult
{
    pid_t pid;       // process ID of the child
    std::string filename; // name of the file we searched for
    std::string path;     // full directory path where file was found
};