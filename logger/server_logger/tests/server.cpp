//
// Created by Des Caldnd on 3/27/2024.
//

#include "server.h"
#include <logger_builder.h>
#include <fstream>
#include <iostream>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>

using json = nlohmann::json;

server::server(uint16_t port)
{

    CROW_ROUTE(app, "/log")
        .methods("POST"_method)
        ([this](const crow::request& req) {
            std::lock_guard<std::mutex> lock(_mut);

            try {
                auto data = json::parse(req.body);

                std::cout << "=== Received full JSON ===" << std::endl;
                std::cout << data.dump(2) << std::endl;
                std::cout << "=== End JSON ===" << std::endl;


                int pid = data["pid"];
                std::string severity_str = data["severity"];
                std::string message = data["message"];
                auto streams = data["streams"];


                logger::severity sev;
                if (severity_str == "TRACE") sev = logger::severity::trace;
                else if (severity_str == "DEBUG") sev = logger::severity::debug;
                else if (severity_str == "INFO") sev = logger::severity::information;
                else if (severity_str == "WARNING") sev = logger::severity::warning;
                else if (severity_str == "ERROR") sev = logger::severity::error;
                else if (severity_str == "CRITICAL") sev = logger::severity::critical;
                else return crow::response(400, "Invalid severity");


                for (const auto& stream : streams)
                {
                    std::string type = stream["type"];
                    if (type == "file")
                    {

                         std::string path = stream["path"];
                         std::ofstream file(path, std::ios::app);
                         if (file) file << message << "\n";
                    }
                    else if (type == "console")
                    {
                        std::cout << message << std::endl;
                    }
                }

                return crow::response(200);
            }
            catch (const json::exception& e)
            {
                return crow::response(400, "JSON parse error: " + std::string(e.what()));
            }
            catch (const std::exception& e)
            {
                return crow::response(500, "Server error: " + std::string(e.what()));
            }
        });


    app.port(port).run();
}