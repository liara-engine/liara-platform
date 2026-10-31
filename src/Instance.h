#pragma once

/**
 * @file Instance.h
 * @brief The state behind a liara_platform_handle_t: its open windows, its event queue and its quit flag.
 */

#include <liara/platform/event.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
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

        /**
         * @brief Returns the next event of this instance, pumping the operating system once per drain.
         *
         * A drain is the run of calls up to one returning false. Its first call pumps; the others only read what that
         * pump queued, so a caller looping until false terminates under a continuous stream of events.
         */
        [[nodiscard]] bool Poll(liara_platform_event_t& outEvent);

        void Enqueue(const liara_platform_event_t& event);

        /// Queues the close request, and raises this instance's quit flag when `window` is its last open window.
        void OnCloseRequested(const Window& window, std::uint64_t timestampNs);

        void OnWindowOpened(const Window& window);

        /// Forgets the window and discards every event still queued for it.
        void OnWindowDestroyed(const Window& window);

        /// This instance's flag only; the process-global one is Shutdown::QuitRequested.
        [[nodiscard]] bool QuitRequested() const noexcept;

        [[nodiscard]] std::size_t OpenWindowCount() const noexcept;

    private:
        std::deque<liara_platform_event_t> m_Events;
        std::vector<const Window*> m_OpenWindows;   ///< Open from creation to destruction, in creation order.
        std::atomic<bool> m_QuitRequested {false};  ///< Atomic: quit_requested may be called from any thread.
        bool m_Draining = false;
    };
}  // namespace Liara::Platform
