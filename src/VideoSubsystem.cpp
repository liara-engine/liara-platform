#include "VideoSubsystem.h"

#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>

namespace
{
    /// Sets the hints the module depends on, then takes one reference on the video subsystem.
    bool StartVideoSubsystem() {
        // The module's own handlers own SIGINT and SIGTERM, and the module decides what closing the last window means,
        // so SDL does neither. The signal hint is read when SDL starts, hence before.
        SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
        SDL_SetHint(SDL_HINT_QUIT_ON_LAST_WINDOW_CLOSE, "0");
        return SDL_InitSubSystem(SDL_INIT_VIDEO);
    }
}  // namespace

namespace Liara::Platform
{
    VideoSubsystem::VideoSubsystem()
        : m_Acquired(StartVideoSubsystem()) {}

    VideoSubsystem::~VideoSubsystem() {
        if (m_Acquired) { SDL_QuitSubSystem(SDL_INIT_VIDEO); }
    }

    VideoSubsystem::VideoSubsystem(VideoSubsystem&& other) noexcept
        : m_Acquired(other.m_Acquired) {
        other.m_Acquired = false;
    }

    bool VideoSubsystem::IsAcquired() const noexcept { return m_Acquired; }

    bool VideoSubsystem::IsActive() noexcept { return SDL_WasInit(SDL_INIT_VIDEO) != 0; }
}  // namespace Liara::Platform
