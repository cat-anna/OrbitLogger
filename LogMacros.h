
#pragma once

//macros prefixed with ORBITLOGGER are internal and should not be used

#define ORBITLOGGER_DISASBLED_ACTION()				do { /* NOP */ } while(false)

#ifdef ORBITLOGGER_DISASBLE_LOGGING

#define ORBITLOGGER_CreateLog(A, ...)				ORBITLOGGER_DISASBLED_ACTION()
#define AddLogOnce(...)								ORBITLOGGER_DISASBLED_ACTION()
#define AddLogOncef(...)							ORBITLOGGER_DISASBLED_ACTION()

#else

#define ORBITLOGGER_CreateLog(A, FLAGS, ...) \
do { \
	ORBITLOGGER_MakeSourceInfo(__srcinfo, A, FLAGS);\
	::OrbitLogger::LogCollector::PushLine(&__srcinfo, __VA_ARGS__); \
} while(false)

#define AddLogOnce(TYPE, A)							do { static bool ___Executed = false; if(!___Executed){ AddLog(TYPE, A); ___Executed = true; }} while (false)
#define AddLogOncef(TYPE, ...)						do { static bool ___Executed = false; if(!___Executed){ AddLogf(TYPE, __VA_ARGS__); ___Executed = true; }} while (false)

#endif

#define ORBITLOGGER_BeginLog(TYPE, FLAGS, A)		do { std::ostringstream __ss; __ss << A; ORBITLOGGER_CreateLog(TYPE, FLAGS, __ss); } while(false)
#define ORBITLOGGER_BeginLogf(TYPE, FLAGS, ...)		do { ORBITLOGGER_CreateLog(TYPE, FLAGS, __VA_ARGS__); } while (false)

#define AddLog(T, A)								__LOG_ACTION_##T(T, A)
#define AddLogf(T, ...)								__LOG_ACTION_F_##T(T, __VA_ARGS__)

#define __LOG_ACTION_Normal(T, A)					ORBITLOGGER_BeginLog(T, 0, A)
#define __LOG_ACTION_F_Normal(T, ...)				ORBITLOGGER_BeginLogf(T, 0, __VA_ARGS__)
#define __LOG_ACTION_Critical(T, A)					ORBITLOGGER_BeginLog(T, 0, A)
#define __LOG_ACTION_F_Critical(T, ...)				ORBITLOGGER_BeginLogf(T, 0, __VA_ARGS__)
#define __LOG_ACTION_Error(T, A)					ORBITLOGGER_BeginLog(T, 0, A)
#define __LOG_ACTION_F_Error(T, ...)				ORBITLOGGER_BeginLogf(T, 0, __VA_ARGS__)
#define __LOG_ACTION_Warning(T, A)					ORBITLOGGER_BeginLog(T, 0, A)
#define __LOG_ACTION_F_Warning(T, ...)				ORBITLOGGER_BeginLogf(T, 0, __VA_ARGS__)
#define __LOG_ACTION_Hint(T, A)						ORBITLOGGER_BeginLog(T, 0, A)
#define __LOG_ACTION_F_Hint(T, ...)					ORBITLOGGER_BeginLogf(T, 0, __VA_ARGS__)

#define __LOG_ACTION_SysInfo(T, A)					ORBITLOGGER_BeginLog(T, 0, A)
#define __LOG_ACTION_F_SysInfo(T, ...)				ORBITLOGGER_BeginLogf(T, 0, __VA_ARGS__)

#define __LOG_ACTION_TODO(T, ...)					ORBITLOGGER_DISASBLED_ACTION()
#define __LOG_ACTION_Bug(T, ...)					ORBITLOGGER_DISASBLED_ACTION()
#define __LOG_ACTION_FixMe(T, ...)					ORBITLOGGER_DISASBLED_ACTION()

#if defined(DEBUG) || defined(DEBUG_LOG)

#define ORBITLOGGER_LINE_DEBUG_FLAG					::OrbitLogger::LogLineFlags::DebugLine

#define __LOG_ACTION_Debug(T, A)					ORBITLOGGER_BeginLog(Normal, ORBITLOGGER_LINE_DEBUG_FLAG, A)
#define __LOG_ACTION_F_Debug(T, ...)				ORBITLOGGER_BeginLogf(Normal, ORBITLOGGER_LINE_DEBUG_FLAG, __VA_ARGS__)
#define __LOG_ACTION_DebugHint(T, A)				ORBITLOGGER_BeginLog(Hint, ORBITLOGGER_LINE_DEBUG_FLAG, A)
#define __LOG_ACTION_F_DebugHint(T, ...)			ORBITLOGGER_BeginLogf(Hint, ORBITLOGGER_LINE_DEBUG_FLAG, __VA_ARGS__)
#define __LOG_ACTION_DebugWarn(T, A)				ORBITLOGGER_BeginLog(Warning, ORBITLOGGER_LINE_DEBUG_FLAG, A)
#define __LOG_ACTION_F_DebugWarn(T, ...)			ORBITLOGGER_BeginLogf(Warning, ORBITLOGGER_LINE_DEBUG_FLAG, __VA_ARGS__)
#define __LOG_ACTION_DebugWarning(T, A)				ORBITLOGGER_BeginLog(Warning, ORBITLOGGER_LINE_DEBUG_FLAG, A)
#define __LOG_ACTION_F_DebugWarning(T, ...)			ORBITLOGGER_BeginLogf(Warning, ORBITLOGGER_LINE_DEBUG_FLAG, __VA_ARGS__)
#define __LOG_ACTION_DebugError(T, A)				ORBITLOGGER_BeginLog(Error, ORBITLOGGER_LINE_DEBUG_FLAG, A)
#define __LOG_ACTION_F_DebugError(T, ...)			ORBITLOGGER_BeginLogf(Error, ORBITLOGGER_LINE_DEBUG_FLAG, __VA_ARGS__)
#define __LOG_ACTION_Thread(T, NAME)				ORBITLOGGER_BeginLog(Hint, 0, "Thread Info: ID:" << std::hex << std::this_thread::get_id() << " Name:" << NAME)

#else

#define __LOG_ACTION_Debug(T, A)					ORBITLOGGER_DISASBLED_ACTION()
#define __LOG_ACTION_F_Debug(T, ...)				ORBITLOGGER_DISASBLED_ACTION()
#define __LOG_ACTION_DebugWarn(T, A)				ORBITLOGGER_DISASBLED_ACTION()
#define __LOG_ACTION_F_DebugWarn(T, ...)			ORBITLOGGER_DISASBLED_ACTION()
#define __LOG_ACTION_DebugWarning(T, A)				ORBITLOGGER_DISASBLED_ACTION()
#define __LOG_ACTION_F_DebugWarning(T, ...)			ORBITLOGGER_DISASBLED_ACTION()
#define __LOG_ACTION_DebugError(T, A)				ORBITLOGGER_DISASBLED_ACTION()
#define __LOG_ACTION_F_DebugError(T, ...)			ORBITLOGGER_DISASBLED_ACTION()
#define __LOG_ACTION_Thread(T, A)					ORBITLOGGER_DISASBLED_ACTION()

#endif

#define __LOG_ACTION_InvalidEnum(T, V)				AddLogf(Error, "Invalid enum value (enum:'%s' value:%u)", typeid(V).name(), static_cast<unsigned>(V))
