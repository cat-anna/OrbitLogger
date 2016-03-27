/*
  * Generated by cppsrc.sh
  * On 2016-01-20 22:50:27,06
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef LoggerThread_H
#define LoggerThread_H

namespace OrbitLogger {

class LogCollector {
public:
	struct LogCollectorImpl;

	static bool Start();
	static bool Stop();
	static bool IsRunning();

	static void PushLine(const LogLineSourceInfo* SourceInfo, const char* fmt, ...);
	static void PushLine(const LogLineSourceInfo* SourceInfo, const std::ostringstream &ss);

	template<class T, class ...ARGS>
	static T* AddLogSink(ARGS ... args) {
		return dynamic_cast<T*>(InsertLogSink(std::make_unique<T>(std::forward<ARGS>(args)...)));
	}

	template<class T, class F, class ...ARGS>
	static bool OpenLogSink(F f, ARGS ... args) {
		auto sink = dynamic_cast<T*>(InsertLogSink(std::make_unique<T>(std::forward<ARGS>(args)...)));
		if (!sink)
			return false;
		f(sink);
		return true;
	}

	static void SetChannelName(LogChannel Channel, const char *Name);
	static void SetChannelState(LogChannel Channel, bool Enabled);

	static bool IsChannelEnabled(LogChannel Channel);
	static bool IsChannelEnabled(const LogLineSourceInfo* SourceInfo) { return IsChannelEnabled(SourceInfo->m_Channel); }

	static iLogSinkBase* InsertLogSink(std::unique_ptr<iLogSinkBase> sink);

	static LogCollectorImpl *GetInstance() { return s_Instance.m_Impl.get(); }
	static void SetExternalInstance(LogCollectorImpl *instance);
protected:
	LogCollector();
 	~LogCollector();

	std::unique_ptr<LogCollectorImpl> m_Impl;
	bool m_LocalInstance;
private: 
	static LogCollector s_Instance;
};

} //namespace OrbitLogger 

#endif
