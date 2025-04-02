#include "server.h"
#include <logger_builder.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

server::server(uint16_t port)
{
    
    CROW_ROUTE(app, "/log").methods(crow::HTTPMethod::POST)
    ([this](const crow::request& req) {
        std::cout << "Server received log: " << req.body << std::endl;
        return crow::response(200, "Log received");
    });

    
    app.bindaddr("0.0.0.0").port(port);

    
    server_thread = std::thread([this]() {
        try {
            app.run();
        } catch(const std::exception& e) {
            std::cerr << "Server error: " << e.what() << std::endl;
        }
    });

    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "Server started on port " << port << std::endl;
}

server::~server() noexcept
{
    std::cout << "Stopping server..." << std::endl;

    
    app.stop();
    running = false;

    
    if (server_thread.joinable()) {
        server_thread.join();
    }

    std::cout << "Server stopped" << std::endl;
}