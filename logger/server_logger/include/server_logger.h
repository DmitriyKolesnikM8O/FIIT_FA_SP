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

    // Приватный конструктор для использования строителем
    server_logger(const std::string& dest, const std::string& format, 
                  const std::unordered_map<logger::severity, std::pair<std::string, bool>>& streams);

    friend server_logger_builder;

    // Статический метод для получения PID процесса кроссплатформенно
    static int inner_getpid();

public:
    // Деструктор
    ~server_logger() noexcept final;

    // Конструктор копирования
    server_logger(server_logger const &other);

    // Оператор присваивания копированием
    server_logger &operator=(server_logger const &other);

    // Конструктор перемещения
    server_logger(server_logger &&other) noexcept;

    // Оператор присваивания перемещением
    server_logger &operator=(server_logger &&other) noexcept;

    // Метод для логирования сообщения с указанным уровнем важности
    [[nodiscard]] logger& log(
        const std::string &message,
        logger::severity severity) & override;
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H