#pragma once

/**
 * @file Instance.h
 * @brief The state behind a liara_platform_handle_t: its open windows, its event queue and its quit flag.
 */

#include <cstddef>
#include <vector>

namespace Liara::Platform
{
    class Window;

    class Instance
    {
    public:
        Instance() = default;
        ~Instance() = default;

        Instance(const Instance&) = delete;
        Instance& operator=(const Instance&) = delete;
        Instance(Instance&&) = delete;
        Instance& operator=(Instance&&) = delete;

        void OnWindowOpened(const Window& window);

        /// Forgets the window. Task 7 also discards the events still queued for it here.
        void OnWindowDestroyed(const Window& window);

        [[nodiscard]] std::size_t OpenWindowCount() const noexcept;

    private:
        std::vector<const Window*> m_OpenWindows;  ///< Open from creation to destruction, in creation order.
    };
}  // namespace Liara::Platform
