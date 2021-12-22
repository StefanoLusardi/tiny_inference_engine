#include "shutdown.hpp"
#include <csignal>

namespace xyz::engine
{
volatile sig_atomic_t shutdown_signal = 0;

bool shutdown_engine() noexcept
{
    return shutdown_signal != 0;
}

void signal_handler(int signal) noexcept
{
    shutdown_signal = signal;
}

void install_shutdown_signal_handlers() noexcept
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGILL, signal_handler);
    std::signal(SIGABRT, signal_handler);
    std::signal(SIGSEGV, signal_handler);
    std::signal(SIGTERM, signal_handler);
}

}