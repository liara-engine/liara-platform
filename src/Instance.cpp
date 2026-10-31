#include "Instance.h"

#include "Events.h"
#include "Window.h"

#include <liara/platform/event.h>

#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>

namespace Liara::Platform
{
    bool Instance::Poll(liara_platform_event_t& outEvent) {
        if (!m_Draining) {
            Events::Pump();
            m_Draining = true;
        }
        if (m_Events.empty()) {
            m_Draining = false;
            return false;
        }
        outEvent = m_Events.front();
        m_Events.pop_front();
        return true;
    }

    void Instance::Enqueue(const liara_platform_event_t& event) { m_Events.push_back(event); }

    void Instance::OnCloseRequested(const Window& window, const std::uint64_t timestampNs) {
        liara_platform_event_t event {};
        event.type = LIARA_PLATFORM_EVENT_WINDOW_CLOSE_REQUESTED;
        event.timestamp_ns = timestampNs;
        event.window = window.Handle();
        Enqueue(event);

        if (m_OpenWindows.size() == 1 && m_OpenWindows.front() == &window) { m_QuitRequested.store(true); }
    }

    void Instance::OnWindowOpened(const Window& window) { m_OpenWindows.push_back(&window); }

    void Instance::OnWindowDestroyed(const Window& window) {
        std::erase(m_OpenWindows, &window);
        std::erase_if(m_Events, [handle = window.Handle()](const liara_platform_event_t& event) {
            return event.window == handle;
        });
    }

    bool Instance::QuitRequested() const noexcept { return m_QuitRequested.load(); }

    std::size_t Instance::OpenWindowCount() const noexcept { return m_OpenWindows.size(); }
}  // namespace Liara::Platform
