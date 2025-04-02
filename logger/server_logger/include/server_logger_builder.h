#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_BUILDER_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_BUILDER_H

#include <logger_builder.h>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "server_logger.h"

class server_logger_builder final:
    public logger_builder
{
    static constexpr const char* DEFAULT_DESTINATION = "http://localhost:9200";
    static constexpr const char* DEFAULT_FORMAT = "%d %t [%s] %m";

    std::string _destination;

    std::unordered_map<logger::severity ,std::pair<std::string, bool>> _output_streams;
    std::string _log_format;

public:

    server_logger_builder() : _destination(DEFAULT_DESTINATION), _log_format(DEFAULT_FORMAT) {}

private:
    

public:

    logger_builder& add_file_stream(
        std::string const &stream_file_path,
        logger::severity severity) & override;

    logger_builder& add_console_stream(
        logger::severity severity) & override;

    logger_builder& transform_with_configuration(
        std::string const &configuration_file_path,
        std::string const &configuration_path) & override;

    logger_builder& set_destination(const std::string& dest) & override;

    logger_builder& clear() & override;

    logger_builder& set_format(const std::string& format) & override;

    [[nodiscard]] logger *build() const override;

};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_BUILDER_H