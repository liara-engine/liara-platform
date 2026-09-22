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

    void SleepUntilNs(const std::uint64_t deadlineNs) {
        const std::uint64_t now = NowNs();
        if (deadlineNs <= now) { return; }

        std::this_thread::sleep_for(std::chrono::nanoseconds(deadlineNs - now));
    }
}  // namespace Liara::Platform::Time
