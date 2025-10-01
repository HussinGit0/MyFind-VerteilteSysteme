/**
 * By: Abo Shaar Hussin, Rosenmayr Alexander.
 * compile: go to build directory, use cmake .. then cmake --build .   .
 * run: Go to /build and use the command: ./MyFind [/path] [file1] [file2] [file3] [options]
 * Options:
 *  -R: Recursive searching
 *  -i: Case insensetive
 **/

#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <filesystem>
#include <dirent.h>
#include <stdexcept>
#include "Options.h"
#include "SearchResult.h"
#include <algorithm>
#include <cctype>

using namespace std;
using namespace filesystem;

/// @brief Parses command line options.
/// @param argc
/// @param argv
/// @return Struct of type Options.
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
        cout << "Error: Duplicate options detected." << endl;
        throw invalid_argument("Options must be provided at most once.");
    }

    return options;
}

/// @brief Takes a file path's name and compares it with a string.
/// @param file1 A path to the first file.
/// @param file2 A string of the name.
/// @param isCaseInsensetive Whether cases should be accounted for.
/// @return A boolean indicating whether the file names are equal.
bool AreFileNamesEqual(const path &file1path, string file2, bool isCaseInsensetive)
{
    string f1 = file1path.string();
    string f2 = file2;

    if (!isCaseInsensetive)
    {
        return f1 == f2;
    }

    // Lambda functions: https://www.w3schools.com/cpp/cpp_functions_lambda.asp
    auto toLower = [](const string &s)
    {
        // Transform: https://www.geeksforgeeks.org/cpp/transform-c-stl-perform-operation-elements/
        string result = s;
        transform(result.begin(), result.end(), result.begin(),
                  [](unsigned char c)
                  {
                      return tolower(c);
                  });
        return result;
    };

    return toLower(f1) == toLower(f2);
}

/// @brief Searches for a file in a directory.
/// @param path Directory to search in.
/// @param filename The file name to look for.
/// @param options Options struct containing user option inputs.
/// @return A vector containing the SearchResult struct.
vector<SearchResult> SearchFile(string &path, string filename, Options options)
{
    vector<SearchResult> results;
    pid_t pid = getpid();
    bool isCaseInsensetive = options.Counter_Option_i;

    if (options.Counter_Option_R == 1)
    {
        // Iterate through entries recusively.
        for (const auto &entry: recursive_directory_iterator(path))
        {
            // Check if current entry is a file, and if its name matches the file we are searching for.
            if (entry.is_regular_file() &&
                AreFileNamesEqual(entry.path().filename(), filename, isCaseInsensetive))
            {
                // Save the aggregate result in the results vector:
                // https://en.cppreference.com/w/cpp/language/aggregate_initialization.html
                results.push_back({pid, entry.path().filename(), entry.path()});
            }
        }
    } else
    {
        // Iterate without recursion.
        for (const auto &entry: directory_iterator(path))
        {
            if (entry.is_regular_file() &&
                AreFileNamesEqual(entry.path().filename(), filename, isCaseInsensetive))
            {
                // Save the aggregate result in the results vector:
                // https://en.cppreference.com/w/cpp/language/aggregate_initialization.html
                results.push_back({pid, entry.path().filename(), entry.path()});
            }
        }
    }

    return results;
}

/// @brief Covnerts a search results.
/// @param results
/// @return A string representing a vector of SearchResults.
string SearchResultToString(const vector<SearchResult> &results)
{
    string output;
    for (const auto &result: results)
    {
        output += to_string(result.pid) + ": " + result.filename + ": " + result.path + "\n";
    }
    return output;
}

int main(int argc, char *argv[])
{
    Options options;
    try
    {
        // This stores the options which the user inputted for recursive or case insensetivity.
        options = parseOptions(argc, argv);
    } catch (const invalid_argument &e)
    {
        cout << "Error: " << e.what() << endl;
        return 1;
    }

    if (optind >= argc)
    {
        cout << "Error: No input directory provided." << endl;
        return 1;
    }

    // The first non-option argument is the search path
    string searchPath = argv[optind];

    optind++;
    // If no file names are provided, return error because there is nothing to search for.
    if (optind >= argc)
    {
        cout << "Error: at least one filename is required!\n";
        return 1;
    }

    // Store all file inputs in one vector for easy access.
    vector<string> filenames;
    for (int i = optind; i < argc; i++)
    {
        filenames.push_back(argv[i]);
    }

    // Testing directory reading (Taken from printaccesstest.c from the course):
    DIR *dirp;
    if ((dirp = opendir(searchPath.c_str())) == NULL)
    {
        cout << "Error: Cannot open directory " << searchPath << "\n";
        return 1;
    }

    // Vector to store all pipes.
    vector<int> pipes_read;
    // Vector to store all child processes.
    vector<pid_t> children;

    for (const auto &filename: filenames)
    {
        int fd[2]; // fd[0] reading operations, fd[1] writing operations
        if (pipe(fd) == -1)
        {
            cout << "Error piping";
            return 1;
        }

        // https://www.geeksforgeeks.org/cpp/create-processes-with-fork-in-cpp/
        pid_t c_pid = fork();
        if (c_pid == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (c_pid > 0) // Parent process
        {
            close(fd[1]); // Close the write operations because the parent doesn't need it.
            pipes_read.push_back(fd[0]);
            children.push_back(c_pid);
        } else // This is a child process
        {
            close(fd[0]); // Close the read operations because the children don't need it.
            vector<SearchResult> results;
            try
            {
                results = SearchFile(searchPath, filename, options);
            } catch (filesystem_error &e)
            {
                cout << "Filesystem error while searching for '" << filename
                        << "' in path '" << searchPath << "' :"
                        << e.what() << "\n";
                return 1;
            }

            // We convert the SearchResult structs to a string.
            string stringResult = SearchResultToString(results);
            // Then send it to the corresponding pipe for the parent.
            write(fd[1], stringResult.c_str(), stringResult.size());
            close(fd[1]);
            _exit(0);
        }
    }

    // Parent reads from all pipes
    char buffer[4096];
    for (int fd: pipes_read)
    {
        ssize_t n;
        while ((n = read(fd, buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[n] = '\0';
            cout << buffer;
        }
        close(fd); // Close the pipe after reading.
    }
}
