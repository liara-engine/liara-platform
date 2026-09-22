#include "Shutdown.h"

#include <atomic>
#include <csignal>

#ifdef _WIN32
    #include <windows.h>
#endif

namespace
{
    /// Written by the signal handler, read by everything else. `volatile sig_atomic_t` is the only thing a POSIX
    /// handler may touch, and `std::atomic` is not guaranteed lock-free for every type, so this is the flag's type
    /// rather than an atomic bool.
    volatile std::sig_atomic_t g_QuitRequested = 0;

    /// Guards the installation itself, so that two threads calling Install() install once between them.
    std::atomic<bool> g_Installed {false};

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
        bool expected = false;
        if (!g_Installed.compare_exchange_strong(expected, true)) { return true; }

#ifdef _WIN32
        if (SetConsoleCtrlHandler(ConsoleHandler, TRUE) == 0) {
            g_Installed.store(false);
            return false;
        }
#else
        // misc-include-cleaner wants a header that directly "provides" sigaction/sigemptyset, but the only such
        // header is the deprecated <signal.h>, and modernize-deprecated-headers requires <csignal> instead. <csignal>
        // is what actually declares these POSIX extensions on every supported platform; the two checks disagree with
        // each other here; modernize-deprecated-headers is the one this file keeps, so misc-include-cleaner yields.
        // NOLINTBEGIN(misc-include-cleaner)
        struct sigaction action {};
        action.sa_handler = PosixHandler;
        sigemptyset(&action.sa_mask);
        // NOLINTEND(misc-include-cleaner)
        action.sa_flags = 0;  // No SA_RESTART: an interrupted wait should return so the caller can resume it itself.

        // NOLINTBEGIN(misc-include-cleaner)
        if (sigaction(SIGINT, &action, nullptr) != 0 || sigaction(SIGTERM, &action, nullptr) != 0) {
            // NOLINTEND(misc-include-cleaner)
            g_Installed.store(false);
            return false;
        }
#endif
        return true;
    }

    bool QuitRequested() { return g_QuitRequested != 0; }
}  // namespace Liara::Platform::Shutdown
