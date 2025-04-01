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
    _output_streams[severity] = {"", true};  // Пустая строка означает консоль
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

    // Читаем файл как строку
    std::string json_str((std::istreambuf_iterator<char>(config_file)), std::istreambuf_iterator<char>());
    if (json_str.empty()) {
        throw std::runtime_error("Config file is empty!");
    }

    // Парсим JSON
    nlohmann::json root;
    root = nlohmann::json::parse(json_str);
    /*try {
        root = nlohmann::json::parse(json_str);
    } catch (const std::exception &e) {
        throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
    }*/

    // Проверяем, содержит ли JSON нужный путь
    if (!root.contains(configuration_path)) {
        throw std::runtime_error("Configuration path not found in JSON: " + configuration_path);
    }

    nlohmann::json config = root[configuration_path];
    if (!config.is_object()) {
        throw std::runtime_error("Configuration path must be a JSON object!");
    }

    // Настраиваем вывод
    if (config.contains("destination")) {
        _destination = config["destination"].get<std::string>();
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
    _destination = "http://localhost:9200";
    return *this;
}

logger *server_logger_builder::build() const
{
    return new server_logger(_destination, _output_streams);
}

logger_builder& server_logger_builder::set_destination(const std::string& dest) &
{
    _destination = dest;
    return *this;
}

logger_builder& server_logger_builder::set_format(const std::string& format) &
{
    _log_format = format;
    return *this;
}