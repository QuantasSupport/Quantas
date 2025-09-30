
#ifndef MEMORY_HPP
#define MEMORY_HPP

#if defined(_WIN32)
    #include <windows.h>
    #include <psapi.h>
#elif defined(__unix__) || defined(__APPLE__)
    #include <sys/resource.h>
#endif

// Returns peak memory usage in kilobytes
size_t getPeakMemoryKB() {
    #if defined(_WIN32)
        PROCESS_MEMORY_COUNTERS info;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info))) {
            return info.PeakWorkingSetSize / 1024;
        } else {
            return 0;
        }
    #elif defined(__unix__) || defined(__APPLE__)
        struct rusage usage;
        if (getrusage(RUSAGE_SELF, &usage) == 0) {
        #if defined(__APPLE__)
            return usage.ru_maxrss / 1024;  // macOS returns bytes
        #else
            return usage.ru_maxrss;        // Linux returns kilobytes
        #endif
        } else {
            return 0;
        }
    #else
        return 0;  // Unsupported platform
    #endif
}

#endif /* MEMORY_HPP */

