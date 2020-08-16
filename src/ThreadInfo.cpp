#include "Platform.h"
#include "orbit_logger.h"

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif defined(LINUX)
#error Missing Linux implementation
#else
#error Unknown OS
#endif

namespace OrbitLogger {

thread_local bool ThreadInfo::_IsMain = false;
thread_local ThreadInfo::Signature ThreadInfo::_ThisThreadSignature = '\0?';

void ThreadInfo::SetName(const char *sign, bool IsMain) {
    if (sign) {
        Signature *signsrc = (Signature *)sign;
        _ThisThreadSignature = *signsrc;
    }
    _IsMain = IsMain;
}

ThreadInfo::NumericID ThreadInfo::GetID() {
#ifdef WINDOWS
    return static_cast<NumericID>(GetCurrentThreadId());
#elif defined(LINUX)
#error Missing Linux implementation
#else
#error Unknown OS
#endif
}

} // namespace OrbitLogger
