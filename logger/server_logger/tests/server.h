#ifndef MP_OS_SERVER_H
#define MP_OS_SERVER_H

#include <crow.h>
#include <unordered_map>
#include <logger.h>
#include <shared_mutex>
#include <thread>

class server
{
    crow::SimpleApp app;
    std::unordered_map<int, std::unordered_map<logger::severity, std::pair<std::string, bool>>> _streams;
    std::shared_mutex _mut;
    std::thread server_thread;

public:
    explicit server(uint16_t port = 9200);
    void stop();

    server(const server&) = delete;
    server& operator=(const server&) = delete;
    server(server&&) noexcept = delete;
    server& operator=(server&&) noexcept = delete;
    ~server() noexcept;
};

#endif //MP_OS_SERVER_H