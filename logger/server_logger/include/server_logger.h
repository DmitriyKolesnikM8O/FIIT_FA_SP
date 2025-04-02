#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H

#include <logger.h>
#include <unordered_map>
// #include <httplib.h>

class server_logger_builder;
class server_logger final:
    public logger
{
    std::string _destination;
    std::unordered_map<logger::severity, std::pair<std::string, bool>> _streams;
    std::string _log_format;

    // httplib::Client _client;

    server_logger(const std::string& dest, 
                 const std::unordered_map<logger::severity, std::pair<std::string, bool>>& streams,
                 const std::string& log_format);

    friend server_logger_builder;

    static int inner_getpid();
public:

    server_logger(server_logger const &other);

    server_logger &operator=(server_logger const &other);

    server_logger(server_logger &&other) noexcept;

    server_logger &operator=(server_logger &&other) noexcept;

    ~server_logger() noexcept final;

public:

    [[nodiscard]] logger& log(
        const std::string &message,
        logger::severity severity) & override;

    
    std::string format_log_message(const std::string& message, logger::severity severity) const;

};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H