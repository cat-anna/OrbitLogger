/*
 * cLog.cpp
 *
 *  Created on: 14-11-2013
 *      Author: Paweu
 */

#include "OrbitLogger.h"
#include "Platform.h"

namespace Log {

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
#endif

} // namespace Log
