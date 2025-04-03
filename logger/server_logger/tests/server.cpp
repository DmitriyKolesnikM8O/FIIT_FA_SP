#include "server.h"
#include <iostream>
#include <thread>

server::server(uint16_t port)
{
    CROW_ROUTE(app, "/log").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        std::cout << "Received log: " << req.body << std::endl;
        return crow::response(200);
    });


    server_thread = std::thread([this, port]() {
        app.port(port).multithreaded().run();
    });


    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void server::stop()
{
    app.stop();
    if (server_thread.joinable()) {
        server_thread.join();
    }
}

server::~server() noexcept
{
    stop();
}