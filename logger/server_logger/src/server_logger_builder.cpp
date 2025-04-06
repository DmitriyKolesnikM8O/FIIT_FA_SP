#include <not_implemented.h>
#include <fstream>
#include "../include/server_logger_builder.h"
#include <nlohmann/json.hpp>
#include <set>

logger_builder& server_logger_builder::add_file_stream(
    std::string const &stream_file_path,
    logger::severity severity) &
{
    auto it = _output_streams.find(severity);
    if (it != _output_streams.end()) {

        it->second.first = stream_file_path;
    } else {

        _output_streams[severity] = {stream_file_path, false};
    }
    return *this;
}

logger_builder& server_logger_builder::add_console_stream(
    logger::severity severity) &
{
    auto it = _output_streams.find(severity);
    if (it != _output_streams.end()) {

        it->second.second = true;
    } else {

        _output_streams[severity] = {"", true};
    }
    return *this;
}

logger_builder& server_logger_builder::transform_with_configuration(
    std::string const &configuration_file_path,
    std::string const &configuration_path) &
{
    if (!std::filesystem::exists(configuration_file_path))
        throw std::runtime_error("Configuration file not found: " + configuration_file_path);

    nlohmann::json config;
    try {
        std::ifstream file(configuration_file_path);
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        file >> config;
    } catch (const std::exception& ex) {
        throw std::runtime_error("Failed to read config file: " + std::string(ex.what()));
    }

    nlohmann::json* section = &config;
    if (!configuration_path.empty()) {
        try {
            section = &config.at(nlohmann::json::json_pointer(configuration_path));
        } catch (...) {
            throw std::runtime_error("Configuration path not found: " + configuration_path);
        }
    }

    if (section->contains("destination"))
        set_destination(section->at("destination").get<std::string>());

    if (section->contains("format"))
        set_format(section->at("format"));

    if (section->contains("streams")) {
        const nlohmann::json& streams = section->at("streams");
        if (!streams.is_array())
            throw std::runtime_error("Streams must be an array");

        for (const auto& stream : streams) {
            const std::vector<std::string> required_fields = {"type", "severities"};
            for (const auto& field : required_fields)
                if (!stream.contains(field))
                    throw std::runtime_error("Missing field in stream: " + field);

            const std::string type = stream.at("type");
            if (type != "file" && type != "console")
                throw std::runtime_error("Invalid stream type: " + type);

            std::vector<logger::severity> severities;
            for (const auto& sev_str : stream.at("severities")) {
                severities.push_back(string_to_severity(sev_str));
            }

            if (type == "file") {
                if (!stream.contains("path")) {
                    throw std::runtime_error("File stream missing 'path'");
                }
                const std::string path = stream.at("path");
                for (auto sev : severities) {
                    add_file_stream(path, sev);
                }
            } else {
                for (auto sev : severities) {
                    add_console_stream(sev);
                }
            }
        }
    }

    return *this;
}

logger_builder& server_logger_builder::clear() &
{
    _destination = "http://127.0.0.1:9200";
    _output_streams.clear();
    return *this;
}

logger *server_logger_builder::build() const
{
    if (_destination.empty())
        throw std::logic_error("Destination address is not set");

    if (_output_streams.empty())
        throw std::logic_error("No output streams configured");

    try
    {
        return new server_logger(
            _destination,
            _format,
            _output_streams
        );
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Failed to create logger: " << ex.what() << std::endl;
        throw;
    }
    catch (...)
    {
        std::cerr << "Unknown error during logger creation" << std::endl;
        throw std::runtime_error("Unknown logger creation error");
    }
}

logger_builder& server_logger_builder::set_destination(const std::string& dest) &
{
    _destination = dest;
    return *this;
}

logger_builder& server_logger_builder::set_format(const std::string &format) &
{
    _format = format;
    return *this;
}