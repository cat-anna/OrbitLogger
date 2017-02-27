#include "../OrbitLogger.h"
#include "MSVCDebuggerSink.h"

#include <Windows.h>

namespace OrbitLogger {

void MSVCDebuggerOutputPolicy::Write(const OrbitLogger::LogLine *line, const char *c) {
	OutputDebugStringA(c);
}

} //namespace OrbitLogger

