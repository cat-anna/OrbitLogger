#include "MSVCDebuggerSink.h"
#include "orbit_logger.h"

#include <Windows.h>

namespace OrbitLogger {

void MSVCDebuggerOutputPolicy::Write(const OrbitLogger::LogLine *line,
                                     const char *c) {
    OutputDebugStringA(c);
}

} // namespace OrbitLogger
