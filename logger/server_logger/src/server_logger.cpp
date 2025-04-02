#include "../include/server_logger.h"
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstring>  

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

server_logger::server_logger(
        const std::string& dest,
        const std::unordered_map<logger::severity, std::pair<std::string, bool>>& streams,
        const std::string& log_format)
        : _destination(dest), _streams(streams), _log_format(log_format)
{
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

server_logger::~server_logger() noexcept = default;

server_logger::server_logger(const server_logger& other)
    : _destination(other._destination), 
      _streams(other._streams),
      _log_format(other._log_format)
{
}

server_logger& server_logger::operator=(const server_logger& other)
{
    if (this != &other) {
        _destination = other._destination;
        _streams = other._streams;
        _log_format = other._log_format;
    }
    return *this;
}

std::string server_logger::format_log_message(const std::string& message, logger::severity severity) const
{
    std::string result = _log_format;
    
    
    size_t pos = 0;
    
    // %d - текущая дата
    while ((pos = result.find("%d", pos)) != std::string::npos) {
        result.replace(pos, 2, logger::current_date_to_string());
        pos += logger::current_date_to_string().length();
    }
    
    // %t - текущее время
    pos = 0;
    while ((pos = result.find("%t", pos)) != std::string::npos) {
        result.replace(pos, 2, logger::current_time_to_string());
        pos += logger::current_time_to_string().length();
    }
    
    // %s - строковое представление severity
    pos = 0;
    while ((pos = result.find("%s", pos)) != std::string::npos) {
        result.replace(pos, 2, logger::severity_to_string(severity));
        pos += logger::severity_to_string(severity).length();
    }
    
    // %m - логгируемое сообщение
    pos = 0;
    while ((pos = result.find("%m", pos)) != std::string::npos) {
        result.replace(pos, 2, message);
        pos += message.length();
    }
    
    // %p - идентификатор процесса (новый формат)
    pos = 0;
    while ((pos = result.find("%p", pos)) != std::string::npos) {
        std::string pid = std::to_string(inner_getpid());
        result.replace(pos, 2, pid);
        pos += pid.length();
    }
    
    return result;
}

logger& server_logger::log(const std::string& message, const logger::severity severity) &
{
    try {
    
        if (const auto it = _streams.find(severity); it == _streams.end() || !it->second.second) {
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
                throw std::runtime_error("Failed to connect to named pipe");
            }

            DWORD bytes_written;
            BOOL write_result = WriteFile(
                    pipe,
                    log_message.c_str(),
                    static_cast<DWORD>(log_message.size()),
                    &bytes_written,
                    nullptr);

            if (!write_result) {
                throw std::runtime_error("Failed to write to named pipe");
            }
        } catch (...) {
            if (pipe != INVALID_HANDLE_VALUE) {
                CloseHandle(pipe);
            }
            throw; 
        }
        
        if (pipe != INVALID_HANDLE_VALUE) {
            CloseHandle(pipe);
        }
    #else
        
        if (_destination.substr(0, 7) == "http://") {
        
            std::string url = _destination.substr(7); 
            std::string host;
            int port = 80;

            size_t colon_pos = url.find(':');
            if (colon_pos != std::string::npos) {
                host = url.substr(0, colon_pos);
                try {
                    port = std::stoi(url.substr(colon_pos + 1));
                } catch (const std::exception& e) {
                    throw std::runtime_error("Invalid port in URL: " + url);
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
                http_request += "Content-Type: application/json\r\n";
                http_request += "Content-Length: " + std::to_string(log_message.size()) + "\r\n";
                http_request += "\r\n" + log_message;

                
                ssize_t bytes_written = write(sockfd, http_request.c_str(), http_request.size());
                if (bytes_written < 0) {
                    throw std::runtime_error("Failed to write to socket");
                }
            } catch (...) {
                if (sockfd >= 0) {
                    close(sockfd);
                }
                throw;
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
                    throw std::runtime_error("Connection to server failed");
                }

                ssize_t bytes_written = write(sockfd, log_message.c_str(), log_message.size());
                if (bytes_written < 0) {
                    throw std::runtime_error("Failed to write to socket");
                }
            } catch (...) {
                if (sockfd >= 0) {
                    close(sockfd);
                }
                throw; 
            }
            
            if (sockfd >= 0) {
                close(sockfd);
            }
        }
    #endif
    } catch (const std::exception& e) {
        
        
        std::cerr << "Logging error: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown logging error occurred" << std::endl;
    }

    return *this;
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