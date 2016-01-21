/*
 * cLog.cpp
 *
 *  Created on: 14-11-2013
 *      Author: Paweu
 */

#include <pch.h>

namespace Log {

#if 0

//----------------------------------------------------------------------------------

LogEngine* LogEngine::_Instance = nullptr;

LogEngine::LogEngine() {
	_Instance = this;

	m_Impl = std::make_unique<LogEngineImpl>();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

LogEngine::~LogEngine() {
}

void LogEngine::DeleteInstance() {
	delete _Instance;
	_Instance = nullptr;
}

#if 0
void LogEngine::ProcessLine(LogLine *line) {
	def _BUILDING_ENGINE_
	if (ConsoleExists() && line->m_Mode < LogLineType::MaxScreenConsole)
		switch (line->m_Mode) {
		case LogLineType::Console:
			break;
		default: {
			std::stringstream ss;
			ss << LogHeader << line->m_Message;
			string text = ss.str();
			auto mode = line->m_Mode;

			::Core::GetEngine()->PushSynchronizedAction([text, mode]() {
				GetConsole()->AddLine(text, (unsigned)mode);
			});
		}
	}
}
#endif

//----------------------------------------------------------------------------------

#if 0
#if defined(DEBUG) && defined(_BUILDING_ENGINE_)

std::list<StaticLogCatcher*>* StaticLogCatcher::_list = 0;

void StaticLogCatcher::DispatchLog() {
	if (!_list) {
		return;
	}

	_list->sort([](const StaticLogCatcher *c1, const StaticLogCatcher *c2) {
		return static_cast<int>(c2->GetType()) - static_cast<int>(c1->GetType()) > 0;
	});

	while (!_list->empty()) {
		_list->front()->DispatchLine();
		delete _list->front();
		_list->pop_front();
	}

	delete _list;
	_list = nullptr;
}

#endif
#endif

//----------------------------------------------------------------------------------

LogSinkBase::LogSinkBase(): m_Previous(nullptr) {
}

LogSinkBase::~LogSinkBase() {
	//Disable();
}

void LogSinkBase::line(const LogLine *line) {
	//Disable();
}

void LogSinkBase::Enable() {
	LOCK_MUTEX(_LogSinkListMutex);
	m_Previous = _LogSinkList;
	_LogSinkList = this;
}

void LogSinkBase::Disable() {
	LOCK_MUTEX(_LogSinkListMutex);
	if (_LogSinkList == this) {
		_LogSinkList = m_Previous;
		m_Previous = nullptr;
		return;
	}

	LogSinkBase *prv = nullptr;
	LogSinkBase *next = _LogSinkList;
	for (; next; prv = next, next = next->m_Previous) {
		if (next != this)
			continue;
		prv->m_Previous = next->m_Previous;
		m_Previous = nullptr;
		break;
	}
}

#endif

} // namespace Log
