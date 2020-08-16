#pragma once

#include <thread>
#include <orbit_logger.h>

namespace OrbitLogger {

class StreamReader {
public:
	StreamReader(FILE* Stream, LogChannel Channel);
	~StreamReader();
private:
	LogChannel m_Channel;
	bool m_ThreadCanRun;
	int fdOutPipe[2];
	int fdOut;

	void ThreadMain();
};

} //namespace OrbitLogger 

