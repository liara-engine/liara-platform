#pragma once

/**
 * @file Events.h
 * @brief Moves the operating system's events into the instances that own the windows they concern.
 */

namespace Liara::Platform::Events
{
    /**
     * @brief Pumps SDL once, translates the events the ABI knows, and routes each to the instance owning its window.
     *
     * SDL keeps one queue for the whole process while several instances may exist, so an event goes to the instance
     * of the window it concerns rather than to whichever instance pumped. Does nothing while the video subsystem is
     * down, since no window exists then. Main thread only.
     */
    void Pump();
}  // namespace Liara::Platform::Events
