/**
 * getopt: (https://stackoverflow.com/questions/52467531/using-getopt-in-c-to-handle-arguments)
 * compile: g++ main.cpp -o myfind
 * run: Go to /build and use the command: ./MyFind -i -R /path/to/dir file1 file2 file3
 * ./MyFind -i -R ~/projects file1 file2 file3
 **/
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <filesystem>
#include <dirent.h>
#include <stdexcept>
#include "Options.h"
<<<<<<< HEAD
=======
#include "SearchResult.h"
>>>>>>> 0699c470143cb0d0a9d894d26974f9b0bae27a54

using namespace std;

// unsigned short Counter_Option_R = 0;
// unsigned short Counter_Option_i = 0;

/// @brief Parses command line options.
/// @param argc
/// @param argv
/// @param Counter_Option_R recursive flag
/// @param Counter_Option_i case insensitive flag
/// @return
Options parseOptions(int argc, char *argv[])
{
    struct Options options;
    int c;
    while ((c = getopt(argc, argv, "Ri")) != EOF)
    {
        switch (c)
        {
        case 'R':
            options.Counter_Option_R++;
            break;
        case 'i':
            options.Counter_Option_i++;
            break;
        default:
            throw invalid_argument("Invalid arguments provided.");
            break;
        }
    }

    if ((options.Counter_Option_i > 1) || (options.Counter_Option_R > 1))
    {
        cerr << "Error: Duplicate options detected." << endl;
        throw invalid_argument("Options must be provided at most once.");
    }

    return options;
}

vector<SearchResult> SearchFile(string &path, string filename, Options options)
{
    vector<SearchResult> results;
    pid_t pid = getpid();

    try
    {
        if (options.Counter_Option_R == 1)
        {
            cout << "Recursive \n";
            for (const auto &entry : filesystem::recursive_directory_iterator(path))
            {
                if (entry.is_regular_file() && entry.path().filename() == filename)
                {
                    SearchResult result;
                    result.pid = pid;
                    result.filename = filename;
                    result.path = entry.path();

                    results.push_back(result);
                }
            }
        }
        else
        {
            for (const auto &entry : filesystem::directory_iterator(path))
            {
                if (entry.is_regular_file() && entry.path().filename() == filename)
                {
                    SearchResult result;
                    result.pid = pid;
                    result.filename = filename;
                    result.path = entry.path();

                    results.push_back(result);
                }
            }
        }
    }
    catch (const filesystem::filesystem_error &e)
    {
        cout << "Oops!";
    }

    return results;
}

int main(int argc, char *argv[])
{
    Options options;
    try
    {
        options = parseOptions(argc, argv);
    }
    catch (const invalid_argument &e)
    {
        cout << "Error: " << e.what() << endl;
        return 1;
    }

    if (optind >= argc)
    {
        cout << "Error: No input file provided." << endl;
        return 1;
    }

    // The first non-option argument is the search path
    string searchPath = argv[optind];

    optind++;
    // If no file names are provided, return error
    if (optind >= argc)
    {
        cout << "Error: at least one filename is required!\n";
        return 1;
    }

    vector<string> filenames;
    for (int i = optind; i < argc; i++)
    {
        filenames.push_back(argv[i]);
    }

    // Testing directory reading:
    DIR *dirp;
    if ((dirp = opendir(searchPath.c_str())) == NULL)
    {
        cerr << "Error: Cannot open directory " << searchPath << "\n";
        return 1;
    }

    /// SEARCH
    vector<SearchResult> results;
    try
    {
        results = SearchFile(searchPath, filenames[0], options);
    }
    catch (filesystem::filesystem_error &e)
    {
        cout << "Oops";
        return 1;
    }

    /// OUTPUT SEARCH
    for (const auto &result : results)
    {
        cout << "pid: " << result.pid << " . file name: " << result.filename << " . path: " << result.path << "\n";
    }

    cout << "terminating program.\n";
    return 0;
}
