#pragma once

/**
 * @file Shutdown.h
 */

namespace Liara::Platform::Shutdown
{
    /**
     * @brief Install the handlers for the required signals. Idempotent, process-global.
     * @return True when the handlers are installed, false when the operating system refused one.
     */
    bool Install();

    /**
     * @brief Read the quit flag. Safe from any thread.
     */
    bool QuitRequested();
}  // namespace Liara::Platform::Shutdown
