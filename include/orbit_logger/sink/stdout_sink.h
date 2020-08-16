#pragma once

#include "orbit_logger.h"
#include <iostream>

namespace OrbitLogger {

struct StdOutOutputPolicy : public LogSinkBasePolicy {
    void Write(const LogLine *line, const char *c) { std::cout << c << std::flush; }
};

template <typename FilteringPolicy, typename FormatPolicy>
struct BaseStdOutSink : public LogSink<StdOutOutputPolicy, FilteringPolicy, FormatPolicy> {
    BaseStdOutSink() {}

    template <typename... ARGS> BaseStdOutSink(ARGS &&... args) {}
};

using StdOutSink = BaseStdOutSink<LogShortFormatter, LogNoFilteringPolicy>;

} // namespace OrbitLogger
