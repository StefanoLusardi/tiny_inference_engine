#pragma once

namespace xyz::engine
{
bool shutdown_engine() noexcept;
void signal_handler(int signal) noexcept;
void install_shutdown_signal_handlers() noexcept;
}