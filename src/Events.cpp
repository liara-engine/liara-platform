#include "Events.h"

#include "Instance.h"
#include "Shutdown.h"
#include "Time.h"
#include "VideoSubsystem.h"
#include "Window.h"

#include <liara/platform/event.h>

#include <algorithm>
#include <cstdint>

#include <SDL3/SDL_events.h>

namespace
{
    using Liara::Platform::Window;

    void RouteCloseRequested(const SDL_WindowEvent& event) {
        if (Window* window = Window::FromSdlId(event.windowID); window != nullptr) {
            window->Owner().OnCloseRequested(*window, Liara::Platform::Time::NowNs());
        }
    }

    void RoutePixelSizeChanged(const SDL_WindowEvent& event) {
        Window* window = Window::FromSdlId(event.windowID);
        if (window == nullptr) { return; }

        window->FillPlaceholder();  // A resize invalidates SDL's window surface; repaint so the window stays visible.

        liara_platform_event_t translated {};
        translated.type = LIARA_PLATFORM_EVENT_WINDOW_RESIZED;
        translated.timestamp_ns = Liara::Platform::Time::NowNs();
        translated.window = window->Handle();
        translated.data.resized.pixel_size = {.width = static_cast<std::uint32_t>(std::max(event.data1, 0)),
                                              .height = static_cast<std::uint32_t>(std::max(event.data2, 0))};
        window->Owner().Enqueue(translated);
    }

    void Route(const SDL_Event& event) {
        switch (event.type) {
            case SDL_EVENT_QUIT: Liara::Platform::Shutdown::RaiseQuit(); break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED: RouteCloseRequested(event.window); break;
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: RoutePixelSizeChanged(event.window); break;
            default: break;  // Everything else waits for an ABI event type that needs it.
        }
    }
}  // namespace

namespace Liara::Platform::Events
{
    void Pump() {
        if (!VideoSubsystem::IsActive()) { return; }

        SDL_PumpEvents();

        // SDL_PeepEvents reads the queue without pumping, so this loop ends once what the pump above produced is
        // consumed. That is what bounds a drain.
        SDL_Event event {};
        while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST) == 1) { Route(event); }
    }
}  // namespace Liara::Platform::Events
