#include <not_implemented.h>
#include "../include/server_logger_builder.h"

logger_builder& server_logger_builder::add_file_stream(
        std::string const &stream_file_path,
        logger::severity severity) &
{
    _output_streams[severity] = {stream_file_path, true};
    return *this;
}

logger_builder& server_logger_builder::add_console_stream(
        logger::severity severity) &
{
    _output_streams[severity] = {"", true};  
    return *this;
}


logger_builder& server_logger_builder::transform_with_configuration(
        std::string const &configuration_file_path,
        std::string const &configuration_path) &
{
    std::ifstream config_file(configuration_file_path);
    if (!config_file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + configuration_file_path);
    }


    std::string json_str((std::istreambuf_iterator<char>(config_file)), std::istreambuf_iterator<char>());
    if (json_str.empty()) {
        throw std::runtime_error("Config file is empty!");
    }


    nlohmann::json root;
    try {
        root = nlohmann::json::parse(json_str);
    } catch (const std::exception &e) {
        throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
    }


    if (!root.contains(configuration_path)) {
        throw std::runtime_error("Configuration path not found in JSON: " + configuration_path);
    }

    nlohmann::json config = root[configuration_path];
    if (!config.is_object()) {
        throw std::runtime_error("Configuration path must be a JSON object!");
    }


    if (config.contains("destination")) {
        _destination = config["destination"].get<std::string>();
    }


    if (config.contains("format")) {
        _log_format = config["format"].get<std::string>();
    }

    if (config.contains("streams")) {
        for (const auto& stream : config["streams"]) {
            std::string type = stream["type"].get<std::string>();
            logger::severity severity = string_to_severity(stream["severity"].get<std::string>());

            if (type == "file") {
                add_file_stream(stream["path"].get<std::string>(), severity);
            } else if (type == "console") {
                add_console_stream(severity);
            }
        }
    }

    return *this;
}



logger_builder& server_logger_builder::clear() &
{
    _output_streams.clear();
    _destination = DEFAULT_DESTINATION; 
    _log_format = DEFAULT_FORMAT; 
    return *this;
}

logger *server_logger_builder::build() const
{
    return new server_logger(_destination, _output_streams, _log_format);
}

logger_builder& server_logger_builder::set_destination(const std::string& dest) &
{
    if (dest.empty()) {
        throw std::invalid_argument("Destination cannot be empty");
    }
    _destination = dest;
    return *this;
}

logger_builder& server_logger_builder::set_format(const std::string& format) &
{
    if (format.empty()) {
        throw std::invalid_argument("Format string cannot be empty");
    }
    _log_format = format;
    return *this;
}
