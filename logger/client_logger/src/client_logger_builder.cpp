#include <filesystem>
#include <utility>
#include "../include/client_logger_builder.h"

using namespace nlohmann;


logger_builder &client_logger_builder::add_file_stream(const std::string &stream_file_path, logger::severity severity) &
{
    if (stream_file_path.empty())
    {
        throw std::invalid_argument("File path cannot be empty");
    }

    auto &severity_entry = _output_streams[severity];
    severity_entry.first.emplace_front(client_logger::refcounted_stream(stream_file_path));
    return *this;
}


logger_builder &client_logger_builder::add_console_stream(logger::severity severity) &
{
    _output_streams[severity].second = true;
    return *this;
}


logger_builder &client_logger_builder::clear() &
{
    _output_streams.clear();
    _format = "%m";
    return *this;
}


logger_builder &client_logger_builder::set_format(const std::string &format) &
{
    if (format.empty())
    {
        throw std::invalid_argument("Format string cannot be empty");
    }
    _format = format;
    return *this;
}


logger_builder &client_logger_builder::set_destination(const std::string &format) &
{
    throw std::logic_error("set_destination is not implemented as per requirements");
}


void client_logger_builder::parse_severity(logger::severity sev, nlohmann::json &j)
{
    if (j.contains("files"))
    {
        for (const auto &file : j["files"])
        {
            std::string file_path = file.get<std::string>();
            if (!file_path.empty())
            {
                _output_streams[sev].first.emplace_front(client_logger::refcounted_stream(file_path));
            }
        }
    }
    if (j.contains("console") && j["console"].get<bool>())
    {
        _output_streams[sev].second = true;
    }
}


logger_builder &client_logger_builder::transform_with_configuration(const std::string &configuration_file_path, const std::string &configuration_path) &
{
    if (!std::filesystem::exists(configuration_file_path))
    {
        return *this;
    }

    std::ifstream file(configuration_file_path);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open configuration file: " + configuration_file_path);
    }

    nlohmann::json config;
    file >> config;
    file.close();

    auto path_parts = configuration_path;
    nlohmann::json *current = &config;

    if (current->contains(path_parts))
    {
        current = &(*current)[path_parts];
    }
    else
    {
        return *this;
    }

    if (current->contains("format"))
    {
        _format = (*current)["format"].get<std::string>();
    }

    if (current->contains("severity"))
    {
        auto &severity_config = (*current)["severity"];
        for (auto &[sev_str, sev_config] : severity_config.items())
        {
            logger::severity sev = logger_builder::string_to_severity(sev_str);
            parse_severity(sev, sev_config);
        }
    }

    return *this;
}


logger *client_logger_builder::build() const
{
    return new client_logger(_output_streams, _format);
}