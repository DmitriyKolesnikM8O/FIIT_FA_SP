#include <string>
#include <sstream>
#include <algorithm>
#include <utility>
#include <mutex>
#include "../include/client_logger.h"
#include <not_implemented.h>

// Мьютекс для обеспечения потокобезопасности при доступе к глобальным потокам
static std::mutex global_streams_mutex;

std::unordered_map<std::string, std::pair<size_t, std::ofstream>> client_logger::refcounted_stream::_global_streams;


logger& client_logger::log(
    const std::string &text,
    logger::severity severity) &
{
    try {
        // Проверяем, существует ли настроенный вывод для данного уровня severity
        auto it = _output_streams.find(severity);
        if (it == _output_streams.end()) return *this;

        // Форматируем сообщение согласно настройкам формата
        std::string formatted = make_format(text, severity);

        // Записываем в каждый файловый поток
        for (auto &stream : it->second.first)
        {
            // Проверяем, что путь не пустой (пустые пути для консольных потоков)
            if (!stream._stream.first.empty() && stream._stream.second)
            {
                try {
                    *stream._stream.second << formatted << std::endl;
                    if (!stream._stream.second->good()) {
                        std::cerr << "Error writing to log file: " << stream._stream.first << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Exception writing to log file: " << stream._stream.first << ": " << e.what() << std::endl;
                }
            }
        }

        // Если настроен вывод в консоль, выводим сообщение
        if (it->second.second)
        {
            try {
                std::cout << formatted << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Exception writing to console: " << e.what() << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in log method: " << e.what() << std::endl;
    }

    return *this;
}

std::string client_logger::make_format(const std::string &message, severity sev) const
{
    try {
        std::string result;
        std::time_t now = std::time(nullptr);
        
        std::tm* tm = nullptr;
        std::tm tm_buf{};
        
        #ifdef _WIN32
        gmtime_s(&tm_buf, &now);
        tm = &tm_buf;
        #else
        tm = std::gmtime(&now);
        if (!tm) {
            throw std::runtime_error("Failed to convert time to GMT");
        }
        #endif
        
        char buffer[80];

        for (size_t i = 0; i < _format.size(); ++i)
        {
            if (_format[i] == '%' && i + 1 < _format.size())
            {
                flag f = char_to_flag(_format[++i]);
                switch (f)
                {
                case flag::DATE:
                    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", tm);
                    result += buffer;
                    break;
                case flag::TIME:
                    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", tm);
                    result += buffer;
                    break;
                case flag::SEVERITY:
                    result += severity_to_string(sev);
                    break;
                case flag::MESSAGE:
                    result += message;
                    break;
                default:
                    result += _format[i];
                    break;
                }
            }
            else
            {
                result += _format[i];
            }
        }
        return result;
    } catch (const std::exception& e) {
        std::cerr << "Error formatting log message: " << e.what() << std::endl;
        // Возвращаем неформатированное сообщение в случае ошибки
        return message;
    }
}

void client_logger::refcounted_stream::open()
{
    // Пустой путь не требует открытия файла
    if (_stream.first.empty()) {
        _stream.second = nullptr;
        return;
    }

    std::lock_guard<std::mutex> lock(global_streams_mutex);
    
    auto it = _global_streams.find(_stream.first);
    if (it == _global_streams.end())
    {
        _global_streams[_stream.first] = std::make_pair(1, std::ofstream(_stream.first));
        if (!_global_streams[_stream.first].second.is_open())
        {
            throw std::runtime_error("Failed to open log file: " + _stream.first);
        }
        _stream.second = &_global_streams[_stream.first].second;
    }
    else
    {
        ++it->second.first;
        _stream.second = &it->second.second;
    }
}

client_logger::client_logger(
    const std::unordered_map<logger::severity, std::pair<std::forward_list<refcounted_stream>, bool>> &streams,
    std::string format) :
    _output_streams(streams),
    _format(std::move(format))
{
    for (auto &[sev, streams_pair] : _output_streams)
    {
        for (auto &stream : streams_pair.first)
        {
            stream.open();
        }
    }
}

client_logger::flag client_logger::char_to_flag(char c) noexcept
{
    switch (c)
    {
    case 'd': return flag::DATE;
    case 't': return flag::TIME;
    case 's': return flag::SEVERITY;
    case 'm': return flag::MESSAGE;
    default:  return flag::NO_FLAG;
    }
}

client_logger::client_logger(const client_logger &other) : _output_streams(other._output_streams), _format(other._format)
{
    // Открываем все потоки для каждого severity
    for (auto &[sev, streams_pair] : _output_streams)
    {
        for (auto &stream : streams_pair.first)
        {
            // Открываем поток, если путь не пустой
            if (!stream._stream.first.empty()) {
                stream.open();
            }
        }
    }
}

client_logger &client_logger::operator=(const client_logger &other)
{
    if (this != &other)
    {
        _output_streams = other._output_streams;
        _format = other._format;
    }
    return *this;
}

client_logger::client_logger(client_logger &&other) noexcept : _output_streams(std::move(other._output_streams)),
    _format(std::move(other._format))
{
}

client_logger &client_logger::operator=(client_logger &&other) noexcept
{
    if (this != &other)
    {
        _output_streams = std::move(other._output_streams);
        _format = std::move(other._format);
    }
    return *this;
}

client_logger::~client_logger() noexcept = default;

client_logger::refcounted_stream::refcounted_stream(const std::string &path) : _stream(path, nullptr)
{
    // Пустой путь не требует открытия файла и увеличения счетчика ссылок
    if (path.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(global_streams_mutex);
    
    auto it = _global_streams.find(path);
    if (it == _global_streams.end())
    {
        _global_streams[path] = std::make_pair(1, std::ofstream(path));
        if (!_global_streams[path].second.is_open()) {
            throw std::runtime_error("Failed to open log file: " + path);
        }
        _stream.second = &_global_streams[path].second;
    }
    else
    {
        ++it->second.first;
        _stream.second = &it->second.second;
    }
}

client_logger::refcounted_stream::refcounted_stream(const client_logger::refcounted_stream &oth) : _stream(oth._stream)
{
    if (_stream.second != nullptr && !_stream.first.empty())
    {
        std::lock_guard<std::mutex> lock(global_streams_mutex);
        
        auto it = _global_streams.find(_stream.first);
        if (it == _global_streams.end())
        {
            _stream.second = nullptr;
            throw std::runtime_error("Global stream not found for path: " + _stream.first);
        }
        ++it->second.first;
    }
}

client_logger::refcounted_stream &
client_logger::refcounted_stream::operator=(const client_logger::refcounted_stream &oth)
{
    if (this != &oth)
    {
        {
            std::lock_guard<std::mutex> lock(global_streams_mutex);
            
            if (_stream.second != nullptr && !_stream.first.empty())
            {
                auto current_it = _global_streams.find(_stream.first);
                if (current_it != _global_streams.end())
                {
                    if (--current_it->second.first == 0)
                    {
                        current_it->second.second.close();
                        _global_streams.erase(current_it);
                    }
                }
            }
        }

        _stream.first = oth._stream.first;
        _stream.second = oth._stream.second;

        if (_stream.second != nullptr && !_stream.first.empty())
        {
            std::lock_guard<std::mutex> lock(global_streams_mutex);
            
            auto new_it = _global_streams.find(_stream.first);
            if (new_it != _global_streams.end())
            {
                ++new_it->second.first;
            }
            else
            {
                _stream.second = nullptr;
                throw std::runtime_error("Global stream not found for path: " + _stream.first);
            }
        }
    }
    return *this;
}

client_logger::refcounted_stream::refcounted_stream(client_logger::refcounted_stream &&oth) noexcept : _stream(std::move(oth._stream))
{
    oth._stream = {oth._stream.first, nullptr};
}

client_logger::refcounted_stream &client_logger::refcounted_stream::operator=(client_logger::refcounted_stream &&oth) noexcept
{
    if (this != &oth) {
        this->~refcounted_stream();
        _stream = std::move(oth._stream);
        oth._stream = {oth._stream.first, nullptr};
    }
    return *this;
}

client_logger::refcounted_stream::~refcounted_stream()
{
    if (!_stream.second || _stream.first.empty()) return;

    std::lock_guard<std::mutex> lock(global_streams_mutex);
    
    auto it = _global_streams.find(_stream.first);
    if (it != _global_streams.end())
    {
        if (--it->second.first == 0)
        {
            it->second.second.close();
            _global_streams.erase(it);
        }
    }
}