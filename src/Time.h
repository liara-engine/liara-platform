#pragma once

/**
 * @file Time.h
 * @brief The module's clock: a monotonic counter, a wall clock, and a sleep to a deadline.
 */

#include <cstdint>

namespace Liara::Platform::Time
{
    /// Nanoseconds from an unspecified origin, never decreasing. Only differences are meaningful.
    std::uint64_t NowNs();

    /// Nanoseconds since the Unix epoch, UTC. Not monotonic; never used to measure a duration.
    std::int64_t WallNs();

    /// Block until NowNs() has reached at least `deadlineNs`. A past deadline returns immediately.
    void SleepUntilNs(std::uint64_t deadlineNs);

    /// Return the resolution of the monotonic clock in nanoseconds. This is the smallest measurable difference between two calls to NowNs().
    std::uint64_t ResolutionNs();
}  // namespace Liara::Platform::Time
