#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_CLIENT_LOGGER_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_CLIENT_LOGGER_H

#include <logger.h>
#include <array>
#include <unordered_map>
#include <forward_list>
#include <fstream>
#include <ctime>

class client_logger_builder;

class client_logger final:
    public logger
{
private:
    //region refcounted_stream

    class refcounted_stream final
    {
        static std::unordered_map<std::string, std::pair<size_t, std::ofstream>> _global_streams;

        std::pair<std::string, std::ofstream*> _stream;
        friend client_logger;
        friend client_logger_builder;
    public:

        explicit refcounted_stream(const std::string& path);

        refcounted_stream(const refcounted_stream& oth);

        refcounted_stream& operator=(const refcounted_stream& oth);

        refcounted_stream(refcounted_stream&& oth) noexcept;

        refcounted_stream& operator=(refcounted_stream&& oth) noexcept;

        //if ofstream* is nullptr initializes it with opened file from global map
        void open();

        ~refcounted_stream();
    };

    //region refcounted_stream

    enum class flag
    { DATE, TIME, SEVERITY, MESSAGE, NO_FLAG };

private:

    std::unordered_map<logger::severity ,std::pair<std::forward_list<refcounted_stream>, bool>> _output_streams;

    std::string _format;


private:

    //opens all streams
    client_logger(const std::unordered_map<logger::severity ,std::pair<std::forward_list<refcounted_stream>, bool>>& streams, std::string format);

    std::string make_format(const std::string& message, severity sev) const;

    static flag char_to_flag(char c) noexcept;

    friend client_logger_builder;
public:

    client_logger(client_logger const &other);

    client_logger &operator=(client_logger const &other);

    client_logger(client_logger &&other) noexcept;

    client_logger &operator=(client_logger &&other) noexcept;

    ~client_logger() noexcept final;

public:

    [[nodiscard]] logger& log(
        const std::string &message,
        logger::severity severity) & override;

};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_CLIENT_LOGGER_H