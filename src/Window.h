#pragma once

/**
 * @file Window.h
 * @brief One SDL window behind a liara_platform_window_handle_t.
 */

#include "VideoSubsystem.h"

#include <liara/extent.h>
#include <liara/native_window.h>
#include <liara/platform/window.h>

#include <memory>
#include <string_view>

#include <SDL3/SDL_video.h>

namespace Liara::Platform
{
    class Instance;

    class Window
    {
        struct SdlWindowDeleter
        {
            void operator()(SDL_Window* window) const noexcept { SDL_DestroyWindow(window); }
        };

    public:
        using SdlWindowPtr = std::unique_ptr<SDL_Window, SdlWindowDeleter>;

        /**
         * @brief Opens a window belonging to `owner`.
         * @param handle The ABI handle that owns the returned object. Events name the window through it.
         * @return The window, or null when the window system could not be reached or refused the window.
         */
        [[nodiscard]] static std::unique_ptr<Window> Create(Instance& owner,
                                                            liara_platform_window_handle_t* handle,
                                                            liara_extent2d_t requestedSize,
                                                            std::string_view title);

        /// Use Create; public only so that std::make_unique can reach it.
        Window(Instance& owner, liara_platform_window_handle_t* handle, VideoSubsystem video, SdlWindowPtr window);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(Window&&) = delete;

        /// The module's window behind an SDL window id, or null when the id is not one of the module's windows.
        [[nodiscard]] static Window* FromSdlId(SDL_WindowID id);

        [[nodiscard]] Instance& Owner() const noexcept;
        [[nodiscard]] liara_platform_window_handle_t* Handle() const noexcept;
        [[nodiscard]] bool PixelSize(liara_extent2d_t& outSize) const;
        [[nodiscard]] bool Native(liara_native_window_t& outNative) const;

        /// Paints the placeholder. Best-effort: a failure leaves the content unspecified, which the contract allows.
        void FillPlaceholder();

    private:
        Instance* m_Owner;
        liara_platform_window_handle_t* m_Handle;
        VideoSubsystem m_Video;  ///< Declared before m_Window, so released after the window is destroyed.
        SdlWindowPtr m_Window;
    };
}  // namespace Liara::Platform
