#include "../include/server_logger.h"
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstring>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

std::string server_logger::format_log_message(const std::string& message, const logger::severity severity) const
{
    const auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    now_c += 10800;
    std::string date(30, '\0');
    std::strftime(date.data(), date.size(), "%d.%m.%Y", std::gmtime(&now_c));
    date.resize(std::strlen(date.c_str()));

    std::string time(30, '\0');
    std::strftime(time.data(), time.size(), "%H:%M:%S", std::gmtime(&now_c));
    time.resize(std::strlen(time.c_str()));

    std::string severity_str;
    switch (severity) {
        case logger::severity::trace: severity_str = "TRACE"; break;
        case logger::severity::debug: severity_str = "DEBUG"; break;
        case logger::severity::information: severity_str = "INFORMATION"; break;
        case logger::severity::warning: severity_str = "WARNING"; break;
        case logger::severity::error: severity_str = "ERROR"; break;
        case logger::severity::critical: severity_str = "CRITICAL"; break;
        default: severity_str = "UNKNOWN"; break;
    }

    const int pid = inner_getpid();
    std::string result;
    for (size_t i = 0; i < _log_format.size(); ++i) {
        if (_log_format[i] == '%' && i + 1 < _log_format.size()) {
            switch (_log_format[i + 1]) {
                case 'd': result += date; break;
                case 't': result += time; break;
                case 's': result += severity_str; break;
                case 'm': result += message; break;
                default: result += _log_format[i]; result += _log_format[i + 1]; break;
            }
            ++i;
        } else {
            result += _log_format[i];
        }
    }
    result += " (PID: " + std::to_string(pid) + ")";
    return result;
}

server_logger::server_logger(
        const std::string& dest,
        const std::unordered_map<logger::severity, std::pair<std::string, bool>>& streams,
        const std::string& log_format)
        : _destination(dest), _streams(streams), _log_format(log_format)
{
    std::cerr << "server_logger constructed with destination: " << _destination << std::endl;
    for (const auto& [severity, stream] : _streams) {
        std::cerr << "Stream for severity " << static_cast<int>(severity) << ": " << stream.first << ", enabled: " << stream.second << std::endl;
    }

#ifndef _WIN32
    if (dest.substr(0, 7) == "http://") {
        return;
    }

    struct stat buffer;
    if (stat(dest.c_str(), &buffer) != 0) {
        throw std::runtime_error("Server socket not available");
    }
#endif
}

server_logger::~server_logger() noexcept
{
}

