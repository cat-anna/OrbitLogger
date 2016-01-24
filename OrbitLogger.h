/*
 * cLog.h
 *
 *  Created on: 14-11-2013
 *      Author: Paweu
 */

#ifndef ORBITLOGGER_H
#define ORBITLOGGER_H

#include <cstdint>
#include <memory>
#include <fstream>
#include <sstream>

#ifdef _MSC_VER
#if _MSC_VER < 1900 && !defined(thread_local)
#define thread_local __declspec(thread)
#endif
#endif

#include "ThreadInfo.h"

namespace OrbitLogger {

#define ORBIT_LOGGER_VERSION			0x0100
#define ORBIT_LOGGER_VERSION_STRING		"1.0"

#if defined(DEBUG) && !defined(ORBITLOGGER_EXTENDED_LOG_COUNTERS)
#define ORBITLOGGER_EXTENDED_LOG_COUNTERS
#endif

struct Configuration {
	enum {
		StringFormatBuffer			= 2 * 4096,
		
		LogLineBufferCapacity		= 1024, /// max amount of log lines buffered in collector tick
		LogLineStringBufferSize		= 128 * LogLineBufferCapacity, /// total size of all log messages in tick
	
		MaxSinkCount				 = 16,
	};
};

enum class LineType {
	Normal,
	Critical,
	Error,
	Warning,
	Hint,
//meta line types
	SysInfo,
	//Script,

	MaxValue,
};

struct LogLineFlags {
	enum {
		DebugLine			= 0x01,
	};
};

struct LogLineSourceInfo {
	LineType m_Mode;
	const char *m_File;
	const char *m_Function;
	unsigned m_Line;
	unsigned m_Flags;

	LogLineSourceInfo(LineType Mode, const char *File, const char *Function, unsigned Line, unsigned Flags = 0) :
			m_Mode(Mode), m_File(File), m_Function(Function), m_Line(Line), m_Flags(Flags) {
	}
};

#define ORBITLOGGER_MakeSourceInfo(NAME, TYPE, FLAGS)\
	static const ::OrbitLogger::LogLineSourceInfo NAME(::OrbitLogger::LineType::TYPE, __FILE__, __FUNCTION__, __LINE__, FLAGS)

struct LogLine {
	const LogLineSourceInfo *m_SourceInfo;
	const char *m_ModeStr;
	const char *m_Message;
	float m_ExecutionSecs;
	ThreadInfo::Signature m_ThreadSign;
	ThreadInfo::NumericID m_ThreadID;
};

#if 0
struct LogLineProxy {
	template <class T>
	LogLineProxy& operator << (T && t) {
		m_ss << t;
		return *this;
	}

	LogLineProxy(LogLine *line): m_Line(line) { }
	LogLineProxy(const LogLineProxy&) = delete;
	LogLineProxy(LogLineProxy && p) {
		m_ss.swap(p.m_ss);
		m_Line = p.m_Line;
		p.m_Line = nullptr;
	}

	~LogLineProxy() {
		if (!m_Line) return;
		//m_Line->SetMessage(m_ss);
		//m_Line->Queue();
	}

	void SetMode(LogLineType mode) { /*if(m_Line) m_Line->SetMode(mode); */}
protected:
	std::ostringstream m_ss;
	LogLine *m_Line;
};

//--------------------------------------------------------------
//--------------------------------------------------------------

#if defined(DEBUG) && defined(_BUILDING_ENGINE_)

struct StaticLogCatcher {
public:
	static void DispatchLog();
	enum class StaticLogType {
		Bug, FixMe, TODO, 
	};
protected:
	virtual void DispatchLine() const = 0;
	virtual StaticLogType GetType() const = 0;

	static void PushLog(StaticLogCatcher* log) {
		if (!_list)
			_list = new std::list < StaticLogCatcher* >() ;
		_list->push_back(log);
	}
private:
	static std::list<StaticLogCatcher*> *_list;
};

template<class info>
struct StaticLogLine : StaticLogCatcher {
	StaticLogLine() {
		PushLog(this);
	}
	~StaticLogLine() {
		_instance = nullptr;
	}
	void DispatchLine() const override {
		info i;
		i.ConstructMessage(
			::Log::LogEngine::BeginLine(::Log::LogLineType::Static, i.GetLine(), i.GetFile(), i.GetFunction()) 
			<< i.GetActionString() << ": " );			
	}
	virtual StaticLogType GetType() const { return info::GetAction(); }
	static StaticLogLine<info>* Get() {
		return _instance;
	}
private: 
	static StaticLogLine<info> *_instance;
};

template<class info>
StaticLogLine<info>* StaticLogLine<info>::_instance = new StaticLogLine<info>();

#define __Log_todo_action(T, ...) do { }while(false)
#define __Log_todo_action_disabled(T, ...)\
	{\
		static const char *__function = __FUNCTION__;\
		struct StaticLogLine_info {\
			using StaticLogType = ::Log::StaticLogCatcher::StaticLogType;\
			unsigned GetLine() { return m_line ; }\
			const char *GetFile() { return __FILE__ ; }\
			const char *GetFunction() { return __function; }\
			static StaticLogType GetAction() { return StaticLogType::T; }\
			const char *GetActionString() { return #T; }\
			void ConstructMessage(::Log::LogLineProxy &out) { out << __VA_ARGS__; }\
			StaticLogLine_info(unsigned l = __LINE__): m_line(l) { }\
		private:\
			unsigned m_line;\
		};\
		static auto __todo_item = ::Log::StaticLogLine<StaticLogLine_info>::Get(); \
	}

#else

#define __Log_todo_action(T, ...)					__disabled_log_action
	
#endif

#endif

} //namespace Log

#include "LogLineSink.h"
#include "LogCollector.h"
#include "LogMacros.h"

#endif // ORBITLOGGER_H 
