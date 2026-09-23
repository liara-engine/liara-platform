#include "Time.h"

#include <chrono>
#include <cstdint>
#include <thread>

namespace Liara::Platform::Time
{
    std::uint64_t NowNs() {
        const auto since = std::chrono::steady_clock::now().time_since_epoch();
        return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(since).count());
    }

    std::int64_t WallNs() {
        // C++20 guarantees system_clock measures Unix time, so this is the epoch the contract promises with no
        // platform-specific code at all.
        const auto since = std::chrono::system_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(since).count();
    }

    // This computes a relative duration from the deadline and sleeps on that, rather than sleeping on the absolute
    // deadline itself. The header's "a signal does not cut the call short" therefore holds only because libstdc++'s
    // sleep_for retries internally with the remainder after a spurious wake-up, not because this function targets an
    // absolute deadline the way the contract describes. The per-platform refinement -
    // clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME) under POSIX, a high-resolution waitable timer under Win32 -
    // sleeps on the deadline directly and closes that gap. It changes no line of ABI.
    void SleepUntilNs(const std::uint64_t deadlineNs) {
        const std::uint64_t now = NowNs();
        if (deadlineNs <= now) { return; }

        std::this_thread::sleep_for(std::chrono::nanoseconds(deadlineNs - now));
    }
}  // namespace Liara::Platform::Time
