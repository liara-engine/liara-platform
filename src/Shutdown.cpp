#include "Shutdown.h"

#include <csignal>
#include <mutex>

#ifdef _WIN32
    #include <windows.h>
#else
    // sigaction/sigemptyset are POSIX, not ISO C, so <csignal> (which only guarantees the C++-standard subset:
    // std::signal, std::raise, std::sig_atomic_t) does not declare them. <signal.h> is the header that does, on
    // every supported platform; modernize-deprecated-headers otherwise wants <csignal> in its place.
    #include <signal.h>  // NOLINT(modernize-deprecated-headers)
#endif

namespace
{
    /// Written by the signal handler, read by everything else. `volatile sig_atomic_t` is the only thing a POSIX
    /// handler may touch, and `std::atomic` is not guaranteed lock-free for every type, so this is the flag's type
    /// rather than an atomic bool.
    volatile std::sig_atomic_t g_QuitRequested = 0;

    /// Guards the installation itself, so that two threads calling Install() install once between them. A mutex
    /// rather than a compare-and-swap on a bool: swapping the flag to true before the OS call is known to have
    /// succeeded would let a second, losing thread observe "installed" and return true while the first thread's
    /// sigaction/SetConsoleCtrlHandler call has not yet run or failed.
    std::mutex g_InstallMutex;
    bool g_Installed = false;  // Guarded by g_InstallMutex.

#ifdef _WIN32
    BOOL WINAPI ConsoleHandler(const DWORD signal) {
        if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT || signal == CTRL_CLOSE_EVENT) {
            g_QuitRequested = 1;
            return TRUE;
        }
        return FALSE;
    }
#else
    extern "C" void PosixHandler(int /*signal*/) { g_QuitRequested = 1; }
#endif
}  // namespace

namespace Liara::Platform::Shutdown
{
    bool Install() {
        const std::scoped_lock lock(g_InstallMutex);
        if (g_Installed) { return true; }

#ifdef _WIN32
        if (SetConsoleCtrlHandler(ConsoleHandler, TRUE) == 0) { return false; }
#else
        struct sigaction action {};
        action.sa_handler = PosixHandler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;  // No SA_RESTART: an interrupted wait should return so the caller can resume it itself.

        if (sigaction(SIGINT, &action, nullptr) != 0 || sigaction(SIGTERM, &action, nullptr) != 0) { return false; }
#endif
        g_Installed = true;
        return true;
    }

    bool QuitRequested() { return g_QuitRequested != 0; }
}  // namespace Liara::Platform::Shutdown
