
#include "PlatformCore.h"
#include "ILSource.h"
#include "ILRunLoop.h"

ILSource::ILSource() : ILObject() {
	_runLoop = NULL;
}

ILSource::~ILSource() {}

void ILSource::setRunLoop(ILRunLoop* rl) {
	_runLoop = rl;
}

ILRunLoop* ILSource::runLoop() {
	return _runLoop;
}

ILTimeInterval ILSource::nextDesiredExecutionTime() {
	return ILAbsoluteTimeDistantFuture;
}
