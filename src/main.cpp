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
#include "SearchResult.h"
#include <cctype>

using namespace std;

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

bool AreFileNamesEqual(const filesystem::path &file1, string file2, bool isCaseInsensetive)
{
    string f1 = file1.string();
    string f2 = file2;

    if (!isCaseInsensetive)
    {
        return f1 == f2;
    }

    // https://www.geeksforgeeks.org/cpp/transform-c-stl-perform-operation-elements/
    auto toLower = [](const string &s)
    {
        string result = s;
        transform(result.begin(), result.end(), result.begin(),
                  [](unsigned char c)
                  { return tolower(c); });
        return result;
    };

    return toLower(f1) == toLower(f2);
}

vector<SearchResult> SearchFile(string &path, string filename, Options options)
{
    vector<SearchResult> results;
    pid_t pid = getpid();
    bool isCaseInsensetive = options.Counter_Option_i;

    try
    {
        if (options.Counter_Option_R == 1)
        {
            cout << "Recursive \n";
            for (const auto &entry : filesystem::recursive_directory_iterator(path))
            {
                if (entry.is_regular_file() &&
                    AreFileNamesEqual(entry.path().filename(), filename, isCaseInsensetive))
                {
                    results.push_back({pid, filename, entry.path()});
                }
            }
        }
        else
        {
            for (const auto &entry : filesystem::directory_iterator(path))
            {
                if (entry.is_regular_file() &&
                    AreFileNamesEqual(entry.path().filename(), filename, isCaseInsensetive))
                {
                    results.push_back({pid, filename, entry.path()});
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

string SearchResultToString(const vector<SearchResult> &results)
{
    string output;
    for (const auto &result : results)
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

    vector<int> pipes_read;
    vector<pid_t> children;

    for (const auto &filename : filenames)
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

        // Parent process
        if (c_pid > 0)
        {
            close(fd[1]);
            pipes_read.push_back(fd[0]);
            children.push_back(c_pid);
        }
        else
        {
            close(fd[0]);
            vector<SearchResult> results;
            try
            {
                results = SearchFile(searchPath, filename, options);
            }
            catch (filesystem::filesystem_error &e)
            {
                cout << "Oops";
                return 1;
            }

            string stringResult = SearchResultToString(results);
            write(fd[1], stringResult.c_str(), stringResult.size());
            close(fd[1]);
            _exit(0);
        }
    }

    // Parent reads from all pipes
    char buffer[4096];
    for (int fd : pipes_read)
    {
        ssize_t n;
        while ((n = read(fd, buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[n] = '\0';
            cout << buffer;
        }
        close(fd);
    }
}