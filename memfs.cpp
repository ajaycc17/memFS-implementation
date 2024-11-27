#include "memfs.h"

// fetch the date in the format DD-MM-YYYY
string getCurrentDate()
{
    auto now = chrono::system_clock::now();
    time_t now_time = chrono::system_clock::to_time_t(now);
    ostringstream oss;
    oss << put_time(localtime(&now_time), "%d-%m-%Y");
    return oss.str();
}

// create files in the filesystem
void MemFS::createFiles(const vector<string> &filenames)
{
    lock_guard<mutex> lock(fs_mutex);
    for (const auto &name : filenames)
    {
        // check if the file already exists
        if (files.find(name) != files.end())
        {
            cerr << "error: another file with " << name << " exists.\n";
            continue;
        }
        // else create the file
        files[name] = {name, "", 0, getCurrentDate(), getCurrentDate()};
        cout << "file " << name << " created successfully.\n";
    }
}

// write content to a file in the filesystem
void MemFS::writeFile(const string &filename, const string &content)
{
    // maximum file size allowed is 2KB
    const size_t MAX_FILE_SIZE = 2048;
    lock_guard<mutex> lock(fs_mutex);
    auto it = files.find(filename);
    // check if the file exists
    if (it == files.end())
    {
        cerr << "error: file " << filename << " does not exist.\n";
        return;
    }
    // check if the content exceeds the maximum allowed size
    if (content.size() > MAX_FILE_SIZE)
    {
        cerr << "error: Content exceeds the maximum allowed size of 2KB for file " << filename << ".\n";
        return;
    }
    // else write the content to the file
    it->second.content = content;
    it->second.size = content.size();
    it->second.last_modified = getCurrentDate();
    cout << "successfully written to " << filename << ".\n";
}

// delete files from the filesystem
void MemFS::deleteFiles(const vector<string> &filenames)
{
    lock_guard<mutex> lock(fs_mutex);
    for (const auto &name : filenames)
    {
        if (files.erase(name))
            cout << "file " << name << " deleted successfully.\n";
        else
            cerr << "error: file " << name << " doesn't exist.\n";
    }
}

// read content from a file in the filesystem
string MemFS::readFile(const string &filename)
{
    lock_guard<mutex> lock(fs_mutex);
    auto it = files.find(filename);
    if (it == files.end())
        throw runtime_error(filename + " does not exist.");
    return it->second.content;
}

// list files in the filesystem
vector<vector<string>> MemFS::listFiles(bool detailed)
{
    lock_guard<mutex> lock(fs_mutex);
    vector<vector<string>> file_list;
    for (const auto &[name, file] : files)
    {
        if (detailed)
            file_list.push_back({to_string(file.size), file.creation_time, file.last_modified, name});
        else
            file_list.push_back({name});
    }
    return file_list;
}
