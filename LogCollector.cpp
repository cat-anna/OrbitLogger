/*
  * Generated by cppsrc.sh
  * On 2016-01-20 22:50:27,06
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>

#include <cstdarg>
#include <thread>
#include <chrono>
#include <atomic>

#include "OrbitLogger.h"

namespace OrbitLogger {

struct LineTypeStringTable {
	LineTypeStringTable() {
		for (unsigned i = 0; i < (unsigned)LineType::MaxValue; ++i) {
			m_Table[i] = " ?? ";
			m_DebugTable[i] = " ?? ";
		}

#define _set(T, NAME) T[(unsigned)LineType:: NAME]

		_set(m_Table, Normal) = "NORM";
		_set(m_Table, Critical) = "CRIT";
		_set(m_Table, Error) = "ERR ";
		_set(m_Table, Warning) = "WARN";
		_set(m_Table, Hint) = "HINT";
		_set(m_Table, SysInfo) = "SYS ";

		_set(m_DebugTable, Normal) = "DBG ";
		_set(m_DebugTable, Critical) = "CRIT";
		_set(m_DebugTable, Error) = "DBGE";
		_set(m_DebugTable, Warning) = "DBGW";
		_set(m_DebugTable, Hint) = "DBGH";
		_set(m_DebugTable, SysInfo) = "SYS ";

#undef _set
	}

	const char *GetRegular(LineType type) const { return m_Table[(unsigned)type]; }
	const char *GetDebug(LineType type) const { return m_DebugTable[(unsigned)type]; }

	const char *Get(LineType type = LineType::Normal, unsigned Flags = 0) const { 
		if (Flags & LogLineFlags::DebugLine)
			return GetDebug(type);
		return GetRegular(type);
	}
private:
	const char *m_Table[(unsigned)LineType::MaxValue];
	const char *m_DebugTable[(unsigned)LineType::MaxValue];
};

//----------------------------------------------------------------------------------

struct LogLineBuffer {
	LogLineBuffer():m_LogLineAllocated(0), m_StringBufferUsage(0) { }
	
	LogLine *AllocLogLine() {
		auto idx = m_LogLineAllocated.fetch_add(1);
		if (idx >= Configuration::LogLineBufferCapacity)
			return nullptr;
		return m_LogLineTable + idx;
	}

	char *AllocateString(size_t len) {
		if (!len)
			return nullptr;
		auto size = len + 1;
		auto pos = m_StringBufferUsage.fetch_add(size);
		if (pos + size >= Configuration::LogLineStringBufferSize)
			return nullptr;
		char *str = m_StringBuffer + pos;
		str[len] = 0;
		return str;
	}

	bool Empty() const {
		return m_LogLineAllocated == 0;
	}
	size_t AllocatedLines() const { return m_LogLineAllocated; }
	LogLine* GetLine(size_t index) { 
		if (index >= AllocatedLines())
			return nullptr;
		return m_LogLineTable + index;
	}

	void Reset() {
		size_t lines = m_LogLineAllocated.exchange(0);
		size_t buffer = m_StringBufferUsage.exchange(0);

		if (lines > Configuration::LogLineBufferCapacity) {
			ORBITLOGGER_MakeSourceInfo(logsrc, Critical, 0);
			auto line = AllocLogLine();
			line->m_SourceInfo = &logsrc;
			line->m_Message = "LogLine buffer overflow!";
			line->m_ThreadSign = ThreadInfo::GetSignature();
			line->m_ThreadID = ThreadInfo::GetID();
			line->m_ExecutionSecs = 0;
		}

		if (buffer > Configuration::LogLineStringBufferSize) {
			ORBITLOGGER_MakeSourceInfo(logsrc, Critical, 0);
			auto line = AllocLogLine();
			line->m_SourceInfo = &logsrc;
			line->m_Message = "LogLine string buffer overflow!";
			line->m_ThreadSign = ThreadInfo::GetSignature();
			line->m_ThreadID = ThreadInfo::GetID();
			line->m_ExecutionSecs = 0;
		}
	}
private:
	std::atomic_size_t m_LogLineAllocated;
	std::atomic_size_t m_StringBufferUsage;

	LogLine m_LogLineTable[Configuration::LogLineBufferCapacity];
	char m_StringBuffer[Configuration::LogLineStringBufferSize];
};

//----------------------------------------------------------------------------------

struct LogCollector::LogCollectorImpl {
	LogCollectorImpl(): m_FirstBuffer(), m_SecondBuffer() {
		m_ExecutionTime = std::chrono::steady_clock::now();

		m_CurrentBuffer = &m_FirstBuffer;
		m_InactiveBuffer = &m_SecondBuffer;

		for (size_t i = 0; i < Configuration::MaxSinkCount; ++i)
			m_SinkTable[i] = nullptr;

		m_ThreadCanRun = true;
		m_ThreadRunning = false;
		std::thread(&LogCollectorImpl::ThreadEntry, this).detach();
	}

	~LogCollectorImpl() {
		m_ThreadCanRun = false;
		for (int i = 0; i < 1000 && m_ThreadRunning; ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		for (size_t i = 0; i < Configuration::MaxSinkCount; ++i)
			m_SinkTable[i] = nullptr;
	}

	void PushLine(const LogLineSourceInfo* SourceInfo, const char* message, size_t length) {
		if (!length)
			return;
		
		std::chrono::duration<double> sec = std::chrono::steady_clock::now() - m_ExecutionTime;

		auto *LogBuffer = m_CurrentBuffer;

		auto line = LogBuffer->AllocLogLine();
		if (!line)
			return;
		auto msgBuffer = LogBuffer->AllocateString(length);
		if (!msgBuffer)
			return;

		memcpy(msgBuffer, message, length);

		line->m_SourceInfo = SourceInfo;
		line->m_Message = msgBuffer;
		line->m_ExecutionSecs = static_cast<float>(sec.count());
		line->m_ThreadSign = ThreadInfo::GetSignature();
		line->m_ThreadID = ThreadInfo::GetID();
	}

	iLogSinkBase* InsertLogSink(std::unique_ptr<iLogSinkBase> sink) {
		for (size_t i = 0; i < Configuration::MaxSinkCount; ++i) {
			auto &table = m_SinkTable[i];
			if (table)
				continue;
			table.swap(sink);
			return table.get();
		}
		return nullptr;
	}
private:
	void ThreadEntry() {
		m_ThreadRunning = true;
		ThreadInfo::SetName("LOGC");
		AddLog(Thread, "LogCollector thread executed");
		try {
			//give some time to settle things
			//std::this_thread::sleep_for(std::chrono::milliseconds(10));
			EngineMain();
		}
		catch (...) {
			//TDB
		}
		m_ThreadRunning = false;
	}

	void EngineMain() {
		//#if defined(DEBUG) && defined(_BUILDING_ENGINE_)
		//StaticLogCatcher::DispatchLog();
		//#endif
		
		while (true) {
			//I consider this switch as thread-safe
			//the writers will just start to use another pointer
			auto *buffer = m_CurrentBuffer;
			m_CurrentBuffer = m_InactiveBuffer;
			m_InactiveBuffer = buffer;

			if (!m_ThreadCanRun && buffer->Empty())
				return;

			//wait for all writers to finish their job
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

			for (size_t i = 0, j = buffer->AllocatedLines(); i < j; ++i)
				ProcessLine(buffer->GetLine(i));

			buffer->Reset();
		}
	}

	void ProcessLine(LogLine *line) {
		auto srcinfo = line->m_SourceInfo;
		if(srcinfo)
			line->m_ModeStr = m_LineTypeTable.Get(srcinfo->m_Mode, srcinfo->m_Flags);
		else
			line->m_ModeStr = m_LineTypeTable.Get();

		for (size_t i = 0; i < Configuration::MaxSinkCount; ++i) {
			auto &sink = m_SinkTable[i];
			if (!sink)
				break;
			try {
				sink->Line(line);
			}
			catch (...) {
			}
		}
	}

	volatile bool m_ThreadCanRun;
	volatile bool m_ThreadRunning;
	std::chrono::steady_clock::time_point m_ExecutionTime;

	LogLineBuffer *m_CurrentBuffer, *m_InactiveBuffer;
	LineTypeStringTable m_LineTypeTable;
	std::unique_ptr<iLogSinkBase> m_SinkTable[Configuration::MaxSinkCount];

	LogLineBuffer m_FirstBuffer, m_SecondBuffer;

};

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

LogCollector LogCollector::s_Instance;

LogCollector::LogCollector() {
	//nothing there
}

LogCollector::~LogCollector() {
	//nothing there
}

//----------------------------------------------------------------------------------

void LogCollector::PushLine(const LogLineSourceInfo* SourceInfo, const char* fmt, ...) {
	if (!s_Instance.m_Impl)
		return;

	char buffer[Configuration::StringFormatBuffer];
	va_list args;
	va_start(args, fmt);
	int length = vsprintf_s(buffer, fmt, args);
	va_end(args);
	if (length < 0)
		return;

	s_Instance.m_Impl->PushLine(SourceInfo, buffer, length);
}

void LogCollector::PushLine(const LogLineSourceInfo* SourceInfo, const std::ostringstream &ss) {
	if (!s_Instance.m_Impl)
		return;

	auto s = ss.str();
	s_Instance.m_Impl->PushLine(SourceInfo, s.c_str(), s.length());
}

//----------------------------------------------------------------------------------

bool LogCollector::Start() {

#ifndef ORBITLOGGER_DISASBLE_LOGGING
	if (!s_Instance.m_Impl)
		s_Instance.m_Impl = std::make_unique<LogCollectorImpl>();
#endif
	return true;
}

bool LogCollector::Stop() {
	s_Instance.m_Impl.reset();
	return true;
}

bool LogCollector::IsRunning() {
	return static_cast<bool>(s_Instance.m_Impl);
}

//----------------------------------------------------------------------------------

iLogSinkBase* LogCollector::InsertLogSink(std::unique_ptr<iLogSinkBase> sink) {
	if (!IsRunning())
		return nullptr;
	return s_Instance.m_Impl->InsertLogSink(std::move(sink));
}

} //namespace OrbitLogger 
