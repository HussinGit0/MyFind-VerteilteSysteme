#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H

#include <string>
#include <sys/types.h>

struct SearchResult
{
    pid_t pid;
    std::string filename;
    std::string path;
};

#endif
