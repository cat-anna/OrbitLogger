#include "OrbitLogger.h"
#include "Platform.h"

#include "StreamReader.h"

#include <fcntl.h>
#include <io.h>

#define READ_FD 0
#define WRITE_FD 1

namespace OrbitLogger {

StreamReader::StreamReader(FILE* Stream, LogChannel Channel) {
	m_ThreadCanRun = false;
	m_Channel = Channel;

	if (_pipe(fdOutPipe, 1024 * 4, O_TEXT) != 0) {
		AddLogf(Error, "Failed to create a pipe!");
		return;
	}

	fdOut = dup(fileno(Stream));
	fflush(Stream);
	if (dup2(fdOutPipe[WRITE_FD], fileno(Stream)) != 0) {
		AddLogf(Error, "Failed to clone stream!");
		return;
	}

	std::ios::sync_with_stdio();
	setvbuf(Stream, NULL, _IONBF, 0); // absolutely needed

	m_ThreadCanRun = true;
	std::thread([this] { ThreadMain(); }).detach();
}

StreamReader::~StreamReader() {
	m_ThreadCanRun = false;
}

void StreamReader::ThreadMain() {
	ThreadInfo::SetName("STRD");
	const LogLineSourceInfo SrcInfo(m_Channel, __FILE__, __FUNCTION__, __LINE__);

	std::string buffer;
	buffer.reserve(1024);
	while (m_ThreadCanRun) {
		char buf[1024];
		int got = _read(fdOutPipe[READ_FD], buf, sizeof(buf));

		if (got <= 0)
			continue;

		for (int i = 0; i < got; ++i) {
			char c = buf[i];
			switch (c) {
			case '\n':
				if (buffer.empty())
					continue;
				if (::OrbitLogger::LogCollector::IsChannelEnabled(&SrcInfo)) 
					::OrbitLogger::LogCollector::PushLine(&SrcInfo, buffer.c_str()); 
				buffer.clear();
			case '\r':
				continue;
			default:
				buffer += c;
			}
		}
	}
}

} //namespace OrbitLogger 
