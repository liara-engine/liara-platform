#pragma once

/**
 * @file VideoSubsystem.h
 * @brief One reference on SDL's video subsystem, held for as long as a window needs it.
 */

namespace Liara::Platform
{
    /**
     * @brief Holds one reference on SDL's video subsystem, which SDL counts.
     *
     * The first reference sets the hints the module depends on and starts the subsystem; the last one released stops
     * it. An instance with no window never takes one, which is what keeps a headless instance off the video subsystem.
     * Main thread only, like every SDL video call.
     */
    class VideoSubsystem
    {
    public:
        VideoSubsystem();
        ~VideoSubsystem();

        VideoSubsystem(const VideoSubsystem&) = delete;
        VideoSubsystem& operator=(const VideoSubsystem&) = delete;
        VideoSubsystem(VideoSubsystem&& other) noexcept;
        VideoSubsystem& operator=(VideoSubsystem&&) = delete;

        /// True when this object holds a reference, false when SDL refused to start the subsystem.
        [[nodiscard]] bool IsAcquired() const noexcept;

        /// True while any reference is held anywhere in the process.
        [[nodiscard]] static bool IsActive() noexcept;

    private:
        bool m_Acquired = false;
    };
}  // namespace Liara::Platform
