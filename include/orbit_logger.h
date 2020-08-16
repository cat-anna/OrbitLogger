#ifndef ORBITLOGGER_H
#define ORBITLOGGER_H

#include <cstdint>
#include <fstream>
#include <memory>
#include <sstream>

#include "orbit_logger/thread_info.h"

namespace OrbitLogger {

#define ORBIT_LOGGER_VERSION 0x0100
#define ORBIT_LOGGER_VERSION_STRING "1.0"

#if defined(DEBUG) && !defined(ORBITLOGGER_EXTENDED_LOG_COUNTERS)
#define ORBITLOGGER_EXTENDED_LOG_COUNTERS
#endif

struct Configuration {
    enum {
        StringFormatBuffer = 2 * 4096,

        LogLineBufferCapacity = 1024,                          // max amount of log lines buffered in collector tick
        LogLineStringBufferSize = 128 * LogLineBufferCapacity, // total size of all log messages in tick

        MaxSinkCount = 16,
    };
};

using LogChannel = uint32_t;

namespace LogChannels {
enum MaxEnum : size_t {
    MaxLogChannels = sizeof(LogChannel) * 8,
};

enum Enum : LogChannel {
    Info,
    Error,
    Warning,
    Hint,
    Debug,
    System,
    Thread,

    reserved0,

    MaxInternalNamedChannel,

    FirstUserChannel = MaxInternalNamedChannel,

};
} // namespace LogChannels

struct LogLineSourceInfo {
    LogChannel m_Channel;
    const char *m_File;
    const char *m_Function;
    unsigned m_Line;

    LogLineSourceInfo(LogChannel Channel, const char *File, const char *Function, unsigned Line)
        : m_Channel(Channel), m_File(File), m_Function(Function), m_Line(Line) {}
};

#define ORBITLOGGER_MakeSourceInfo(NAME, TYPE)                                                                         \
    static const ::OrbitLogger::LogLineSourceInfo NAME(::OrbitLogger::LogChannels::TYPE, __FILE__, __FUNCTION__,       \
                                                       __LINE__)

struct LogLine {
    const LogLineSourceInfo *m_SourceInfo;
    const char *m_ModeStr;
    const char *m_Message;
    float m_ExecutionSecs;
    ThreadInfo::Signature m_ThreadSign;
    ThreadInfo::NumericID m_ThreadID;
};

} // namespace OrbitLogger

#include "orbit_logger/log_collector.h"
#include "orbit_logger/log_line_sink.h"
#include "orbit_logger/log_marcos.h"

#ifndef BUILDING_ORBIT_LOGGER
#if __has_include(<orbit_logger_config.h>)
#include <orbit_logger_config.h>
#else
#error No orbit logger configuration
#endif
#endif

#endif // ORBITLOGGER_H
