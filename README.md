# OrbitLogger

Simple asynchronous logger

TODO: this document

## Quick introduction

```c++

#include <orbit_logger.h>
#include <OrbitLogger/src/sink/FileSink.h>
#include <OrbitLogger/src/sink/MSVCDebuggerSink.h>

int main() {
    //just to improve readability
    using OrbitLogger::LogCollector;
    using OrbitLogger::StdFileLoggerSink;
    using OrbitLogger::StdNoDebugFileLoggerSink;
    using OrbitLogger::MSVCDebuggerSink;

    //start OrbitLogger collector engine
    LogCollector::Start();

    //Add file sink, file will be appended after open
    LogCollector::AddLogSink<StdFileLoggerSink>("all.log");
    //Add file sink, file will be cleared after open
    LogCollector::AddLogSink<StdFileLoggerSink>("lastsession.log", false);
    //Add file sink, file will be appended after open, but debug channel will be skipped
    LogCollector::AddLogSink<StdNoDebugFileLoggerSink>("filtered.log");
    //Add MSVC debugger sink
    LogCollector::AddLogSink<MSVCDebuggerSink>();

    //tell engine to capture stdout, by default captured to Info channel
    LogCollector::SetCaptureStdOut();
    //tell engine to capture stderr, by default captured to Error channel
    LogCollector::SetCaptureStdErr();

    //log something
    AddLogf(Error, "Some error line");
    AddLog(Info, "some" << " stream" << " printing");
    AddLogf(Debug, "Some debug line");//will not be put into 'filtered.log'

    DebugLogf(Info, "Some debug info line"); //will be put to all outputs
    //DebugLogf macro is compiled out in release mode

    //stop OrbitLogger collector engine
    //must be done to ensure all pending lines are flushed to their outputs
    LogCollector::Stop();

    return 0;
}

```

## Asynchronous logging
## Log channels
## Log sinks
## Configuration
