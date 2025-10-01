#include <string>
#include <sys/types.h>

using std::string;

/// @brief Test
struct SearchResult
{
    pid_t pid;       // process ID of the child
    string filename; // name of the file we searched for
    string path;     // full directory path where file was found
};