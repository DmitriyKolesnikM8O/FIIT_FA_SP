#include <not_implemented.h>
#include <httplib.h>
#include "../include/server_logger.h"
#include <nlohmann/json.hpp>
#include <regex>
#include <fstream>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

static std::string get_current_date() {
    auto now = std::chrono::system_clock::now();
    auto in_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    gmtime_r(&in_time, &tm_buf);
    std::stringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%d");
    return ss.str();
}

static std::string get_current_time() {
    auto now = std::chrono::system_clock::now();
    auto in_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    gmtime_r(&in_time, &tm_buf);
    std::stringstream ss;
    ss << std::put_time(&tm_buf, "%H:%M:%S");
    return ss.str();
}

server_logger::~server_logger() noexcept = default;

logger& server_logger::log(
    const std::string &text,
    logger::severity severity) &
{

    std::string formatted = _format;

    formatted = std::regex_replace(formatted, std::regex("%d"), get_current_date());
    formatted = std::regex_replace(formatted, std::regex("%t"), get_current_time());
    formatted = std::regex_replace(formatted, std::regex("%s"), severity_to_string(severity));
    formatted = std::regex_replace(formatted, std::regex("%m"), text);


    nlohmann::json payload = {
        {"pid", inner_getpid()},
        {"severity", severity_to_string(severity)},
        {"message", formatted},
        {"streams", nlohmann::json::array()}
    };


    if(auto it = _streams.find(severity); it != _streams.end())
    {
        const auto& [path, is_console] = it->second;

        if(is_console) {
            payload["streams"].push_back({{"type", "console"}});

            std::cout << formatted << std::endl;
        }

        if(!path.empty()) {
            payload["streams"].push_back({
                {"type", "file"},
                {"path", path}
            });
            

            std::ofstream file(path, std::ios::app);
            if(file.is_open()) {
                file << formatted << std::endl;
                file.close();
            } else {
                std::cerr << "Failed to open log file: " << path << std::endl;
            }
        }
    }

    _client.set_connection_timeout(2);
    auto res = _client.Post("/log", payload.dump(), "application/json");

    return *this;
}

server_logger::server_logger(const std::string& dest,
                             const std::string& format,
                             const std::unordered_map<logger::severity, std::pair<std::string, bool>> &streams)
                              : _client(dest.c_str()),
                                _destination(std::move(dest)),
                                _format(std::move(format)),
                                _streams(std::move(streams))
{
    _client.set_connection_timeout(2);
    _client.set_read_timeout(5);
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
    _destination(std::move(other._destination)),
    _format(other._format),
    _streams(other._streams)
{}

server_logger &server_logger::operator=(const server_logger &other)
{
    if (this != &other) {
        _destination = other._destination;
        _client = httplib::Client(_destination);
        _format = other._format;
        _streams = other._streams;
    }
    return *this;
}

server_logger::server_logger(server_logger &&other) noexcept
    : _client(std::move(other._client)),
    _destination(std::move(other._destination)),
    _format(std::move(other._format)),
    _streams(std::move(other._streams))
{}

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