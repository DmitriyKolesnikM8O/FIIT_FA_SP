#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_BUILDER_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_BUILDER_H

#include <logger_builder.h>
#include <unordered_map>
#include <filesystem>
#include "server_logger.h"

/**
 * @class server_logger_builder
 * @brief Реализация паттерна "Строитель" для server_logger
 * 
 * Класс предоставляет интерфейс для поэтапного создания 
 * объекта server_logger с настройкой всех его параметров.
 */
class server_logger_builder final:
    public logger_builder
{
private:
    // Адрес сервера для отправки логов
    std::string _destination;
    
    // Формат логов
    std::string _format;

    // Потоки вывода с соответствующими severity
    std::unordered_map<logger::severity, std::pair<std::string, bool>> _output_streams;

public:
    /**
     * @brief Конструктор с установкой значений по умолчанию
     */
    server_logger_builder() : _destination("http://127.0.0.1:9200"), _format("%d %t %s %m") {}

public:
    /**
     * @brief Добавляет файловый поток для заданного уровня важности
     * @param stream_file_path Путь к файлу
     * @param severity Уровень важности
     * @return Ссылка на строитель для цепочки вызовов
     */
    logger_builder& add_file_stream(
        std::string const &stream_file_path,
        logger::severity severity) & override;

    /**
     * @brief Добавляет консольный поток для заданного уровня важности
     * @param severity Уровень важности
     * @return Ссылка на строитель для цепочки вызовов
     */
    logger_builder& add_console_stream(
        logger::severity severity) & override;

    /**
     * @brief Настраивает логгер на основе конфигурационного файла
     * @param configuration_file_path Путь к файлу конфигурации
     * @param configuration_path Путь к конфигурации внутри файла
     * @return Ссылка на строитель для цепочки вызовов
     */
    logger_builder& transform_with_configuration(
        std::string const &configuration_file_path,
        std::string const &configuration_path) & override;

    /**
     * @brief Устанавливает адрес сервера для отправки логов
     * @param dest URL-адрес сервера (например, "http://127.0.0.1:9200")
     * @return Ссылка на строитель для цепочки вызовов
     */
    logger_builder& set_destination(const std::string& dest) & override;

    /**
     * @brief Очищает все настройки и устанавливает значения по умолчанию
     * @return Ссылка на строитель для цепочки вызовов
     */
    logger_builder& clear() & override;

    /**
     * @brief Устанавливает формат сообщений лога
     * @param format Формат с использованием спецификаторов %d, %t, %s, %m
     * @return Ссылка на строитель для цепочки вызовов
     */
    logger_builder& set_format(const std::string& format) & override;

    /**
     * @brief Создает объект server_logger на основе настроенных параметров
     * @return Указатель на созданный объект logger
     * @throws std::logic_error если не настроены необходимые параметры
     */
    [[nodiscard]] logger *build() const override;
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_BUILDER_H