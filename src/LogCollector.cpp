#include <thread>
#include <chrono>
#include <atomic>
#include <cassert>
#include <array>

#include "OrbitLogger.h"
#include "Platform.h"
#include "StreamReader.h"

namespace OrbitLogger {

struct LineTypeStringTable {
	LineTypeStringTable() {
		for (unsigned i = 0; i < LogChannels::MaxLogChannels; ++i) {
			char buf[16];
			sprintf_s(buf, "CH%02d", i);
			Set(i, buf);
		}
#define _set(NAME, V) Set(LogChannels:: NAME, V)
		_set(Info, "INFO");
		_set(Error, "ERR ");
		_set(Warning, "WARN");
		_set(Hint, "HINT");
		_set(System, "SYS ");
		_set(Thread, "THRD");
		_set(Debug, "DBG ");
#undef _set
	}
	const char *Get(LogChannel type = LogChannels::Info) const { return m_Table[type].Name; }
	void Set(LogChannel type, const char *Name) {
		auto &e = m_Table[type];
		e.value = 0;
		for (size_t i = 0; i < 4; ++i) {
			char c = Name[i];
			if (!c)
				break;
			e.Name[i] = c;
		}
	}
private:
	union Entry {
		char Name[8];
		uint64_t value;
	};
	Entry m_Table[LogChannels::MaxLogChannels];
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
			ORBITLOGGER_MakeSourceInfo(logsrc, Error);
			auto line = AllocLogLine();
			line->m_SourceInfo = &logsrc;
			line->m_Message = "LogLine buffer overflow!";
			line->m_ThreadSign = ThreadInfo::GetSignature();
			line->m_ThreadID = ThreadInfo::GetID();
			line->m_ExecutionSecs = 0;
		}
		if (buffer > Configuration::LogLineStringBufferSize) {
			ORBITLOGGER_MakeSourceInfo(logsrc, Error);
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
		m_DisabledChannels = 0;
		for (auto &it : m_ChannelCounter)
			it.store(0);

		for (LogChannel it = 0; it < LogChannels::MaxInternalNamedChannel; ++it)
			SetChannelState(it, true);
#ifdef DEBUG
		m_DisabledChannels &= ~(1 << LogChannels::Debug);
#endif

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

		m_StdOutReader.reset();
		m_StdErrReader.reset();

		for (int i = 0; i < 1000 && m_ThreadRunning; ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		for (size_t i = 0; i < Configuration::MaxSinkCount; ++i)
			m_SinkTable[i] = nullptr;
	}

	void PushLineCopy(const LogLineSourceInfo* SourceInfo, const char* message, size_t length) {
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

	void PushLinePtr(const LogLineSourceInfo* SourceInfo, const char* message) {
		std::chrono::duration<double> sec = std::chrono::steady_clock::now() - m_ExecutionTime;

		auto *LogBuffer = m_CurrentBuffer;

		auto line = LogBuffer->AllocLogLine();
		if (!line)
			return;

		line->m_SourceInfo = SourceInfo;
		line->m_Message = message;
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
	
	bool IsLineEnabled(const LogLineSourceInfo* SourceInfo) {
		LogChannel bit = 1 << SourceInfo->m_Channel;
		++m_ChannelCounter[SourceInfo->m_Channel];
		return (m_DisabledChannels & bit) == 0;
	}

	bool IsChannelEnabled(LogChannel Channel) {
		LogChannel bit = 1 << Channel;
		return (m_DisabledChannels & bit) == 0;
	}

	void SetChannelName(LogChannel Channel, const char *Name) {
		m_LineTypeTable.Set(Channel, Name);
	}
	void SetChannelState(LogChannel Channel, bool Enabled) {
		if (Enabled)
			m_DisabledChannels &= ~(1 << Channel);
		else
			m_DisabledChannels |= (1 << Channel);
	}

	bool SetCaptureStdOut(LogChannel ch) {
		if (m_StdOutReader)
			return false;
		m_StdOutReader = std::make_unique<StreamReader>(stdout, ch);
		return true;
	}
	bool SetCaptureStdErr(LogChannel ch) {
		if (m_StdErrReader)
			return false;
		m_StdErrReader = std::make_unique<StreamReader>(stderr, ch);
		return true;
	}

	bool GetChannelInfo(ChannelInfoTable &table) {
		for (size_t i = 0; i < table.size(); ++i) {
			auto &it = table[i];
			it.m_Channel = static_cast<LogChannel>(i);

			if (s_Instance.m_Impl) {
				it.m_Enabled = IsChannelEnabled(it.m_Channel);
				it.m_LinesPushed = m_ChannelCounter[it.m_Channel].load();
				it.m_Name = m_LineTypeTable.Get(it.m_Channel);
			} else {
				it.m_Enabled = false;
				it.m_LinesPushed = 0;
				it.m_Name = "";
			}
		}
		return true;
	}

private:
	void ThreadEntry() {
		m_ThreadRunning = true;
		ThreadInfo::SetName("LOGC");
		AddLog(Info, "LogCollector thread executed");
		try {
			EngineMain();
		}
		catch (...) {
			//TDB
		}
		m_ThreadRunning = false;
	}

	void EngineMain() {
		
		for (;;) {
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
			line->m_ModeStr = m_LineTypeTable.Get(srcinfo->m_Channel);
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
	LogChannel m_DisabledChannels;
	std::array<std::atomic<uint32_t>, LogChannels::MaxLogChannels> m_ChannelCounter;
	std::unique_ptr<iLogSinkBase> m_SinkTable[Configuration::MaxSinkCount];
	std::unique_ptr<StreamReader> m_StdOutReader;
	std::unique_ptr<StreamReader> m_StdErrReader;

	LogLineBuffer m_FirstBuffer, m_SecondBuffer;
};

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

LogCollector LogCollector::s_Instance;

LogCollector::LogCollector() {
	m_LocalInstance = false;
}

LogCollector::~LogCollector() {
}

//----------------------------------------------------------------------------------

bool LogCollector::SetCaptureStdOut(LogChannel ch) {
	if (!s_Instance.m_Impl)
		return false;
	return s_Instance.m_Impl->SetCaptureStdOut(ch);
}

bool LogCollector::SetCaptureStdErr(LogChannel ch) {
	if (!s_Instance.m_Impl)
		return false;
	return s_Instance.m_Impl->SetCaptureStdErr(ch);
}

//----------------------------------------------------------------------------------

bool LogCollector::GetChannelInfo(ChannelInfoTable &table) {
	if (!s_Instance.m_Impl)
		return false;
	return s_Instance.m_Impl->GetChannelInfo(table);
}

//----------------------------------------------------------------------------------

bool LogCollector::IsChannelEnabled(LogChannel Channel) {
	assert(Channel < LogChannels::MaxLogChannels);
	if (!s_Instance.m_Impl)
		return false;

	return s_Instance.m_Impl->IsChannelEnabled(Channel);
}

bool LogCollector::IsLineEnabled(const LogLineSourceInfo* SourceInfo) {
	assert(SourceInfo->m_Channel < LogChannels::MaxLogChannels);
	if (!s_Instance.m_Impl)
		return false;

	return s_Instance.m_Impl->IsLineEnabled(SourceInfo);
}

void LogCollector::PushLinePtr(const LogLineSourceInfo* SourceInfo, const char* line) {
	if (!s_Instance.m_Impl || !line)
		return;

	s_Instance.m_Impl->PushLinePtr(SourceInfo, line);
}

void LogCollector::PushLineCopy(const LogLineSourceInfo* SourceInfo, const char* line, size_t length) {
	if (!s_Instance.m_Impl || !line)
		return;

	s_Instance.m_Impl->PushLineCopy(SourceInfo, line, length);
}

void LogCollector::PushLine(const LogLineSourceInfo* SourceInfo, const std::ostringstream &ss) {
	if (!s_Instance.m_Impl)
		return;
	auto s = ss.str();
	s_Instance.m_Impl->PushLineCopy(SourceInfo, s.c_str(), s.length());
}

//----------------------------------------------------------------------------------

void LogCollector::SetChannelName(LogChannel Channel, const char *Name, bool EnableChannel) {
	assert(Channel < LogChannels::MaxLogChannels);
	if (!s_Instance.m_Impl)
		return;
	s_Instance.m_Impl->SetChannelName(Channel, Name);
	s_Instance.m_Impl->SetChannelState(Channel, EnableChannel);
}

void LogCollector::SetChannelState(LogChannel Channel, bool Enabled) {
	assert(Channel < LogChannels::MaxLogChannels);
	if (!s_Instance.m_Impl)
		return;
	s_Instance.m_Impl->SetChannelState(Channel, Enabled);
}

void LogCollector::SetExternalInstance(LogCollectorImpl *instance) {
	s_Instance.Stop();
	s_Instance.m_LocalInstance = false;
	s_Instance.m_Impl.reset(instance);
}

//----------------------------------------------------------------------------------

bool LogCollector::Start() {
	Stop();
	ThreadInfo::SetName("MAIN", true);

#ifndef ORBITLOGGER_DISASBLE_LOGGING
	s_Instance.m_LocalInstance = true;
	if (!s_Instance.m_Impl)
		s_Instance.m_Impl = std::make_unique<LogCollectorImpl>();
#endif
	return true;
}

bool LogCollector::Stop() {
	if (s_Instance.m_LocalInstance)
		s_Instance.m_Impl.reset();
	else
		s_Instance.m_Impl.release();
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
