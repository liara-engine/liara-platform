#include "Window.h"

#include "Instance.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_surface.h>

namespace
{
    /// The SDL window property through which an SDL window id leads back to its Liara::Platform::Window.
    constexpr const char* WINDOW_PROPERTY = "liara.platform.window";

    constexpr std::uint8_t PLACEHOLDER_RED = 0x1E;
    constexpr std::uint8_t PLACEHOLDER_GREEN = 0x25;
    constexpr std::uint8_t PLACEHOLDER_BLUE = 0x30;

    /// The largest side the module asks SDL for, 16384 being the largest 2D image dimension common GPUs support.
    /// The placeholder paints through SDL's window surface, which SDL backs with a GPU renderer wherever the driver has
    /// no software framebuffer (Wayland, the offscreen driver by default), and Mesa crashes binding a context to a
    /// surface of absurd size. `requested_size` is a request, so clamping it stays within the contract.
    constexpr std::uint32_t MAX_WINDOW_SIDE = 16384;

    /// SDL takes sizes as `int`; the clamp also keeps a large request from wrapping into a negative size.
    int ToSdlSize(const std::uint32_t size) { return static_cast<int>(std::min(size, MAX_WINDOW_SIDE)); }
}  // namespace

namespace Liara::Platform
{
    std::unique_ptr<Window> Window::Create(Instance& owner,
                                           liara_platform_window_handle_t* handle,
                                           const liara_extent2d_t requestedSize,
                                           const std::string_view title) {
        VideoSubsystem video;
        if (!video.IsAcquired()) { return nullptr; }

        const std::string terminatedTitle(title);  // SDL wants a terminator, which the ABI does not promise.
        SdlWindowPtr sdlWindow(SDL_CreateWindow(terminatedTitle.c_str(),
                                                ToSdlSize(requestedSize.width),
                                                ToSdlSize(requestedSize.height),
                                                SDL_WINDOW_RESIZABLE));
        if (sdlWindow == nullptr) { return nullptr; }

        auto window = std::make_unique<Window>(owner, handle, std::move(video), std::move(sdlWindow));
        window->FillPlaceholder();
        return window;
    }

    Window::Window(Instance& owner, liara_platform_window_handle_t* handle, VideoSubsystem video, SdlWindowPtr window)
        : m_Owner(&owner)
        , m_Handle(handle)
        , m_Video(std::move(video))
        , m_Window(std::move(window)) {
        SDL_SetPointerProperty(SDL_GetWindowProperties(m_Window.get()), WINDOW_PROPERTY, this);
        m_Owner->OnWindowOpened(*this);
    }

    Window::~Window() { m_Owner->OnWindowDestroyed(*this); }

    Window* Window::FromSdlId(const SDL_WindowID id) {
        SDL_Window* sdlWindow = SDL_GetWindowFromID(id);
        if (sdlWindow == nullptr) { return nullptr; }
        return static_cast<Window*>(
            SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindow), WINDOW_PROPERTY, nullptr));
    }

    Instance& Window::Owner() const noexcept { return *m_Owner; }

    liara_platform_window_handle_t* Window::Handle() const noexcept { return m_Handle; }

    bool Window::PixelSize(liara_extent2d_t& outSize) const {
        int width = 0;
        int height = 0;
        if (!SDL_GetWindowSizeInPixels(m_Window.get(), &width, &height)) { return false; }
        outSize = {.width = static_cast<std::uint32_t>(std::max(width, 0)),
                   .height = static_cast<std::uint32_t>(std::max(height, 0))};
        return true;
    }

    bool Window::Native(liara_native_window_t& outNative) const {
        outNative = {};
        const SDL_PropertiesID properties = SDL_GetWindowProperties(m_Window.get());

        // Every key is defined on every platform, and only the backend in use fills its own.
        if (void* surface = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
            surface != nullptr) {
            outNative.kind = LIARA_NATIVE_WINDOW_KIND_WAYLAND;
            outNative.handle.wayland.display =
                SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
            outNative.handle.wayland.surface = surface;
            return true;
        }
        if (void* display = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
            display != nullptr) {
            outNative.kind = LIARA_NATIVE_WINDOW_KIND_XLIB;
            outNative.handle.xlib.dpy = display;
            outNative.handle.xlib.window =
                static_cast<std::uint64_t>(SDL_GetNumberProperty(properties, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));
            return true;
        }
        if (void* hwnd = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
            hwnd != nullptr) {
            outNative.kind = LIARA_NATIVE_WINDOW_KIND_WIN32;
            outNative.handle.win32.hinstance =
                SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr);
            outNative.handle.win32.hwnd = hwnd;
            return true;
        }

        outNative.kind = LIARA_NATIVE_WINDOW_KIND_NONE;
        return false;
    }

    void Window::FillPlaceholder() {
        // Scaffolding, to delete once a renderer presents to this window through a swapchain: two producers attaching
        // buffers to one surface would fight. Until then it is what makes the window visible at all, since Wayland,
        // XWayland included, never shows a surface nothing was committed to.
        SDL_Surface* surface = SDL_GetWindowSurface(m_Window.get());
        if (surface == nullptr) { return; }
        SDL_FillSurfaceRect(surface,
                            nullptr,
                            SDL_MapSurfaceRGB(surface, PLACEHOLDER_RED, PLACEHOLDER_GREEN, PLACEHOLDER_BLUE));
        SDL_UpdateWindowSurface(m_Window.get());
    }
}  // namespace Liara::Platform
