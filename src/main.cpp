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
#include <sys/wait.h>
#include <cstring>
#include "Options.h"
#include "SearchResult.h"

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
    closedir(dirp);

    /// FORK AND SEARCH FOR EACH FILE
    vector<SearchResult> allResults;
    vector<pid_t> childPids;
    vector<int> readPipes;

    // Erstelle für jede Datei einen Child-Prozess mit fork()
    for (const string &filename : filenames)
    {
        int pipefd[2];
        if (pipe(pipefd) == -1)
        {
            cerr << "Error creating pipe for file: " << filename << endl;
            continue;
        }

        pid_t pid = fork();
        if (pid == -1)
        {
            cerr << "Error forking process for file: " << filename << endl;
            close(pipefd[0]);
            close(pipefd[1]);
            continue;
        }

        if (pid == 0)
        {
            // Child-Prozess: Führe SearchFile aus und sende Ergebnisse über Pipe
            close(pipefd[0]); // Schließe Read-End in Child

            try
            {
                vector<SearchResult> results = SearchFile(searchPath, filename, options);

                // Schreibe Anzahl der Ergebnisse zuerst
                size_t numResults = results.size();
                write(pipefd[1], &numResults, sizeof(numResults));

                // Schreibe jedes SearchResult direkt in die Pipe
                for (const SearchResult &result : results)
                {
                    // Schreibe pid
                    write(pipefd[1], &result.pid, sizeof(result.pid));

                    // Schreibe filename length und filename
                    size_t filenameLen = result.filename.length();
                    write(pipefd[1], &filenameLen, sizeof(filenameLen));
                    write(pipefd[1], result.filename.c_str(), filenameLen);

                    // Schreibe path length und path
                    size_t pathLen = result.path.length();
                    write(pipefd[1], &pathLen, sizeof(pathLen));
                    write(pipefd[1], result.path.c_str(), pathLen);
                }

                close(pipefd[1]);
                exit(0);
            }
            catch (const exception &e)
            {
                cerr << "Child process error for file " << filename << ": " << e.what() << endl;
                close(pipefd[1]);
                exit(1);
            }
        }
        else
        {
            // Parent-Prozess: Speichere PID und Read-Pipe für später
            close(pipefd[1]); // Schließe Write-End in Parent
            childPids.push_back(pid);
            readPipes.push_back(pipefd[0]);
        }
    }

    // Parent-Prozess: Lese Ergebnisse von allen Pipes
    for (int readPipe : readPipes)
    {
        size_t numResults;
        if (read(readPipe, &numResults, sizeof(numResults)) == sizeof(numResults))
        {
            for (size_t i = 0; i < numResults; i++)
            {
                SearchResult result;

                // Lese pid
                read(readPipe, &result.pid, sizeof(result.pid));

                // Lese filename
                size_t filenameLen;
                read(readPipe, &filenameLen, sizeof(filenameLen));
                char *filenameBuffer = new char[filenameLen + 1];
                read(readPipe, filenameBuffer, filenameLen);
                filenameBuffer[filenameLen] = '\0';
                result.filename = string(filenameBuffer);
                delete[] filenameBuffer;

                // Lese path
                size_t pathLen;
                read(readPipe, &pathLen, sizeof(pathLen));
                char *pathBuffer = new char[pathLen + 1];
                read(readPipe, pathBuffer, pathLen);
                pathBuffer[pathLen] = '\0';
                result.path = string(pathBuffer);
                delete[] pathBuffer;

                allResults.push_back(result);
            }
        }
        close(readPipe);
    }

    // Warte auf alle Child-Prozesse
    for (pid_t childPid : childPids)
    {
        int status;
        waitpid(childPid, &status, 0);
        if (WEXITSTATUS(status) != 0)
        {
            cerr << "Child process " << childPid << " exited with error" << endl;
        }
    }

    /// OUTPUT SEARCH RESULTS
    for (const auto &result : allResults)
    {
        cout << "pid: " << result.pid << " . file name: " << result.filename << " . path: " << result.path << "\n";
    }

    cout << "terminating program.\n";
    return 0;
}
