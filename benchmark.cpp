#include "memfs.h"
#include "memMeasure.h"

// structure to hold benchmark results
struct resStruct
{
    int thread_count;
    int operation_count;
    double elapsed_time;
    double cpu_time;
    size_t memory_used;
    double avg_latency;
};

// generate random file names
vector<string> generateFileNames(int count, const string &prefix = "file")
{
    vector<string> filenames;
    for (int i = 1; i <= count; ++i)
        filenames.push_back(prefix + to_string(i) + ".txt");
    return filenames;
}

// function to get current memory usage in bytes
size_t getCurrentRSS()
{
    ifstream statm("/proc/self/statm");
    size_t rss = 0;
    if (statm)
    {
        statm >> rss;
        rss *= sysconf(_SC_PAGESIZE);
    }
    return rss;
}

// function to get CPU time used by the process
double getCpuTime()
{
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return (usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6) +
           (usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6);
}

// benchmark workload for memFS
void benchmarkWorkload(MemFS &memfs, int thread_count, int operation_count, atomic<size_t> &total_latency)
{
    // sample content generation of 2KB
    const string sample_content(2000, 'A');
    // split workload among threads
    int files_per_thread = operation_count / thread_count;
    auto filenames = generateFileNames(operation_count);

    // lambda for thread operations
    auto threadTask = [&](int start_index, int end_index)
    {
        for (int i = start_index; i < end_index; ++i)
        {
            try
            {
                auto start_time = chrono::high_resolution_clock::now();
                memfs.createFiles({filenames[i]});
                memfs.writeFile(filenames[i], sample_content);
                memfs.readFile(filenames[i]);
                memfs.deleteFiles({filenames[i]});
                auto end_time = chrono::high_resolution_clock::now();
                chrono::duration<double, micro> latency = end_time - start_time;
                total_latency.fetch_add(latency.count());
            }
            catch (const exception &e)
            {
                cerr << "Error in thread: " << e.what() << endl;
            }
        }
    };

    // create and execute threads
    vector<thread> threads;
    for (int t = 0; t < thread_count; ++t)
    {
        int start_index = t * files_per_thread;
        int end_index = (t == thread_count - 1) ? operation_count : start_index + files_per_thread;
        threads.emplace_back(threadTask, start_index, end_index);
    }

    // wait for threads to finish
    for (auto &t : threads)
        t.join();
}

// main function to benchmark memFS
int main()
{
    MemFS memfs;

    // given settings for benchmarking
    vector<int> thread_counts = {1, 2, 4, 8, 16};
    vector<int> operation_counts = {100, 1000, 10000};
    vector<resStruct> res;

    // benchmarking for different thread counts and operation counts
    for (int operation_count : operation_counts)
    {
        cout << "\nBenchmarking for " << operation_count << " operations:\n";
        for (int thread_count : thread_counts)
        {
            atomic<size_t> total_latency(0);

            // start the timer and record initial memory/CPU usage
            double initial_cpu_time = getCpuTime();
            // auto initial_memory = ProcessMemoryTracker::takeSnapshot();
            auto start_time = chrono::high_resolution_clock::now();

            // perform the workload
            benchmarkWorkload(memfs, thread_count, operation_count, total_latency);

            // record final memory/CPU usage
            double final_cpu_time = getCpuTime();
            auto final_memory = ProcessMemoryTracker::takeSnapshot();
            auto end_time = chrono::high_resolution_clock::now();

            // calculate elapsed time and metrics
            chrono::duration<double, milli> elapsed = end_time - start_time;
            double cpu_usage = final_cpu_time - initial_cpu_time;
            // auto memory_used = (final_memory.peak_usage > initial_memory.peak_usage) ? (final_memory.peak_usage - initial_memory.peak_usage) : 0;
            double avg_latency = total_latency.load() / static_cast<double>(operation_count);

            // store results
            res.push_back({thread_count, operation_count, elapsed.count(), cpu_usage, final_memory.peak_usage, avg_latency});
        }
    }

    // print the results
    cout << "\nBenchmark results:\n";
    int count = 0;
    for (auto it : res)
    {
        cout << "Threads: " << it.thread_count << " | Operations: " << it.operation_count
             << " | Elapsed Time: " << it.elapsed_time << " ms"
             << " | CPU Time: " << it.cpu_time << " s | Memory Used: " << ProcessMemoryTracker::formatBytes(it.memory_used)
             << " | Average Latency: " << it.avg_latency << " \u03BCs\n";
        ++count;
        if (count % thread_counts.size() == 0)
            cout << endl;
    }
    return 0;
}