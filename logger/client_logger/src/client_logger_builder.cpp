#include <filesystem>
#include <utility>
#include <not_implemented.h>
#include "../include/client_logger_builder.h"
#include <not_implemented.h>

using namespace nlohmann;

logger_builder& client_logger_builder::add_file_stream(
    const std::string &stream_file_path,
    logger::severity severity) &
{
    if (stream_file_path.empty())
    {
        throw std::invalid_argument("File path cannot be empty");
    }
    
    std::filesystem::path path(stream_file_path);
    if (!path.parent_path().empty() && !std::filesystem::exists(path.parent_path()))
    {
        std::filesystem::create_directories(path.parent_path());
    }

    auto &severity_entry = _output_streams[severity];
    
    // Проверка на дубликаты путей файлов
    /*
    for (const auto &stream : severity_entry.first)
    {
        if (stream._stream.first == stream_file_path)
        {
            // Путь файла уже существует для данного уровня логирования, не добавляем дубликат
            return *this;
        }
    }
    */
    
    severity_entry.first.emplace_front(client_logger::refcounted_stream(stream_file_path));
    return *this;
}

logger_builder& client_logger_builder::add_console_stream(
    logger::severity severity) &
{
    _output_streams[severity].first.emplace_front(client_logger::refcounted_stream(""));
    _output_streams[severity].second = true;
    return *this;
}

logger_builder& client_logger_builder::transform_with_configuration(
    std::string const &configuration_file_path,
    std::string const &configuration_path) &
{
    std::ifstream file(configuration_file_path);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open configuration file: " + configuration_file_path);
    }

    json config;
    try {
        file >> config;
    } catch (const json::parse_error& e) {
        throw std::runtime_error("JSON parse error: " + std::string(e.what()));
    }
    file.close();

    auto settings = config.at(configuration_path);

    if (settings.contains("format"))
    {
        _format = settings["format"].get<std::string>();
    }

    if (settings.contains("streams"))
    {
        for (const auto& stream : settings["streams"])
        {
            std::string type = stream["type"].get<std::string>();

            logger::severity severity;
            std::string severity_str = stream["severity"].get<std::string>();

            if (severity_str == "trace") severity = logger::severity::trace;
            else if (severity_str == "debug") severity = logger::severity::debug;
            else if (severity_str == "information") severity = logger::severity::information;
            else if (severity_str == "warning") severity = logger::severity::warning;
            else if (severity_str == "error") severity = logger::severity::error;
            else if (severity_str == "critical") severity = logger::severity::critical;
            else
            {
                throw std::runtime_error("Unknown severity level: " + severity_str);
            }

            if (type == "console")
            {
                add_console_stream(severity);
            }
            else if (type == "file")
            {
                std::string path = stream["path"].get<std::string>();
                add_file_stream(path, severity);
            }
            else
            {
                throw std::runtime_error("Unknown stream type: " + type);
            }
        }
    }

    return *this;
}

logger_builder& client_logger_builder::clear() &
{
    _output_streams.clear();
    _format = "%m";
    return *this;
}

logger *client_logger_builder::build() const
{
    if (_output_streams.empty())
    {
        throw std::runtime_error("No output streams configured");
    }

    return new client_logger(_output_streams, _format);
}

logger_builder& client_logger_builder::set_format(const std::string &format) &
{
    _format = format;
    return *this;
}

logger_builder& client_logger_builder::set_destination(const std::string &format) &
{
    throw not_implemented("logger_builder *client_logger_builder::set_destination(const std::string &format)", "invalid call");
}