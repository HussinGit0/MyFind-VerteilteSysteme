#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H

#include <string>
#include <sys/types.h>
using std::string;

struct SearchResult
{
    pid_t pid;
    string filename;
    string path;
};

#endif
