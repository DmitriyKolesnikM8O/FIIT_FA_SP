#include "../include/server_logger.h"
#include <sys/stat.h>
#include <fstream>
#include <chrono>
#include <cstring>  // Для работы с strncpy

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>  // Для функций работы с IP-адресами
#include <netinet/in.h>  // Для структуры sockaddr_in
#endif

server_logger::server_logger(
        const std::string& dest,
        const std::unordered_map<logger::severity, std::pair<std::string, bool>>& streams)
        : _destination(dest), _streams(streams)
{
#ifndef _WIN32
    // Проверяем, начинается ли dest с "http://"
    if (dest.substr(0, 7) == "http://") {
        // Если это HTTP URL, то проверок файлов делать не надо
        return;
    }

    // Для UNIX сокетов проверяем существование файла
    struct stat buffer;
    if (stat(dest.c_str(), &buffer) != 0) {
        throw std::runtime_error("Server socket not available");
    }
#endif
}

server_logger::~server_logger() noexcept = default;

logger& server_logger::log(const std::string& message, logger::severity severity) &
{
    auto it = _streams.find(severity);
    if (it == _streams.end() || !it->second.second) {
        return *this;
    }

    std::ostringstream log_stream;
    log_stream << "[" << current_datetime_to_string() << "] "
               << "[" << severity_to_string(severity) << "] "
               << "[PID:" << inner_getpid() << "] "
               << message << "\n";

    const std::string log_message = log_stream.str();

#ifdef _WIN32
    // Windows код без изменений
    HANDLE pipe = CreateFileA(
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
        CloseHandle(pipe);
        throw std::runtime_error("Failed to write to named pipe");
    }

    CloseHandle(pipe);
#else
    // Для Linux проверяем, HTTP это или UNIX сокет
    if (_destination.substr(0, 7) == "http://") {
        // Извлекаем хост и порт из URL
        std::string url = _destination.substr(7); // Убираем "http://"
        std::string host;
        int port = 80;

        size_t colon_pos = url.find(':');
        if (colon_pos != std::string::npos) {
            host = url.substr(0, colon_pos);
            // Добавляем проверку, что после двоеточия есть число
            try {
                port = std::stoi(url.substr(colon_pos + 1));
            } catch (const std::exception& e) {
                throw std::runtime_error("Invalid port in URL: " + url);
            }
        } else {
            host = url;
        }

        // Используем TCP сокет для HTTP
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            throw std::runtime_error("Socket creation failed");
        }

        struct sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        // Преобразуем IP в двоичный формат
        if (host == "localhost") {
            // Для localhost используем 127.0.0.1
            server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        } else {
            // Для других адресов используем inet_pton
            if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
                close(sockfd);
                throw std::runtime_error("Invalid address: " + host);
            }
        }

        // Пробуем подключиться
        if (connect(sockfd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
            close(sockfd);
            throw std::runtime_error("Connection to server failed: " + std::string(strerror(errno)) +
                                    " (host: " + host + ", port: " + std::to_string(port) + ")");
        }

        // Формируем HTTP запрос
        std::string http_request = "POST /log HTTP/1.1\r\n";
        http_request += "Host: " + host + "\r\n";
        http_request += "Content-Type: application/json\r\n";
        http_request += "Content-Length: " + std::to_string(log_message.size()) + "\r\n";
        http_request += "\r\n" + log_message;

        // Отправляем запрос
        ssize_t bytes_written = write(sockfd, http_request.c_str(), http_request.size());
        if (bytes_written < 0) {
            close(sockfd);
            throw std::runtime_error("Failed to write to socket");
        }

        close(sockfd);
    } else {
        // Используем UNIX domain сокет (код без изменений)
        int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sockfd < 0) {
            throw std::runtime_error("Socket creation failed");
        }

        struct sockaddr_un server_addr{};
        server_addr.sun_family = AF_UNIX;
        strncpy(server_addr.sun_path, _destination.c_str(), sizeof(server_addr.sun_path) - 1);

        if (connect(sockfd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
            close(sockfd);
            throw std::runtime_error("Connection to server failed");
        }

        ssize_t bytes_written = write(sockfd, log_message.c_str(), log_message.size());
        if (bytes_written < 0) {
            close(sockfd);
            throw std::runtime_error("Failed to write to socket");
        }

        close(sockfd);
    }
#endif

    return *this;
}

server_logger::server_logger(server_logger&& other) noexcept
        : _destination(std::move(other._destination)),
          _streams(std::move(other._streams))
{
}

server_logger& server_logger::operator=(server_logger&& other) noexcept
{
    if (this != &other) {
        _destination = std::move(other._destination);
        _streams = std::move(other._streams);
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