#pragma once

/**
 * @file Shutdown.h
 * @brief The process-global quit flag and the handlers that set it.
 *
 * Free functions rather than a type: there is nothing here to instantiate, since a POSIX signal handler can reach
 * process-global state and nothing else. `docs/code-style/naming.md` names this case, a file holding free functions in
 * a namespace taking the name of its concern.
 */

namespace Liara::Platform::Shutdown
{
    /**
     * @brief Install the handlers for the signals that mean "stop". Idempotent, process-global.
     * @return True when the handlers are installed, false when the operating system refused one.
     */
    bool Install();

    /**
     * @brief Read the quit flag. Safe from any thread.
     */
    bool QuitRequested();
}  // namespace Liara::Platform::Shutdown
