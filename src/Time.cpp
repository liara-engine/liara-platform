#include "Time.h"

#include <cstdint>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#elif defined(__linux__) || defined(__unix__)
    #include <cerrno>
    #include <ctime>
#endif

namespace Liara::Platform::Time
{

    std::uint64_t NowNs() {
#ifdef _WIN32
        LARGE_INTEGER frequency;
        LARGE_INTEGER counter;

        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&counter);

        uint64_t seconds = static_cast<uint64_t>(counter.QuadPart) / static_cast<uint64_t>(frequency.QuadPart);
        uint64_t remainder = static_cast<uint64_t>(counter.QuadPart) % static_cast<uint64_t>(frequency.QuadPart);
        return (seconds * 1000000000ULL) + ((remainder * 1000000000ULL) / static_cast<uint64_t>(frequency.QuadPart));
#elif defined(__linux__) || defined(__unix__)
        timespec ts {};
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
            return (static_cast<std::uint64_t>(ts.tv_sec) * 1'000'000'000ULL) + static_cast<std::uint64_t>(ts.tv_nsec);
        }
        return 0;  // Fallback to 0 if clock_gettime fails
#endif
    }

    std::int64_t WallNs() {
#ifdef _WIN32
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        ULARGE_INTEGER uli;
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;

        // Difference between 1601 and 1970 in 100-nanosecond intervals
        constexpr std::int64_t EPOCH_DIFFERENCE_100NS = 116444736000000000ULL;

        int64_t ns_since_epoch = static_cast<int64_t>(uli.QuadPart - EPOCH_DIFFERENCE_100NS) * 100LL;
        return ns_since_epoch;
#elif defined(__linux__) || defined(__unix__)
        timespec ts {};
        if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
            return (static_cast<std::int64_t>(ts.tv_sec) * 1'000'000'000LL) + static_cast<std::int64_t>(ts.tv_nsec);
        }
        return 0;  // Fallback to 0 if clock_gettime fails
#endif
    }

    void SleepUntilNs(const std::uint64_t deadlineNs) {
        const uint64_t nowNs = NowNs();

        if (deadlineNs <= nowNs) {
            return;  // Deadline has already passed
        }

        const uint64_t durationNs = deadlineNs - nowNs;

#ifdef _WIN32
        // Convert nanoseconds to milliseconds for Sleep function
        DWORD duration_ms = static_cast<DWORD>(durationNs / 1'000'000ULL);
        Sleep(duration_ms);
#elif defined(__linux__) || defined(__unix__)
        timespec ts {};
        ts.tv_sec = static_cast<time_t>(durationNs / 1'000'000'000ULL);
        ts.tv_nsec = static_cast<time_t>(durationNs % 1'000'000'000ULL);

        timespec remaining {};
        while (nanosleep(&ts, &remaining) == -1 && errno == EINTR) {
            ts = remaining;  // Continue sleeping for the remaining time
        }
#endif
    }

    std::uint64_t ResolutionNs() {
#ifdef _WIN32
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        return static_cast<std::uint64_t>(1'000'000'000ULL) / static_cast<std::uint64_t>(frequency.QuadPart);
#elif defined(__linux__) || defined(__unix__)
        if (timespec ts {}; clock_getres(CLOCK_MONOTONIC, &ts) == 0) {
            return (static_cast<std::uint64_t>(ts.tv_sec) * 1'000'000'000ULL) + static_cast<std::uint64_t>(ts.tv_nsec);
        }
        return 1;  // Fallback to 1 nanosecond if clock_getres fails
#endif
    }
}  // namespace Liara::Platform::Time
