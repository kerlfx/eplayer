#ifndef ELOG_H_
#define ELOG_H_

// #pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>

#include "threadpool.h"

enum class LogLevel : std::int8_t
{
    LOG_OFF = -1,
    LOG_ERROR,
    LOG_WARM,
    LOG_INFO,
    LOG_DEBUG,
};

class ELog
{
private:
    static ELog *elog_ptr;
    static std::mutex init_mutex;

    LogLevel log_level;

    std::string log_level_head[static_cast<int>(LogLevel::LOG_DEBUG) + 1];

    ThreadPool tpl;

    ELog(LogLevel level = LogLevel::LOG_INFO);
    ~ELog();

    template <class OS> auto osp(OS &&value);
    template <class OS, class... OSArgs>
    auto osp(OS &&value, OSArgs &&...fargs);

    std::shared_ptr<std::string> elogGetTimeStr();

public:
    size_t elogOutGetQueueSize();

    LogLevel elogOutGetLevel();
    void elogOutSetLevel(LogLevel level);

    template <class... OSArgs>
    auto elogOut(LogLevel log_level, OSArgs &&...fargs);

    static auto elgoPtr()
    {

        if (elog_ptr == nullptr)
        {
            std::unique_lock<std::mutex> lck(init_mutex);
            if (elog_ptr == nullptr)
            {
                elog_ptr = new ELog;
            }
        }
        return elog_ptr;
    }
};

template <class... OSArgs>
auto ELog::elogOut(LogLevel log_level, OSArgs &&...fargs)
{
    auto time_str = elogGetTimeStr();

    tpl.toQueue(
        [log_level, time_str, fargs...]
        {
            std::cout << ELog::elgoPtr()->osp(
                ELog::elgoPtr()
                    ->log_level_head[static_cast<int>(log_level)]
                    .data(),
                "|", time_str->data(), "| ", fargs...);
            return;
        });
}

template <class OS> auto ELog::osp(OS &&value)
{
    std::ostringstream obuf;
    obuf << value << "\n";
    return obuf.str();
}

template <class OS, class... OSArgs>
auto ELog::osp(OS &&value, OSArgs &&...args)
{
    std::ostringstream obuf;
    obuf << value << osp(args...);
    return obuf.str();
};

#define LOG_SET_lEVEL(x) ELog::elgoPtr()->elogOutSetLevel(x);

#define LOG_I(...)                                                             \
    if (ELog::elgoPtr()->elogOutGetLevel() >= LogLevel::LOG_INFO)              \
    {                                                                          \
        ELog::elgoPtr()->elogOut(LogLevel::LOG_INFO, __FUNCTION__, "(",        \
                                 __LINE__, "):", __VA_ARGS__);                 \
    }
#define LOG_W(...)                                                             \
    if (ELog::elgoPtr()->elogOutGetLevel() >= LogLevel::LOG_WARM)              \
    {                                                                          \
        ELog::elgoPtr()->elogOut(LogLevel::LOG_WARM, __FUNCTION__, "(",        \
                                 __LINE__, "):", __VA_ARGS__);                 \
    }
#define LOG_E(...)                                                             \
    if (ELog::elgoPtr()->elogOutGetLevel() >= LogLevel::LOG_ERROR)             \
    {                                                                          \
        ELog::elgoPtr()->elogOut(LogLevel::LOG_ERROR, __FUNCTION__, "(",       \
                                 __LINE__, "):", __VA_ARGS__);                 \
    }
#define LOG_D(...)                                                             \
    if (ELog::elgoPtr()->elogOutGetLevel() >= LogLevel::LOG_DEBUG)             \
    {                                                                          \
        ELog::elgoPtr()->elogOut(LogLevel::LOG_DEBUG, __FUNCTION__, "(",       \
                                 __LINE__, "):", __VA_ARGS__);                 \
    }

#endif // ELOG_H_