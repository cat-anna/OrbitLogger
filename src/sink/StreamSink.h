#pragma once

#include "../LogLineSink.h"

namespace OrbitLogger {

struct CStreamOutputPolicy : public LogSinkBasePolicy {
	CStreamOutputPolicy() : m_Stream(nullptr), m_Close(false) {}
	~CStreamOutputPolicy() {
		CloseStream();
	}
	void SetStream(FILE *Stream, bool AutoClose = true) {
		CloseStream();
		m_Stream = Stream;
		m_Close = AutoClose;
		GetSinkBase().PrintBanner();
	}

	void SetStdOut() { SetStream(stdout, false); }
	void SetStdErr() { SetStream(stderr, false); }

	void Write(const LogLine *line, const char *c) {
		if (m_Stream) {
			fprintf(m_Stream, c);
			fflush(m_Stream);
		}
	}

	void CloseStream() {
		if (m_Close && m_Stream)
			fclose(m_Stream);
		m_Stream = nullptr;
		m_Close = false;
	}
protected:
	FILE *m_Stream;
	bool m_Close;
};

//---------------------------------------------------------------------------------

template<typename FilteringPolicy, typename FormatPolicy>
struct BaseStreamLoggerSink : public LogSink <CStreamOutputPolicy, FilteringPolicy, FormatPolicy > {
	BaseStreamLoggerSink() {}

	template<typename ... ARGS>
	BaseStreamLoggerSink(ARGS&&...args) {
		SetStream(std::forward<ARGS>(args)...);
	}
};

//---------------------------------------------------------------------------------

using StdCStreamLoggerSink = BaseStreamLoggerSink <LogStandardFormatter, LogNoFilteringPolicy >;
using StdNoDebugCStreamLoggerSink = BaseStreamLoggerSink <LogStandardFormatter, LogNoDebugFilteringPolicy >;

} //namespace OrbitLogger
