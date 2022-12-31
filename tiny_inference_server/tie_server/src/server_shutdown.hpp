#pragma once

namespace tie::server
{
bool shutdown_server() noexcept;
void signal_handler(int signal) noexcept;
void install_shutdown_signal_handlers() noexcept;
void reset_shutdown_signal_handlers() noexcept;
}