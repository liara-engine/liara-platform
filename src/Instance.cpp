#include "Instance.h"

#include "Window.h"

#include <vector>

namespace Liara::Platform
{
    void Instance::OnWindowOpened(const Window& window) { m_OpenWindows.push_back(&window); }

    void Instance::OnWindowDestroyed(const Window& window) { std::erase(m_OpenWindows, &window); }

    std::size_t Instance::OpenWindowCount() const noexcept { return m_OpenWindows.size(); }
}  // namespace Liara::Platform
