#include <unistd.h>
#include <ios>
#include <fstream>
#include <string>
using namespace std;

class ProcessMemoryTracker
{
public:
    // returns the current resident set size (RSS) in bytes
    static size_t getCurrentMemoryUsage()
    {
        string token;
        size_t size = 0;
        // try reading from /proc/self/status for more accurate RSS
        ifstream status("/proc/self/status");
        while (status >> token)
            if (token == "VmRSS:")
            {
                status >> size;
                return size * 1024;
            }
        // fallback to /proc/self/statm if status read fails
        ifstream statm("/proc/self/statm");
        size_t vm, rss;
        if (statm >> vm >> rss)
            return rss * sysconf(_SC_PAGESIZE);
        return 0;
    }

    // format bytes to a human-readable string with better precision
    static string formatBytes(size_t bytes)
    {
        double size = static_cast<double>(bytes);
        if (bytes < 1024)
            return to_string(bytes) + " B";
        if (bytes < 1048576)
            return to_string(static_cast<int>(size / 1024)) + " KB";
        if (bytes < 1073741824)
            return to_string(static_cast<int>(size / 1048576)) + " MB";
        return to_string(static_cast<int>(size / 1073741824)) + " GB";
    }

    // structure to hold memory usage metrics
    struct MemorySnapshot
    {
        size_t memory_usage;
        size_t peak_usage;
    };

    // take a snapshot of the current memory usage
    static MemorySnapshot takeSnapshot()
    {
        // read current memory usage
        size_t current = getCurrentMemoryUsage();
        // read peak memory usage from status file
        ifstream status("/proc/self/status");
        string token;
        size_t peak = 0;
        while (status >> token)
            if (token == "VmHWM:")
            {
                status >> peak;
                peak *= 1024;
                break;
            }
        return {current, peak};
    }
};