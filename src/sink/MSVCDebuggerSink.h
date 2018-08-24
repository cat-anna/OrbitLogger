#pragma once

#include "../LogLineSink.h"

namespace OrbitLogger {

struct MSVCDebuggerOutputPolicy {
	void Write(const OrbitLogger::LogLine *line, const char *c);
};

struct MSVCDebuggerFormatPolicy {
	void Format(const OrbitLogger::LogLine *line, char* buffer, size_t buffer_size) {
		auto src = line->m_SourceInfo;
		const char* file = src->m_File;
		sprintf_s(buffer, buffer_size, "%s(%d) : %s : %s\n", file, src->m_Line, line->m_ModeStr, line->m_Message);
	}
};

struct MSVCDebuggerFilteringPolicy {
	bool Filter(const OrbitLogger::LogLine *line) const { 
        if(line->m_SourceInfo == nullptr)
            return false;
        auto ch = line->m_SourceInfo->m_Channel;
        switch (ch) {
        case OrbitLogger::LogChannels::Error:
        case OrbitLogger::LogChannels::Warning:
            return true;
        default:
            return false;
        }
    }
};

using MSVCDebuggerSink = LogSink < MSVCDebuggerOutputPolicy, MSVCDebuggerFormatPolicy, MSVCDebuggerFilteringPolicy >;

} //namespace OrbitLogger
