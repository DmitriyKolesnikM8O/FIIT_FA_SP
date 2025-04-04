#include <string>
#include <sstream>
#include <algorithm>
#include <utility>
#include <filesystem>
#include "../include/client_logger.h"

std::unordered_map<std::string, std::pair<size_t, std::ofstream>> client_logger::refcounted_stream::_global_streams;


client_logger::refcounted_stream::refcounted_stream(const std::string &path)
    : _stream(path, nullptr)
{
    if (const auto it = _global_streams.find(path); it == _global_streams.end())
    {
        _global_streams.emplace(path, std::make_pair(1, std::ofstream()));
    }
    else
    {
        it->second.first++;
    }
}


client_logger::refcounted_stream::refcounted_stream(const refcounted_stream &oth)
    : _stream(oth._stream)
{
    if (!_stream.first.empty())
    {
        _global_streams[_stream.first].first++;
    }
}


client_logger::refcounted_stream &client_logger::refcounted_stream::operator=(const refcounted_stream &oth)
{
    if (this != &oth)
    {

        if (!_stream.first.empty())
        {
            if (const auto it = _global_streams.find(_stream.first); it != _global_streams.end())
            {
                it->second.first--;
                if (it->second.first == 0 && it->second.second.is_open())
                {
                    it->second.second.close();
                    _global_streams.erase(it);
                }
            }
        }
        // Копируем новый поток и увеличиваем счетчик
        _stream = oth._stream;
        if (!_stream.first.empty())
        {
            _global_streams[_stream.first].first++;
        }
    }
    return *this;
}


client_logger::refcounted_stream::refcounted_stream(refcounted_stream &&oth) noexcept
    : _stream(std::move(oth._stream))
{
    oth._stream = {"", nullptr};
}


client_logger::refcounted_stream &client_logger::refcounted_stream::operator=(refcounted_stream &&oth) noexcept
{
    if (this != &oth)
    {

        if (!_stream.first.empty())
        {
            if (const auto it = _global_streams.find(_stream.first); it != _global_streams.end())
            {
                it->second.first--;
                if (it->second.first == 0 && it->second.second.is_open())
                {
                    it->second.second.close();
                    _global_streams.erase(it);
                }
            }
        }

        _stream = std::move(oth._stream);
        oth._stream = {"", nullptr};
    }
    return *this;
}


void client_logger::refcounted_stream::open()
{
    if (_stream.second == nullptr && !_stream.first.empty())
    {
        if (const auto it = _global_streams.find(_stream.first); it != _global_streams.end())
        {
            if (!it->second.second.is_open())
            {
                it->second.second.open(_stream.first, std::ios::out | std::ios::app);
                if (!it->second.second.is_open())
                {
                    throw std::runtime_error("Failed to open file stream: " + _stream.first);
                }
            }
            _stream.second = &it->second.second;
        }
    }
}


client_logger::refcounted_stream::~refcounted_stream()
{
    if (!_stream.first.empty())
    {
        if (const auto it = _global_streams.find(_stream.first); it != _global_streams.end())
        {
            it->second.first--;
            if (it->second.first == 0)
            {
                if (it->second.second.is_open())
                {
                    it->second.second.close();
                }
                _global_streams.erase(it);
            }
        }
    }
}


client_logger::client_logger(
    const std::unordered_map<logger::severity, std::pair<std::forward_list<refcounted_stream>, bool>> &streams,
    std::string format)
    : _output_streams(streams), _format(std::move(format))
{
    for (auto &severity_entry : _output_streams)
    {
        auto &stream_list = severity_entry.second.first;
        auto prev = stream_list.before_begin();
        auto curr = stream_list.begin();

        while (curr != stream_list.end())
        {
            try
            {
                curr->open();
                prev = curr;
                ++curr;
            }
            catch (const std::exception &e)
            {

                if (prev == stream_list.before_begin())
                {
                    stream_list.pop_front();
                    curr = stream_list.begin();
                }
                else
                {

                    curr = stream_list.erase_after(prev);
                }
            }
        }
    }
}


client_logger::flag client_logger::char_to_flag(char c) noexcept
{
    switch (c)
    {
    case 'd':
        return flag::DATE;
    case 't':
        return flag::TIME;
    case 's':
        return flag::SEVERITY;
    case 'm':
        return flag::MESSAGE;
    default:
        return flag::NO_FLAG;
    }
}



std::string client_logger::make_format(const std::string &message, severity sev) const
{
    std::string result;

    for (size_t i = 0; i < _format.size(); ++i)
    {
        if (_format[i] == '%' && i + 1 < _format.size())
        {
            flag f = char_to_flag(_format[++i]);
            switch (f)
            {
                case flag::DATE:
                    result += current_date_to_string();
                break;
                case flag::TIME:
                    result += current_time_to_string();
                break;
                case flag::SEVERITY:
                    result += severity_to_string(sev);
                break;
                case flag::MESSAGE:
                    result += message;
                break;
                case flag::NO_FLAG:
                    result += '%';
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
}

logger &client_logger::log(const std::string &text, logger::severity severity) &
{
    if (const auto it = _output_streams.find(severity); it != _output_streams.end())
    {
        const std::string formatted_message = make_format(text, severity);
        for (const auto &stream : it->second.first)
        {
            if (stream._stream.second)
            {
                (*stream._stream.second) << formatted_message << std::endl;
            }
        }
        if (it->second.second)
        {
            std::cout << formatted_message << std::endl;
        }
    }
    return *this;
}


client_logger::client_logger(const client_logger &other)
    : _output_streams(other._output_streams), _format(other._format)
{
    for (auto &severity_entry : _output_streams)
    {
        for (auto &stream : severity_entry.second.first)
        {
            stream.open();
        }
    }
}


client_logger &client_logger::operator=(const client_logger &other)
{
    if (this != &other)
    {
        _output_streams.clear();
        _output_streams = other._output_streams;
        _format = other._format;
        for (auto &severity_entry : _output_streams)
        {
            for (auto &stream : severity_entry.second.first)
            {
                stream.open();
            }
        }
    }
    return *this;
}


client_logger::client_logger(client_logger &&other) noexcept
    : _output_streams(std::move(other._output_streams)), _format(std::move(other._format))
{
    other._output_streams.clear();
    other._format = "%m";
}


client_logger &client_logger::operator=(client_logger &&other) noexcept
{
    if (this != &other)
    {
        _output_streams.clear();
        _output_streams = std::move(other._output_streams);
        _format = std::move(other._format);
        other._output_streams.clear();
        other._format = "%m";
    }
    return *this;
}


client_logger::~client_logger() noexcept
{
    _output_streams.clear();
}