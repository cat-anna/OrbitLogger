
#pragma once

//macros prefixed with ORBITLOGGER are internal and should not be used

#define ORBITLOGGER_DISASBLED_ACTION()				do { /* NOP */ } while(false)

#ifdef ORBITLOGGER_DISASBLE_LOGGING

#define ORBITLOGGER_CreateLog(A, ...)				ORBITLOGGER_DISASBLED_ACTION()
#define AddLogOnce(...)								ORBITLOGGER_DISASBLED_ACTION()
#define AddLogOncef(...)							ORBITLOGGER_DISASBLED_ACTION()

#else

#define ORBITLOGGER_CreateLog(A, ...) \
do { \
	ORBITLOGGER_MakeSourceInfo(__srcinfo, A); \
	if(::OrbitLogger::LogCollector::IsChannelEnabled(&__srcinfo)) \
		::OrbitLogger::LogCollector::PushLine(&__srcinfo, __VA_ARGS__); \
} while(false)

#define AddLogOnce(CHANNEL, A)							do { static bool ___Executed = false; if(!___Executed){ AddLog(CHANNEL, A); ___Executed = true; }} while (false)
#define AddLogOncef(CHANNEL, ...)						do { static bool ___Executed = false; if(!___Executed){ AddLogf(CHANNEL, __VA_ARGS__); ___Executed = true; }} while (false)

#endif

#define ORBITLOGGER_BeginLog(CHANNEL, A)		do { std::ostringstream __ss; __ss << A; ORBITLOGGER_CreateLog(CHANNEL, __ss); } while(false)
#define ORBITLOGGER_BeginLogf(CHANNEL, ...)		do { ORBITLOGGER_CreateLog(CHANNEL, __VA_ARGS__); } while (false)

#define AddLog(CHANNEL, A)						ORBITLOGGER_BeginLog(CHANNEL, A)
#define AddLogf(CHANNEL, ...)					ORBITLOGGER_BeginLogf(CHANNEL, __VA_ARGS__)

#define LogInvalidEnum(V)						AddLogf(Error, "Invalid enum value (enum:'%s' value:%lu)", typeid(V).name(), static_cast<unsigned long>(V))

#define LOG_ABSTRACT_FUNCTION()					AddLogf(Error, "Function is abstract. Check overriding function.")
#define LOG_NOT_IMPLEMENTED()					AddLogOncef(Warning, "Function needs implementation")
#define LOG_DEPRECATED()						AddLogOncef(Warning, "Function is deprecated!")
	