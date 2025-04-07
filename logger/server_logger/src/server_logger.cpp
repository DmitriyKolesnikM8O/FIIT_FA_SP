#include <not_implemented.h>
#include <httplib.h>
#include "../include/server_logger.h"
#include <nlohmann/json.hpp>
#include <regex>
#include <fstream>
#include <stdexcept>
#include <utility>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

namespace {

    std::string get_current_date() {
        try {
            const auto now = std::chrono::system_clock::now();
            auto in_time = std::chrono::system_clock::to_time_t(now);
            std::tm tm_buf{};
            
            #ifdef _WIN32
            gmtime_s(&tm_buf, &in_time);
            #else
            if (gmtime_r(&in_time, &tm_buf) == nullptr) {
                throw std::runtime_error("Failed to convert time to GMT");
            }
            #endif
            
            std::stringstream ss;
            ss << std::put_time(&tm_buf, "%d.%m.%Y");
            return ss.str();
        } catch (const std::exception& e) {
            std::cerr << "Error getting date: " << e.what() << std::endl;
            return "[DATE ERROR]";
        }
    }

    std::string get_current_time() {
        try {
            const auto now = std::chrono::system_clock::now();
            auto in_time = std::chrono::system_clock::to_time_t(now);
            std::tm tm_buf{};
            
            #ifdef _WIN32
            gmtime_s(&tm_buf, &in_time);
            #else
            if (gmtime_r(&in_time, &tm_buf) == nullptr) {
                throw std::runtime_error("Failed to convert time to GMT");
            }
            #endif
            
            std::stringstream ss;
            ss << std::put_time(&tm_buf, "%H:%M:%S");
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count() % 1000;
            ss << "." << std::setfill('0') << std::setw(3) << ms;
            return ss.str();
        } catch (const std::exception& e) {
            std::cerr << "Error getting time: " << e.what() << std::endl;
            return "[TIME ERROR]";
        }
    }


    bool validate_format(const std::string& format) {

        return format.find('%') != std::string::npos;
    }

    bool validate_url(const std::string& url) {

        if (url.empty()) {
            return false;
        }
        

        return url.substr(0, 7) == "http://" || url.substr(0, 8) == "https://";
    }
    

    std::string convert_severity_for_server(const std::string& severity) {
        if (severity == "INFORMATION") {
            return "INFO";
        }
        if (severity == "WARNING") {
            return "WARN";
        }
        if (severity == "ERROR") {
            return "ERR";
        }
        if (severity == "CRITICAL") {
            return "CRIT";
        }
        if (severity == "TRACE") {
            return "TRC";
        }
        if (severity == "DEBUG") {
            return "DBG";
        }
        return severity;
    }
}

server_logger::~server_logger() noexcept = default;

logger& server_logger::log(
    const std::string &message,
    const logger::severity severity) &
{

    if (message.empty()) {
        std::cerr << "Warning: Empty log message" << std::endl;

    }

    try {

        std::string formatted = _format;
        std::map<std::string, std::string> replacements = {
            {"%d", get_current_date()},
            {"%t", get_current_time()},
            {"%s", severity_to_string(severity)},
            {"%m", message}
        };

        for (const auto& [pattern, replacement] : replacements) {
            formatted = std::regex_replace(formatted, std::regex(pattern), replacement);
        }


        std::string server_severity = convert_severity_for_server(severity_to_string(severity));


        nlohmann::json payload;
        try {
            payload = {
                {"process_id", inner_getpid()},
                {"level", server_severity},
                {"log_message", formatted},
                {"destinations", nlohmann::json::array()}
            };
        } catch (const nlohmann::json::exception& e) {
            std::cerr << "Error creating JSON payload: " << e.what() << std::endl;
            throw;
        }


        if (const auto it = _streams.find(severity); it != _streams.end()) {
            const auto& [path, is_console] = it->second;


            if (is_console) {
                payload["destinations"].push_back({{"output_type", "console"}});
                std::cout << formatted << std::endl;
            }


            if (!path.empty()) {
                payload["destinations"].push_back({
                    {"output_type", "file"},
                    {"file_path", path}
                });
            }
        }


        try {
            _client.set_connection_timeout(2);

            if (auto res = _client.Post("/log", payload.dump(), "application/json"); !res) {
                std::cerr << "HTTP error: " << httplib::to_string(res.error()) << std::endl;
            } else if (res->status != 200) {
                std::cerr << "Server error: Status " << res->status << ", Body: " << res->body << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception during HTTP request: " << e.what() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Logger error: " << e.what() << std::endl;

    }

    return *this;
}

server_logger::server_logger(const std::string& dest,
                             const std::string& format,
                             const std::unordered_map<logger::severity, std::pair<std::string, bool>> &streams)
    : _client(dest.c_str()),
      _destination(dest),
      _format(format),
      _streams(streams)
{

    if (!validate_url(_destination)) {
        std::cerr << "Warning: Invalid server URL format: " << _destination << std::endl;
    }

    if (!validate_format(_format)) {
        std::cerr << "Warning: Format string may not contain valid format specifiers" << std::endl;
    }


    _client.set_connection_timeout(2);
    _client.set_read_timeout(5);
    

    _client.set_default_headers({
        {"User-Agent", "ServerLogger/1.0"}
    });
}

int server_logger::inner_getpid()
{
#ifdef _WIN32
    return ::_getpid();
#else
    return getpid();
#endif
}

server_logger::server_logger(const server_logger &other)
    : _client(other._destination),
      _destination(other._destination),
      _format(other._format),
      _streams(other._streams)
{

    _client.set_connection_timeout(2);
    _client.set_read_timeout(5);
    _client.set_default_headers({
        {"User-Agent", "ServerLogger/1.0"}
    });
}

server_logger &server_logger::operator=(const server_logger &other)
{
    if (this != &other) {
        _destination = other._destination;
        _client = httplib::Client(_destination);
        _format = other._format;
        _streams = other._streams;
        

        _client.set_connection_timeout(2);
        _client.set_read_timeout(5);
        _client.set_default_headers({
            {"User-Agent", "ServerLogger/1.0"}
        });
    }
    return *this;
}

server_logger::server_logger(server_logger &&other) noexcept
    : _client(std::move(other._client)),
      _destination(std::move(other._destination)),
      _format(std::move(other._format)),
      _streams(std::move(other._streams))
{

}

server_logger &server_logger::operator=(server_logger &&other) noexcept
{
    if (this != &other) {
        _destination = std::move(other._destination);
        _client = std::move(other._client);
        _format = std::move(other._format);
        _streams = std::move(other._streams);
    }
    return *this;
}