/*
  * Generated by cppsrc.sh
  * On 2016-01-20 22:48:52,08
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef Sink_H
#define Sink_H

namespace OrbitLogger {

class iLogSinkBase {
public:
	iLogSinkBase();
 	virtual ~iLogSinkBase();

	virtual void Line(const LogLine *line) = 0;
	virtual void RawLine(const char *line) = 0;

	void PrintBanner();
};

//----------------------------------------------------------------------------------

template<class OutputPolicy, class FilteringPolicy, class FormatPolicy>
struct LogSink : public iLogSinkBase, public OutputPolicy, public FilteringPolicy, public FormatPolicy {
	virtual void Line(const LogLine *line) override {
		if (!Filter(line))
			return;
		char buffer[Configuration::StringFormatBuffer];
		Format(line, buffer, sizeof(buffer) - 1);
		Write(line, buffer);
	}
	virtual void RawLine(const char *line) override {
		Write(nullptr, line);
	}
};

//---------------------------------------------------------------------------------

struct LogSinkBasePolicy {
	virtual ~LogSinkBasePolicy() { }
protected:
	iLogSinkBase& GetSinkBase() { return dynamic_cast<iLogSinkBase&>(*this); }
};

//----------------------------------------------------------------------------------

struct LogNoFilteringPolicy : public LogSinkBasePolicy {
	bool Filter(const LogLine *line) const { return true; }
};

struct LogNoDebugFilteringPolicy : public LogSinkBasePolicy {
	bool Filter(const LogLine *line) const {
		return line->m_SourceInfo->m_Channel != LogChannels::Debug;
	}
};

//----------------------------------------------------------------------------------

struct LogStandardFormatter : public LogSinkBasePolicy {
	LogStandardFormatter();
	void Format(const LogLine *line, char* buffer, size_t buffer_size);
protected:
#ifdef ORBITLOGGER_EXTENDED_LOG_COUNTERS
	unsigned NextLine() { return ++m_Line; }
	unsigned NextType(LogChannel t) { return ++m_Type[(unsigned)t]; }
	unsigned m_Line;
	unsigned m_Type[LogChannels::MaxLogChannels];
#endif
};

//---------------------------------------------------------------------------------

} //namespace OrbitLogger 

#endif