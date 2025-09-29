/**
 * getopt: (https://stackoverflow.com/questions/52467531/using-getopt-in-c-to-handle-arguments)
 * compile: g++ main.cpp -o myfind
 * run: ./myfind -i -R /path/to/dir file1 file2 file3
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <filesystem>
#include <dirent.h>
#include <stdexcept>

using namespace std;

struct Options
{
    unsigned short Counter_Option_R = 0;
    unsigned short Counter_Option_i = 0;
};

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

    // getopt() on C++ does not support strings, so vectors are used.
    vector<string> filenames;
    for (int i = optind; i < argc; i++)
    {
        filenames.push_back(argv[i]);
    }

    // just making sure everything is working. Remove later:
    cout << "Option i:" << options.Counter_Option_i << "\n";
    cout << "Option r:" << options.Counter_Option_R << "\n";

    cout << "Search path: " << searchPath << "\n";

    cout << "Files:\n";
    for (size_t i = 0; i < filenames.size(); i++)
    {
        cout << "  " << filenames[i] << "\n";
    }
    //////////////////////////////////////////////////

    // Testing directory reading:
    DIR *dirp;
    if ((dirp = opendir(searchPath.c_str())) == NULL)
    {
        cerr << "Error: Cannot open directory " << searchPath << "\n";
        return 1;
    }

    // Testing file reading
    struct dirent *direntp;
    while ((direntp = readdir(dirp)) != NULL)
    {
        if (direntp->d_name[0] == '.') // skip files with . at the start (e.g.: . and ..)
            continue;

        if (direntp->d_name == filenames[0]) // only checks for the first file in the list atm
            cout << "File found: ";

        cout << direntp->d_name << "\n";
    }
}
