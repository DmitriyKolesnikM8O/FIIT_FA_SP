#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H

#define CPPHTTPLIB_NO_COMPRESSION
#include <logger.h>
#include <unordered_map>
#include <httplib.h>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <map>

class server_logger_builder;
class server_logger final:
    public logger
{
private:
    httplib::Client _client;
    std::string _destination;
    std::string _format;
    std::unordered_map<logger::severity, std::pair<std::string, bool>> _streams;

    
    server_logger(const std::string& dest, const std::string& format, 
                  const std::unordered_map<logger::severity, std::pair<std::string, bool>>& streams);

    friend server_logger_builder;

    
    static int inner_getpid();

public:
    
    ~server_logger() noexcept final;

    
    server_logger(server_logger const &other);

    
    server_logger &operator=(server_logger const &other);

    
    server_logger(server_logger &&other) noexcept;

    
    server_logger &operator=(server_logger &&other) noexcept;

    
    [[nodiscard]] logger& log(
        const std::string &message,
        logger::severity severity) & override;
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H