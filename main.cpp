#include "memfs.h"

// parse filenames from input stream
vector<string> parseFileNames(istringstream &stream)
{
    vector<string> filenames;
    string filename;

    // split by filenames by space
    while (stream >> ws && getline(stream, filename, ' '))
        if (!filename.empty())
            filenames.push_back(filename);
    return filenames;
}

// process the command and execute the corresponding operation
void processCommand(MemFS &memfs, const string &command)
{
    // get the command
    istringstream stream(command);
    string cmd;
    stream >> cmd;

    // for creation of files
    if (cmd == "create")
    {
        string arg;
        stream >> arg;
        // check for multiple file creation
        if (arg == "-n")
        {
            int count;
            stream >> count;
            auto filenames = parseFileNames(stream);
            if (filenames.size() != static_cast<size_t>(count))
            {
                cerr << "error: mismatch in file count.\n";
                return;
            }
            memfs.createFiles(filenames);
        }
        // single file creation
        else
        {
            // push back the filename to the stream
            for (auto it = arg.rbegin(); it != arg.rend(); ++it)
                stream.putback(*it);
            // get the filename from stream
            string filename;
            stream >> filename;
            // create the file
            if (!filename.empty())
                memfs.createFiles({filename});
            else
                cerr << "error: filename cannot be empty.\n";
        }
    }
    // write the content of the files
    else if (cmd == "write")
    {
        string arg;
        stream >> arg;
        // check for multiple file writing
        if (arg == "-n")
        {
            int count;
            stream >> count;
            for (int i = 0; i < count; ++i)
            {
                string filename, content;
                stream >> filename;
                // ignore the opening quote and read the content within quotes
                stream.ignore(numeric_limits<streamsize>::max(), '"');
                getline(stream, content, '"');
                // write the content to the file
                memfs.writeFile(filename, content);
            }
        }
        // single file writing
        else
        {
            // push back the filename to the stream
            for (auto it = arg.rbegin(); it != arg.rend(); ++it)
                stream.putback(*it);
            string filename, content;
            stream >> filename;
            // ignore the opening quote and read the content within quotes
            stream.ignore(numeric_limits<streamsize>::max(), '"');
            getline(stream, content, '"');
            // write the content to the file
            memfs.writeFile(filename, content);
        }
    }
    // delete the files
    else if (cmd == "delete")
    {
        string arg;
        stream >> arg;
        // check for multiple file deletion
        if (arg == "-n")
        {
            int count;
            stream >> count;
            auto filenames = parseFileNames(stream);
            if (filenames.size() != static_cast<size_t>(count))
            {
                cerr << "error: mismatch in file count.\n";
                return;
            }
            memfs.deleteFiles(filenames);
        }
        // single file deletion
        else
        {
            // push back the filename to the stream
            for (auto it = arg.rbegin(); it != arg.rend(); ++it)
                stream.putback(*it);
            string filename;
            stream >> filename;
            // delete the file
            memfs.deleteFiles({filename});
        }
    }
    // read the content of the files
    else if (cmd == "read")
    {
        string filename;
        stream >> filename;
        // try to read the file content
        try
        {
            string content = memfs.readFile(filename);
            cout << content << "\n";
        }
        catch (const exception &e)
        {
            cerr << "error: " << e.what() << "\n";
        }
    }
    // list the files
    else if (cmd == "ls")
    {
        string arg;
        // if '-l' option is provided, list detailed information
        bool detailed = false;
        if (stream >> arg && arg == "-l")
        {
            detailed = true;
            // print the labels
            cout << left << setw(10) << "size"
                 << setw(20) << "created"
                 << setw(20) << "last modified"
                 << setw(20) << "filename"
                 << "\n";
        }
        // list the files
        auto files = memfs.listFiles(detailed);
        // print the files
        if (detailed)
            for (const auto &file : files)
                cout << left << setw(10) << file[0]
                     << setw(20) << file[1]
                     << setw(20) << file[2]
                     << setw(20) << file[3]
                     << "\n";
        else
            for (const auto &file : files)
                cout << file[0] << "\n";
    }
    // exit the program
    else if (cmd == "exit")
    {
        cout << "exiting memFS.\n";
        exit(0);
    }
    // any other command is invalid
    else
        cerr << "invalid command provided.\n";
}

// main function to interact with the filesystem
int main()
{
    // create the filesystem and holder for command
    MemFS memfs;
    string command;

    // command prompt for the filesystem
    cout << "memFS> ";
    while (getline(cin, command))
    {
        // process the command if available
        if (!command.empty())
            processCommand(memfs, command);
        cout << "memFS> ";
    }
    return 0;
}