logger& server_logger::log(const std::string& message, const logger::severity severity) &
{
    try {
        if (const auto it = _streams.find(severity); it == _streams.end() || !it->second.second) {
            std::cerr << "No stream found or stream disabled for severity: " << static_cast<int>(severity) << std::endl;
            return *this;
        }

        std::string formatted_message = format_log_message(message, severity);
        std::ostringstream log_stream;
        log_stream << formatted_message << "\n";
        const std::string log_message = log_stream.str();

#ifdef _WIN32
        HANDLE pipe = INVALID_HANDLE_VALUE;
        try {
            pipe = CreateFileA(
                _destination.c_str(),
                GENERIC_WRITE,
                0,
                nullptr,
                OPEN_EXISTING,
                0,
                nullptr);

            if (pipe == INVALID_HANDLE_VALUE) {
                throw std::runtime_error("Failed to open pipe: " + _destination);
            }

            DWORD bytes_written;
            if (!WriteFile(pipe, log_message.c_str(), log_message.size(), &bytes_written, nullptr)) {
                throw std::runtime_error("Failed to write to pipe");
            }
        } catch (const std::exception& e) {
            std::cerr << "Logging error: " << e.what() << std::endl;
        }

        if (pipe != INVALID_HANDLE_VALUE) {
            CloseHandle(pipe);
        }
#else
        if (_destination.substr(0, 7) == "http://") {
            std::string url = _destination.substr(7);
            std::string host;
            int port = 80;

            if (size_t colon_pos = url.find(':'); colon_pos != std::string::npos) {
                host = url.substr(0, colon_pos);
                std::string port_str = url.substr(colon_pos + 1);
                if (port_str.empty() || !std::all_of(port_str.begin(), port_str.end(), ::isdigit)) {
                    throw std::runtime_error("Invalid port in URL: " + url);
                }
                port = std::stoi(port_str);
                if (port <= 0 || port > 65535) {
                    throw std::runtime_error("Port out of range: " + std::to_string(port));
                }
            } else {
                host = url;
            }

            int sockfd = -1;
            try {
                sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd < 0) {
                    throw std::runtime_error("Socket creation failed");
                }

                struct sockaddr_in server_addr{};
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(port);

                if (host == "localhost") {
                    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
                } else if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
                    throw std::runtime_error("Invalid address: " + host);
                }

                if (connect(sockfd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
                    throw std::runtime_error("Connection to server failed: " + std::string(strerror(errno)) +
                                          " (host: " + host + ", port: " + std::to_string(port) + ")");
                }

                std::string http_request = "POST /log HTTP/1.1\r\n";
                http_request += "Host: " + host + "\r\n";
                http_request += "Content-Type: text/plain\r\n";
                http_request += "Content-Length: " + std::to_string(log_message.size()) + "\r\n";
                http_request += "\r\n" + log_message;

                if (ssize_t bytes_written = write(sockfd, http_request.c_str(), http_request.size()); bytes_written < 0) {
                    throw std::runtime_error("Failed to write to socket");
                }

                char buffer[1024];
                ssize_t bytes_read = read(sockfd, buffer, sizeof(buffer) - 1);
                if (bytes_read < 0) {
                    throw std::runtime_error("Failed to read response from server");
                }
                buffer[bytes_read] = '\0';
            } catch (const std::exception& e) {
                std::cerr << "Logging error: " << e.what() << std::endl;
            }

            if (sockfd >= 0) {
                close(sockfd);
            }
        } else {
            int sockfd = -1;
            try {
                sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
                if (sockfd < 0) {
                    throw std::runtime_error("Socket creation failed");
                }

                struct sockaddr_un server_addr{};
                server_addr.sun_family = AF_UNIX;
                strncpy(server_addr.sun_path, _destination.c_str(), sizeof(server_addr.sun_path) - 1);

                if (connect(sockfd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
                    throw std::runtime_error("Connection to server failed: " + _destination);
                }

                if (ssize_t bytes_written = write(sockfd, log_message.c_str(), log_message.size()); bytes_written < 0) {
                    throw std::runtime_error("Failed to write to socket");
                }
            } catch (const std::exception& e) {
                std::cerr << "Logging error: " << e.what() << std::endl;
            }

            if (sockfd >= 0) {
                close(sockfd);
            }
        }
#endif

        std::string stream_name = _streams.at(severity).first;
        std::cerr << "Attempting to write to stream: " << stream_name << " for severity: " << static_cast<int>(severity) << std::endl;

        if (stream_name == "console") {
            std::cout << log_message;
        } else {
            std::ofstream file(stream_name, std::ios::out | std::ios::app);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open file: " + stream_name);
            }
            file << log_message;
            file.close();
        }

        return *this;
    } catch (const std::exception& e) {
        std::cerr << "Logging error: " << e.what() << std::endl;
        return *this;
    }
}

server_logger::server_logger(server_logger&& other) noexcept
        : _destination(std::move(other._destination)),
          _streams(std::move(other._streams)),
          _log_format(std::move(other._log_format))
{
}

server_logger& server_logger::operator=(server_logger&& other) noexcept
{
    if (this != &other) {
        _destination = std::move(other._destination);
        _streams = std::move(other._streams);
        _log_format = std::move(other._log_format);
    }
    return *this;
}

int server_logger::inner_getpid()
{
#ifdef _WIN32
    return ::_getpid();
#else
    return ::getpid();
#endif
}