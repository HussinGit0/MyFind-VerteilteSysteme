/**
 * getopt: (https://stackoverflow.com/questions/52467531/using-getopt-in-c-to-handle-arguments)
 * compile: g++ main.cpp -o myfind
 * run: ./myfind -i -R /path/to/dir file1 file2 file3
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

unsigned short Counter_Option_R = 0;
unsigned short Counter_Option_i = 0;

/// @brief Parses command line options.
/// @param argc
/// @param argv
/// @param Counter_Option_R recursive flag
/// @param Counter_Option_i case insensitive flag
/// @return 0 if successful, 1 if error occured.
int parseOptions(int argc, char *argv[], unsigned short &Counter_Option_R, unsigned short &Counter_Option_i)
{
    int c;
    while ((c = getopt(argc, argv, "Ri")) != EOF)
    {
        switch (c)
        {
        case 'R':
            Counter_Option_R++;
            break;
        case 'i':
            Counter_Option_i++;
            break;
        default:
            return 1;
            break;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int result = parseOptions(argc, argv, Counter_Option_R, Counter_Option_i);

    // Arguement error handling
    if (result != 0)
    {
        cerr << "Error: Invalid options provided." << endl;
        return 1;
    }

    if ((Counter_Option_i > 1) || (Counter_Option_R > 1))
    {
        cerr << "Error: Duplicate options detected." << endl;
        return 1;
    }

    if (optind >= argc)
    {
        cerr << "Error: No input file provided." << endl;
        return 1;
    }

    // The first non-option argument is the search path
    string searchPath = argv[optind];

    optind++;
    // If no file names are provided, return error
    if (optind >= argc)
    {
        cerr << "Error: at least one filename is required!\n";
        return 1;
    }

    // getopt() on C++ does not support strings, so vectors are used.
    vector<string> filenames;
    for (int i = optind; i < argc; i++)
    {
        filenames.push_back(argv[i]);
    }

    // just making sure everything is working. Remove later:
    cout << "Option i:" << Counter_Option_i << "\n";
    cout << "Option r:" << Counter_Option_R << "\n";

    cout << "Search path: " << searchPath << "\n";

    cout << "Files:\n";
    for (size_t i = 0; i < filenames.size(); i++)
    {
        cout << "  " << filenames[i] << "\n";
    }
    //////////////////////////////////////////////////
}
