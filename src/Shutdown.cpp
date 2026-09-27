#include "Shutdown.h"

#include <csignal>
#include <mutex>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <signal.h>  // NOLINT(modernize-deprecated-headers)
#endif

namespace
{
    /// Written by the signal handler, read by everything else.
    volatile std::sig_atomic_t gQuitRequested = 0;

    /// Guards the installation itself, so that two threads calling Install() install once between them. A mutex
    /// rather than a compare-and-swap on a bool: swapping the flag to true before the OS call is known to have
    /// succeeded would let a second, losing thread observe "installed" and return true while the first thread's
    /// sigaction/SetConsoleCtrlHandler call has not yet run or failed.
    std::mutex gInstallMutex;
    bool gInstalled = false;  ///< Guarded by g_InstallMutex.

#ifdef _WIN32
    BOOL WINAPI ConsoleHandler(const DWORD signal) {
        if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT || signal == CTRL_CLOSE_EVENT) {
            gQuitRequested = 1;
            return TRUE;
        }
        return FALSE;
    }
#else
    extern "C" void PosixHandler(int /*signal*/) { gQuitRequested = 1; }
#endif
}  // namespace

namespace Liara::Platform::Shutdown
{
    bool Install() {
        const std::scoped_lock lock(gInstallMutex);
        if (gInstalled) { return true; }

#ifdef _WIN32
        if (SetConsoleCtrlHandler(ConsoleHandler, TRUE) == 0) { return false; }
#else
        struct sigaction action {};
        action.sa_handler = PosixHandler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;  // No SA_RESTART: an interrupted wait should return so the caller can resume it itself.

        if (sigaction(SIGINT, &action, nullptr) != 0 || sigaction(SIGTERM, &action, nullptr) != 0) { return false; }
#endif
        gInstalled = true;
        return true;
    }

    bool QuitRequested() { return gQuitRequested != 0; }
}  // namespace Liara::Platform::Shutdown
