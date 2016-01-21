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
	static bool Start();
	static bool Stop();
	static bool IsRunning();

	static void PushLine(const LogLineSourceInfo* SourceInfo, const char* fmt, ...);
	static void PushLine(const LogLineSourceInfo* SourceInfo, const std::ostringstream &ss);

	template<class T, class ...ARGS>
	static iLogSinkBase* AddLogSink(ARGS ... args) {
		return InsertLogSink(std::make_unique<T>(std::forward<ARGS>(args)...));
	}

	template<class T, class F, class ...ARGS>
	static bool OpenLogSink(F f, ARGS ... args) {
		auto sink = dynamic_cast<T*>(InsertLogSink(std::make_unique<T>(std::forward<ARGS>(args)...)));
		if (!sink)
			return false;
		f(sink);
		return true;
	}

	static iLogSinkBase* InsertLogSink(std::unique_ptr<iLogSinkBase> sink);
protected:
	LogCollector();
 	~LogCollector();

	struct LogCollectorImpl;
	std::unique_ptr<LogCollectorImpl> m_Impl;
private: 
	static LogCollector s_Instance;
};

} //namespace OrbitLogger 

#endif
