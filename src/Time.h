#pragma once

/**
 * @file Time.h
 * @brief The module's clock: a monotonic counter, a wall clock, and a sleep to a deadline.
 *
 * Free functions rather than a type, because there is nothing here to instantiate. `docs/code-style/naming.md` names
 * this case, a file holding free functions in a namespace taking the name of its concern.
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
}  // namespace Liara::Platform::Time
