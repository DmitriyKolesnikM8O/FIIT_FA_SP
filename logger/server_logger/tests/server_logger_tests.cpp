#include <crow.h>
#include <server_logger_builder.h>
#include <thread>
#include <chrono>
#include <iostream>


class test_server {
    crow::SimpleApp app;
    std::thread server_thread;
    bool running = true;

public:
    explicit test_server(uint16_t port = 9200) {
        
        CROW_ROUTE(app, "/log").methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            std::cout << "Received log: " << req.body << std::endl;
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

        std::cout << "Test server started on port " << port << std::endl;
    }

    ~test_server() {
        
        app.stop();
        if (server_thread.joinable()) {
            server_thread.join();
        }
        std::cout << "Test server stopped" << std::endl;
    }
};

int main()
{
    
    test_server server(9200);

    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Starting logger tests..." << std::endl;

    server_logger_builder builder;

    builder.add_file_stream("a.txt", logger::severity::trace)
           .add_file_stream("b.txt", logger::severity::debug)
           .add_console_stream(logger::severity::trace)
           .add_file_stream("a.txt", logger::severity::information);

    std::unique_ptr<logger> log(builder.build());
    std::cout << "Logger created" << std::endl;

    log->trace("good").debug("debug");

    log->trace("IT is a very long strange message !!!!!!!!!!%%%%%%%%\tzdtjhdjh")
        .information("bfldknbpxjxjvpxvjbpzjbpsjbpsjkgbpsejegpsjpegesjpvbejpvjzepvgjs");

    std::cout << "Logs sent" << std::endl;

    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "Test completed successfully" << std::endl;
    return 0;
}