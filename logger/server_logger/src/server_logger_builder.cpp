#include <not_implemented.h>
#include <fstream>
#include "../include/server_logger_builder.h"
#include <nlohmann/json.hpp>
#include <set>
#include <stdexcept>
#include <regex>

namespace {

    bool is_valid_url(const std::string& url) {

        if (url.empty()) {
            return false;
        }
        

        if (url.substr(0, 7) != "http://" && url.substr(0, 8) != "https://") {
            return false;
        }
        

        return true;
    }


    bool is_valid_format(const std::string& format) {

        bool has_date = format.find("%d") != std::string::npos;
        bool has_time = format.find("%t") != std::string::npos;
        bool has_severity = format.find("%s") != std::string::npos;
        const bool has_message = format.find("%m") != std::string::npos;
        

        return has_message;
    }
}

logger_builder& server_logger_builder::add_file_stream(
    std::string const &stream_file_path,
    const logger::severity severity) &
{
    try {

        if (stream_file_path.empty()) {
            throw std::invalid_argument("File path cannot be empty");
        }


        {
            if (std::ofstream test_file(stream_file_path, std::ios::app); !test_file.is_open()) {
                throw std::runtime_error("Cannot open file: " + stream_file_path);
            }
        }

        if (const auto it = _output_streams.find(severity); it != _output_streams.end()) {
            it->second.first = stream_file_path;
        } else {
            _output_streams[severity] = {stream_file_path, false};
        }
        return *this;
    } catch (const std::exception& e) {
        std::cerr << "Error adding file stream: " << e.what() << std::endl;

        return *this;
    }
}

logger_builder& server_logger_builder::add_console_stream(
    const logger::severity severity) &
{

    if (const auto it = _output_streams.find(severity); it != _output_streams.end()) {
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
    try {
        if (!std::filesystem::exists(configuration_file_path)) {
            throw std::runtime_error("Configuration file not found: " + configuration_file_path);
        }

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

        if (section->contains("destination")) {
            auto dest = section->at("destination").get<std::string>();
            if (!is_valid_url(dest)) {
                std::cerr << "Warning: Invalid URL format in config: " << dest << std::endl;
            }
            set_destination(dest);
        }

        if (section->contains("format")) {
            std::string format = section->at("format");
            if (!is_valid_format(format)) {
                std::cerr << "Warning: Format in config might be invalid: " << format << std::endl;
            }
            set_format(format);
        }

        if (section->contains("streams")) {
            const nlohmann::json& streams = section->at("streams");
            if (!streams.is_array()) {
                throw std::runtime_error("Streams must be an array");
            }

            for (const auto& stream : streams) {
                for (const std::vector<std::string> required_fields = {"type", "severities"}; const auto& field : required_fields) {
                    if (!stream.contains(field)) {
                        throw std::runtime_error("Missing field in stream: " + field);
                    }
                }

                const std::string type = stream.at("type");
                if (type != "file" && type != "console") {
                    throw std::runtime_error("Invalid stream type: " + type);
                }

                std::vector<logger::severity> severities;
                for (const auto& sev_str : stream.at("severities")) {
                    try {
                        severities.push_back(string_to_severity(sev_str));
                    } catch (const std::exception& e) {
                        std::cerr << "Warning: Invalid severity in config: " << sev_str << std::endl;
                        continue;
                    }
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
    } catch (const std::exception& e) {
        std::cerr << "Error in transform_with_configuration: " << e.what() << std::endl;

        return *this;
    }
}

logger_builder& server_logger_builder::clear() &
{
    _destination = "http://127.0.0.1:9200";
    _format = "%d %t %s %m";
    _output_streams.clear();
    return *this;
}

logger *server_logger_builder::build() const
{
    try {

        if (_destination.empty()) {
            throw std::logic_error("Destination address is not set");
        }
        


        if (_output_streams.empty()) {
            throw std::logic_error("No output streams configured");
        }

        if (_format.empty()) {
            throw std::logic_error("Format string is empty");
        }

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
    if (dest.empty()) {
        std::cerr << "Warning: Empty destination provided, using default" << std::endl;
        return *this;
    }
    
    if (!is_valid_url(dest)) {
        std::cerr << "Warning: Invalid URL format: " << dest << std::endl;
    }
    
    _destination = dest;
    return *this;
}

logger_builder& server_logger_builder::set_format(const std::string &format) &
{
    if (format.empty()) {
        std::cerr << "Warning: Empty format provided, using default" << std::endl;
        return *this;
    }
    
    if (!is_valid_format(format)) {
        std::cerr << "Warning: Format might not contain all required placeholders" << std::endl;
    }
    
    _format = format;
    return *this;
}