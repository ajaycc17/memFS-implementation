#ifndef MEMFS_H
#define MEMFS_H

#include <iostream>
#include <stdexcept>
#include <vector>
#include <thread>
#include <string>
#include <sstream>
#include <unordered_map>
#include <mutex>
#include <ctime>
#include <chrono>
#include <iomanip>

#include <fstream>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <atomic>
using namespace std;

// file structure
struct File
{
    string name;
    string content;
    size_t size;
    string creation_time;
    string last_modified;
};

// class for in-memory file system
class MemFS
{
private:
    // searchable by filename
    unordered_map<string, File> files;
    mutex fs_mutex;

public:
    // core operations
    void createFiles(const vector<string> &filenames);
    void writeFile(const string &filename, const string &content);
    void deleteFiles(const vector<string> &filenames);
    string readFile(const string &filename);
    vector<vector<string>> listFiles(bool detailed = false);
};

#endif
