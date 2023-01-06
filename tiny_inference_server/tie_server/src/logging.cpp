#include "logging.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <iostream>

namespace tie::server
{
void setup_loggers(const tie::server::server_config& config)
{
    std::vector<spdlog::sink_ptr> sinks;

    if (config.enable_console_logger)
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::from_str(config.console_logger_level));
        sinks.push_back(console_sink);
    }

    if (config.enable_file_logger)
    {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(config.log_file, true);
        file_sink->set_level(spdlog::level::from_str(config.file_logger_level));
        sinks.push_back(file_sink);
    }
    
    auto logger = std::make_shared<spdlog::logger>("TIE", std::begin(sinks), std::end(sinks));
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::info);
    
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
}

}